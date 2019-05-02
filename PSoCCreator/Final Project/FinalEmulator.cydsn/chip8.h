#include "project.h"

void newline_handler(uint8_t* dma_chan, uint8_t* dma_td,
       volatile uint8_t* vga_update_flag, uint64_t vram[]);

void isr_tmr_handler(uint8_t* snd, uint8_t* tmr);

void execute(uint8_t ram[], uint64_t vram[32], uint8_t* snd, 
                        uint8_t* tmr, volatile uint8_t* vga_update_flag);

uint8_t key_pressed(uint8_t key);
uint8_t wait_key();

void oled_initialize();

