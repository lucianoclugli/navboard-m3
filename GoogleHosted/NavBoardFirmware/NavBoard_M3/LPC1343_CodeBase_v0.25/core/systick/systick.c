/**************************************************************************/
/*! 
    @file     systick.c
    @author   K. Townsend (microBuilder.eu) modified by M. Ryan (ryanmechatronics.com)
    @date     02 Sept 2010
    @version  0.10


    @section DESCRIPTION

    Controls the 24-bit 'system tick' clock, which can be used as a
    generic timer or to control time sharing with an embedded real-time
    operating system (such as FreeRTOS).

    Original file developed by microbuilder.eu

    @section Example

    @code 
    #include "core/cpu/cpu.h"
    #include "core/systick/systick.h"

    void main (void)
    {
      cpuInit();

      // Start systick timer with one tick every 10ms
      systickInit(10);

      while(1)
      {
      }
    }
    @endcode

    @section LICENSE

    Software License Agreement (BSD License)

    Copyright (c) 2010, microBuilder SARL
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    3. Neither the name of the copyright holders nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/**************************************************************************/

#include "systick.h"

volatile uint32_t msTicks;             // 1ms tick counter

void SysTick_Handler (void)
{
  msTicks++;
}

uint32_t SysTick_Get (void)
{
  return msTicks;
}

uint32_t SysTick_DeltaTime_ms( uint32_t StartTime)
{
  //Computes and returns difference in sys tick counts.  Assumes 1 ms counter

  uint32_t returnedMS;
  
  returnedMS = (msTicks - StartTime);

  if (returnedMS > 0x00FFFFFF) //Greater than the 24 bit holder, so we wrapped
  {
    returnedMS += 0x00FFFFFF;  

  }

  return (returnedMS);
}

static uint32_t systickConfig(uint32_t ticks)
{ 
  // Check if 'ticks' is greater than maximum value
  if (ticks > SYSTICK_STRELOAD_MASK)
  {
    return (1);
  }
                     
  // Set reload register
  SYSTICK_STRELOAD  = (ticks & SYSTICK_STRELOAD_MASK) - 1;

  // Load the systick counter value
  SYSTICK_STCURR = 0;

  // Enable systick IRQ and timer
  SYSTICK_STCTRL = SYSTICK_STCTRL_CLKSOURCE |
                   SYSTICK_STCTRL_TICKINT |
                   SYSTICK_STCTRL_ENABLE;

  return (0);
}

/**************************************************************************/
/*! 
    @brief      Initialises the systick timer

    @param[in]  delayMs
                The number of milliseconds between each tick of the systick
                timer.
                  
    @note       The shortest possible delay is 1 millisecond, which will 
                allow fine grained delays, but will cause more load on the
                system than a 10mS delay.  The resolution of the systick
                timer needs to be balanced with the amount of processing
                time you can spare.  The delay should really only be set
                to 1 mS if you genuinely have a need for 1mS delays,
                otherwise a higher value like 5 or 10 mS is probably
                more appropriate.
*/
/**************************************************************************/
void systickInit (uint32_t delayMs)
{
  systickConfig ((CFG_CPU_CCLK / 1000) * delayMs);
}

/**************************************************************************/
/*! 
    @brief      Causes a blocking delay for 'delayTicks' ticks on the
                systick timer.  For example: systickDelay(100) would cause
                a blocking delay for 100 ticks of the systick timer.

    @param[in]  delayTicks
                The number of systick ticks to cause a blocking delay for

    @Note       This function takes into account the fact that the tick
                counter may eventually roll over to 0 once it reaches
                0xFFFFFFFF.
*/
/**************************************************************************/
void systickDelay (uint32_t delayTicks) 
{
  uint32_t curTicks;
  curTicks = msTicks;

  // Make sure delay is at least 1 tick in case of division, etc.
  if (delayTicks == 0) delayTicks = 1;

  if (curTicks > 0xFFFFFFFF - delayTicks)
  {
    // Rollover will occur during delay
    while (msTicks >= curTicks)
    {
      while (msTicks < (delayTicks - (0xFFFFFFFF - curTicks)));
    }      
  }
  else
  {
    while ((msTicks - curTicks) < delayTicks);
  }
}
