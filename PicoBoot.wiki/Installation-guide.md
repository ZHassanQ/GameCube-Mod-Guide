## 0. Word of caution

There are a lot of botched installation pictures all over the internet. It became ridiculous at that point. Please don't attempt the installation until you have enough soldering experience. You won't be able to magically do it after watching one video on YouTube. It requires actual practice! Try on some scrap electronics first.

## 1. Prerequisites

You obviously need a soldering iron, soldering wire, flux and 26-28 AWG wires. I recommend T12 soldering station ([Amazon](https://amzn.to/3bgBbOW), [AliExpress](https://s.click.aliexpress.com/e/_DEiTBWJ)).

* Raspberry Pi Pico ([Amazon](https://amzn.to/3JbIsMw), [AliExpress](https://s.click.aliexpress.com/e/_DmWBvwb))
* SD Gecko/WiiSD ([Amazon](https://amzn.to/3cPCEMs), [AliExpress](https://s.click.aliexpress.com/e/_Dm43n6J)) or SD2SP2 ([Amazon](https://amzn.to/3I1cCSl), [AliExpress](https://amzn.to/3PNamBe))
* FAT32 or exFAT formatted SD card
* Good quality wires, 26-28 AWG ([Amazon](https://amzn.to/3w6jvwQ), [AliExpress](https://s.click.aliexpress.com/e/_DldVJQr))

Links above are affiliate and I get small commission 💵 if you buy these products. Thanks to everyone who used links above 🙏

## 2. Flashing Raspberry Pi Pico board

Go to [the latest release page](https://github.com/webhdx/PicoBoot/releases/latest) and download `picoboot.uf2` file. Now connect Raspberry Pi Pico board to your computer while holding `BOOTSEL` button pressed. If it's been done correctly you'll see `RPI-RP2` mass storage device show up. Drag and drop `picoboot.uf2` file to that device. It'll automatically eject and green LED will light up if it was programmed correctly. Unplug USB cable from Pico and proceed with hardware installation.

## 3. Preparing SD card

Format your SD card to FAT32 or exFAT. Download the latest Swiss release from [here](https://github.com/emukidid/swiss-gc/releases/latest) and grab `swiss_rXXXX.dol` file, rename it to `ipl.dol` and copy to the root of your SD card.

## 4. Hardware installation

![Wiring diagram](https://raw.githubusercontent.com/webhdx/PicoBoot/main/assets/Wiring%20diagram.jpg)

* Install PicoBoot on the fan assembly, behind front controller plate. Any other location like Serial Port 1 slot is not recommended and won't work reliably.
* Use only official Raspberry Pi Pico/Pico W boards. Alternative boards are not supported.
* Pay attention to the wires you're using! 26 AWG stranded silicone wires are my best recommendation. This is very important and using wrong type of cables caused many people to pull their hair out.
* To remove the heatsink easily, run the console for 10-15 minutes. Once it's heated up, the heatsink should come off without using excessive force. You can twist it back and forth a few times to make thermal pads loosen up. Do not use ANY prying tools. Use your hands and be gentle. Be careful to not knock off any components from the board, especially DA15 which seems to be often ripped during heatsink removal. 