/* This file specifies the framebuffer configuration for the displays that are supported. */
/* The order in this file determines priority if multiple drivers are enabled */

#pragma once

#include "sdl.h"

#ifdef CONFIG_DRIVER_FRAMEBUFFER_ENABLE
    #define FB_WIDTH 800
    #define FB_HEIGHT 600
    #define FB_SIZE FB_WIDTH * FB_HEIGHT * 3
    #define FB_TYPE_24BPP
    #define PIXEL_SIZE 24
    #define FB_ALPHA_ENABLED
    #define FB_FLUSH(buffer,eink_flags,x0,y0,x1,y1) sdlFlush(buffer, x0, y0, x1, y1)
    #define COLOR_FILL_DEFAULT 0xFFFFFF
    #define COLOR_TEXT_DEFAULT 0x000000
#endif
