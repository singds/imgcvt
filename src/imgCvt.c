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

//____________________________________________________________INCLUDES - DEFINES
#include "imgCvt.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <getopt.h>
#include "lodepng/lodepng.h"

#define L_PRINT_GEN_ERR                                fprintf (stderr, "ERROR ON %s:%d\n", __FILE__, __LINE__)
#define L_NELEMENTS(array)                             (sizeof (array) / sizeof (array[0]))

typedef void (*FuncWritePxl_t) (FILE *f, uint8_t *inClr);
typedef void (*FuncTraversePixel_t) (FILE *f, char *img, uint32_t w, uint32_t h, FuncWritePxl_t wrPxl);

//____________________________________________________________PRIVATE PROTOTYPES
static void PrintHelp (void);
static void Convert (void);
static void GetBeInt32t (uint8_t *leVal, int32_t val);

static void WriteClrARGB8888 (FILE *f, uint8_t *inClr);
static void WriteClrBGRA8888 (FILE *f, uint8_t *inClr);
static void WriteClrRGB565LE (FILE *f, uint8_t *inClr);
static void WriteClrRGB565BE (FILE *f, uint8_t *inClr);
static void WriteClrARGB565LE (FILE *f, uint8_t *inClr);
static void WriteClrARGB565BE (FILE *f, uint8_t *inClr);


static void TraversePixelOri0   (FILE *f, char *img, uint32_t w, uint32_t h, FuncWritePxl_t wrPxl);
static void TraversePixelOri90  (FILE *f, char *img, uint32_t w, uint32_t h, FuncWritePxl_t wrPxl);
static void TraversePixelOri180 (FILE *f, char *img, uint32_t w, uint32_t h, FuncWritePxl_t wrPxl);
static void TraversePixelOri270 (FILE *f, char *img, uint32_t w, uint32_t h, FuncWritePxl_t wrPxl);

//___________________________________________________________________PRIVATE VAR
/* image file path */
static char *ArgIn_FnameImg = NULL;
/* output file path */
static char *ArgIn_FnameOut = NULL;
/* output color format */
static int8_t ArgIn_ClrFomat = IMGCVT_CLR_FORMAT_ARGB8888;
/* output pxl orientation */
static int8_t ArgIn_Ori = IMGCVT_ORI_0;

/* pixel write function (default RGBA8888 output) */
FuncWritePxl_t WritePxl = WriteClrARGB8888;

struct
{
    const char *name; // color format string name
    FuncWritePxl_t func_write;
} PxlFormatTable[] =
{
    [IMGCVT_CLR_FORMAT_ARGB8888] =  { "argb8888", WriteClrARGB8888 },
    [IMGCVT_CLR_FORMAT_BGRA8888] =  { "bgra8888", WriteClrBGRA8888 },
    [IMGCVT_CLR_FORMAT_RGB565LE] =  { "rgb565le", WriteClrRGB565LE },
    [IMGCVT_CLR_FORMAT_RGB565BE] =  { "rgb565be", WriteClrRGB565BE },
    [IMGCVT_CLR_FORMAT_ARGB565LE] = { "argb565le", WriteClrARGB565LE },
    [IMGCVT_CLR_FORMAT_ARGB565BE] = { "argb565be", WriteClrARGB565BE },
};

/* pixel traversal function (default rotation 0) */
FuncTraversePixel_t TraversePixel = TraversePixelOri0;
FuncTraversePixel_t TraversePixelTable[] =
{
    [IMGCVT_ORI_0] =   TraversePixelOri0,
    [IMGCVT_ORI_90] =  TraversePixelOri90,
    [IMGCVT_ORI_180] = TraversePixelOri180,
    [IMGCVT_ORI_270] = TraversePixelOri270,
};

//____________________________________________________________________GLOBAL VAR

//______________________________________________________________GLOBAL FUNCTIONS

//_____________________________________________________________PRIVATE FUNCTIONS

/* Application entry point.
    Args:
- <argc>[in] command line argument's number.
- <argv>[in] command line argument's list.
    Ret:
0 on success.
*/
int main (int argc, char *argv[])
{
    int c; /* option identifier character */
    /* flag meaning all provided arguments are ok */
    bool argsOk = true;

    /* parse command line options */
    while ((c = getopt (argc, argv, ":o:f:r:h")) != -1)
    {
        switch (c)
        {
            /* print the help */
            case 'h':
            {
                PrintHelp ( );
                return 0;
            }

            /* output destination */
            case 'o':
            {
                ArgIn_FnameOut = optarg;
                break;
            }

            /* specify output format */
            case 'f':
            {
                ArgIn_ClrFomat = -1;
                for (int i = 0; i < L_NELEMENTS (PxlFormatTable); i++)
                {
                    if (strcmp (optarg, PxlFormatTable[i].name) == 0)
                        ArgIn_ClrFomat = i;
                }
                
                if (ArgIn_ClrFomat != -1) // get the corresponding put function
                    WritePxl = PxlFormatTable[ArgIn_ClrFomat].func_write;
                else
                {
                    argsOk = false;
                    fprintf (stderr, "%s is not a valid color format\n", optarg);
                }
                break;
            }

            /* specify image rotation */
            case 'r':
            {
                int rotation;

                ArgIn_Ori = -1;
                rotation = atoi (optarg);
                if (rotation == 0)
                    ArgIn_Ori = IMGCVT_ORI_0;
                else if (rotation == 90)
                    ArgIn_Ori = IMGCVT_ORI_90;
                else if (rotation == 180)
                    ArgIn_Ori = IMGCVT_ORI_180;
                else if (rotation == 270)
                    ArgIn_Ori = IMGCVT_ORI_270;
                
                if (ArgIn_Ori != -1) // get the corresponding pixel traversal function
                    TraversePixel = TraversePixelTable[ArgIn_Ori];
                else
                {
                    argsOk = false;
                    fprintf (stderr, "%s is not a valid rotation\n", optarg);
                }
                break;
            }

            /* missing option argument */
            case ':':
            {
                argsOk = false;
                fprintf (stderr, "missing option argument for -%c option\n", optopt);
                break;
            }
            
            /* unknown option */
            default: /* '?' */
            {
                argsOk = false;
                fprintf (stderr, "-%c is not a valid option\n", optopt);
                break;
            }
        }
    }

    /* the user most provide the input image file */
    if (optind == argc -1)
    {   /* we get the input image file name */
        ArgIn_FnameImg = argv[optind];
    }
    else
    {   /* the user provided not even one or more than one input file name */
        argsOk = false;
        fprintf (stderr, "you must porvide at least one and only one input image file\n");
    }

    if (ArgIn_FnameOut == NULL)
    {
        argsOk = false;
        fprintf (stderr, "-o with specified output destination is mandatory\n");
    }

    if (argsOk)
        Convert ( );
}

//_____________________________________________________________PRIVATE FUNCTIONS
/* Print help using the command line arguments.
    Args:
    Ret:
*/
static void PrintHelp (void)
{
    printf ("\n");
    printf ("\
imgcvt use:\n\
    fontcvt [OPTIONS] ... IMAGE_FILE -o OUTPUT_NAME\n");
    printf ("\n");
    printf ("\
-f) Output color format. (default argb8888).\n");
    printf ("\
-r) Output image rotation. (default 0). Valid options are:\n\
    (  0) ( 90) (180) (270)\n");
    printf ("\
-o) Specify the output filename. (mandatory)\n");
    printf ("\
-h) Print this help and exit.\n");
}

/* Main program function, called after all input oprions are parsed.
    Args:
    Ret:
*/
static void Convert (void)
{
    uint32_t error;
    uint8_t* image = 0;
    int32_t width, height;

    error = lodepng_decode32_file(&image, &width, &height, ArgIn_FnameImg);
    if(error)
        printf("error %u: %s\n", error, lodepng_error_text(error));
    else
    {
        /*use image here*/
        FILE *f = fopen (ArgIn_FnameOut, "wb");
        if (f == NULL)
            printf ("i can't open the output file\n");
        else
        {
            uint8_t beWidth[4]; // big endian width
            uint8_t beHeigth[4]; // big endian height
            uint8_t bePxlOffset[4]; // big pixel offset
            uint8_t clrFormat; // output image color format
            uint8_t ori; // output image orientation

            clrFormat = ArgIn_ClrFomat;
            ori = ArgIn_Ori;
            GetBeInt32t (beWidth, width);
            GetBeInt32t (beHeigth, height);
            GetBeInt32t (bePxlOffset, 32);

            /* print simple file header */
            fprintf (f, "RAW");
            fprintf (f, "v01");
            fwrite (&ori, 1, sizeof (ori), f);
            fwrite (&clrFormat, 1, sizeof (clrFormat), f);
            fwrite (beWidth, 1, sizeof (beWidth), f); // width
            fwrite (beHeigth, 1, sizeof (beHeigth), f); // height
            fwrite (bePxlOffset, 1, sizeof (bePxlOffset), f); // height
            for (int i = 0; i < (32 - 20); i++)
                fprintf (f, "-");

            /* print image pixels */
            TraversePixel (f, image, width, height, WritePxl);
            fclose (f);
        }
    }

    free(image);
}

/* Get big endian 32bit number rappresentation.
    Args: <leVal>[out] little endian number rappresentation.
          <val>[in] value to get le rappresentation.
    Ret:
*/
static void GetBeInt32t (uint8_t *leVal, int32_t val)
{
    leVal[3] = (val >>  0) & 0xff;
    leVal[2] = (val >>  8) & 0xff;
    leVal[1] = (val >> 16) & 0xff;
    leVal[0] = (val >> 24) & 0xff;
}

/* Write all image pixel to file.
    Args: <f>[in] append all pixel to the file.
          <img>[in] RGBA8888 pixel map.
          <w>[in] image width.
          <h>[in] image height.
          <wrPxl>[in] function used to write pixel in file.
    Ret:
*/
static void TraversePixelOri0 (FILE *f, char *img, uint32_t w, uint32_t h, FuncWritePxl_t wrPxl)
{
    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            uint8_t *inClr;

            inClr = &img[(x + y * w) * 4];
            wrPxl (f, inClr);
        }
    }
}

/* Write all image pixel to file.
    Args: <f>[in] append all pixel to the file.
          <img>[in] RGBA8888 pixel map.
          <w>[in] image width.
          <h>[in] image height.
          <wrPxl>[in] function used to write pixel in file.
    Ret:
*/
static void TraversePixelOri90 (FILE *f, char *img, uint32_t w, uint32_t h, FuncWritePxl_t wrPxl)
{
    for (int x = w - 1; x >= 0; x--)
    {
        for (int y = 0; y < h; y++)
        {
            uint8_t *inClr;

            inClr = &img[(x + y * w) * 4];
            wrPxl (f, inClr);
        }
    }
}

/* Write all image pixel to file.
    Args: <f>[in] append all pixel to the file.
          <img>[in] RGBA8888 pixel map.
          <w>[in] image width.
          <h>[in] image height.
          <wrPxl>[in] function used to write pixel in file.
    Ret:
*/
static void TraversePixelOri180 (FILE *f, char *img, uint32_t w, uint32_t h, FuncWritePxl_t wrPxl)
{
    for (int y = h - 1; y >= 0; y--)
    {
        for (int x = w - 1; x >= 0; x--)
        {
            uint8_t *inClr;

            inClr = &img[(x + y * w) * 4];
            wrPxl (f, inClr);
        }
    }
}

/* Write all image pixel to file.
    Args: <f>[in] append all pixel to the file.
          <img>[in] RGBA8888 pixel map.
          <w>[in] image width.
          <h>[in] image height.
          <wrPxl>[in] function used to write pixel in file.
    Ret:
*/
static void TraversePixelOri270 (FILE *f, char *img, uint32_t w, uint32_t h, FuncWritePxl_t wrPxl)
{
    for (int x = 0; x < w; x++)
    {
        for (int y = h - 1; y >= 0; y--)
        {
            uint8_t *inClr;

            inClr = &img[(x + y * w) * 4];
            wrPxl (f, inClr);
        }
    }
}

/* Add pixel to file.
    Args: <f>[in] append pxl to this file.
          <inClr>[in] RGBA8888 input color.
    Ret:
*/
static void WriteClrARGB8888 (FILE *f, uint8_t *inClr)
{
    uint8_t outClr[4];

    outClr[0] = inClr[3];
    outClr[1] = inClr[0];
    outClr[2] = inClr[1];
    outClr[3] = inClr[2];
    fwrite (&outClr, 1, sizeof (outClr), f);
}

/* Add pixel to file.
    Args: <f>[in] append pxl to this file.
          <inClr>[in] RGBA8888 input color.
    Ret:
*/
static void WriteClrBGRA8888 (FILE *f, uint8_t *inClr)
{
    uint8_t outClr[4];

    outClr[0] = inClr[2];
    outClr[1] = inClr[1];
    outClr[2] = inClr[0];
    outClr[3] = inClr[3];
    fwrite (&outClr, 1, sizeof (outClr), f);
}

/* Add pixel to file.
    Args: <f>[in] append pxl to this file.
          <inClr>[in] RGBA8888 input color.
    Ret:
*/
static void WriteClrRGB565LE (FILE *f, uint8_t *inClr)
{
    uint8_t outClr[2];
    uint16_t color; // rgb565 color

    color =  (inClr[0] >> 3) << (6 + 5);
    color += (inClr[1] >> 2) << (5);
    color += (inClr[2] >> 3) << (0);

    outClr[0] = (color) & 0xff;
    outClr[1] = (color >> 8) & 0xff;
    fwrite (&outClr, 1, sizeof (outClr), f);
}

/* Add pixel to file.
    Args: <f>[in] append pxl to this file.
          <inClr>[in] RGBA8888 input color.
    Ret:
*/
static void WriteClrRGB565BE (FILE *f, uint8_t *inClr)
{
    uint8_t outClr[2];
    uint16_t color; // rgb565 color

    color =  (inClr[0] >> 3) << (6 + 5);
    color += (inClr[1] >> 2) << (5);
    color += (inClr[2] >> 3) << (0);

    outClr[0] = (color >> 8) & 0xff;
    outClr[1] = (color) & 0xff;
    fwrite (&outClr, 1, sizeof (outClr), f);
}

/* Add pixel to file.
    Args: <f>[in] append pxl to this file.
          <inClr>[in] RGBA8888 input color.
    Ret:
*/
static void WriteClrARGB565LE (FILE *f, uint8_t *inClr)
{
    uint8_t outClr[3];
    uint16_t color; // rgb565 color

    color =  (inClr[0] >> 3) << (6 + 5);
    color += (inClr[1] >> 2) << (5);
    color += (inClr[2] >> 3) << (0);

    outClr[0] = inClr[3];
    outClr[1] = (color) & 0xff;
    outClr[2] = (color >> 8) & 0xff;
    fwrite (&outClr, 1, sizeof (outClr), f);
}

/* Add pixel to file.
    Args: <f>[in] append pxl to this file.
          <inClr>[in] RGBA8888 input color.
    Ret:
*/
static void WriteClrARGB565BE (FILE *f, uint8_t *inClr)
{
    uint8_t outClr[3];
    uint16_t color; // rgb565 color

    color =  (inClr[0] >> 3) << (6 + 5);
    color += (inClr[1] >> 2) << (5);
    color += (inClr[2] >> 3) << (0);

    outClr[0] = inClr[3];
    outClr[1] = (color >> 8) & 0xff;
    outClr[2] = (color) & 0xff;
    fwrite (&outClr, 1, sizeof (outClr), f);
}
