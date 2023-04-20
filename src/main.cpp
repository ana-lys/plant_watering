/*
 * This file is part of the µOS++ distribution.
 *   (https://github.com/micro-os-plus)
 * Copyright (c) 2014 Liviu Ionescu.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include "diag/trace.h"

#include "timer.h"
#include "led.h"

// ----------------------------------------------------------------------------
//
// Standalone STM32F4 led blink sample (trace via DEBUG).
//
// In debug configurations, demonstrate how to print a greeting message
// on the trace device. In release configurations the message is
// simply discarded.
//
// Then demonstrates how to blink a led with 1 Hz, using a
// continuous loop and SysTick delays.
//
// Trace support is enabled by adding the TRACE macro definition.
// By default the trace messages are forwarded to the DEBUG output,
// but can be rerouted to any device or completely suppressed, by
// changing the definitions required in system/src/diag/trace-impl.c
// (currently OS_USE_TRACE_ITM, OS_USE_TRACE_SEMIHOSTING_DEBUG/_STDOUT).
//

// Definitions visible only within this translation unit.
namespace
{
  // ----- Timing definitions -------------------------------------------------

  // Keep the LED on for 2/3 of a second.
  constexpr timer::ticks_t BLINK_ON_TICKS = timer::FREQUENCY_HZ * 3 / 4;
  constexpr timer::ticks_t BLINK_OFF_TICKS = timer::FREQUENCY_HZ
      - BLINK_ON_TICKS;
}

// ----- LED definitions ------------------------------------------------------

#if defined(STM32F401xE)

#warning "Assume a NUCLEO-F401RE board, PA5, active high."

// PA5
#define BLINK_PORT_NUMBER         (0)
#define BLINK_PIN_NUMBER          (5)
#define BLINK_ACTIVE_LOW          (false)

led blinkLeds[1] =
  {
    { BLINK_PORT_NUMBER, BLINK_PIN_NUMBER, BLINK_ACTIVE_LOW },
  };

#elif defined(STM32F407xx)

#warning "Assume a STM32F4-Discovery board, PD12-PD15, active high."

#define BLINK_PORT_NUMBER         (3)
#define BLINK_PIN_NUMBER_GREEN    (12)
#define BLINK_PIN_NUMBER_ORANGE   (13)
#define BLINK_PIN_NUMBER_RED      (14)
#define BLINK_PIN_NUMBER_BLUE     (15)
#define BLINK_ACTIVE_LOW          (false)

led blinkLeds[4] =
  {
    { BLINK_PORT_NUMBER, BLINK_PIN_NUMBER_GREEN, BLINK_ACTIVE_LOW },
    { BLINK_PORT_NUMBER, BLINK_PIN_NUMBER_ORANGE, BLINK_ACTIVE_LOW },
    { BLINK_PORT_NUMBER, BLINK_PIN_NUMBER_RED, BLINK_ACTIVE_LOW },
    { BLINK_PORT_NUMBER, BLINK_PIN_NUMBER_BLUE, BLINK_ACTIVE_LOW },
  };

#elif defined(STM32F411xE)

#warning "Assume a NUCLEO-F411RE board, PA5, active high."

#define BLINK_PORT_NUMBER         (0)
#define BLINK_PIN_NUMBER          (5)
#define BLINK_ACTIVE_LOW          (false)

led blinkLeds[1] =
  {
    { BLINK_PORT_NUMBER, BLINK_PIN_NUMBER, BLINK_ACTIVE_LOW },
  };

#elif defined(STM32F429xx)

#warning "Assume a STM32F429I-Discovery board, PG13-PG14, active high."

#define BLINK_PORT_NUMBER         (6)
#define BLINK_PIN_NUMBER_GREEN    (13)
#define BLINK_PIN_NUMBER_RED      (14)
#define BLINK_ACTIVE_LOW          (false)

led blinkLeds[2] =
  {
    { BLINK_PORT_NUMBER, BLINK_PIN_NUMBER_GREEN, BLINK_ACTIVE_LOW },
    { BLINK_PORT_NUMBER, BLINK_PIN_NUMBER_RED, BLINK_ACTIVE_LOW },
  };

#else

#warning "Unknown board, assume PA5, active high."

#define BLINK_PORT_NUMBER         (0)
#define BLINK_PIN_NUMBER          (5)
#define BLINK_ACTIVE_LOW          (false)

led blinkLeds[1] =
  {
    { BLINK_PORT_NUMBER, BLINK_PIN_NUMBER, BLINK_ACTIVE_LOW },
  };

#endif

// ----- main() ---------------------------------------------------------------

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

uint32_t water_level = 50;

void configure_Button(void)
{
	__HAL_RCC_GPIOA_CLK_ENABLE(); //Enable clock to GPIOA
	GPIO_InitTypeDef PushButton;  // declare a variable of type struct GPIO_InitTypeDef
	PushButton.Mode = GPIO_MODE_INPUT; // set pin mode to input
	PushButton.Pin = GPIO_PIN_0;  // select pin PA0 only
	PushButton.Pull = GPIO_NOPULL; // set no internal pull-up or pull-down resistor
	HAL_GPIO_Init(GPIOA, &PushButton); //  initialize PA0 pins by passing port name and address of PushButton struct
}

void sensor_daily_update(void){
	int rand_int = rand() % 25;
    water_level -= rand_int;
}

void motor_activate(void){
	water_level++;
}

void motor_stop(void){

}

int main(int argc, char* argv[])
{
  // Send a greeting to the trace device (skipped on Release).
  trace_puts("Plant Watering SYSTEM!");

  // At this stage the system clock should have already been configured
  // at high speed.
  trace_printf("System clock: %u Hz\n", SystemCoreClock);
  timer timer;
  timer.start ();

  // Perform all necessary initialisations for the LEDs.
  for (size_t i = 0; i < (sizeof(blinkLeds) / sizeof(blinkLeds[0])); ++i)
    {
      blinkLeds[i].power_up ();
    }

  uint32_t minute = 0;
  configure_Button(); // call Push button GPIO pins initialization function

     while(1)
     {	 minute++;
         if(minute % 1440 == 0){
         sensor_daily_update();
         trace_printf("Water level is at : %u \n",water_level);
         }

    	 timer.sleep (1);
    	  if(water_level < 50)
      	  {
    		  motor_activate();
     		  blinkLeds[0].turn_on ();
    		  blinkLeds[2].turn_off ();
    		  trace_puts("MOTOR IS ON!");
    		  trace_printf("Water level is at : %u \n",water_level);
    		  timer.sleep (120);
    		  if( water_level == 50){
    			  motor_stop();
    			  blinkLeds[0].turn_off ();
    			  blinkLeds[2].turn_on ();
    			  trace_puts("MOTOR IS OFF!");
    		  }
    	  }


     }
//  for (size_t i = 0; i < (sizeof(blinkLeds) / sizeof(blinkLeds[0])); ++i)
//    {
//      blinkLeds[i].turn_on ();
//    }
//
//  // First second is long.
//  timer.sleep (timer::FREQUENCY_HZ);
//
//  for (size_t i = 0; i < (sizeof(blinkLeds) / sizeof(blinkLeds[0])); ++i)
//    {
//      blinkLeds[i].turn_off ();
//    }
//
//  timer.sleep (BLINK_OFF_TICKS);
//
//  ++seconds;
//  trace_printf ("Second %u\n", seconds);
//
//  if ((sizeof(blinkLeds) / sizeof(blinkLeds[0])) > 1)
//    {
//      // Blink individual LEDs.
//      for (size_t i = 0; i < (sizeof(blinkLeds) / sizeof(blinkLeds[0])); ++i)
//        {
//          blinkLeds[i].turn_on ();
//          timer.sleep (BLINK_ON_TICKS);
//
//          blinkLeds[i].turn_off ();
//          timer.sleep (BLINK_OFF_TICKS);
//
//          ++seconds;
//          trace_printf ("Second %u\n", seconds);
//        }
//
//      // Blink binary.
//      while (1)
//        {
//          for (size_t l = 0; l < (sizeof(blinkLeds) / sizeof(blinkLeds[0]));
//              ++l)
//            {
//              blinkLeds[l].toggle ();
//              if (blinkLeds[l].isOn ())
//                {
//                  break;
//                }
//            }
//          timer.sleep (timer::FREQUENCY_HZ);
//
//          ++seconds;
//          trace_printf ("Second %u\n", seconds);
//        }
//      // Infinite loop, never return.
//    }
//  else
//    {
//      while (1)
//        {
//          blinkLeds[0].turn_on ();
//          timer.sleep (BLINK_ON_TICKS);
//
//          blinkLeds[0].turn_off ();
//          timer.sleep (BLINK_OFF_TICKS);
//
//          ++seconds;
//          trace_printf ("Second %u\n", seconds);
//        }
//      // Infinite loop, never return.
//    }
  return 0;
}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
