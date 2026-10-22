// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Artur Bieniek
// Copyright (C) 2026 VapeDoom port
//
// Input: the CST816D touch panel of the vape and its one key, presented to
// the port as the six buttons the DMC-100 has.
//
// The engine's chord resolver (port/game/i_input.c) wants five buttons plus
// POWER as a shift:
//
//   V-A   forward     HOLD  backward    MODE turn left
//   SAVE  turn right  LIGHT fire        POWER shift / menu
//
// Nothing says those bits have to come from five switches. The touch panel
// gives a position, and a position can press two of them at once, so one
// finger on the left part of the glass drives: up-left is forward and turn
// left together, which no pair of real keys on this device could do.
//
// The glass carries everything needed to play:
//
//   left part      steering cross. Up walks, down backs up, left and right
//                  turn, and the corners do both at once.
//   right strip    use on top, fire below.
//   top-left corner  the menu. Small enough that a thumb busy steering
//                  never grazes it.
//
// The key (PE15) fires as well. It is a bonus, not a requirement - which pin
// it hangs on was written nowhere and was guessed wrong in this code for a
// long time; see key_bits().
//
// The panel is read in the SysTick interrupt, not in the game loop: the
// engine spends most of a frame inside the renderer, and input sampled there
// would arrive in bursts. The read costs about 180 us and runs every 16 ms.

#include "lcd.h"
#include "platform.h"
#include "py32_hw.h"

// ---- bit-banged I2C on PB8/PB9 ----
// The datasheet maps these to I2C1, but the stock firmware drives them as
// plain pins and the stock image contains no I2C peripheral address at all,
// so the chip is reached the same way here.
#define TOUCH_ADDR 0x15u  // CST816D, found by scanning
#define QDEL_CYC 180u     // ~1.25 us, about 400 kHz

static void qdel(void) { spin_cycles(QDEL_CYC); }

static inline void scl_rel(void) { pin_high(GPIOB, PIN_SCL); }
static inline void scl_lo(void) { pin_low(GPIOB, PIN_SCL); }
static inline void sda_rel(void) { pin_high(GPIOB, PIN_SDA); }
static inline void sda_lo(void) { pin_low(GPIOB, PIN_SDA); }
static inline uint32_t sda_rd(void) { return pin_read(GPIOB, PIN_SDA); }

static void scl_rel_wait(void)
{
    uint32_t guard = 500;
    scl_rel();
    qdel();
    while (!pin_read(GPIOB, PIN_SCL) && --guard)
        qdel();
}

static void i2c_start(void)
{
    sda_rel();
    scl_rel_wait();
    sda_lo();
    qdel();
    scl_lo();
    qdel();
}

static void i2c_stop(void)
{
    sda_lo();
    qdel();
    scl_rel_wait();
    sda_rel();
    qdel();
}

static uint32_t i2c_wr(uint8_t b)
{
    uint32_t nack;
    for (unsigned i = 0; i < 8; i++) {
        if (b & 0x80u)
            sda_rel();
        else
            sda_lo();
        b = (uint8_t)(b << 1);
        qdel();
        scl_rel_wait();
        scl_lo();
        qdel();
    }
    sda_rel();
    qdel();
    scl_rel_wait();
    nack = sda_rd();
    scl_lo();
    qdel();
    return nack;
}

static uint8_t i2c_rd(uint32_t ack)
{
    uint8_t v = 0;
    sda_rel();
    for (unsigned i = 0; i < 8; i++) {
        qdel();
        scl_rel_wait();
        v = (uint8_t)((v << 1) | sda_rd());
        scl_lo();
        qdel();
    }
    if (ack)
        sda_lo();
    else
        sda_rel();
    qdel();
    scl_rel_wait();
    scl_lo();
    qdel();
    sda_rel();
    return v;
}

static int tc_read(uint8_t reg, uint8_t *buf, unsigned n)
{
    i2c_start();
    if (i2c_wr(TOUCH_ADDR << 1) || i2c_wr(reg)) {
        i2c_stop();
        return -1;
    }
    i2c_start();
    if (i2c_wr((TOUCH_ADDR << 1) | 1u)) {
        i2c_stop();
        return -1;
    }
    for (unsigned i = 0; i < n; i++)
        buf[i] = i2c_rd(i + 1u < n);
    i2c_stop();
    return 0;
}

static void tc_write(uint8_t reg, uint8_t val)
{
    i2c_start();
    (void)i2c_wr(TOUCH_ADDR << 1);
    (void)i2c_wr(reg);
    (void)i2c_wr(val);
    i2c_stop();
}

// ---- touch to buttons ----
// Screen coordinates, the same 320 x 240 the engine draws on.
#define PAD_RIGHT 250  // left of this is the movement pad
#define PAD_CX 125
#define PAD_CY 120
#define PAD_DX 45  // dead zone of the turn axis
#define PAD_DY 40  // dead zone of the move axis

// The menu corner. Small and right at the edge: a thumb steering
// forward-left lands in the middle of that cell, not in its outermost
// corner.
#define MENU_W 44
#define MENU_H 36
// How long a touch of the corner reports POWER.
#define MENU_MS 150u

// The corner is a button, not a key you hold.
//
// The resolver gives POWER three meanings: a tap opens the menu, one second
// held toggles it, four seconds open the quit prompt - and its yes calls
// I_Quit, i.e. plat_power_off(). Under a thumb on a real switch that is a
// sensible ladder. On glass it is not: a finger left lying in the corner
// walks through all three, up to powering the device off, without anyone's
// help.
//
// So the corner reports POWER for MENU_MS and nothing after that, as long as
// the finger has not left it. One press, one menu.
static uint8_t menu_latched;
static uint32_t menu_t0;

static void menu_rearm(void) { menu_latched = 0; }

static uint32_t touch_bits(int sx, int sy)
{
    uint32_t b = 0;
    if (sx < MENU_W && sy < MENU_H) {
        if (!menu_latched) {
            menu_latched = 1;
            menu_t0 = plat_ms();
        }
        return plat_ms() - menu_t0 < MENU_MS ? BTN_BIT(BTN_POWER) : 0u;
    }
    menu_rearm();
    if (sx >= PAD_RIGHT) {
        // Action strip: use above, fire below. Use is the shift plus fire,
        // which is what the resolver turns into KEY_USE.
        if (sy < LCD_H / 2)
            b |= BTN_BIT(BTN_POWER) | BTN_BIT(BTN_LIGHT);
        else
            b |= BTN_BIT(BTN_LIGHT);
        return b;
    }
    if (sx < PAD_CX - PAD_DX)
        b |= BTN_BIT(BTN_MODE);  // turn left
    else if (sx > PAD_CX + PAD_DX)
        b |= BTN_BIT(BTN_SAVE);  // turn right
    if (sy < PAD_CY - PAD_DY)
        b |= BTN_BIT(BTN_VA);  // forward
    else if (sy > PAD_CY + PAD_DY)
        b |= BTN_BIT(BTN_HOLD);  // backward
    return b;
}

// The panel counts in its own orientation: x across the short axis, y along
// the long one. Which raw axis is which screen axis is forced by the ranges
// (173 against 240, 323 against 320) and needs no decision. Which *end* of
// each axis is which does, and no register answers it: it is how the glass
// sits on the module.
//
// It is settled all the same, and from three measurements made during
// bring-up rather than from an assumption:
//
//  1. The bring-up firmware (MADCTL 0xC0) computed lx = 322 - ry and drew lx
//     back to row address 322 - lx, i.e. to ry. The crosshair sat under the
//     finger - confirmed on the device. So row address ry is where the
//     finger is, under 0xC0.
//  2. The captures 30_mad_{00,40,80,C0} of that same bring-up firmware show
//     what the MADCTL bits do: 0x40 against 0xC0 mirrors the long axis and
//     leaves the line order standing - MY reverses the row-address
//     direction. 0x80 mirrors the short one.
//  3. Here MADCTL 0x40 applies, and lcd_window maps screen x straight onto
//     the row address. A finger at ry therefore writes row 319 - ry, and
//     under 0x40 that is exactly where row ry sat under 0xC0: under the
//     finger again.
//
// The short axis stands the same way in both firmwares (MX is set in 0x40 as
// in 0xC0), and both compute sy from rx without a sign.
//
// Injection does not answer this question - it bypasses the sensor and would
// only confirm its own arithmetic. The switch stays in case a different
// glass variant turns up.
#ifndef TOUCH_FLIP_X
#define TOUCH_FLIP_X 0
#endif

static volatile uint16_t last_sx, last_sy;
static volatile uint8_t last_down;

// Controller raw values to screen coordinates. Its own function so that
// injected raw values take the same path as real ones - a mapping that only
// holds in the test proves nothing.
static void map_raw(unsigned rx, unsigned ry, int *sx, int *sy)
{
    if (rx >= PANEL_W)
        rx = PANEL_W - 1;
    if (ry >= PANEL_H)
        ry = PANEL_H - 1;
#if TOUCH_FLIP_X
    *sx = (int)(ry * LCD_W / PANEL_H);
#else
    *sx = (int)((PANEL_H - 1u - ry) * LCD_W / PANEL_H);
#endif
    *sy = (int)(rx * LCD_H / PANEL_W);
    dbg_touch_pos(rx, ry, (uint32_t)*sx, (uint32_t)*sy);
}

// ---- injecting input over SWD ----
// The device has no other interface, and a finger cannot be put in the same
// place twice. The debugger can write RAM while the CPU runs, so this block
// is here for the host to fill in; with the magic set, touch_poll takes its
// values instead of the controller's.
//
// The three modes deliberately separate what would otherwise collapse into
// one test:
//   1  screen coordinates - checks the zone layout, the chord resolver and
//      the game, without touching the axis question.
//   2  controller raw values - checks map_raw as well.
//   3  button bits directly - for menu paths no finger has to hit.
#define TINJ_MAGIC 0x544F5543u  // "TOUC"

struct touch_inject {
    volatile uint32_t magic;  // 0x00
    volatile uint32_t mode;   // 0x04  0 off, 1 screen, 2 raw, 3 bits
    volatile int32_t x;       // 0x08
    volatile int32_t y;       // 0x0C
    volatile uint32_t bits;   // 0x10  mode 3
    volatile uint32_t down;   // 0x14  0 means finger lifted
    volatile uint32_t seq;    // 0x18  counts every poll handled
    volatile uint32_t out;    // 0x1C  the button bits produced, to read back
};

__attribute__((used)) struct touch_inject tinj;

static uint32_t touch_inject_poll(void)
{
    uint32_t bits = 0;
    int sx = -1, sy = -1;

    tinj.seq++;
    if (tinj.down) {
        switch (tinj.mode) {
        case 1u:
            sx = tinj.x;
            sy = tinj.y;
            dbg_touch_pos(0xFFFFu, 0xFFFFu, (uint32_t)sx, (uint32_t)sy);
            bits = touch_bits(sx, sy);
            break;
        case 2u:
            map_raw((unsigned)tinj.x, (unsigned)tinj.y, &sx, &sy);
            bits = touch_bits(sx, sy);
            break;
        default:
            bits = tinj.bits;
            break;
        }
        last_sx = (uint16_t)sx;
        last_sy = (uint16_t)sy;
    }
    if (!tinj.down)
        menu_rearm();
    last_down = tinj.down ? 1u : 0u;
    dbg_touch(1, tinj.down ? 1 : 0);
    tinj.out = bits;
    return bits;
}

static uint32_t touch_poll(void)
{
    uint8_t r[7];
    int sx, sy;
    int ok;

    if (tinj.magic == TINJ_MAGIC && tinj.mode)
        return touch_inject_poll();

    ok = tc_read(0x00u, r, 7) == 0;
    dbg_touch(ok, ok && (r[2] & 0x0Fu) != 0);
    if (!ok || (r[2] & 0x0Fu) == 0) {
        last_down = 0;
        menu_rearm();
        return 0;
    }
    map_raw((unsigned)(((r[3] & 0x0Fu) << 8) | r[4]),
            (unsigned)(((r[5] & 0x0Fu) << 8) | r[6]), &sx, &sy);
    last_sx = (uint16_t)sx;
    last_sy = (uint16_t)sy;
    last_down = 1;
    return touch_bits(sx, sy);
}

// ---- key ----
// PE15, an input with a pull-up: pressed, it reads low.
//
// Pinout.txt does not name the pin, and PC5 stood here beside it for a long
// time without anyone ever showing that the key hangs on it. It does not.
// Counted out, the stock firmware's configuration leaves only five digital
// inputs in the whole chip, and exactly one of them behaves like a key -
// measured with tools/pins.py while somebody pressed it:
//
//   PE15  pull-up,   reads high, goes low on a press   <- the key
//   PE6   pull-down, flips on its own twice a second   <- a signal
//   PC13  no pull,   reads low
//   PC5   no pull - floats and delivers noise
//
// PC5 without a pull resistor is why it was taken out again: it changed
// during the measurement with nobody touching it, and on the fire key that
// would be a shot out of nowhere.
#define KEY_PORT GPIOE
#define KEY_PIN 15

static uint32_t key_bits(void)
{
    return pin_read(KEY_PORT, KEY_PIN) ? 0u : BTN_BIT(BTN_LIGHT);
}

// ---- debounce and edge ring ----
#define RING 8

static volatile uint32_t state;      // debounced
static uint32_t cand, cand_n;        // candidate and how many samples agree
static uint32_t touch_state;         // last bits from the panel
static uint8_t ring[RING];
static volatile uint8_t rd_i, wr_i;
static uint16_t tick;

uint32_t plat_buttons(void) { return state; }

int plat_event(void)
{
    if (rd_i == wr_i)
        return -1;
    int e = ring[rd_i % RING];
    rd_i++;
    return e;
}

static void push_edge(int b, int press)
{
    if ((uint8_t)(wr_i - rd_i) >= RING)
        return;  // full: drop, callers can resync from plat_buttons
    ring[wr_i % RING] = (uint8_t)(b | (press ? EV_PRESS : 0));
    wr_i++;
}

void buttons_sample(void)
{
    uint32_t now;

    // The panel is slow to read, the keys are not. 16 ms is well inside one
    // 35 Hz game tic.
    if (++tick >= 16u) {
        tick = 0;
        touch_state = touch_poll();
    }
    now = key_bits() | touch_state;

    if (now != cand) {
        cand = now;
        cand_n = 0;
    } else if (cand_n < 3u) {
        cand_n++;
        if (cand_n == 3u && cand != state) {
            uint32_t changed = cand ^ state;
            uint32_t pwr = BTN_BIT(BTN_POWER);
            state = cand;
            // The order of the edges is not arbitrary here. The chord
            // resolver (port/game/i_input.c) recognises the shift key by it
            // being *already* held when the direction key goes down -
            // `shift = others & PWR`. Five switches deliver that by
            // themselves, because one thumb presses after the other. A
            // glass surface does not: a finger on the "USE" strip sets
            // POWER and LIGHT in the same sample, and in the counting order
            // of the bits LIGHT would come first. Use would have turned
            // into a shot, and on releasing POWER the resolver would have
            // tapped an ESCAPE as well, taking the press for a single tap -
            // the menu would have opened. So: POWER down first, up last.
            if ((changed & pwr) && (cand & pwr))
                push_edge(BTN_POWER, 1);
            for (int b = 0; b < BTN_COUNT; b++)
                if (b != BTN_POWER && (changed & BTN_BIT(b)))
                    push_edge(b, (cand & BTN_BIT(b)) != 0);
            if ((changed & pwr) && !(cand & pwr))
                push_edge(BTN_POWER, 0);
        }
    }
}

// Two screens at startup, about six seconds in all.
//
// The first names where this comes from. The engine is Artur Bieniek's
// DMC100-Doom - a Doom that fits a meter with 32 KB of RAM, which is the
// only reason it fits a vape at all - and the board, its bring-up and the
// screen-share firmware this port grew out of are atc1441's.
//
// The second is the one you need twice and then never again: a glass
// surface looks the same everywhere, and nothing on it says where turning
// stops and walking starts.
//
// Both are skipped when the host has taken over the injection block, so
// tools/touch.py does not spend six seconds waiting for a legend nobody
// reads.
static int boot_skip(void) { return tinj.magic == TINJ_MAGIC; }

static void boot_wait(uint32_t ms)
{
    for (uint32_t t0 = plat_ms(); plat_ms() - t0 < ms;)
        if (boot_skip())
            return;
}

static void boot_splash(void)
{
    const uint16_t red = 0xF800u, fg = 0xFFFFu, dim = 0x8410u;

    lcd_fill(0, 0, LCD_W - 1, LCD_H - 1, 0);
    lcd_text(96, 26, "DOOM", red, 0, 4);
    lcd_text(48, 72, "on Aspire PIXO", fg, 0, 2);

    lcd_text(32, 112, "port by @atc1441", fg, 0, 2);

    lcd_text(16, 150, "engine: DMC100-Doom by Artur Bieniek", dim, 0, 1);
    lcd_text(16, 164, "github.com/ArturBieniek4/DMC100-Doom", dim, 0, 1);
    lcd_text(0, 192, "github.com/atc1441/Vape_DOOM_ScreenShare", dim, 0, 1);

    boot_wait(2500u);
}

void input_legend(void)
{
    const uint16_t fg = 0xFFFFu, hi = 0xFFE0u, dim = 0x4208u;

    boot_splash();

    lcd_fill(0, 0, LCD_W - 1, LCD_H - 1, 0);
    // Outline the areas: steering cross on the left, action strip on the
    // right, menu corner at the top left.
    lcd_fill(PAD_RIGHT - 2, 0, PAD_RIGHT, LCD_H - 1, dim);
    lcd_fill(PAD_RIGHT, LCD_H / 2 - 2, LCD_W - 1, LCD_H / 2, dim);
    lcd_fill(PAD_CX - PAD_DX, PAD_CY - PAD_DY, PAD_CX + PAD_DX, PAD_CY + PAD_DY, dim);
    lcd_fill(0, 0, MENU_W, MENU_H, dim);

    lcd_text(6, 4, "MENU", hi, dim, 1);
    lcd_text(60, 8, "CONTROLS", hi, 0, 2);

    lcd_text(6, 44, "LEFT AREA:", fg, 0, 1);
    lcd_text(6, 56, "UP WALK, DOWN BACK", fg, 0, 1);
    lcd_text(6, 68, "LEFT/RIGHT TURN", fg, 0, 1);
    lcd_text(6, 80, "CORNERS DO BOTH", fg, 0, 1);

    lcd_text(6, 104, "RIGHT STRIP:", fg, 0, 1);
    lcd_text(6, 116, "TOP USE, BOTTOM FIRE", fg, 0, 1);

    lcd_text(6, 140, "TOP LEFT CORNER: MENU", hi, 0, 1);
    lcd_text(6, 164, "KEY: FIRE (EXTRA)", fg, 0, 1);
    lcd_text(6, 176, "IN MENU: GLASS PICKS, KEY IS OK", fg, 0, 1);

    lcd_text(PAD_RIGHT + 6, 40, "USE", hi, 0, 1);
    lcd_text(PAD_RIGHT + 6, 150, "FIRE", hi, 0, 1);

    boot_wait(3500u);
    lcd_fill(0, 0, LCD_W - 1, LCD_H - 1, 0);
}

void buttons_init(void)
{
    // PB8/PB9 as open drain with a pull-up. The stock firmware leaves PB8 a
    // push-pull output, which would fight the chip on the clock line.
    GPIO_OTYPER(GPIOB) |= (1u << PIN_SCL) | (1u << PIN_SDA);
    GPIO_PUPDR(GPIOB) = (GPIO_PUPDR(GPIOB) & ~((3u << (2 * PIN_SCL)) | (3u << (2 * PIN_SDA)))) |
                        (1u << (2 * PIN_SCL)) | (1u << (2 * PIN_SDA));
    pin_mode(GPIOB, PIN_SCL, 1);
    pin_mode(GPIOB, PIN_SDA, 1);
    sda_rel();
    scl_rel();

    // Free a slave that still holds the line from before the reset.
    for (unsigned i = 0; i < 9; i++) {
        scl_lo();
        qdel();
        scl_rel();
        qdel();
    }
    i2c_stop();

    // Keep the controller awake: it sleeps on its own otherwise and the
    // first touch after that is lost.
    tc_write(0xFEu, 0x01u);  // DisAutoSleep
    tc_write(0xECu, 0x01u);  // no auto reset
}
