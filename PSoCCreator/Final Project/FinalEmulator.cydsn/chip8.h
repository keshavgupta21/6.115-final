#include "project.h"
#include "u8g2.h"

CY_ISR_PROTO(newline_handler);

CY_ISR_PROTO(isr_oled_handler);

CY_ISR_PROTO(isr_tmr_handler);

void execute(uint8_t ram[]);

uint8_t key_pressed(uint8_t key);
uint8_t wait_key();

void oled_initialize(u8g2_t *u8g2);
