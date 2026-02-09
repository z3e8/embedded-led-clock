/*
 * This template file
 */

 #include <ti/devices/msp/msp.h>
 #include <stdbool.h>
 #include "delay.h"
 #include "initialize_leds.h"
 #include "led_board.h" 
 #include "state_machine_logic.h"
 
 // LFCLK runs at 32 kHz
 #define LFCLK_FREQUENCY_HZ (32000u)
 #define TIMG0_LOAD_VALUE_FOR_FIXED_WAKEUPS (LFCLK_FREQUENCY_HZ / CLOCK_PWM_WAKEUP_FREQUENCY_HZ)
 
 static volatile uint32_t g_timg0_zero_event_counter = 0u;
 
 static void InitializeTimerG0ForFixedRateStandbyWakeups(void)
 {
     // enable timer 
     TIMG0->GPRCM.RSTCTL =
         (GPIO_RSTCTL_KEY_UNLOCK_W | GPIO_RSTCTL_RESETSTKYCLR_CLR |
          GPIO_RSTCTL_RESETASSERT_ASSERT);
     TIMG0->GPRCM.PWREN =
         (GPIO_PWREN_KEY_UNLOCK_W | GPIO_PWREN_ENABLE_ENABLE);
     delay_cycles(16); // allow timer power/reset to settle
 
     // LFCLK so timer runs during STANDBY
     TIMG0->CLKSEL = GPTIMER_CLKSEL_LFCLK_SEL_ENABLE;
 
     // periodic down-counting config 
     TIMG0->COUNTERREGS.CTRCTL = GPTIMER_CTRCTL_REPEAT_REPEAT_1;
 
     // interrupt on zero evnt 
     TIMG0->CPU_INT.IMASK |= GPTIMER_CPU_INT_IMASK_Z_SET;
 
     // timer clock enable
     TIMG0->COMMONREGS.CCLKCTL = GPTIMER_CCLKCTL_CLKEN_ENABLED;
 
     // deep sleep config
     SYSCTL->SOCLOCK.PMODECFG = SYSCTL_PMODECFG_DSLEEP_STANDBY;
     SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
     SYSCTL->SOCLOCK.MCLKCFG |= SYSCTL_MCLKCFG_STOPCLKSTBY_ENABLE;
 
     // 1 Hz period and timer start 
     TIMG0->COUNTERREGS.LOAD = TIMG0_LOAD_VALUE_FOR_FIXED_WAKEUPS;
     TIMG0->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_EN_ENABLED;
 
     // interrupt line in NVIC
     NVIC_EnableIRQ(TIMG0_INT_IRQn);
 }
 
 int main(void)
 {
     InitializeGPIO(); 
 
     ClockState clock; 
     ClockState_Init(&clock); 
 
     uint32_t gpioa_val = LED_GPIOA_MASK; // all off 
     uint32_t gpiob_val = LED_GPIOB_MASK; // all off 
 
     // display initial state
    ClockState_Tick(&clock, false, &gpioa_val, &gpiob_val);
     update_reg(&GPIOA->DOUT31_0, gpioa_val, LED_GPIOA_MASK);
     update_reg(&GPIOB->DOUT31_0, gpiob_val, LED_GPIOB_MASK);
 
     // periodic wakeups
     InitializeTimerG0ForFixedRateStandbyWakeups();
 
     // Functional
     while (1) {
         if (g_timg0_zero_event_counter == 0u) {
             __WFI(); // sleep until an interrupt wakes the CPU
         }
 
         bool has_pending_timer_tick = false;
         __disable_irq();
         if (g_timg0_zero_event_counter != 0u) {
             g_timg0_zero_event_counter--;
             has_pending_timer_tick = true;
         }
         __enable_irq();
 
         if (!has_pending_timer_tick) {
             continue; // woke for a non-timer interrupt; ignore
         }
 
        // read button state
        uint32_t gpiob_in = GPIOB->DIN31_0;
        bool button_is_pressed = ((gpiob_in & BUTTON_GPIOB_BIT) == 0u);

        ClockState_Tick(&clock, button_is_pressed, &gpioa_val, &gpiob_val); 
 
         // write only led bits
         update_reg(&GPIOA->DOUT31_0, gpioa_val, LED_GPIOA_MASK); 
         update_reg(&GPIOB->DOUT31_0, gpiob_val, LED_GPIOB_MASK); 
     }
 }
 
 void TIMG0_IRQHandler(void)
 {
     // handle interrupt by reading IIDX
     switch (TIMG0->CPU_INT.IIDX) {
         case GPTIMER_CPU_INT_IIDX_STAT_Z:
             g_timg0_zero_event_counter++;
             break;
         default:
             break;
     }
 }
 
 /*
  * Copyright (c) 2023, Texas Instruments Incorporated
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without
  * modification, are permitted provided that the following conditions
  * are met:
  *
  * *  Redistributions of source code must retain the above copyright
  *    notice, this list of conditions and the following disclaimer.
  *
  * *  Redistributions in binary form must reproduce the above copyright
  *    notice, this list of conditions and the following disclaimer in the
  *    documentation and/or other materials provided with the distribution.
  *
  * *  Neither the name of Texas Instruments Incorporated nor the names of
  *    its contributors may be used to endorse or promote products derived
  *    from this software without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
  * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
  * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
  * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
  * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
  * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
  * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
  * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
  * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  */
 