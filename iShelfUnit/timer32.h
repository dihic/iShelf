/****************************************************************************
 *   $Id:: timer32.h 3635 2010-06-02 00:31:46Z usb00423                     $
 *   Project: NXP LPC11xx software example
 *
 *   Description:
 *     This file contains definition and prototype for 32-bit timer 
 *     configuration.
 *
 ****************************************************************************
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * products. This software is supplied "AS IS" without any warranties.
 * NXP Semiconductors assumes no responsibility or liability for the
 * use of the software, conveys no license or title under any patent,
 * copyright, or mask work right to the product. NXP Semiconductors
 * reserves the right to make changes in the software without
 * notification. NXP Semiconductors also make no representation or
 * warranty that such application will be suitable for the specified
 * use without further testing or modification.
****************************************************************************/
#ifndef __TIMER32_H 
#define __TIMER32_H

#include <stdint.h>

/* The test is either MAT_OUT or CAP_IN. Default is MAT_OUT. */
#define TIMER_MATCH		0

#define EMC0	4
#define EMC1	6
#define EMC2	8
#define EMC3	10

#define MATCH0	(1<<0)
#define MATCH1	(1<<1)
#define MATCH2	(1<<2)
#define MATCH3	(1<<3)

#ifndef TIME_INTERVAL
#define TIME_INTERVAL(f)	((uint32_t)(SystemCoreClock/(f)) - 1)
/* depending on the SystemFrequency and SystemAHBFrequency setting, 
if SystemFrequency = 60Mhz, SystemAHBFrequency = 1/4 SystemAHBFrequency, 
10mSec = 150.000-1 counts */
#endif

#ifdef __cplusplus
extern "C" {
#endif

//extern void TIMER32_1_IRQHandler(void);
	
void delay32Ms(uint8_t timer_num, uint32_t delayInMs);
//void TIMER32_0_IRQHandler(void);
//void TIMER32_1_IRQHandler(void);
void enable_timer32(uint8_t timer_num);
void disable_timer32(uint8_t timer_num);
void reset_timer32(uint8_t timer_num);
void init_timer32(uint8_t timer_num, uint32_t timerInterval);
void init_timer32PWM(uint8_t timer_num, uint32_t period, uint8_t match_enable);
void setMatch_timer32PWM (uint8_t timer_num, uint8_t match_nr, uint32_t value);
	
#ifdef __cplusplus
}
#endif

#endif /* end __TIMER32_H */
/*****************************************************************************
**                            End Of File
******************************************************************************/
