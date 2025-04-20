#ifndef __GPIO_INTERRUPT_H
#define __GPIO_INTERRUPT_H

#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include "math.h"
#include "esp_err.h"
#include "esp_log.h"

void gpio_isr_init(void);
bool gpio_key_isr(void);

#endif