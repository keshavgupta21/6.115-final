#include "project.h"
#include "chip8.h"

uint8_t divide_count = 0;

void isr_tmr_handler(uint8_t* snd, uint8_t* tmr){
    divide_count = (divide_count == 0) ? 100 : (divide_count - 1);
    if (divide_count == 0){
        *snd = ((*snd) == 0) ? 0 : ((*snd) - 1);
        *tmr = ((*tmr) == 0) ? 0 : ((*tmr) - 1);
        if (*snd) {
            pin_snd_Write(1);
        } else {
            pin_snd_Write(0);
        }
    }
}