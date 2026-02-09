// board led mapping and bit masks 

#ifndef LED_BOARD_H
#define LED_BOARD_H

#include <stdint.h>
#include <ti/devices/msp/msp.h>

// these leds are active-low (0 = on, 1 = off) 

// outer ring (hours) bits 
#define LED12A_GPIOA_BIT (1u << 0)   // this code was written by ai
#define LED1A_GPIOA_BIT  (1u << 26)  
#define LED2A_GPIOA_BIT  (1u << 24)  
#define LED3A_GPIOA_BIT  (1u << 22)  
#define LED4A_GPIOA_BIT  (1u << 18)  
#define LED5A_GPIOA_BIT  (1u << 16)  
#define LED6A_GPIOA_BIT  (1u << 14)  
#define LED7A_GPIOA_BIT  (1u << 12)  
#define LED8A_GPIOA_BIT  (1u << 10)  
#define LED9A_GPIOA_BIT  (1u << 8)   
#define LED10A_GPIOA_BIT (1u << 6)   
#define LED11A_GPIOA_BIT (1u << 28)  

// inner ring (seconds) bits 
#define LED12B_GPIOA_BIT (1u << 27)  
#define LED1B_GPIOA_BIT  (1u << 25)  
#define LED2B_GPIOA_BIT  (1u << 23)  
#define LED3B_GPIOA_BIT  (1u << 21)  
#define LED4B_GPIOA_BIT  (1u << 17)  
#define LED5B_GPIOA_BIT  (1u << 15)  
#define LED6B_GPIOA_BIT  (1u << 13)  
#define LED7B_GPIOA_BIT  (1u << 11)  
#define LED8B_GPIOA_BIT  (1u << 9)   
#define LED9B_GPIOA_BIT  (1u << 7)   
#define LED10B_GPIOA_BIT (1u << 5)   
#define LED11B_GPIOA_BIT (1u << 1)   

// all led bits per port 
#define LED_GPIOA_MASK                                                        \
    (LED12A_GPIOA_BIT | LED1A_GPIOA_BIT | LED2A_GPIOA_BIT | LED3A_GPIOA_BIT | \
     LED4A_GPIOA_BIT | LED5A_GPIOA_BIT | LED6A_GPIOA_BIT | LED7A_GPIOA_BIT | \
     LED8A_GPIOA_BIT | LED9A_GPIOA_BIT | LED10A_GPIOA_BIT | LED11A_GPIOA_BIT | \
     LED12B_GPIOA_BIT | \
     LED1B_GPIOA_BIT | LED2B_GPIOA_BIT | LED3B_GPIOA_BIT | LED4B_GPIOA_BIT | \
     LED5B_GPIOA_BIT | LED6B_GPIOA_BIT | LED7B_GPIOA_BIT | LED8B_GPIOA_BIT | \
     LED9B_GPIOA_BIT | \
     LED10B_GPIOA_BIT | LED11B_GPIOA_BIT) // this code was written by ai

// on the current pcb all leds are on gpioa, so this is 0 
#define LED_GPIOB_MASK (0u) 

// iomux pincm entries for each used pin 
#define LED12A_PINCM IOMUX_PINCM1   // pa0  
#define LED11B_PINCM IOMUX_PINCM2   // pa1  
#define LED11A_PINCM IOMUX_PINCM3   // pa28 
#define LED10B_PINCM IOMUX_PINCM10  // pa5  
#define LED10A_PINCM IOMUX_PINCM11  // pa6  
#define LED9B_PINCM  IOMUX_PINCM14  // pa7  
#define LED9A_PINCM  IOMUX_PINCM19  // pa8  
#define LED8B_PINCM  IOMUX_PINCM20  // pa9  
#define LED8A_PINCM  IOMUX_PINCM21  // pa10 
#define LED7B_PINCM  IOMUX_PINCM22  // pa11 
#define LED7A_PINCM  IOMUX_PINCM34  // pa12 
#define LED6B_PINCM  IOMUX_PINCM35  // pa13 
#define LED6A_PINCM  IOMUX_PINCM36  // pa14 
#define LED5B_PINCM  IOMUX_PINCM37  // pa15 
#define LED5A_PINCM  IOMUX_PINCM38  // pa16 
#define LED4B_PINCM  IOMUX_PINCM39  // pa17 
#define LED4A_PINCM  IOMUX_PINCM40  // pa18 
#define LED3B_PINCM  IOMUX_PINCM46  // pa21 
#define LED3A_PINCM  IOMUX_PINCM47  // pa22 
#define LED2B_PINCM  IOMUX_PINCM53  // pa23 
#define LED2A_PINCM  IOMUX_PINCM54  // pa24 
#define LED1B_PINCM  IOMUX_PINCM55  // pa25 
#define LED1A_PINCM  IOMUX_PINCM59  // pa26 
#define LED12B_PINCM IOMUX_PINCM60  // pa27 

#define BUTTON_GPIOB_BIT (1u << 8)  // pb8
#define BUTTON_PINCM IOMUX_PINCM25

#endif /* LED_BOARD_H */

