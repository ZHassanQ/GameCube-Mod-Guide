### Can I use new Pico W board?

It depends. According to the reports it works but the green LED won't light up which may be confusing at first. I haven't tested it personally therfore I can't provide you with any support at this time. It's planned to fully support Pico W and WiFi functionalities in the future.

### My console doesn't start at all - I see no video output at all

This indicates wiring issue. Please read [[Installation guide]] and [[Troubleshooting tips]].

### My console goes to vanilla GC menu instead of Swiss

Please read [[Troubleshooting tips]].

### I don't understand how it's better than XenoGC

XenoGC is a drive modchip, it can only patch disc data on the fly. This means you have to use a boot disk to run Swiss and play games from an SD card. PicoBoot uses completely different approach - injects custom payload during console boot sequence. This means it can load any application instead of a built in GameCube menu. It will work even if your disc drive is not working.

### I installed your modchip and now my console doesn't work

Sorry. I do not take reponsibility for any damage done by installing this modchip. Do it at your own risk!

### Can I use other RP2040 boards?

Yes, go for it. Just respect the [license agreements](https://github.com/webhdx/PicoBoot/blob/main/LICENSE) and don't expect me to provide any support for your board. PicoBoot only supports official Raspberry Pi Pico module at the moment.

### Will it work if I have XenoGC installed?

Yes, you can use it with XenoGC intalled.

### I appreciate your work. Can I support you in any way?

This project is free and available for everyone. If you want to support it anyway, consider using [:heart: Sponsor](https://github.com/sponsors/webhdx) button.