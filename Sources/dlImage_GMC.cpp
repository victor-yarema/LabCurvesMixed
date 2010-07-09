////////////////////////////////////////////////////////////////////////////////
//
// LabCurves
//
// Copyright (C) 2009,2010 Michael Munzert <mail@mm-log.com>
//
// This file is part of LabCurves.
//
// LabCurves is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 3 of the License.
//
// LabCurves is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with LabCurves.  If not, see <http://www.gnu.org/licenses/>.
//
////////////////////////////////////////////////////////////////////////////////

#include "dlImage.h"
#include "dlError.h"
#include <wand/magick_wand.h>

#ifdef _OPENMP
  #include <omp.h>
#endif

#include <stdlib.h>
#include <stdio.h>

#include <lcms2.h>

// Write Image
dlImage* dlImage::dlGMCWriteImage(const char* FileName,
                                  const uint8_t* ProfileBuffer,
                                  const long ProfileSize) {

  long unsigned int Width  = m_Width;
  long unsigned int Height = m_Height;

  MagickWand *mw;

  mw = NewMagickWand();
  MagickSetSize(mw, Width, Height);
  MagickReadImage(mw,"xc:none");
  MagickSetImageFormat(mw,"RGB");
  MagickSetImageDepth(mw,16);
  MagickSetImageType(mw,TrueColorType);
  MagickSetImageOption(mw, "tiff", "alpha", "associated");

  MagickSetImagePixels(mw,0,0,Width,Height,"RGB",ShortPixel,(unsigned char*) m_Image);

  if (ProfileSize > 0)
    MagickSetImageProfile(mw,"ICC",ProfileBuffer,ProfileSize);

  // Redering intent
  MagickSetImageRenderingIntent(mw,PerceptualIntent);

  // Depth
  MagickSetImageDepth(mw,m_Depth);

  // Compression
  MagickSetImageCompression(mw, LZWCompression);

  MagickWriteImage(mw, FileName);
  DestroyMagickWand(mw);
  return this;
}



