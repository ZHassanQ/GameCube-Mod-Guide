❗ Make sure your Raspberry Pi Pico build environment is set up on your machine.

Build Makefile and all required build scripts:
```shell
# cmake .
```

Then grab any DOL file you'd like to boot and run processing script:
```shell
# ./process_ipl.py iplboot.dol ipl.h
```

Do not change `ipl.h` output file name.

Once it's ready and `ipl.h` file has been created you can build the firmware:

```shell
# make
```

If everything worked you should see new file `picoboot.uf2` created in the main project directory. Now hold `BOOTSEL` button on Raspberry Pi Pico and connect USB cable. New mass storage device will appear. Copy `picoboot.uf2` file to `RPI-RP2` device. Once it's done it'll automatically eject itself. Disconnect the cable and you're all done.