#pragma once
#include <Arduino.h>

void draw_character( byte destx, byte desty, byte character, uint16_t color );
void draw_string( byte destx, byte desty, uint16_t color, const char *string );
void draw_string( byte destx, byte desty, byte r, byte g, byte b, const char *string );
