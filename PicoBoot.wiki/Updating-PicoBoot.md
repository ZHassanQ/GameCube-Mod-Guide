❗ In order to update PicoBoot you need a microUSB cable and physical access to the Raspberry Pico board inside your console. 

1. Grab the latest `picoboot.uf` file from [Releases page](https://github.com/webhdx/PicoBoot/releases/latest)
2. Hold down `BOOTSEL` button on Raspberry Pi Pico board and connect USB cable to our computer.
3. A new mass storage device will appear.
4. Copy `picoboot.uf2` file onto that device.
5. It will automatically eject itself once it finishes writing the firmware.
6. Disconnect the USB cable and test your console.

❗ **Important** ❗  Due to current PicoBoot wiring method you should desolder 3.3V wire for the time of update. This is done to prevent supplying 3.3V to the console which results in overloading power regulator on Pico board. It'll get very hot if you don't desolder the wire. If you do it quickly enough it shouldn't do any harm to the Pico but it's recommended to just desolder the wire and resolder it once you're done updating.