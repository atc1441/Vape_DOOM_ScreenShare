# Custom firmware for the Aspire PIXO Vape

This repo is made together with this explanation video:(click on the image)

[https://www.youtube.com/watch?v=rVsvtEj9iqE](https://www.youtube.com/watch?v=rVsvtEj9iqE)

[![YoutubeVideo](https://img.youtube.com/vi/rVsvtEj9iqE/0.jpg)](https://www.youtube.com/watch?v=rVsvtEj9iqE)

The PIXO Aspire is a ~30€ VAPE which got quite some power in terms of Computing hardware

While it can not directly Play DOOM by utilizing the USB Connection and its Display we can use a screen share cheat to hack DOOM onto the Vape^^ This does basically allow to run anything on it like videos etc.

Find the Custom firmware and the WebTool in this Repo


![](VAPE_DOOMed.jpg)

# DOOM running natively on the Vape

The screen share above streams DOOM *to* the vape. Since then DOOM also runs
**on** it: the PY32F403 renders every frame itself, reads the map data from
the 16 MB SPI flash and takes input from the touch panel. No PC involved.

The engine is [DMC100-Doom](https://github.com/ArturBieniek4/DMC100-Doom) by
**Artur Bieniek** - a Doom built to run on a meter with 32 KB of RAM, with a
strip renderer that never holds a framebuffer and map data streamed from
serial flash. That is the whole reason this fits on a vape at all.

What is in this repo is the platform layer that puts it on this board: panel
driver, SPI pump shared with the NOR flash, touch input, memory map, build
rules and the host-side tools.

    19.8 fps, 189 KB of flash, episode 1 complete

Find it in [PixoAspire_DOOM_Port](PixoAspire_DOOM_Port). No Doom data is
shipped - bring your own `doom1.wad`.

# WebTool

Find the Screen Share WebSerial Tool here:

[VapeCloudStreamer.html](VapeCloudStreamer.html)

# Firmware Backups

You can find firmware dumps [here](Firmware_Files)

# Hardware Infos

Inside we can find

- Puxa PY32F403XC 64KB RAM 256KB Flash ARM
- External 16MB SPI Flash
- BLE Chip
- Battery
- LCD 323x173 Pixel
- Heating Coil
- Hardware Watchdog on PC6
- Vape Mic
- Vibration Motor
- Battery Charger
- more...

![](PCB_Top.jpg)

![](PCB_Bottom.jpg)

