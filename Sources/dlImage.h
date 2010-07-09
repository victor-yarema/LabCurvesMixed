////////////////////////////////////////////////////////////////////////////////
//
// LabCurves
//
// Copyright (C) 2008,2009 Jos De Laender
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

#ifndef DLIMAGE_H
#define DLIMAGE_H

#include "dlDefines.h"
#include "dlConstants.h"

// A forward declaration to the curve class.

class dlCurve;

// Class containing an image and its operations.

class dlImage {
public:

// The image , assumed 3 channels and 0..0xffff values.
// Representation for RGB
// [0] = R
// [1] = G
// [2] = B
uint16_t (*m_Image)[3];

// Width and height of the image
uint16_t m_Width;
uint16_t m_Height;

// Bit depth at input time
short m_Depth;

// Nr of colors in the image (probably always 3 ?)
short m_Colors;

// Color space.
// Can be one of
//   dlSpace_sRGB_D65         1
//   dlSpace_AdobeRGB_D65     2
//   dlSpace_WideGamutRGB_D65 3
//   dlSpace_ProPhotoRGB_D65  4
//   dlSpace_LAB              10
//   dlSpace_XYZ              11
short m_ColorSpace;

// Constructor
dlImage();

// Destructor
~dlImage();

// Initialize it from another image.
// Copying is always deep (so including copying the image).
dlImage* Set(const dlImage *Origin);

// Initialize it from a dropped image (fwrite dropped).
dlImage* Set(const uint16_t Width,
             const uint16_t Height,
             const short    NrColors,
             const short    NrBytesPerColor,
             const char*    FileName);

// Crop
dlImage* Crop(const uint16_t X,
              const uint16_t Y,
              const uint16_t W,
              const uint16_t H,
              const short    InPlace=1);

// Apply a curve to an image.
//   ChannelMask : has a '1' on the bitposition of the channel that needs
//                 to be operated on. Typical 7 for RGB, 1 for LAB on L
dlImage* ApplyCurve(const dlCurve *Curve,
                    const uint8_t ChannelMask);

dlImage* ApplySaturationCurve(const dlCurve *Curve,
                              const short Mode,
                              const short Type);

dlImage* Bin(const short ScaleFactor);

dlImage* lcmsLabToRGBSimple();

// View LAB
dlImage* ViewLAB(const short Channel);

dlImage* dlGMOpenImage(const char* FileName,
                       long& ProfileSize,
                       uint8_t* &ProfileBuffer,
                       int& Success);

dlImage* dlGMCWriteImage(const char* FileName,
                         const uint8_t* ProfileBuffer,
                         const long ProfileSize);

};

#endif
////////////////////////////////////////////////////////////////////////////////
