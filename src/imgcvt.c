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
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include "lodepng/lodepng.h"

#define L_PRINT_GEN_ERR                                fprintf (stderr, "ERROR ON %s:%d\n", __FILE__, __LINE__)
#define L_NELEMENTS(array)                             (sizeof (array) / sizeof (array[0]))

enum
{
    L_CLR_FORMAT_RGBA8888,
    L_CLR_FORMAT_ARGB8888,
    L_CLR_FORMAT_RGBA5658,
    L_CLR_FORMAT_ARGB8565,
};

//____________________________________________________________PRIVATE PROTOTYPES
static void PrintHelp (void);
static void Convert (void);
static void GetBeInt32t (uint8_t *leVal, int32_t val);

static void WriteClrRGBA8888 (FILE *f, uint8_t *inClr);
static void WriteClrARGB8888 (FILE *f, uint8_t *inClr);
static void WriteClrRGBA5658 (FILE *f, uint8_t *inClr);
static void WriteClrARGB8565 (FILE *f, uint8_t *inClr);

//___________________________________________________________________PRIVATE VAR
/* image file path */
static char *ArgIn_FnameImg = NULL;
/* output file path */
static char *ArgIn_FnameOut = NULL;
/* output color format */
static int8_t ArgIn_ClrFomat = L_CLR_FORMAT_RGBA8888;

/* pixel write function (default RGBA8888 output) */
static void (*WritePxl) (FILE *f, uint8_t *inClr) = WriteClrRGBA8888;

static void (*WritePxlTable[]) (FILE *f, uint8_t *) =
{
    WriteClrRGBA8888,
    WriteClrARGB8888,
    WriteClrRGBA5658,
    WriteClrARGB8565,
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
    while ((c = getopt (argc, argv, ":o:f:h")) != -1)
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
                if (strcmp (optarg, "rgba8888") == 0)
                    ArgIn_ClrFomat = L_CLR_FORMAT_RGBA8888;
                else if (strcmp (optarg, "argb8888") == 0)
                    ArgIn_ClrFomat = L_CLR_FORMAT_ARGB8888;
                else if (strcmp (optarg, "rgba5658") == 0)
                    ArgIn_ClrFomat = L_CLR_FORMAT_RGBA5658;
                else if (strcmp (optarg, "argb8565") == 0)
                    ArgIn_ClrFomat = L_CLR_FORMAT_ARGB8565;
                
                
                if (ArgIn_ClrFomat != -1) // get the corresponding put function
                    WritePxl = WritePxlTable[ArgIn_ClrFomat];
                else
                {
                    argsOk = false;
                    fprintf (stderr, "%s is not a valid color format\n", optarg);
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
-f) Output color format. (default rgba8888). Valid options are:\n\
    * rgba8888\n\
    * argb8888\n\
    * rgba5658\n\
    * argb8565\n");
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
            uint8_t clrFormat; // output image color format

            clrFormat = ArgIn_ClrFomat;
            GetBeInt32t (beWidth, width);
            GetBeInt32t (beHeigth, height);

            /* print simple file header */
            fprintf (f, "RAWv01");
            fwrite (&clrFormat, 1, sizeof (clrFormat), f); // color format
            fwrite (beWidth, 1, sizeof (beWidth), f); // width
            fwrite (beHeigth, 1, sizeof (beHeigth), f); // height

            /* print image pixels */
            for (int y = 0; y < height; y++)
            {
                for (int x = 0; x < width; x++)
                {
                    uint8_t *inClr;

                    inClr = &image[(x + y * width) * 4];
                    WritePxl (f, inClr);
                }
            }
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

/* Add pixel to file.
    Args: <f>[in] append pxl to this file.
          <inClr>[in] RGBA8888 input color.
    Ret:
*/
static void WriteClrRGBA8888 (FILE *f, uint8_t *inClr)
{
    uint8_t outClr[4];

    outClr[0] = inClr[0];
    outClr[1] = inClr[1];
    outClr[2] = inClr[2];
    outClr[3] = inClr[3];
    fwrite (&outClr, 1, sizeof (outClr), f);
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
static void WriteClrRGBA5658 (FILE *f, uint8_t *inClr)
{
    uint8_t outClr[3];
    uint16_t color; // rgb565 color

    color =  (inClr[0] >> 3) << (6 + 5);
    color += (inClr[1] >> 2) << (5);
    color += (inClr[2] >> 3) << (0);

    outClr[0] = (color >> 8) & 0xff;
    outClr[1] = (color) & 0xff;
    outClr[2] = inClr[3];
    fwrite (&outClr, 1, sizeof (outClr), f);
}

/* Add pixel to file.
    Args: <f>[in] append pxl to this file.
          <inClr>[in] RGBA8888 input color.
    Ret:
*/
static void WriteClrARGB8565 (FILE *f, uint8_t *inClr)
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
