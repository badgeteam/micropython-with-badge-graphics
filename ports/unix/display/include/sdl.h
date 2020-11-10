#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
extern void sdlInit();
extern void sdlFlush(const uint8_t* buffer, uint16_t x0, uint16_t x1, uint16_t y0, uint16_t y1);
#ifdef __cplusplus
}
#endif
