#ifndef state_machine_logic_include
#define state_machine_logic_include

#include <stdint.h> 
#include <stdbool.h>

// Phase 3 config
// timer interrupt wakes the MCU at CLOCK_PWM_WAKEUP_FREQUENCY_HZ.
// PWM period is CLOCK_PWM_SUBSTEPS_PER_PERIOD 
// duty cycle is CLOCK_PWM_LED_ON_SUBSTEPS / CLOCK_PWM_SUBSTEPS_PER_PERIOD.
//
// note:
// - CLOCK_PWM_WAKEUP_FREQUENCY_HZ must divide 32000 (LFCLK), be a multiple of CLOCK_PWM_SUBSTEPS_PER_PERIOD, and should be high enough to avoid visible flicker 
// written by AI
#define CLOCK_PWM_WAKEUP_FREQUENCY_HZ (800u)
#define CLOCK_PWM_SUBSTEPS_PER_PERIOD (4u)
#define CLOCK_PWM_LED_ON_SUBSTEPS (1u) // 25% duty cycle 

// clock advances once per second 
#define CLOCK_ADVANCE_FREQUENCY_HZ (1u)

#define BUTTON_DEBOUNCE_MS (5u)
#define BUTTON_DEBOUNCE_TICKS ((CLOCK_PWM_WAKEUP_FREQUENCY_HZ * BUTTON_DEBOUNCE_MS) / 1000u)
#define BUTTON_LONG_PRESS_TICKS (CLOCK_PWM_WAKEUP_FREQUENCY_HZ)
#define FLASH_TOGGLE_FREQUENCY_HZ (2u)
#define FLASH_TOGGLE_TICKS (CLOCK_PWM_WAKEUP_FREQUENCY_HZ / FLASH_TOGGLE_FREQUENCY_HZ)
#define BRIGHTNESS_LEVEL_COUNT (16u)

// written by AI
typedef enum {
    CLOCK_PWM_SUBSTATE_LED_ON = 0,
    CLOCK_PWM_SUBSTATE_LED_OFF_1 = 1,
    CLOCK_PWM_SUBSTATE_LED_OFF_2 = 2,
    CLOCK_PWM_SUBSTATE_LED_OFF_3 = 3,
} ClockPwmSubstate;

typedef enum {
    MODE_NORMAL = 0,
    MODE_SET_HOUR = 1,
    MODE_SET_MINUTE = 2,
    MODE_SET_BRIGHTNESS = 3,
} OperatingMode;

typedef enum {
    BUTTON_STATE_IDLE = 0,
    BUTTON_STATE_DEBOUNCE_PRESS = 1,
    BUTTON_STATE_PRESSED = 2,
    BUTTON_STATE_DEBOUNCE_RELEASE = 3,
} ButtonState;

typedef struct { 
    uint8_t inner_idx; // 0..11 
    uint8_t outer_idx; // 0..11 
    ClockPwmSubstate pwm_substate;
    uint16_t wakeup_ticks_since_last_clock_advance;

    OperatingMode operating_mode;
    ButtonState button_state;
    uint16_t button_debounce_ticks;
    uint16_t button_press_ticks;
    bool button_long_press_sent;
    uint16_t flash_ticks;
    bool flash_on;
    uint8_t brightness_level;
    uint8_t brightness_counter;
} ClockState; 

void ClockState_Init(ClockState *s); 
void ClockState_Tick(ClockState *s, bool button_is_pressed, uint32_t *gpioa_val, uint32_t *gpiob_val); // this code was written by ai

#endif /* state_machine_logic_include */
