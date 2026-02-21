#include QMK_KEYBOARD_H
#include "timer.h"

// ===== Layers =====
enum layers {
    _L0,
    _L1,
    _L2,
    _L3, 
    _L4  // SETTINGS
};

// ===== Custom keycodes =====
enum custom_keycodes {
    LAYER_DOWN = SAFE_RANGE,
    LAYER_UP,
    BTN_HUE,
    BTN_SAT,
    BTN_VAL,
    RGB_MODE_NEXT,
    RGB_MODE_PREV,
    RGB_SPEED_UP,
    RGB_SPEED_DOWN,
    RGB_EFFECT_TOGGLE,
    RGB_EFFECT_SINGLE
};

// ===== Splash bitmap =====
const char splash_bitmap[] PROGMEM = {
    0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,
    0x90,0x91,0x92,0x93,0x94,
    0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,
    0xB0,0xB1,0xB2,0xB3,0xB4,
    0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,
    0xD0,0xD1,0xD2,0xD3,0xD4,0x00
};

// ===== Keymaps =====
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

    [_L0] = {
        { KC_MPRV, KC_MPLY, KC_MNXT },
        { KC_PSCR, KC_COPY, KC_PASTE },
        { LAYER_DOWN, KC_MUTE, LAYER_UP }
    },

    [_L1] = {
        { KC_1, KC_2, KC_3 },
        { KC_4, KC_5, KC_6 },
        { LAYER_DOWN, KC_7, LAYER_UP }
    },

    [_L2] = {
        { KC_1, KC_2, KC_3 },
        { KC_4, KC_5, KC_6 },
        { LAYER_DOWN, KC_7, LAYER_UP }
    },

    [_L3] = {
        { KC_1, KC_2, KC_3 },
        { KC_4, KC_5, KC_6 },
        { LAYER_DOWN, KC_7, LAYER_UP }
    },

    // Layer 4 = Settings
    [_L4] = {
        { BTN_VAL, BTN_SAT, BTN_HUE },
        { RGB_MODE_PREV, RGB_EFFECT_SINGLE, RGB_MODE_NEXT },
        { LAYER_DOWN, QK_BOOT, LAYER_UP }
    }
};

// ===== RGB encoder state =====
typedef enum { NONE, HUE, SAT, VAL } rgb_mode_t;
static rgb_mode_t active_rgb_mode = NONE;
static uint16_t rgb_timer = 0;
#define RGB_TIMEOUT 2000  // 2 seconden

// ===== NIEUW: Volume lock timer =====
static uint16_t volume_lock_timer = 0;
#define VOLUME_LOCK_TIME 3000  // 3 seconden

// ===== Layer Switching & Custom Keycodes =====
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (!record->event.pressed) return true;

    uint8_t current = get_highest_layer(layer_state);

    switch (keycode) {
        case LAYER_UP:
            layer_move((current + 1) % 5);
            return false;
        case LAYER_DOWN:
            layer_move((current + 4) % 5);
            return false;

        // Settings layer RGB buttons
        case BTN_HUE:
            if (current == _L4) {
                active_rgb_mode = HUE;
                rgb_timer = timer_read();
                volume_lock_timer = timer_read();   // volume blokkeren
            }
            return false;

        case BTN_SAT:
            if (current == _L4) {
                active_rgb_mode = SAT;
                rgb_timer = timer_read();
                volume_lock_timer = timer_read();   // volume blokkeren
            }
            return false;

        case BTN_VAL:
            if (current == _L4) {
                active_rgb_mode = VAL;
                rgb_timer = timer_read();
                volume_lock_timer = timer_read();   // volume blokkeren
            }
            return false;

        case RGB_MODE_NEXT:
            if (current == _L4) rgblight_step();
            return false;

        case RGB_MODE_PREV:
            if (current == _L4) rgblight_step_reverse();
            return false;

        case RGB_SPEED_UP:
            if (current == _L3) rgblight_increase_speed();
            return false;

        case RGB_SPEED_DOWN:
            if (current == _L3) rgblight_decrease_speed();
            return false;

        case RGB_EFFECT_TOGGLE:
            if (current == _L4) rgblight_toggle();
            return false;

        case RGB_EFFECT_SINGLE:
            if (current == _L4) rgblight_mode(RGBLIGHT_MODE_STATIC_LIGHT);
            return false;
    }

    return true;
}

// ===== Encoder handling =====
bool encoder_update_user(uint8_t index, bool clockwise) {

    uint8_t layer = get_highest_layer(layer_state);

    // 1️⃣ RGB encoder actief?
    if (active_rgb_mode != NONE &&
        layer == _L4 &&
        timer_elapsed(rgb_timer) <= RGB_TIMEOUT) {

        switch (active_rgb_mode) {
            case HUE:
                if (clockwise) rgblight_increase_hue();
                else rgblight_decrease_hue();
                break;
            case SAT:
                if (clockwise) rgblight_increase_sat();
                else rgblight_decrease_sat();
                break;
            case VAL:
                if (clockwise) rgblight_increase_val();
                else rgblight_decrease_val();
                break;
            default:
                break;
        }

        return false;  // GEEN volume
    }

    // 2️⃣ Volume tijdelijk geblokkeerd?
    if (timer_elapsed(volume_lock_timer) <= VOLUME_LOCK_TIME) {
        return false;  // nog geen volume toegestaan
    }

    // 3️⃣ Normaal gedrag (volume mag weer)
    return true;
}

#ifdef OLED_ENABLE

static bool splash_done = false;
static uint16_t splash_timer = 0;

oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    return OLED_ROTATION_0;
}

bool oled_task_user(void) {

    if (!splash_done) {
        splash_timer = timer_read();
        oled_clear();
        oled_write_P(splash_bitmap, false); 
        splash_done = true;
        return false;
    }

    if (timer_elapsed(splash_timer) < 2000) return false;

    static uint8_t last_layer = 255;
    uint8_t layer = get_highest_layer(layer_state);

    if (layer != last_layer) {
        oled_clear();
        oled_set_cursor(0, 0);
        switch (layer) {
            case _L0: oled_write_ln_P(PSTR("Layer 0"), false); break;
            case _L1: oled_write_ln_P(PSTR("Layer 1"), false); break;
            case _L2: oled_write_ln_P(PSTR("Layer 2"), false); break;
            case _L3: oled_write_ln_P(PSTR("Layer 3"), false); break;
            case _L4: oled_write_ln_P(PSTR("SETTINGS"), false); break;
            default:  oled_write_ln_P(PSTR("?"), false); break;
        }
        last_layer = layer;
    }

    return false;
}

#endif