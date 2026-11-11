#ifndef ENCODER
#define ENCODER

#include "rotaryDecoder8.h"

void mux_encoder_setup();
bool mux_encoder_update();

int32_t mux_encoder_get_values(uint8_t i);

#endif
