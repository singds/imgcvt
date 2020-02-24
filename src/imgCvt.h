/*
MIT License

Copyright (c) 2020 singds

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef IMGCVT_H_INCLUDED
#define IMGCVT_H_INCLUDED

#include <stdint.h>

enum
{
    IMGCVT_CLR_FORMAT_ARGB8888,
    IMGCVT_CLR_FORMAT_BGRA8888,
    IMGCVT_CLR_FORMAT_RGB565LE, // little endian
    IMGCVT_CLR_FORMAT_RGB565BE, // big endian
    IMGCVT_CLR_FORMAT_ARGB565LE, // little endian
    IMGCVT_CLR_FORMAT_ARGB565BE, // big endian
};

enum
{
    IMGCVT_ORI_0,
    IMGCVT_ORI_90,
    IMGCVT_ORI_180,
    IMGCVT_ORI_270,
};

typedef struct
{   // all number are stored in BIG ENDIAN
    char magic[3]; // magic identifier (always RAW)
    char version[3]; // version vXX style
    uint8_t orientation; // pixel map orientation
    uint8_t color_format; // pixel color format
    uint32_t width;
    uint32_t height;
    uint32_t pxl_offset;
} imgcvt_Header_t;

#endif // IMGCVT_H_INCLUDED
