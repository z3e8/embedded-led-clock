#include "state_machine_logic.h"
#include "led_board.h" 
#include <stdbool.h>
#include <ti/devices/msp/msp.h>

// index 0 is the 12 o'clock led 
static const uint32_t outer_gpioa_bits[12] = { 
    LED12A_GPIOA_BIT, LED1A_GPIOA_BIT,  LED2A_GPIOA_BIT,  LED3A_GPIOA_BIT,
    LED4A_GPIOA_BIT,  LED5A_GPIOA_BIT,  LED6A_GPIOA_BIT,  LED7A_GPIOA_BIT,
    LED8A_GPIOA_BIT,  LED9A_GPIOA_BIT,  LED10A_GPIOA_BIT, LED11A_GPIOA_BIT
}; // this code was written by ai

static const uint32_t inner_gpioa_bits[12] = { 
    LED12B_GPIOA_BIT, LED1B_GPIOA_BIT,  LED2B_GPIOA_BIT,  LED3B_GPIOA_BIT,
    LED4B_GPIOA_BIT,  LED5B_GPIOA_BIT,  LED6B_GPIOA_BIT, LED7B_GPIOA_BIT,
    LED8B_GPIOA_BIT,  LED9B_GPIOA_BIT,  LED10B_GPIOA_BIT, LED11B_GPIOA_BIT
}; 

// constants 
#define CLOCK_PWM_FREQUENCY_HZ (CLOCK_PWM_WAKEUP_FREQUENCY_HZ / CLOCK_PWM_SUBSTEPS_PER_PERIOD)
#define WAKEUP_TICKS_PER_CLOCK_ADVANCE (CLOCK_PWM_WAKEUP_FREQUENCY_HZ / CLOCK_ADVANCE_FREQUENCY_HZ)

static uint8_t wrap_12(uint8_t idx) { 
    if (idx == 11) { 
        return 0; 
    }
    return (uint8_t)(idx + 1); 
}

static ClockPwmSubstate NextPwmSubstate(ClockPwmSubstate current_substate)
{
    switch (current_substate) {
        case CLOCK_PWM_SUBSTATE_LED_ON:
            return CLOCK_PWM_SUBSTATE_LED_OFF_1;
        case CLOCK_PWM_SUBSTATE_LED_OFF_1:
            return CLOCK_PWM_SUBSTATE_LED_OFF_2;
        case CLOCK_PWM_SUBSTATE_LED_OFF_2:
            return CLOCK_PWM_SUBSTATE_LED_OFF_3;
        case CLOCK_PWM_SUBSTATE_LED_OFF_3:
        default:
            return CLOCK_PWM_SUBSTATE_LED_ON;
    }
}

static bool ShouldIlluminateLedsThisWakeup(ClockPwmSubstate pwm_substate, uint8_t brightness_level,uint8_t brightness_counter)
{
    bool base_pwm_on = (pwm_substate == CLOCK_PWM_SUBSTATE_LED_ON);
    bool brightness_gate_on = (brightness_counter <= brightness_level);
    return (base_pwm_on && brightness_gate_on);
}

void ClockState_Init(ClockState *s) { 
    s->inner_idx = 0; // start at 12 
    s->outer_idx = 0; // start at 12 
    s->pwm_substate = CLOCK_PWM_SUBSTATE_LED_ON;
    s->wakeup_ticks_since_last_clock_advance = 0u;

    s->operating_mode = MODE_NORMAL;
    s->button_state = BUTTON_STATE_IDLE;
    s->button_debounce_ticks = 0u;
    s->button_press_ticks = 0u;
    s->button_long_press_sent = false;
    s->flash_ticks = 0u;
    s->flash_on = true;
    s->brightness_level = (uint8_t)(BRIGHTNESS_LEVEL_COUNT - 1u);
    s->brightness_counter = 0u;
}

void ClockState_Tick(ClockState *s, bool button_is_pressed, uint32_t *gpioa_val, uint32_t *gpiob_val) { 
    uint32_t gpioa = LED_GPIOA_MASK; // all off (active-low LEDs)
    uint32_t gpiob = 0u; // no leds on gpiob on this pcb 
    bool short_press_event = false;
    bool long_press_event = false;

    switch (s->button_state) {
        case BUTTON_STATE_IDLE:
            if (button_is_pressed) {
                s->button_state = BUTTON_STATE_DEBOUNCE_PRESS;
                s->button_debounce_ticks = 1u;
            }
            break;
        case BUTTON_STATE_DEBOUNCE_PRESS:
            if (button_is_pressed) {
                s->button_debounce_ticks++;
                if (s->button_debounce_ticks >= BUTTON_DEBOUNCE_TICKS) {
                    s->button_state = BUTTON_STATE_PRESSED;
                    s->button_press_ticks = 0u;
                    s->button_long_press_sent = false;
                }
            } else {
                s->button_state = BUTTON_STATE_IDLE;
                s->button_debounce_ticks = 0u;
            }
            break;
        case BUTTON_STATE_PRESSED:
            if (button_is_pressed) {
                s->button_press_ticks++;
                if (!s->button_long_press_sent &&
                    s->button_press_ticks >= BUTTON_LONG_PRESS_TICKS) {
                    long_press_event = true;
                    s->button_long_press_sent = true;
                }
            } else {
                s->button_state = BUTTON_STATE_DEBOUNCE_RELEASE;
                s->button_debounce_ticks = 1u;
            }
            break;
        case BUTTON_STATE_DEBOUNCE_RELEASE:
        default:
            if (!button_is_pressed) {
                s->button_debounce_ticks++;
                if (s->button_debounce_ticks >= BUTTON_DEBOUNCE_TICKS) {
                    if (!s->button_long_press_sent) {
                        short_press_event = true;
                    }
                    s->button_state = BUTTON_STATE_IDLE;
                    s->button_debounce_ticks = 0u;
                }
            } else {
                s->button_state = BUTTON_STATE_PRESSED;
                s->button_press_ticks = 0u;
                s->button_long_press_sent = false;
            }
            break;
    }

    if (long_press_event) {
        if (s->operating_mode == MODE_NORMAL) {
            s->operating_mode = MODE_SET_HOUR;
        } else if (s->operating_mode == MODE_SET_HOUR) {
            s->operating_mode = MODE_SET_MINUTE;
        } else if (s->operating_mode == MODE_SET_MINUTE) {
            s->operating_mode = MODE_SET_BRIGHTNESS;
        } else {
            s->operating_mode = MODE_NORMAL;
        }
        s->flash_ticks = 0u;
        s->flash_on = true;
    }

    if (short_press_event) {
        if (s->operating_mode == MODE_SET_HOUR) {
            s->outer_idx = wrap_12(s->outer_idx);
        } else if (s->operating_mode == MODE_SET_MINUTE) {
            s->inner_idx = wrap_12(s->inner_idx);
        } else if (s->operating_mode == MODE_SET_BRIGHTNESS) {
            if (s->brightness_level + 1u >= BRIGHTNESS_LEVEL_COUNT) {
                s->brightness_level = 0u;
            } else {
                s->brightness_level++;
            }
        } else {
            // ignore short press in normal mode
        }
    }

    // pwm control
    s->flash_ticks++;
    if (s->flash_ticks >= FLASH_TOGGLE_TICKS) {
        s->flash_ticks = 0u;
        s->flash_on = !s->flash_on;
    }

    s->brightness_counter++;
    if (s->brightness_counter >= BRIGHTNESS_LEVEL_COUNT) {
        s->brightness_counter = 0u;
    }

    bool show_hour = true;
    bool show_minute = true;
    if (s->operating_mode == MODE_SET_HOUR && !s->flash_on) {
        show_hour = false;
    } else if (s->operating_mode == MODE_SET_MINUTE && !s->flash_on) {
        show_minute = false;
    } else if (s->operating_mode == MODE_SET_BRIGHTNESS && !s->flash_on) {
        show_hour = false;
        show_minute = false;
    } else {
        // normal mode keeps both on
    }

    if (ShouldIlluminateLedsThisWakeup(s->pwm_substate, s->brightness_level, s->brightness_counter)) {
        if (show_hour) {
            gpioa &= ~outer_gpioa_bits[s->outer_idx];
        }
        if (show_minute) {
            gpioa &= ~inner_gpioa_bits[s->inner_idx];
        }
    }

    *gpioa_val = gpioa; 
    *gpiob_val = gpiob; 

    // advance pwm state 
    s->pwm_substate = NextPwmSubstate(s->pwm_substate);

    // advance clock state 
    if (s->operating_mode == MODE_NORMAL) {
        s->wakeup_ticks_since_last_clock_advance++;
        if (s->wakeup_ticks_since_last_clock_advance >= WAKEUP_TICKS_PER_CLOCK_ADVANCE) {
            s->wakeup_ticks_since_last_clock_advance = 0u;

            // inner moves every second, outer moves after inner completes a rotation 
            if (s->inner_idx == 11) {
                s->inner_idx = 0;
                s->outer_idx = wrap_12(s->outer_idx);
            } else {
                s->inner_idx = (uint8_t)(s->inner_idx + 1);
            }
        }
    } else {
        s->wakeup_ticks_since_last_clock_advance = 0u;
    }
}
