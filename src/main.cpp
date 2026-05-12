/**
 * @file    magic_Grimoire.ino
 * @brief   ESP32-based magic Grimoire controller
 *
 * Hardware:
 *  - ESP32 Lolin D32
 *  - RedMP3 module  (UART2: RX=16, TX=17)
 *  - APDS-9960 gesture sensor (I2C: SDA=21, SCL=22)
 *  - Capacitive touch pin (GPIO 2)
 *  - NeoPixel strip 1 : 28 LEDs on GPIO 18  (outer ring / wand body)
 *  - NeoPixel strip 2 :  9 LEDs on GPIO 25  (inner ring / tip)
 *
 * Spell system:
 *  A DIR_NEAR gesture activates input mode.
 *  The user then performs a directional sequence to cast a spell.
 *  Each spell triggers an audio track and a unique LED animation.
 *
 * Porting guide (LED layout):
 *  If your LED strips differ from the default wiring, only edit the
 *  "Hardware Configuration" section below.  All animations reference
 *  logical pixel indices defined there, so no other changes are needed.
 */

// =============================================================================
//  Dependencies
// =============================================================================
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "RedMP3.h"
#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <SparkFun_APDS9960.h>

// =============================================================================
//  Hardware Configuration
//  ↓ Edit this section to match your specific wiring / LED layout.
// =============================================================================

// --- Pin assignments ---
#define PIN_MP3_RX      16
#define PIN_MP3_TX      17
#define PIN_TOUCH        2
#define PIN_NEOPIXEL_1  18   // Outer strip (28 LEDs)
#define PIN_NEOPIXEL_2  25   // Inner strip  (9 LEDs)
#define PIN_I2C_SDA     21
#define PIN_I2C_SCL     22

// --- NeoPixel strip sizes ---
#define NUM_PIXELS_1    28
#define NUM_PIXELS_2     9

// --- Audio track indices on the MP3 module (1-based) ---
#define TRACK_AMBIENT        1
#define TRACK_UP             2
#define TRACK_DOWN           3
#define TRACK_LEFT           4
#define TRACK_RIGHT          5
#define TRACK_DOWN_LEFT      6
#define TRACK_DOWN_RIGHT     7
#define TRACK_UP_LEFT        8
#define TRACK_UP_RIGHT       9
#define TRACK_FAR           10
#define TRACK_NEPHTEAR      11
#define TRACK_DAOSDORG      12
#define TRACK_DORAGATE      13
#define TRACK_REELSEIDEN    14
#define TRACK_REAMSTROHA    15
#define TRACK_SHIELD        16
#define TRACK_WALDGOSE      17
#define TRACK_SORGANEIL     18
#define TRACK_ZOLTRAAK      19
#define TRACK_JILWER        20
#define TRACK_CATASTRAVIA   21
#define TRACK_HEALING       22
#define TRACK_NEAR          23

// --- Volume levels (0–30) ---
#define VOL_LOW     7
#define VOL_MID    11
#define VOL_HIGH   14
#define VOL_LOUDER 17
#define VOL_MAX    30

// --- Animation timing (ms) ---
#define ANIM_STEP_DELAY      50   // Delay between each movement step
#define ANIM_SPELL_PAUSE   1500   // Pause before playing a spell track
#define INPUT_TIMEOUT_MS   5000   // Input mode auto-exit timeout

// --- LED brightness levels (0–255) ---
#define BRIGHTNESS_DIM   2
#define BRIGHTNESS_MID  10

// =============================================================================
//  Colour Palette
//  Define all colours here so spell animations are easy to adjust.
// =============================================================================
// Note: Colors are initialized in setup() because strip objects must exist first.
static uint32_t COLOR_CYAN_TEAL;   // Direction gestures (cardinal)
static uint32_t COLOR_HOT_PINK;    // Direction gestures (diagonal)
static uint32_t COLOR_PALE_YELLOW; // Special gestures (Near/Far)
static uint32_t COLOR_BLACK;       // Off

// Spell-specific colours
static uint32_t COLOR_NEPHTEAR;    // Ice blue
static uint32_t COLOR_DAOSDORG;    // Flame red
static uint32_t COLOR_DORAGATE;    // Dark goldenrod
static uint32_t COLOR_REELSEIDEN;  // Pale turquoise
static uint32_t COLOR_REAMSTROHA;  // Royal blue
static uint32_t COLOR_SHIELD;      // (reuses COLOR_CYAN_TEAL)
static uint32_t COLOR_WALDGOSE;    // Red
static uint32_t COLOR_SORGANEIL;   // Yellow
static uint32_t COLOR_ZOLTRAAK;    // Deep sky blue
static uint32_t COLOR_JILWER;      // Cyan
static uint32_t COLOR_CATASTRAVIA; // Gold
static uint32_t COLOR_HEALING;     // Green-yellow

// =============================================================================
//  Global Objects
// =============================================================================
HardwareSerial       MP3Serial(2);
MP3                  mp3(MP3Serial);
Adafruit_NeoPixel    strip1(NUM_PIXELS_1, PIN_NEOPIXEL_1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel    strip2(NUM_PIXELS_2, PIN_NEOPIXEL_2, NEO_GRB + NEO_KHZ800);
SparkFun_APDS9960    apds;

// =============================================================================
//  State Variables
// =============================================================================
static bool    isPlaying  = false;
static bool    inputMode  = false;
static uint8_t touchFlag  = 0;

// =============================================================================
//  Forward Declarations
// =============================================================================
void performNearGesture();
void LED_BLACK();

// =============================================================================
//  Colour Initialisation
//  Call once in setup() after strip objects are ready.
// =============================================================================
void initColors() {
    COLOR_CYAN_TEAL   = strip1.Color( 64, 224, 208);
    COLOR_HOT_PINK    = strip1.Color(255, 105, 180);
    COLOR_PALE_YELLOW = strip1.Color(255, 250, 180);
    COLOR_BLACK       = strip1.Color(  0,   0,   0);

    COLOR_NEPHTEAR    = strip1.Color(135, 206, 235);
    COLOR_DAOSDORG    = strip1.Color(255,  30,   0);
    COLOR_DORAGATE    = strip1.Color(184, 134,  11);
    COLOR_REELSEIDEN  = strip1.Color(175, 238, 238);
    COLOR_REAMSTROHA  = strip1.Color( 65, 105, 225);
    COLOR_SHIELD      = COLOR_CYAN_TEAL;
    COLOR_WALDGOSE    = strip1.Color(255,   0,   0);
    COLOR_SORGANEIL   = strip1.Color(255, 255,   0);
    COLOR_ZOLTRAAK    = strip1.Color(  0, 191, 255);
    COLOR_JILWER      = strip1.Color(  0, 255, 255);
    COLOR_CATASTRAVIA = strip1.Color(255, 215,   0);
    COLOR_HEALING     = strip1.Color(173, 255,  47);
}

// =============================================================================
//  Low-level LED Helpers
// =============================================================================

/**
 * @brief Turn off all LEDs on both strips.
 */
void LED_BLACK() {
    for (int i = 0; i < NUM_PIXELS_1; i++) strip1.setPixelColor(i, COLOR_BLACK);
    strip1.show();
    for (int i = 0; i < NUM_PIXELS_2; i++) strip2.setPixelColor(i, COLOR_BLACK);
    strip2.show();
}

/**
 * @brief Fill both strips with a colour at a given brightness (used for spell flash effects).
 * @param color  Target colour.
 * @param pixels Number of pixels to light on strip1 (from index 0). Pass NUM_PIXELS_1 for all.
 */
void fillBothStrips(uint32_t color, int pixels = NUM_PIXELS_1) {
    strip1.setBrightness(BRIGHTNESS_DIM);
    strip2.setBrightness(BRIGHTNESS_DIM);
    for (int i = 0; i < pixels;       i++) strip1.setPixelColor(i, color);
    for (int i = 0; i < NUM_PIXELS_2; i++) strip2.setPixelColor(i, color);
    strip1.show();
    strip2.show();
}

/**
 * @brief Ramp up brightness on both strips to produce a gentle fade-in fill.
 * @param color Target colour.
 */
void fillBothStripsRamp(uint32_t color) {
    for (int b = 1; b <= 2; b++) {
        strip1.setBrightness(b);
        strip2.setBrightness(b);
        for (int i = 0; i < NUM_PIXELS_1; i++) strip1.setPixelColor(i, color);
        for (int i = 0; i < NUM_PIXELS_2; i++) strip2.setPixelColor(i, color);
        strip1.show();
        strip2.show();
        delay(100);
    }
}

/**
 * @brief Light a single pixel on the chosen strip, then turn it off.
 * @param stripNum  1 = strip1, 2 = strip2.
 * @param pixel     Pixel index.
 * @param brightness LED brightness.
 * @param color     Colour to display.
 */
void LED_FLASH_PIXEL(int stripNum, int pixel, int brightness, uint32_t color) {
    Adafruit_NeoPixel& s = (stripNum == 1) ? strip1 : strip2;
    s.setBrightness(brightness);
    s.setPixelColor(pixel, color);
    s.show();
    vTaskDelay(pdMS_TO_TICKS(ANIM_STEP_DELAY));
    s.setPixelColor(pixel, COLOR_BLACK);
    s.show();
}

/**
 * @brief Light two pixels simultaneously (optionally with a diagonal-pair companion),
 *        then turn them off.
 *
 * The diagonal-pair logic mirrors the original behaviour: on strip1, pixels 16–27
 * each have a companion at index+1 when diagonal=true.
 */
void LED_FLASH_TWO_PIXELS(
    int prevStrip, int nextStrip,
    int prevPixel, int nextPixel,
    int brightness, uint32_t color,
    bool diagonal = false)
{
    auto applyPixel = [&](Adafruit_NeoPixel& s, int idx, uint32_t c) {
        s.setBrightness(brightness);
        s.setPixelColor(idx, c);
        if (diagonal && &s == &strip1 && idx >= 16 && idx <= 27)
            s.setPixelColor(idx + 1, c);
        s.show();
    };

    Adafruit_NeoPixel& sp = (prevStrip == 1) ? strip1 : strip2;
    Adafruit_NeoPixel& sn = (nextStrip == 1) ? strip1 : strip2;

    applyPixel(sp, prevPixel, color);
    applyPixel(sn, nextPixel, color);
    vTaskDelay(pdMS_TO_TICKS(ANIM_STEP_DELAY));
    applyPixel(sp, prevPixel, COLOR_BLACK);
    applyPixel(sn, nextPixel, COLOR_BLACK);
}

/**
 * @brief Animate a 7-step movement sequence across the two strips.
 *
 * @param strips   Array of 7 strip numbers (1 or 2) for each step.
 * @param pixels   Array of 7 pixel indices for each step.
 * @param color    Colour to use.
 * @param brightness LED brightness.
 * @param diagonal Enable diagonal-pair mode for strip1 pixels 16–27.
 */
void LED_MOVEMENT(
    const int* strips,
    const int* pixels,
    uint32_t color,
    int brightness,
    bool diagonal = false)
{
    // First step: single bright flash.
    LED_FLASH_PIXEL(strips[0], pixels[0], BRIGHTNESS_MID, color);

    // Steps 2–7: two simultaneous pixels to create a flowing trail.
    for (int i = 1; i < 7; i++) {
        LED_FLASH_TWO_PIXELS(
            strips[i - 1], strips[i],
            pixels[i - 1], pixels[i],
            brightness, color, diagonal);
    }
}

// =============================================================================
//  Direction Gesture Animations
//  Each function encodes a 7-step path through the LED geometry.
// =============================================================================

void performRightGesture() {
    LED_BLACK();
    const int strips[7] = { 1, 1, 2, 2, 2, 1, 1 };
    const int pixels[7] = { 8, 22, 4, 8, 0, 16, 0 };
    LED_MOVEMENT(strips, pixels, COLOR_CYAN_TEAL, BRIGHTNESS_DIM);
}

void performLeftGesture() {
    LED_BLACK();
    const int strips[7] = { 1, 1, 2, 2, 2, 1, 1 };
    const int pixels[7] = { 0, 16, 0, 8, 4, 22, 8 };
    LED_MOVEMENT(strips, pixels, COLOR_CYAN_TEAL, BRIGHTNESS_DIM);
}

void performUpGesture() {
    LED_BLACK();
    const int strips[7] = { 1, 1, 2, 2, 2, 1, 1 };
    const int pixels[7] = { 4, 19, 2, 8, 6, 25, 12 };
    LED_MOVEMENT(strips, pixels, COLOR_CYAN_TEAL, BRIGHTNESS_DIM);
}

void performDownGesture() {
    LED_BLACK();
    const int strips[7] = { 1, 1, 2, 2, 2, 1, 1 };
    const int pixels[7] = { 12, 25, 6, 8, 2, 19, 4 };
    LED_MOVEMENT(strips, pixels, COLOR_CYAN_TEAL, BRIGHTNESS_DIM);
}

void performDownLeftGesture() {
    LED_BLACK();
    const int strips[7] = { 1, 1, 2, 2, 2, 1, 1 };
    const int pixels[7] = { 14, 26, 7, 8, 3, 20, 6 };
    LED_MOVEMENT(strips, pixels, COLOR_HOT_PINK, BRIGHTNESS_DIM, true);
}

void performDownRightGesture() {
    LED_BLACK();
    const int strips[7] = { 1, 1, 2, 2, 2, 1, 1 };
    const int pixels[7] = { 10, 23, 5, 8, 1, 17, 2 };
    LED_MOVEMENT(strips, pixels, COLOR_HOT_PINK, BRIGHTNESS_DIM, true);
}

void performUpLeftGesture() {
    LED_BLACK();
    const int strips[7] = { 1, 1, 2, 2, 2, 1, 1 };
    const int pixels[7] = { 2, 17, 1, 8, 5, 23, 10 };
    LED_MOVEMENT(strips, pixels, COLOR_HOT_PINK, BRIGHTNESS_DIM, true);
}

void performUpRightGesture() {
    LED_BLACK();
    const int strips[7] = { 1, 1, 2, 2, 2, 1, 1 };
    const int pixels[7] = { 6, 20, 3, 8, 7, 26, 14 };
    LED_MOVEMENT(strips, pixels, COLOR_HOT_PINK, BRIGHTNESS_DIM, true);
}

void performFarGesture() {
    LED_BLACK();
    const int strips[7] = { 1, 1, 2, 2, 2, 1, 1 };
    const int pixels[7] = { 24, 20, 16, 12, 8, 4, 0 };
    LED_MOVEMENT(strips, pixels, COLOR_PALE_YELLOW, BRIGHTNESS_DIM);
}

// =============================================================================
//  Spell LED Animations
// =============================================================================

/**
 * @brief Flash a fixed set of pixels in a colour, repeating with increasing
 *        intervals to create a build-up effect (used by Shield, Sorganeil, etc.).
 *
 * @param pattern1  First set of (strip, pixel) pairs:  { s0,p0, s1,p1, ... }.
 * @param len1      Number of pairs in pattern1.
 * @param pattern2  Second set of (strip, pixel) pairs (alternating frame).
 * @param len2      Number of pairs in pattern2.
 * @param color     Colour to display.
 * @param pulses    How many pulse cycles to perform.
 */
void pulsePattern(
    const int* pattern1, int len1,
    const int* pattern2, int len2,
    uint32_t color,
    int pulses = 3)
{
    auto applySet = [&](const int* pat, int len) {
        for (int i = 0; i < len; i += 2) {
            Adafruit_NeoPixel& s = (pat[i] == 1) ? strip1 : strip2;
            s.setPixelColor(pat[i + 1], color);
        }
        strip1.show();
        strip2.show();
    };

    for (int p = 0; p < pulses; p++) {
        applySet(pattern1, len1);
        delay(p * 50);
        LED_BLACK();
        delay(p * 50);

        applySet(pattern2, len2);
        delay(p * 100);
        LED_BLACK();
        delay(p * 100);
    }
    // Leave final frame on (caller should call CheckPlayAndBlack to clear).
    applySet(pattern1, len1);
}

// --- Nephtear: Arrows of Ice ---
void animNephtear() {
    fillBothStripsRamp(COLOR_NEPHTEAR);
}

// --- Daosdorg: Wind becomes Hellfire ---
void animDaosdorg() {
    fillBothStripsRamp(COLOR_DAOSDORG);
}

// --- Doragate: Change rocks into bullets ---
void animDoragate() {
    fillBothStripsRamp(COLOR_DORAGATE);
}

// --- Reelseiden: Spell that slashes almost anything ---
void animReelseiden() {
    fillBothStripsRamp(COLOR_REELSEIDEN);
    // Continuously alternate diagonal gestures while audio plays.
    int count = 0;
    int data;
    delay(1000);
    while (true) {
        if (count % 2 == 0) performDownRightGesture();
        else                 performDownLeftGesture();
        data = mp3.getStatus();
        delay(1000);
        if (data == 0) break;
        count++;
    }
}

// --- Reamstroha: Water Manipulation ---
void animReamstroha() {
    // Inner ring fills first, then outer ring fills from centre outward.
    for (int i = 0; i < NUM_PIXELS_2; i++) {
        strip2.setPixelColor(i, COLOR_REAMSTROHA);
        strip2.show();
        delay(100);
    }
    for (int i = 15; i >= 0; i--) {
        strip1.setPixelColor(i, COLOR_REAMSTROHA);
        strip1.show();
        delay(50);
    }
}

// --- Shield ---
void animShield() {
    // Two interlocking ring patterns pulse with increasing speed.
    const int pat1[] = {
        1,26, 1,12, 1,24, 2,6,
        1, 0, 1,27, 2, 0, 1,17,
        1,18, 2, 2, 1,20, 1, 4,
        2, 4, 1,23, 1, 8, 1,21
    };
    const int pat2[] = {
        1,14, 1,26, 2, 7, 1,27,
        1,24, 1,10, 1,23, 2, 5,
        2, 3, 1,21, 1, 6, 1,20,
        1,17, 2, 1, 1,18, 1, 2
    };
    pulsePattern(pat1, 32, pat2, 32, COLOR_SHIELD);
}

// --- Waldgose: Tornado Winds ---
void animWaldgose() {
    // Rapid sequential fill mimics a spinning wind effect.
    for (int i = 0; i < NUM_PIXELS_1; i++) {
        strip1.setPixelColor(i, COLOR_WALDGOSE);
        strip1.show();
        delay((int)(50 - i * 1.5f));
    }
    for (int i = 0; i < NUM_PIXELS_2; i++) {
        strip2.setPixelColor(i, COLOR_WALDGOSE);
        strip2.show();
        delay(100 - (NUM_PIXELS_1 * 2) - (i * 2));
    }
}

// --- Sorganeil: Spell of Binding ---
void animSorganeil() {
    // A circular binding ring pulses (same pattern for both frames here).
    const int pat[] = {
        1,14, 2,7, 2,3, 1,6,
        2, 8,
        1,10, 2,5, 2,1, 1,2
    };
    pulsePattern(pat, 18, pat, 18, COLOR_SORGANEIL);
}

// --- Zoltraak: Offensive Magic ---
void animZoltraak() {
    // Rapid flash of the lower half of strip1 and all of strip2.
    for (int pass = 0; pass < 3; pass++) {
        for (int i = 0; i < 16;          i++) strip1.setPixelColor(i, COLOR_ZOLTRAAK);
        for (int i = 0; i < NUM_PIXELS_2; i++) strip2.setPixelColor(i, COLOR_ZOLTRAAK);
        strip1.show(); strip2.show();
        delay(pass * 50);

        LED_BLACK();
        delay(pass * 50);

        for (int i = 0; i < 16;          i++) strip1.setPixelColor(i, COLOR_ZOLTRAAK);
        for (int i = 0; i < NUM_PIXELS_2; i++) strip2.setPixelColor(i, COLOR_ZOLTRAAK);
        strip1.show(); strip2.show();
        delay(pass * 100);

        LED_BLACK();
        delay(pass * 100);
    }
    // Leave final frame on.
    for (int i = 0; i < 16;          i++) strip1.setPixelColor(i, COLOR_ZOLTRAAK);
    for (int i = 0; i < NUM_PIXELS_2; i++) strip2.setPixelColor(i, COLOR_ZOLTRAAK);
    strip1.show(); strip2.show();
}

// --- Jilwer: Spell for High-Speed Movement ---
void animJilwer() {
    fillBothStripsRamp(COLOR_JILWER);
}

// --- Catastravia: Lights of Judgement ---
void animCatastravia() {
    // Two alternating patterns (ring + cross) pulse with the gold colour.
    const int pat1[] = {
        1,14, 2,7, 2,3, 1,6,
        2, 8,
        1,10, 2,5, 2,1, 1,2
    };
    const int pat2[] = {
        1,12, 1,25, 2,6,
        2, 8,
        2, 2, 1,19, 1,4,
        1, 8, 1,22, 2,4,
        2, 0, 1,16, 1,0
    };
    pulsePattern(pat1, 18, pat2, 30, COLOR_CATASTRAVIA);

    // Leave both patterns on simultaneously.
    for (int i = 0; i < 18; i += 2) {
        Adafruit_NeoPixel& s = (pat1[i] == 1) ? strip1 : strip2;
        s.setPixelColor(pat1[i + 1], COLOR_CATASTRAVIA);
    }
    for (int i = 0; i < 30; i += 2) {
        Adafruit_NeoPixel& s = (pat2[i] == 1) ? strip1 : strip2;
        s.setPixelColor(pat2[i + 1], COLOR_CATASTRAVIA);
    }
    strip1.show();
    strip2.show();
}

// --- Healing ---
void animHealing() {
    fillBothStripsRamp(COLOR_HEALING);
}

// =============================================================================
//  Utility: Wait for MP3 to finish, then clear LEDs.
// =============================================================================
void waitForAudioThenBlack() {
    vTaskDelay(pdMS_TO_TICKS(1000));
    while (true) {
        int status = mp3.getStatus();
        delay(20);
        if (status == 0) break;
    }
    LED_BLACK();
    delay(500);
    LED_BLACK();
}

// =============================================================================
//  Spell Dispatcher
//  Maps gesture sequences to spell animations + audio.
// =============================================================================

/**
 * @brief Play an audio track and run an animation, then clear.
 */
void castSpell(int volume, int track, void (*animFn)()) {
    delay(ANIM_SPELL_PAUSE);
    mp3.setVolume(volume);
    mp3.pause();
    mp3.playWithIndex(track);
    animFn();
    waitForAudioThenBlack();
    inputMode = false;
}

/**
 * @brief Evaluate the current gesture sequence and cast a spell if matched.
 * @param seq   Pointer to the gesture sequence array.
 * @param len   Number of gestures recorded so far.
 * @return true if a spell was matched and cast.
 */
bool tryMatchSpell(const int* seq, int len) {

    // --- Nephtear: Left → Right → Left ---
    if (len == 3
        && seq[0] == DIR_LEFT && seq[1] == DIR_RIGHT && seq[2] == DIR_LEFT) {
        castSpell(VOL_MID, TRACK_NEPHTEAR, animNephtear);
        return true;
    }
    // --- Daosdorg: Up → Down → Left → Right ---
    if (len == 4
        && seq[0] == DIR_UP   && seq[1] == DIR_DOWN
        && seq[2] == DIR_LEFT && seq[3] == DIR_RIGHT) {
        castSpell(VOL_LOUDER, TRACK_DAOSDORG, animDaosdorg);
        return true;
    }
    // --- Doragate: Left → Down-Left → Right ---
    if (len == 3
        && seq[0] == DIR_LEFT && seq[1] == DIR_DOWN_LEFT && seq[2] == DIR_RIGHT) {
        castSpell(VOL_LOUDER, TRACK_DORAGATE, animDoragate);
        return true;
    }
    // --- Reelseiden: Up → Up-Right → Down → Down-Right → Up ---
    if (len == 5
        && seq[0] == DIR_UP       && seq[1] == DIR_UP_RIGHT
        && seq[2] == DIR_DOWN     && seq[3] == DIR_DOWN_RIGHT
        && seq[4] == DIR_UP) {
        castSpell(VOL_LOUDER, TRACK_REELSEIDEN, animReelseiden);
        return true;
    }
    // --- Reamstroha: Left → Up → Right → Down → Up → Left ---
    if (len == 6
        && seq[0] == DIR_LEFT && seq[1] == DIR_UP
        && seq[2] == DIR_RIGHT && seq[3] == DIR_DOWN
        && seq[4] == DIR_UP   && seq[5] == DIR_LEFT) {
        castSpell(VOL_HIGH, TRACK_REAMSTROHA, animReamstroha);
        return true;
    }
    // --- Shield: Right → Down-Right → Left ---
    if (len == 3
        && seq[0] == DIR_RIGHT && seq[1] == DIR_DOWN_RIGHT && seq[2] == DIR_LEFT) {
        castSpell(VOL_HIGH, TRACK_SHIELD, animShield);
        return true;
    }
    // --- Waldgose: Up → Down-Right → Down → Up-Left ---
    if (len == 4
        && seq[0] == DIR_UP        && seq[1] == DIR_DOWN_RIGHT
        && seq[2] == DIR_DOWN      && seq[3] == DIR_UP_LEFT) {
        castSpell(VOL_HIGH, TRACK_WALDGOSE, animWaldgose);
        return true;
    }
    // --- Sorganeil: Left → Up → Right → Down → Up-Left → Down-Right → Up → Down ---
    if (len == 8
        && seq[0] == DIR_LEFT      && seq[1] == DIR_UP
        && seq[2] == DIR_RIGHT     && seq[3] == DIR_DOWN
        && seq[4] == DIR_UP_LEFT   && seq[5] == DIR_DOWN_RIGHT
        && seq[6] == DIR_UP        && seq[7] == DIR_DOWN) {
        castSpell(VOL_HIGH, TRACK_SORGANEIL, animSorganeil);
        return true;
    }
    // --- Zoltraak: Up → Right → Down ---
    if (len == 3
        && seq[0] == DIR_UP && seq[1] == DIR_RIGHT && seq[2] == DIR_DOWN) {
        castSpell(VOL_HIGH, TRACK_ZOLTRAAK, animZoltraak);
        return true;
    }
    // --- Jilwer: Left → Up-Left → Down → Right ---
    if (len == 4
        && seq[0] == DIR_LEFT    && seq[1] == DIR_UP_LEFT
        && seq[2] == DIR_DOWN    && seq[3] == DIR_RIGHT) {
        castSpell(VOL_HIGH, TRACK_JILWER, animJilwer);
        return true;
    }
    // --- Catastravia: Down-Right → Up → Up-Left → Down → Right ---
    if (len == 5
        && seq[0] == DIR_DOWN_RIGHT && seq[1] == DIR_UP
        && seq[2] == DIR_UP_LEFT    && seq[3] == DIR_DOWN
        && seq[4] == DIR_RIGHT) {
        castSpell(VOL_HIGH, TRACK_CATASTRAVIA, animCatastravia);
        return true;
    }
    // --- Healing: Down-Left → Right → Up ---
    if (len == 3
        && seq[0] == DIR_DOWN_LEFT && seq[1] == DIR_RIGHT && seq[2] == DIR_UP) {
        castSpell(VOL_LOUDER, TRACK_HEALING, animHealing);
        return true;
    }

    return false;
}

// =============================================================================
//  Gesture Handling
// =============================================================================

/**
 * @brief Read one gesture from the APDS-9960.
 *        Plays a directional audio cue and animates the LED path for each
 *        recognised direction.  Returns the gesture code or 0xFF if none.
 */
uint8_t handleGesture() {
    if (!apds.isGestureAvailable()) return 0xFF;

    uint8_t gesture = apds.readGesture();

    switch (gesture) {
        case DIR_UP:
            mp3.setVolume(VOL_LOW); mp3.pause(); mp3.playWithIndex(TRACK_UP);
            performUpGesture();      break;
        case DIR_DOWN:
            mp3.setVolume(VOL_LOW); mp3.pause(); mp3.playWithIndex(TRACK_DOWN);
            performDownGesture();    break;
        case DIR_LEFT:
            mp3.setVolume(VOL_LOW); mp3.pause(); mp3.playWithIndex(TRACK_LEFT);
            performLeftGesture();    break;
        case DIR_RIGHT:
            mp3.setVolume(VOL_LOW); mp3.pause(); mp3.playWithIndex(TRACK_RIGHT);
            performRightGesture();   break;
        case DIR_NEAR:
            mp3.setVolume(VOL_HIGH); mp3.pause(); mp3.playWithIndex(TRACK_NEAR);
            performNearGesture();    break;
        case DIR_FAR:
            mp3.setVolume(VOL_LOW); mp3.pause(); mp3.playWithIndex(TRACK_FAR);
            performFarGesture();     break;
        case DIR_DOWN_LEFT:
            mp3.setVolume(VOL_LOW); mp3.pause(); mp3.playWithIndex(TRACK_DOWN_LEFT);
            performDownLeftGesture();  break;
        case DIR_DOWN_RIGHT:
            mp3.setVolume(VOL_LOW); mp3.pause(); mp3.playWithIndex(TRACK_DOWN_RIGHT);
            performDownRightGesture(); break;
        case DIR_UP_LEFT:
            mp3.setVolume(VOL_LOW); mp3.pause(); mp3.playWithIndex(TRACK_UP_LEFT);
            performUpLeftGesture();    break;
        case DIR_UP_RIGHT:
            mp3.setVolume(VOL_LOW); mp3.pause(); mp3.playWithIndex(TRACK_UP_RIGHT);
            performUpRightGesture();   break;
        default: break;
    }
    return gesture;
}

// =============================================================================
//  Near Gesture: Spell Input Mode
// =============================================================================

/**
 * @brief Enter spell input mode.
 *
 * Fills both strips sequentially as a visual "ready" indicator, then waits
 * up to INPUT_TIMEOUT_MS for a directional sequence.  Calls tryMatchSpell()
 * after each new gesture; exits on match, timeout, or sequence overflow.
 */
void performNearGesture() {
    strip1.setBrightness(BRIGHTNESS_DIM);
    strip2.setBrightness(BRIGHTNESS_DIM);

    // Sequential fill animation: strip1 then strip2.
    for (int i = 0; i < NUM_PIXELS_1; i++) {
        strip1.setPixelColor(i, COLOR_PALE_YELLOW);
        strip1.show();
        delay((int)(50 - i * 1.5f));
    }
    for (int i = 0; i < NUM_PIXELS_2; i++) {
        strip2.setPixelColor(i, COLOR_PALE_YELLOW);
        strip2.show();
        delay(100 - (NUM_PIXELS_1 * 2) - (i * 2));
    }

    // --- Input loop ---
    const int MAX_SEQUENCE = 10;
    int sequence[MAX_SEQUENCE] = {};
    int sequenceLen = 0;
    unsigned long lastInputTime = millis();
    inputMode = true;

    while (inputMode) {
        if (millis() - lastInputTime > INPUT_TIMEOUT_MS) {
            LED_BLACK();
            inputMode = false;
            break;
        }

        uint8_t gesture = handleGesture();
        bool isDirectional =
            gesture == DIR_LEFT      || gesture == DIR_RIGHT     ||
            gesture == DIR_UP        || gesture == DIR_DOWN       ||
            gesture == DIR_UP_LEFT   || gesture == DIR_UP_RIGHT   ||
            gesture == DIR_DOWN_LEFT || gesture == DIR_DOWN_RIGHT;

        if (isDirectional && sequenceLen < MAX_SEQUENCE) {
            sequence[sequenceLen++] = gesture;
            lastInputTime = millis();

            if (tryMatchSpell(sequence, sequenceLen)) break;
        }
    }
}

// =============================================================================
//  Touch Sensor
// =============================================================================

uint16_t readTouchSensor() {
    uint16_t val = touchRead(PIN_TOUCH);
    if (val >= 40) {
        if (touchFlag == 0) return val;
        delay(10);
        touchFlag = 0;
        return val;
    } else {
        if (touchFlag != 0) return 80;
        touchFlag = 1;
        delay(10);
        return val;
    }
}

/**
 * @brief Toggle play/pause via the capacitive touch sensor.
 *        Called from loop(); acts only when a touch is detected.
 */
void checkTouch() {
    if (readTouchSensor() < 40) {
        if (isPlaying) mp3.pause();
        else           mp3.play();
        isPlaying = !isPlaying;
    }
}

// =============================================================================
//  Subsystem Initialisation
// =============================================================================

void initMP3() {
    MP3Serial.begin(9600, SERIAL_8N1, PIN_MP3_RX, PIN_MP3_TX);
    mp3.setVolume(VOL_LOW);
    mp3.playWithIndex(TRACK_AMBIENT);
}

void initNeoPixels() {
    strip1.begin();
    strip1.setBrightness(BRIGHTNESS_MID);
    strip2.begin();
    strip2.setBrightness(BRIGHTNESS_MID);
}

void initGestureSensor() {
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    Wire.setClock(100000);
    delay(600);

    if (apds.init())
        Serial.println(F("APDS-9960 initialised."));
    else
        Serial.println(F("APDS-9960 init FAILED."));

    if (apds.enableGestureSensor(false))
        Serial.println(F("Gesture sensor running."));
    else
        Serial.println(F("Gesture sensor enable FAILED."));
}

// =============================================================================
//  Arduino Entry Points
// =============================================================================

void setup() {
    Serial.begin(115200);
    initMP3();
    initNeoPixels();
    initColors();
    initGestureSensor();
}

void loop() {
    checkTouch();
    if (!inputMode) handleGesture();
}
