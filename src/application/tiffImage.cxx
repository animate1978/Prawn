
#include <fltk/run.h>
#include <fltk/error.h>
#include <fltk/string.h>

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "tiffImage.h"
#include <tiffio.h>

using namespace fltk;
//
// 'tiffImage::tiffImage()' - Load a PNM image...
//
bool tiffImage::fetch()
{

  // turn off Warnings (too many specific tags that confuse libtiff)
  TIFFErrorHandler warn = TIFFSetWarningHandler(0);

  // open file
  TIFF* tiff = TIFFOpen(name, "r");

  // turn warnings back on
  TIFFSetWarningHandler(warn);

  if (!tiff) return false;

  // get bitmap information
  uint32 w, h;
  uint16 orient;

  TIFFGetField(tiff, TIFFTAG_IMAGEWIDTH, &w);
  TIFFGetField(tiff, TIFFTAG_IMAGELENGTH, &h);

  TIFFGetField(tiff, TIFFTAG_ORIENTATION, &orient);

  // load bitmap
  uint32* raster = (uint32*) _TIFFmalloc(w * h * sizeof (uint32));
  if (!raster) return false;

  if (!TIFFReadRGBAImageOriented(tiff, w, h, raster, 0,0))
  {
    _TIFFfree (raster);
    return false;
  }

  TIFFClose (tiff);


  // init SharedImage structure (allocate memory)
  setsize(w, h);
  //setpixeltype(MONO);
  setpixeltype(RGB);


  // store the image
  unsigned char* image = (unsigned char*)raster;
  for (int y = 0; y < h; y ++) {
    uchar* linebuf = linebuffer(y);
    uchar* ptr = linebuf;

    for (int x = w; x > 0; x--)
    {
      // transform RGBA to RGB
      *ptr++ = *image++;
      *ptr++ = *image++;
      *ptr++ = *image++;
      image++;
    }

    setpixels(linebuf,y);
  }

  return true;
}

tiffImage::tiffImage(const char *n) : SharedImage()
{
  name = n;
}


