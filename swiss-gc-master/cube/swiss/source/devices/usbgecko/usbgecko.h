/**
 * USB Gecko Driver for Gamecube & Wii
 *
 * Communicates with a client on the PC which serves file data over USB Gecko
 * Written by emu_kidid
**/

#ifndef USBGECKO_H
#define USBGECKO_H

#include <gccore.h>
#include "deviceHandler.h"

// Returns 1 if the PC side is ready, 0 otherwise
s32 usbgecko_pc_ready();

// Read from the remote file, returns amount read
s32 usbgecko_read_file(void *buffer, u32 length, u32 offset, char* filename);

// Write to the remote file, returns amount written
s32 usbgecko_write_file(void *buffer, u32 length, u32 offset, char* filename);

// Opens a directory on the PC end
s32 usbgecko_open_dir(char *filename);

// Returns the next file in the directory opened with usbgecko_open_dir, NULL on end.
file_handle* usbgecko_get_entry();

void usbgecko_lock_file(s32 lock);

#endif

