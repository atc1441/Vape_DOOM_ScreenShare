# DJI RC-N1 as a game pad

The DJI RC-N1 remote controller (model RC231, sold with the Mini, Air and Mavic drones) works
as a game pad for DOOM on the meter. It is the only controller supported. Nothing else in the
firmware depends on this directory except the hook points listed below.

This project is not affiliated with, endorsed or sponsored by DJI or Google. DJI and RC-N1 are
trademarks of their owner, used here only to name the hardware this project works with.

| File | What it is |
|---|---|
| `dmc_dji.c`, `dmc_dji.h` | The driver, part of `DOOM.BIN` |
| `dji_test.c` | Host test of the driver against a fake USB controller, with and without the 06/01 requests (`make -j$(nproc) dji-test`, also run by `make test`) |
| `rcprobe.c` | RCPROBE, a separate app for the meter that logs what the controller sends (`make -j$(nproc) rcprobe`) |
| `rclog.py` | Prints the log RCPROBE writes |

## Use

1. Connect the meter's USB-C port to the controller's **top** port, the one for the phone,
   with a USB-C to USB-C cable. The controller is the USB host on that port.
2. Switch the controller on. No drone is needed.
3. Switch the meter on. The driver runs from the title screen on, in the menu and in the
   game. The meter's buttons keep working at the same time.

The driver is off on the installer and data error screens, while the data drive is open on
USB and during the benchmark until its results screen is left. The meter has one USB port:
unplug the controller before you choose Options, USB drive, and connect the computer instead.

| Controller | Game | Menu |
|---|---|---|
| Left stick up and down | Move forward and back (analog) | Up and down |
| Left stick left and right | Strafe (analog) | Left and right, which only change the sound level |
| Right stick left and right | Turn (analog) | |
| Shutter/Record | Fire | Select, yes in a prompt |
| Photo/Video | Use (doors, switches) | |
| Fn | | Select, yes in a prompt |
| Pause/RTH | Opens the menu | Closes the menu, no in a prompt |
| Left dial | Next weapon (turned right), previous weapon (turned left) | |

On the title screen Shutter/Record, Fn and Pause/RTH open the menu. The dial changes the
weapon by one step per turn: turn it and let it come back to the middle before the next step.
The right stick's vertical axis, the flight mode switch and the power button are not used.
In the menu the left stick acts as the arrow keys once it is pushed past about 60 % of its
travel, at once and again every 8 tics (about a quarter of a second) while it is held there.
The stick settings (dead zone, response curve, menu flick and repeat, dial steps) are
constants in `port/game/pad.h`.

Hook points in the rest of the firmware:

- `port/game/fe_play.c` calls `dji_start()` when play mode begins (the title screen), on the
  meter build only, and `dji_stop()` before the data drive on USB or the benchmark take over.
- `port/game/i_dmc.c` calls `dji_tic()` from `I_StartTic` once per tic (the identity switch,
  the time-outs).
- `platform/startup.c` has a weak `OTGFS1_IRQHandler` in the vector table, the driver's
  handler replaces it. The driver also defines `usbd_ram`, the state of the USB device
  controller driver `platform/usbd.c`, which the data drive on USB (`port/game/dmc_usb.c`)
  uses as well.
- The driver writes the controller's state to `dmc_pad` (`port/game/pad.h`).
  `G_BuildTiccmd` in `engine/g_game.c` adds the analog axes, fire and use to the ticcmd when
  no demo plays and the menu is closed. `pad_tic` in `port/game/i_input.c` turns
  the other buttons, the stick flicks and the dial into key presses, next to the meter's
  buttons but outside their chord resolver: Pause/RTH is Escape, Fn and Shutter/Record are
  Enter (y in a prompt, nothing in a level), the flicks are arrow keys in the menu and the
  dial steps through the weapons like the meter's weapon chords.

The Renode build of the firmware never starts the driver (Renode has no model of the USB
controller).

## RCPROBE

RCPROBE replaces DOOM in the meter's flash. It plays the phone on the controller's top port
and logs the setup requests, the AOA strings and the DUML frames the controller sends, with a
guided timeline that asks for the sticks, the dial and each button in turn. It does not log
every frame. Frames of command set 06 except 06/1E and replies to 02 are logged unless they
repeat the last frame of their kind apart from the sequence number and the CRC. Of the other
kinds only the first frame is logged. The log header counts every kind. From the second step
of the timeline on RCPROBE sends the request 06/01 every 100 ms, bare and wrapped in turn
(even sequence numbers from 0x2000 bare, odd ones wrapped), and counts the replies to each.

RCPROBE uses only the USB IDs 18D1:4EE1 and 18D1:2D00, with the manufacturer string
`dmc100-doom`, the product string `RCPROBE` and no serial number. It comes back as 18D1:2D00
after the AOA switch. While the controller does nothing with one identity (no configuration
within 10 s, no AOA request within 6 s), RCPROBE tries the other.

1. `make -j$(nproc) rcprobe` builds `build/rc-n1/RCPROBE.BIN`.
2. Make `RCLOG.DAT`, the file RCPROBE writes its log into: a file of zeros of 8 KB or more (RCPROBE uses at most 1 MB of it).
   16 KB holds one run of the timeline, 64 KB leaves room for about 3 minutes of stick
   movement after it. For 64 KB: `dd if=/dev/zero of=RCLOG.DAT bs=4096 count=16`
3. Put the meter in upgrade mode: unplug the cable, switch the meter off, hold HOLD and SAVE
   and press POWER, then plug the meter into the computer. The bootloader shows the firmware
   drive.
4. Make room on the firmware drive. With the Freedoom build of DOOM installed about 100 KB
   are free, enough for `RCPROBE.BIN` (about 16 KB) and a 64 KB `RCLOG.DAT`. For a bigger log
   delete `DOOMX.DAT` first.
5. Copy `RCPROBE.BIN` and `RCLOG.DAT` to the top folder of the drive. `RCPROBE.BIN` must be
   the only `.BIN` file on the drive. Wait until the copy has finished (eject or `sync`),
   then press SAVE on the meter. The bootloader flashes `RCPROBE.BIN` and deletes it from the
   drive.
6. Unplug the meter from the computer, connect it to the controller as above, switch the
   controller on, switch the meter on if it is off and follow the screen. The timeline takes
   66 s, with a beep at every step.
7. Any meter key but POWER saves the log. When the screen says DONE, or LOG FULL, a key also
   ends the log (LOG ENDED, nothing more is written). POWER held for 2 s ends the log, saves
   it and switches the meter off. Unplug the controller before you switch the meter off: a
   meter that is still powered over USB restarts, and RCPROBE then starts a new log over the
   old one.
8. Put the meter in upgrade mode as in step 3, copy `RCLOG.DAT` to the computer and print it:
   `python3 -I controller/rc-n1/rclog.py RCLOG.DAT` (`--summary` for the summary only).
9. To get DOOM back, delete `RCLOG.DAT` and `DOOMRES.DAT` on the firmware drive (`DOOM.BIN`
   needs the room, copy `DOOMRES.DAT` again after the flash), copy `DOOM.BIN`, and
   `DOOMCODE.DMC` and `DOOMX.DAT` of the same build if they are not on the drive any more, and
   press SAVE.

The log contains the controller's serial number (the AOA string "serial"), and `rclog.py`
prints it in the timeline and in the summary. Do not publish raw logs or their printout as
they are.

## Protocol notes

What follows was found with RCPROBE on the meter and with the controller's bottom port on a
PC, and is what `dmc_dji.c` implements. Bytes in dumps, DUML addresses, command numbers such as
06/05 (command set 06, command id 05) and USB IDs are hexadecimal. Other hexadecimal numbers
carry 0x, everything else (lengths, offsets, axis values, times, counts) is decimal.

### Two USB ports

- The **bottom** port is for a PC. The controller is a USB device (2CA3:1020, high speed)
  with CDC ACM interfaces. One of them carries DUML v1 frames without any wrapper. The stick
  read 06/01 is answered there, the buttons are never reported on that port.
- The **top** port is for the phone. The controller is the USB host and powers the phone. It
  talks to the phone through the Android Open Accessory (AOA) protocol, version 2. The meter
  takes the phone's place.

18D1:4EE1 and 18D1:2D00 are Google's USB IDs, the second one prescribed by the AOA protocol
for an accessory. The driver and RCPROBE use them only so that the controller talks to the
meter as it does to a phone.

### AOA handshake

1. The meter enumerates as 18D1:4EE1 (full speed, one configuration, self powered) with one
   vendor-class interface that has a bulk IN endpoint 0x81 and a bulk OUT endpoint 0x01 of
   64 bytes.
2. The controller sets the address, reads the descriptors, sets configuration 1, then sends
   the AOA requests: GET_PROTOCOL (request type 0xC0, request 51, the meter answers protocol
   version 2), six SEND_STRING (0x40, 52, index 0 to 5) and START (0x40, 53). The strings are
   the manufacturer `DJI`, the model `com.dji.logiclink`, the description `DJI RC-N1`, a
   version, the URI `www.dji.com` and the controller's serial number. The meter receives and
   drops them.
3. After START the meter acknowledges the status stage, leaves the bus about 50 ms later,
   stays off for 200 ms and comes back as the accessory 18D1:2D00 with the same interface.
   The controller configures it at once and starts streaming. If the accessory gets a bus
   reset and no SET_CONFIGURATION within 5 s, the driver goes back to the phone identity.
4. Other IN requests, string descriptors included, are stalled. Other OUT requests (for
   example the AOA HID and audio requests) are acknowledged and their data dropped.

### Message wrapper

Every message on the accessory's bulk endpoints, in both directions, is:

```
55 CC 49 57   magic
LL LL LL LL   length of the DUML frame, u32 little endian
...           one DUML v1 frame
00 ...        zero padding up to 512 bytes (the controller pads, the meter sends no padding)
```

The controller does not answer a DUML request sent without the wrapper. On the 64-byte
full speed endpoints a message of 512 bytes spans 8 packets. The driver only parses frames
that lie inside one packet, which holds for the two frames it needs: both end within the
first packet of their message.

### DUML v1 frame

| Offset | Size | Field |
|---|---|---|
| 0 | 1 | 0x55 |
| 1 | 2 | bits 0 to 9: frame length including both CRCs, bits 10 to 15: version, 1 |
| 3 | 1 | CRC8 of bytes 0 to 2 |
| 4 | 1 | sender |
| 5 | 1 | receiver |
| 6 | 2 | sequence number, little endian |
| 8 | 1 | type: bit 7 is set in a reply, the requests below use 0x40 |
| 9 | 1 | command set |
| 10 | 1 | command id |
| 11 | n | payload, n = length - 13 |
| length - 2 | 2 | CRC16 of bytes 0 to length - 3, little endian |

Both CRCs are reflected and have no final XOR. CRC8: polynomial 0x31 (0x8C reflected),
initial value 0x77. CRC16: polynomial 0x1021 (0x8408 reflected), initial value 0x3692. Run
over a part with its CRC appended, either gives 0.

Addresses seen: 06 is the controller itself, 02 is the phone app, 0E sends the replies to
06/01 and the messages of command set 09, 2D sends 00/0E. The driver does not look at the
sender or the receiver. It checks the CRCs of every frame, takes the push 06/05 and the reply
to 06/01 by command set, command id and length, and drops all other frames.

### Buttons and sticks: the push 06/05

About every 125 ms the controller sends 06/05 from 06 to 02 (type 0x00, 13 bytes of
payload):

| Payload bytes | Content |
|---|---|
| 0, 1 | right stick X |
| 2, 3 | right stick Y |
| 4, 5 | left stick Y |
| 6, 7 | left stick X |
| 8, 9 | left dial |
| 10 | not decoded |
| 11 | mask 0x08 Pause/RTH, mask 0x30 the flight mode switch |
| 12 | mask 0x40 Shutter/Record, mask 0x01 Photo/Video, mask 0x10 Fn |

Axes are u16 little endian from 364 to 1684, centre 1024. X grows to the right, Y grows
upward (stick pushed away), the dial grows when turned right and springs back to 1024. The
power button is not reported.

### Faster sticks: the read 06/01

8 Hz is too slow for turning, so the driver also asks for the sticks. It sends the request
06/01 from 02 to 06 (type 0x40, no payload), wrapped as above, about every 57 ms (two game
tics). The two requests it alternates between, with the sequence numbers 0x2000 and 0x2001:

```
55 CC 49 57 0D 00 00 00  55 0D 04 33 02 06 00 20 40 06 01 DF 27
55 CC 49 57 0D 00 00 00  55 0D 04 33 02 06 01 20 40 06 01 9B 2C
```

The reply comes from 0E to 02 (type 0x80). Its payload has a status byte, then eight groups
of three bytes: one byte and a u16 little endian. The first five groups hold the same five
axes in the same order as the push, so axis k is the u16 at payload byte 2 + 3 k. The buttons
stay those of the last push.

`DJI_POLL` is a C define of `dmc_dji.c`, not a make option. Compiled with `-DDJI_POLL=0` the
driver sends no requests and takes the sticks from the push alone. `make dji-test` tests both
builds.

The driver drops the pad (no input from it) after a bus reset, a suspend or 0.5 s without a
push or a reply.

### Other traffic

Besides 06/05 the controller sends, in RCPROBE's log of about 80 s: 06/1E and 00/F1 from 06
(type 0x20) about once a second each, 00/0E from 2D about three times a second, and messages
of command set 09 from 0E, some of them to the receivers CE and EE, about four a second in
all. That makes about 16 messages, about 140 packets, a second without requests. The replies
to the driver's requests, about 17 a second, add as many packets again. On the meter the
handler runs in the OTGFS interrupt, and more than 200 interrupts within one millisecond
switch the driver off.
