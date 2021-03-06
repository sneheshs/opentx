/****************************************************************************
 *  Copyright (c) 2014 by Michael Blandford. All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the author nor the names of its contributors may
 *     be used to endorse or promote products derived from this software
 *     without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 *  THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 *  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 *  THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 *  SUCH DAMAGE.
 *
 ****************************************************************************
 * Other Authors:
 * - Andre Bernet
 * - Bertrand Songis
 * - Bryan J. Rentoul (Gruvin)
 * - Cameron Weeks
 * - Erez Raviv
 * - Jean-Pierre Parisy
 * - Karl Szmutny
 * - Michal Hlavinka
 * - Pat Mackenzie
 * - Philip Moss
 * - Rob Thomson
 * - Romolo Manfredini
 * - Thomas Husterer
 *
 ****************************************************************************/

#if !defined(SIMU)

#include <stdint.h>

#if defined(PCBSKY9X)
  #include "AT91SAM3S4.h"
#else
  #include "board_taranis.h"
#endif

#if defined(PCBTARANIS)
void bwdt_reset()
{
  IWDG->KR = 0xAAAA;		// reload
}
#endif

#if defined(PCBTARANIS)
// TODO needed?
__attribute__ ((section(".bootrodata"), used))
void _bootStart(void);
#endif

#if defined(PCBTARANIS)
__attribute__ ((section(".isr_boot_vector"), used))
const uint32_t BootVectors[] = {
  (uint32_t) &_estack,
  (uint32_t) (void (*)(void)) ((unsigned long) &_bootStart) };
#endif

#if defined(PCBTARANIS)
__attribute__ ((section(".bootrodata.*"), used))
#elif defined(PCBSKY9X)
__attribute__ ((section(".bootrodata"), used))
#endif

const uint8_t BootCode[] = {
#include "bootloader.lbm"
};

#if defined(PCBTARANIS)
__attribute__ ((section(".bootrodata"), used))
void _bootStart()
{
  RCC_AHB1PeriphClockCmd(PWR_RCC_AHB1Periph | KEYS_RCC_AHB1Periph | LCD_RCC_AHB1Periph | BACKLIGHT_RCC_AHB1Periph | I2C_RCC_AHB1Periph | SD_RCC_AHB1Periph, ENABLE);

  // turn soft power ON now
  GPIOD->BSRRL = 1;
  GPIOD->MODER = (GPIOD->MODER & 0xFFFFFFFC) | 1;

  GPIOC->PUPDR = 0x00000004;
  GPIOE->PUPDR = 0x00000040;

  uint32_t i;
  for (i = 0; i < 50000; i += 1) {
    bwdt_reset();
  }

  if (!(TRIMS_GPIO_REG_LHR & TRIMS_GPIO_PIN_LHR) && !(TRIMS_GPIO_REG_RHL & TRIMS_GPIO_PIN_RHL)) {
    // Bootloader needed
    const uint8_t *src;
    uint8_t *dest;
    uint32_t size;

    bwdt_reset();
    size = sizeof(BootCode);
    src = BootCode;
    dest = (uint8_t *) 0x20000000;

    for (; size; size -= 1) {
      *dest++ = *src++;
    }
    // Could check for a valid copy to RAM here
    // Go execute bootloader
    bwdt_reset();

    uint32_t address = *(uint32_t *) 0x20000004;

    ((void (*)(void)) (address))();		// Go execute the loaded application
  }

// run_application() ;
  asm(" mov.w	r1, #134217728");
  // 0x8000000
  asm(" add.w	r1, #32768");
  // 0x8000

  asm(" movw	r0, #60680");
  // 0xED08
  asm(" movt	r0, #57344");
  // 0xE000
  asm(" str	r1, [r0, #0]");
  // Set the VTOR

  asm("ldr	r0, [r1, #0]");
  // Stack pointer value
  asm("msr msp, r0");
  // Set it
  asm("ldr	r0, [r1, #4]");
  // Reset address
  asm("mov.w	r1, #1");
  asm("orr		r0, r1");
  // Set lsbit
  asm("bx r0");
  // Execute application
}
#endif

#endif
