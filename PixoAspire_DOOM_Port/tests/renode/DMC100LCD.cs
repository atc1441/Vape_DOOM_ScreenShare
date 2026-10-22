// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
//
// Minimal ST7789V model for the DMC-100 Renode tests. Decodes CASET/RASET/RAMWR in RGB565
// (8-bit SPI frames) into a 240x320 frame memory indexed by (column, row) as sent, so MADCTL
// is not applied here: the test maps the memory to the screen. Every command and its
// parameters go to a log, RAMWR data is logged as a byte count and, with CaptureRamwr set,
// kept as raw bytes. GPIO 0 is D/C (PB12). GPIO 1 is wired to CS (PB14) but ignored: every
// byte counts, whatever the level of CS.

using System;
using System.IO;
using System.Text;
using Antmicro.Renode.Core;
using Antmicro.Renode.Peripherals.SPI;

namespace Antmicro.Renode.Peripherals.SPI
{
    public class DMC100LCD : ISPIPeripheral, IGPIOReceiver
    {
        public DMC100LCD()
        {
            frame = new ushort[Width * Height];
            log = new StringBuilder();
            Reset();
        }

        public void Reset()
        {
            dc = true;
            command = -1;
            xs = 0;
            xe = Width - 1;
            ys = 0;
            ye = Height - 1;
            x = 0;
            y = 0;
            haveHigh = false;
            Array.Clear(frame, 0, frame.Length);
            PixelWrites = 0;
            CommandCount = 0;
            log.Clear();
            ramwrBytes = 0;
        }

        public void OnGPIO(int number, bool value)
        {
            if(number == 0)
            {
                dc = value;
            }
        }

        public byte Transmit(byte data)
        {
            if(!dc)
            {
                EndCommand();
                command = data;
                argCount = 0;
                CommandCount++;
                log.AppendFormat("{0:X2}", data);
                if(command == 0x2C)
                {
                    x = xs;
                    y = ys;
                    haveHigh = false;
                }
                return 0xFF;
            }
            switch(command)
            {
                case 0x2A:
                case 0x2B:
                    if(argCount < 4)
                    {
                        args[argCount] = data;
                    }
                    if(argCount == 3)
                    {
                        var s = (args[0] << 8) | args[1];
                        var e = (args[2] << 8) | args[3];
                        if(command == 0x2A)
                        {
                            xs = s;
                            xe = e;
                        }
                        else
                        {
                            ys = s;
                            ye = e;
                        }
                    }
                    break;
                case 0x2C:
                    Pixel(data);
                    ramwrBytes++;
                    if(CaptureRamwr)
                    {
                        ramwr.WriteByte(data);
                    }
                    argCount++;
                    return 0xFF;
            }
            log.AppendFormat(" {0:X2}", data);
            argCount++;
            return 0xFF;
        }

        public void FinishTransmission()
        {
        }

        // Writes the commands seen since the last call, one per line, and starts a new log
        public void SaveCommandLog(string path)
        {
            var inRamwr = command == 0x2C;
            EndCommand();
            File.WriteAllText(path, log.ToString());
            log.Clear();
            if(inRamwr)
            {
                command = 0x2C;
                log.Append("2C+");
            }
        }

        // Writes the RAMWR data bytes captured since the last call
        public void SaveRamwrData(string path)
        {
            File.WriteAllBytes(path, ramwr.ToArray());
            ramwr.SetLength(0);
        }

        public bool CaptureRamwr { get; set; }

        public void SaveScreenshot(string path)
        {
            using(var f = File.Create(path))
            {
                var header = Encoding.ASCII.GetBytes(String.Format("P6\n{0} {1}\n255\n", Width, Height));
                f.Write(header, 0, header.Length);
                var rgb = new byte[Width * Height * 3];
                for(var i = 0; i < frame.Length; i++)
                {
                    var p = frame[i];
                    var r = (p >> 11) & 0x1F;
                    var g = (p >> 5) & 0x3F;
                    var b = p & 0x1F;
                    rgb[3 * i] = (byte)((r << 3) | (r >> 2));
                    rgb[3 * i + 1] = (byte)((g << 2) | (g >> 4));
                    rgb[3 * i + 2] = (byte)((b << 3) | (b >> 2));
                }
                f.Write(rgb, 0, rgb.Length);
            }
        }

        public ulong PixelWrites { get; private set; }
        public ulong CommandCount { get; private set; }

        private void EndCommand()
        {
            if(command < 0)
            {
                return;
            }
            if(command == 0x2C)
            {
                log.AppendFormat(" [{0}]", ramwrBytes);
                ramwrBytes = 0;
            }
            log.Append('\n');
            command = -1;
        }

        private void Pixel(byte data)
        {
            if(!haveHigh)
            {
                high = data;
                haveHigh = true;
                return;
            }
            haveHigh = false;
            if(x >= 0 && x < Width && y >= 0 && y < Height)
            {
                frame[y * Width + x] = (ushort)((high << 8) | data);
            }
            PixelWrites++;
            if(++x > xe)
            {
                x = xs;
                if(++y > ye)
                {
                    y = ys;
                }
            }
        }

        private const int Width = 240;
        private const int Height = 320;
        private readonly ushort[] frame;
        private readonly int[] args = new int[4];
        private readonly StringBuilder log;
        private readonly MemoryStream ramwr = new MemoryStream();
        private bool dc;
        private int command;
        private int argCount;
        private ulong ramwrBytes;
        private int xs, xe, ys, ye, x, y;
        private bool haveHigh;
        private byte high;
    }
}
