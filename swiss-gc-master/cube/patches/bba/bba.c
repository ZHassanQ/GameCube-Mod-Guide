/* 
 * Copyright (c) 2017-2022, Extrems <extrems@extremscorner.org>
 * 
 * This file is part of Swiss.
 * 
 * Swiss is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Swiss is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * with Swiss.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "bba.h"
#include "common.h"
#include "dolphin/exi.h"
#include "dolphin/os.h"
#include "emulator.h"
#include "frag.h"
#include "interrupt.h"
#include "ipl.h"

static struct {
	bool lock;
	uint8_t rwp;
	uint8_t rrp;
	bba_page_t (*page)[8];
	ppc_context_t *entry;
	ppc_context_t *exit;
} _bba;

static struct {
	void *buffer;
	uint32_t length;
	uint32_t offset;
	bool read, patch;
} dvd;

#include "tcpip.c"

#define BBA_CMD_IRMASKALL		0x00
#define BBA_CMD_IRMASKNONE		0xF8

#define BBA_NCRA				0x00		/* Network Control Register A, RW */
#define BBA_NCRA_RESET			(1<<0)	/* RESET */
#define BBA_NCRA_ST0			(1<<1)	/* ST0, Start transmit command/status */
#define BBA_NCRA_ST1			(1<<2)	/* ST1,  " */
#define BBA_NCRA_SR				(1<<3)	/* SR, Start Receive */

#define BBA_IR 0x09		/* Interrupt Register, RW, 00h */
#define BBA_IR_FRAGI       (1<<0)	/* FRAGI, Fragment Counter Interrupt */
#define BBA_IR_RI          (1<<1)	/* RI, Receive Interrupt */
#define BBA_IR_TI          (1<<2)	/* TI, Transmit Interrupt */
#define BBA_IR_REI         (1<<3)	/* REI, Receive Error Interrupt */
#define BBA_IR_TEI         (1<<4)	/* TEI, Transmit Error Interrupt */
#define BBA_IR_FIFOEI      (1<<5)	/* FIFOEI, FIFO Error Interrupt */
#define BBA_IR_BUSEI       (1<<6)	/* BUSEI, BUS Error Interrupt */
#define BBA_IR_RBFI        (1<<7)	/* RBFI, RX Buffer Full Interrupt */

#define BBA_RWP  0x16/*+0x17*/	/* Receive Buffer Write Page Pointer Register */
#define BBA_RRP  0x18/*+0x19*/	/* Receive Buffer Read Page Pointer Register */

#define BBA_WRTXFIFOD 0x48/*-0x4b*/	/* Write TX FIFO Data Port Register */

#define BBA_RX_STATUS_BF      (1<<0)
#define BBA_RX_STATUS_CRC     (1<<1)
#define BBA_RX_STATUS_FAE     (1<<2)
#define BBA_RX_STATUS_FO      (1<<3)
#define BBA_RX_STATUS_RW      (1<<4)
#define BBA_RX_STATUS_MF      (1<<5)
#define BBA_RX_STATUS_RF      (1<<6)
#define BBA_RX_STATUS_RERR    (1<<7)

#define BBA_INIT_TLBP	0x00
#define BBA_INIT_BP		0x01
#define BBA_INIT_RHBP	0x0f
#define BBA_INIT_RWP	BBA_INIT_BP
#define BBA_INIT_RRP	BBA_INIT_BP

static void exi_clear_interrupts(int32_t chan, bool exi, bool tc, bool ext)
{
	EXI[chan][0] = (EXI[chan][0] & (0x3FFF & ~0x80A)) | (ext << 11) | (tc << 3) | (exi << 1);
}

static void exi_select(void)
{
	EXI[EXI_CHANNEL_0][0] = (EXI[EXI_CHANNEL_0][0] & 0x405) | ((1 << EXI_DEVICE_2) << 7) | (EXI_SPEED_32MHZ << 4);
}

static void exi_deselect(void)
{
	EXI[EXI_CHANNEL_0][0] &= 0x405;
}

static void exi_imm_write(uint32_t data, uint32_t len)
{
	EXI[EXI_CHANNEL_0][4] = data;
	EXI[EXI_CHANNEL_0][3] = ((len - 1) << 4) | (EXI_WRITE << 2) | 0b01;
	while (EXI[EXI_CHANNEL_0][3] & 0b01);
}

static uint32_t exi_imm_read(uint32_t len)
{
	EXI[EXI_CHANNEL_0][3] = ((len - 1) << 4) | (EXI_READ << 2) | 0b01;
	while (EXI[EXI_CHANNEL_0][3] & 0b01);
	return EXI[EXI_CHANNEL_0][4] >> ((4 - len) * 8);
}

static uint32_t exi_imm_read_write(uint32_t data, uint32_t len)
{
	EXI[EXI_CHANNEL_0][4] = data;
	EXI[EXI_CHANNEL_0][3] = ((len - 1) << 4) | (EXI_READ_WRITE << 2) | 0b01;
	while (EXI[EXI_CHANNEL_0][3] & 0b01);
	return EXI[EXI_CHANNEL_0][4] >> ((4 - len) * 8);
}

static void exi_immex_write(const uint8_t *buf __attribute((vector_size(4))), uint32_t len)
{
	uint32_t xlen;

	do {
		xlen = MIN(len, 4);
		exi_imm_write((uint32_t)*buf++, xlen);
	} while (len -= xlen);
}

static void exi_dma_write(const void *buf, uint32_t len, bool sync)
{
	EXI[EXI_CHANNEL_0][1] = (uint32_t)buf;
	EXI[EXI_CHANNEL_0][2] = OSRoundUp32B(len);
	EXI[EXI_CHANNEL_0][3] = (EXI_WRITE << 2) | 0b11;
	while (sync && (EXI[EXI_CHANNEL_0][3] & 0b01));
}

static void exi_dma_read(void *buf, uint32_t len, bool sync)
{
	EXI[EXI_CHANNEL_0][1] = (uint32_t)buf;
	EXI[EXI_CHANNEL_0][2] = OSRoundUp32B(len);
	EXI[EXI_CHANNEL_0][3] = (EXI_READ << 2) | 0b11;
	while (sync && (EXI[EXI_CHANNEL_0][3] & 0b01));
}

static uint8_t bba_in8(uint16_t reg)
{
	uint8_t val;

	exi_select();
	exi_imm_write(0x80 << 24 | reg << 8, 4);
	val = exi_imm_read(1);
	exi_deselect();

	return val;
}

static void bba_out8(uint16_t reg, uint8_t val)
{
	exi_select();
	exi_imm_write(0xC0 << 24 | reg << 8, 4);
	exi_imm_write(val << 24, 1);
	exi_deselect();
}

static uint8_t bba_cmd_in8(uint8_t reg)
{
	uint8_t val;

	exi_select();
	val = exi_imm_read_write(0x00 << 24 | reg << 24, 4);
	exi_deselect();

	return val;
}

static void bba_cmd_out8(uint8_t reg, uint8_t val)
{
	exi_select();
	exi_imm_write(0x40 << 24 | reg << 24 | val, 4);
	exi_deselect();
}

static void bba_ins(uint16_t reg, void *val, uint32_t len)
{
	exi_select();
	exi_imm_write(0x80 << 24 | reg << 8, 4);
	exi_clear_interrupts(EXI_CHANNEL_0, 0, 1, 0);
	exi_dma_read(val, len, 0);

	bool lock = _bba.lock;
	_bba.lock = false;
	if (!setjmp(_bba.entry))
		longjmp(_bba.exit, 1);
	_bba.lock = lock;

	exi_deselect();
}

static void bba_outs(uint16_t reg, const void *val, uint32_t len)
{
	exi_select();
	exi_imm_write(0xC0 << 24 | reg << 8, 4);
	exi_dma_write(val, len, 1);
	exi_deselect();
}

void bba_transmit_fifo(const void *data, size_t size)
{
	if (!size) return;
	DCStoreRange(__builtin_assume_aligned(data, 32), size);

	while (bba_in8(BBA_NCRA) & (BBA_NCRA_ST0 | BBA_NCRA_ST1));
	bba_outs(BBA_WRTXFIFOD, data, size);
	bba_out8(BBA_NCRA, (bba_in8(BBA_NCRA) & ~BBA_NCRA_ST0) | BBA_NCRA_ST1);
}

void bba_receive_dma(bba_page_t page, size_t size)
{
	if (!size) return;
	DCInvalidateRange(__builtin_assume_aligned(page, 32), size);

	int page_wrap = MIN(size, (BBA_INIT_RHBP + 1 - _bba.rrp) << 8);

	if (page_wrap < size) {
		bba_ins(_bba.rrp << 8, page, page_wrap);
		_bba.rrp = BBA_INIT_RRP;
		bba_ins(_bba.rrp << 8, page + page_wrap, size - page_wrap);
	} else
		bba_ins(_bba.rrp << 8, page, size);
}

static void bba_receive(void)
{
	_bba.rwp = bba_in8(BBA_RWP);
	_bba.rrp = bba_in8(BBA_RRP);

	while (_bba.rrp != _bba.rwp) {
		bba_page_t *page = *_bba.page;
		bba_header_t *bba = (bba_header_t *)page;
		size_t size = sizeof(bba_page_t);

		DCInvalidateRange(__builtin_assume_aligned(page, 32), size);
		bba_ins(_bba.rrp << 8, page, size);
		_bba.rrp = _bba.rrp % BBA_INIT_RHBP + 1;

		size = bba->length - sizeof(*bba);

		eth_input(page, (void *)bba->data, size);
		bba_out8(BBA_RRP, _bba.rrp = bba->next);
		_bba.rwp = bba_in8(BBA_RWP);
	}
}

static void bba_interrupt(void)
{
	uint8_t ir = bba_in8(BBA_IR);

	if (ir & BBA_IR_RI) bba_receive();

	bba_out8(BBA_IR, ir);
}

static void exi_callback()
{
	if (!setjmp(_bba.exit))
		longjmp(_bba.entry, 1);
}

static void exi_coroutine()
{
	if (EXILock(EXI_CHANNEL_0, EXI_DEVICE_2, exi_callback)) {
		_bba.lock = true;

		set_interrupt_handler(OS_INTERRUPT_EXI_0_TC, exi_callback);
		unmask_interrupts(OS_INTERRUPTMASK_EXI_0_TC);

		uint8_t status = bba_cmd_in8(0x03);
		bba_cmd_out8(0x02, BBA_CMD_IRMASKALL);

		if (status & 0x80) bba_interrupt();

		bba_cmd_out8(0x03, status);
		bba_cmd_out8(0x02, BBA_CMD_IRMASKNONE);

		mask_interrupts(OS_INTERRUPTMASK_EXI_0_TC);

		_bba.lock = false;
		EXIUnlock(EXI_CHANNEL_0);
	}

	if (!setjmp(_bba.entry))
		longjmp(_bba.exit, 1);
}

static void exi_interrupt_handler(OSInterrupt interrupt, OSContext *context)
{
	exi_clear_interrupts(EXI_CHANNEL_2, 1, 0, 0);
	exi_callback();
}

void bba_init(void **arenaLo, void **arenaHi)
{
	*arenaHi -= sizeof(*_bba.exit);  _bba.exit  = *arenaHi;
	*arenaHi -= sizeof(*_bba.entry); _bba.entry = *arenaHi;

	void **sp = *arenaHi; sp[-2] = sp;
	_bba.entry->gpr[1] = (intptr_t)sp - 8;
	_bba.entry->lr = (intptr_t)exi_coroutine;

	*arenaHi -= sizeof(*_bba.page);  _bba.page  = *arenaHi;

	set_interrupt_handler(OS_INTERRUPT_EXI_2_EXI, exi_interrupt_handler);
	unmask_interrupts(OS_INTERRUPTMASK_EXI_2_EXI);
}

void schedule_read(OSTick ticks)
{
	void read_callback(void *address, uint32_t length)
	{
		DCStoreRangeNoSync(address, length);

		dvd.buffer += length;
		dvd.length -= length;
		dvd.offset += length;
		dvd.read = !!dvd.length;

		schedule_read(0);
	}
	#ifndef ASYNC_READ
	OSCancelAlarm(&read_alarm);
	#endif

	if (!dvd.read) {
		di_complete_transfer();
		return;
	}

	#ifdef ASYNC_READ
	frag_read_async(*VAR_CURRENT_DISC, dvd.buffer, dvd.length, dvd.offset, read_callback);
	#else
	dvd.patch = frag_read_patch(*VAR_CURRENT_DISC, dvd.buffer, dvd.length, dvd.offset, read_callback);

	if (dvd.patch)
		OSSetAlarm(&read_alarm, ticks, trickle_read);
	#endif
}

void perform_read(uint32_t address, uint32_t length, uint32_t offset)
{
	if ((*VAR_IGR_TYPE & 0x80) && offset == 0x2440) {
		*VAR_CURRENT_DISC = FRAGS_APPLOADER;
		*VAR_SECOND_DISC = 0;
	}

	dvd.buffer = OSPhysicalToCached(address);
	dvd.length = length;
	dvd.offset = offset;
	dvd.read = true;

	schedule_read(0);
}

#ifndef ASYNC_READ
void trickle_read()
{
	if (dvd.read && dvd.patch) {
		OSTick start = OSGetTick();
		int size = frag_read(*VAR_CURRENT_DISC, dvd.buffer, dvd.length, dvd.offset);
		DCStoreRangeNoSync(dvd.buffer, size);
		OSTick end = OSGetTick();

		dvd.buffer += size;
		dvd.length -= size;
		dvd.offset += size;
		dvd.read = !!dvd.length;

		schedule_read(OSDiffTick(end, start));
	}
}
#endif

bool change_disc(void)
{
	if (*VAR_SECOND_DISC) {
		*VAR_CURRENT_DISC ^= 1;
		return true;
	}

	return false;
}

void reset_devices(void)
{
	while (EXI[EXI_CHANNEL_0][3] & 0b000001);
	while (EXI[EXI_CHANNEL_1][3] & 0b000001);
	while (EXI[EXI_CHANNEL_2][3] & 0b000001);

	reset_device();
	ipl_set_config(0);
}
