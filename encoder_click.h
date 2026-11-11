#ifndef ENCODER_CLICK
#define ENCODER_CLICK

#include "rotaryDecoder8.h"

void encoder_click_setup();

bool encoder_click_update();

bool is_step_active(uint8_t i);

bool is_bpm_active();

int32_t get_bpm_value();

#endif
