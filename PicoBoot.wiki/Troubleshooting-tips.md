## My console boots to vanilla GC menu

In most cases this suggests there are issues with your SD card setup. Many people experienced this with SD2SP2 adapters which are very often terrible quality. I suggest trying different SD2SP2 adapter, preferably sideloading one (so called "GB Player compatible") which are, in my experience, more reliable. Getting one of memory card form factor adapters (SD Gecko, WiiSD) is also a good idea. These adapters pretty much always work. Also this is the adapter you want to get if you are using DOL-101 model.

Starting with [PicoBoot v0.2](https://github.com/webhdx/PicoBoot/releases/tag/v0.2) release you can hold D-Pad Down during boot to see debug output from iplboot. You should see similar output as shown below:

[[/assets/iplboot_debug_output.png]]

```
Mounted GAMECUBE as sda
Reading /ipl.dol
```

This means it properly detected my memory card named GAMECUBE in Memory Card Slot A, which is true because I've used SD Gecko adapter. If you don't see similar message it means your card couldn't be detected, you've used wrong file system (FAT32 is recommended) or there is no `ipl.dol` file on the memory card.

## Orange light is on but it doesn't display anything on the screen

First rule out any shorts between adjacent points. It's also common for GP4 and GP6&7 short to the heatsink if you soldered wires over U10 chip.

If you are 100% sure your soldering is okay and there are no shorts, you are probably experiencing interference introduced to data lines because of the wires length. There is a golden rule: use as short wires as possible. 10 centimeters (~4 inches) is what I'd consider as a maximum recommended length. The type of wire is also very important. Use 26-28 AWG stranded wires. Single core or thick wires can easily break or in worst case rip pads from the motherboard if you put too much stress on them. If that happens it's a game over for your console and it's beyond fixable at that point. 

PicoBoot placement also plays a huge role in reliability - install it near the fan, it's guaranteed to work in that spot. Any other placement is not guaranteed to work, especially placing PicoBoot in Serial Port 1 slot as it's confirmed to be unreliable.

#### Other possible solutions:
- Try different power supply, ref. https://github.com/webhdx/PicoBoot/issues/32

## Power light comes off shortly after hitting the power button

This indicates shorted power line. Check your soldering and make sure wires are not touching any other point than the one mentioned in the diagram. Please be careful, there is 12V power line on the SP1 connector where you need to solder 3 wires. It's possible to short it to data lines and fry CPU.

