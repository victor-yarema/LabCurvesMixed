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

// std stuff needs to be declared apparently for jpeglib
// which seems a bug in the jpeglib header ?
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <cfloat>
#include <cmath>
#include <QMessageBox>

#include <lcms2.h>

#ifdef _OPENMP
  #include <omp.h>
#endif

#ifdef WIN32
#define NO_JPEG
#endif

#include "dlConstants.h"
#include "dlError.h"
#include "dlImage.h"
#include "dlCurve.h"
#include "dlConstants.h"

////////////////////////////////////////////////////////////////////////////////
//
// Constructor.
//
////////////////////////////////////////////////////////////////////////////////

dlImage::dlImage() {
  m_Width              = 0;
  m_Height             = 0;
  m_Image              = NULL;
  m_Depth              = 0;
  m_Colors             = 0;
  m_ColorSpace         = dlSpace_sRGB_D65;
};

////////////////////////////////////////////////////////////////////////////////
//
// Destructor.
//
////////////////////////////////////////////////////////////////////////////////

dlImage::~dlImage() {
  FREE(m_Image);
}


////////////////////////////////////////////////////////////////////////////////
//
// Set from drop (dirty quick debug function : read a dump to file)
//
////////////////////////////////////////////////////////////////////////////////

dlImage* dlImage::Set(const uint16_t Width,
                      const uint16_t Height,
                      const short    NrColors,
                      const short    NrBytesPerColor,
                      const char*    FileName) {

  m_Width  = Width;
  m_Height = Height;

  // Free a maybe preexisting and allocate space.
  FREE(m_Image);
  m_Image = (uint16_t (*)[3]) CALLOC(m_Width*m_Height,sizeof(*m_Image));
  dlMemoryError(m_Image,__FILE__,__LINE__);

  uint16_t (*Buffer)[NrColors];
  Buffer=(uint16_t (*)[NrColors])CALLOC(Width*Height,NrBytesPerColor*NrColors);
  FILE *InputFile = fopen(FileName,"rb");
  if (!InputFile) {
    dlLogError(dlError_FileOpen,FileName);
    return NULL;
  }
  if ((size_t)(Width*Height) !=
       fread(Buffer,NrBytesPerColor*NrColors,Width*Height,InputFile)) {
    dlLogError(dlError_FileOpen,FileName);
    return NULL;
  }
  FCLOSE(InputFile);
#pragma omp parallel for
  for (uint32_t i=0; i<(uint32_t)Height*Width; i++) {
    for (short c=0; c<3; c++) {
      m_Image[i][c] = Buffer[i][c];
      //printf("DEBUG : Buffer[i][c] : %d\n",Buffer[i][c]);
      if (NrBytesPerColor == 1) m_Image[i][c] = m_Image[i][c] << 1;
    }
  }

  FREE(Buffer);

  m_Depth = 8;
  m_Colors = 3;
  m_ColorSpace = dlSpace_sRGB_D65;

  return this;
}

////////////////////////////////////////////////////////////////////////////////
//
// Set
//
////////////////////////////////////////////////////////////////////////////////

dlImage* dlImage::Set(const dlImage *Origin) { // Always deep

  assert(NULL != Origin);

  m_Width              = Origin->m_Width;
  m_Height             = Origin->m_Height;
  m_Depth              = Origin->m_Depth;
  m_Colors             = Origin->m_Colors;
  m_ColorSpace         = Origin->m_ColorSpace;

  // And a deep copying of the image.
  // Free a maybe preexisting.
  FREE(m_Image);
  // Allocate new.
  m_Image = (uint16_t (*)[3]) CALLOC(m_Width*m_Height,sizeof(*m_Image));
  dlMemoryError(m_Image,__FILE__,__LINE__);
  memcpy(m_Image,Origin->m_Image,m_Width*m_Height*sizeof(*m_Image));
  return this;
}


////////////////////////////////////////////////////////////////////////////////
//
// ApplyCurve
//
////////////////////////////////////////////////////////////////////////////////

dlImage* dlImage::ApplyCurve(const dlCurve *Curve,
                             const uint8_t ChannelMask) {

  assert (NULL != Curve);
  assert (m_Colors == 3);
  assert (m_ColorSpace != dlSpace_XYZ);
#pragma omp parallel for default(shared)
  for (uint32_t i=0; i< (uint32_t)m_Height*m_Width; i++) {
    if (ChannelMask & 1) m_Image[i][0] = Curve->m_Curve[ m_Image[i][0] ];
    if (ChannelMask & 2) m_Image[i][1] = Curve->m_Curve[ m_Image[i][1] ];
    if (ChannelMask & 4) m_Image[i][2] = Curve->m_Curve[ m_Image[i][2] ];
  }

  return this;
}

////////////////////////////////////////////////////////////////////////////////
//
// ApplyCurve
//
////////////////////////////////////////////////////////////////////////////////

dlImage* dlImage::ApplySaturationCurve(const dlCurve *Curve,
                                       const short Mode,
                                       const short Type) {

// Best solution would be to use the Lab <-> Lch conversion from lcms.
// This should be faster without sacrificing much quality.

  assert (m_ColorSpace == dlSpace_Lab);
  // neutral value for a* and b* channel
  const float WPH = 0x8080;

  float ValueA = 0.0;
  float ValueB = 0.0;

  if (Type == 0) { // by luma
#pragma omp parallel for schedule(static) private(ValueA, ValueB)
    for(uint32_t i = 0; i < (uint32_t) m_Width*m_Height; i++) {
      // Factor by hue
      ValueA = (float)m_Image[i][1]-WPH;
      ValueB = (float)m_Image[i][2]-WPH;
      float Hue = 0;
      if (ValueA == 0.0 && ValueB == 0.0) {
        Hue = 0;   // value for grey pixel
      } else {
        Hue = atan2f(ValueB,ValueA);
      }
      while (Hue < 0) Hue += 2.*dlPI;

      float Factor = Curve->m_Curve[CLIP((int32_t)(Hue/dlPI*WPH))]/(float)0x7fff;
      if (Factor == 1.0) continue;
      Factor *= Factor;
      float m = 0;
      if (Mode == 1) {
        float Col = powf(ValueA * ValueA + ValueB * ValueB, 0.125);
        Col /= 0xd; // normalizing to 0..1

        if (Factor > 1)
          // work more on desaturated pixels
          m = Factor*(1-Col)+Col;
        else
          // work more on saturated pixels
          m = Factor*Col+(1-Col);
      } else {
        m = Factor;
      }
      m_Image[i][1] = CLIP((int32_t)(m_Image[i][1] * m + WPH * (1. - m)));
      m_Image[i][2] = CLIP((int32_t)(m_Image[i][2] * m + WPH * (1. - m)));
    }
  } else { // by chroma
#pragma omp parallel for schedule(static) private(ValueA, ValueB)
    for(uint32_t i = 0; i < (uint32_t) m_Width*m_Height; i++) {
      // Factor by luminance
      float Factor = Curve->m_Curve[m_Image[i][0]]/(float)0x7fff;
      if (Factor == 1.0) continue;
      Factor *= Factor;
      float m = 0;
      if (Mode == 1) {
        ValueA = (float)m_Image[i][1]-WPH;
        ValueB = (float)m_Image[i][2]-WPH;
        float Col = powf(ValueA * ValueA + ValueB * ValueB, 0.125);
        Col /= 0xd; // normalizing to 0..1

        if (Factor > 1)
          // work more on desaturated pixels
          m = Factor*(1-Col)+Col;
        else
          // work more on saturated pixels
          m = Factor*Col+(1-Col);
      } else {
        m = Factor;
      }
      m_Image[i][1] = CLIP((int32_t)(m_Image[i][1] * m + WPH * (1. - m)));
      m_Image[i][2] = CLIP((int32_t)(m_Image[i][2] * m + WPH * (1. - m)));
    }
  }
  return this;
}

////////////////////////////////////////////////////////////////////////////////
//
// Crop
//
////////////////////////////////////////////////////////////////////////////////

dlImage* dlImage::Crop(const uint16_t X,
                       const uint16_t Y,
                       const uint16_t W,
                       const uint16_t H,
                       const short    InPlace) {

  assert(m_Colors ==3);
  assert( (X+W) <= m_Width);
  assert( (Y+H) <= m_Height);

  uint16_t (*CroppedImage)[3] =
    (uint16_t (*)[3]) CALLOC(W*H,sizeof(*m_Image));
  dlMemoryError(CroppedImage,__FILE__,__LINE__);

#pragma omp parallel for schedule(static)
  for (uint16_t Row=0;Row<H;Row++) {
    for (uint16_t Column=0;Column<W;Column++) {
      CroppedImage[Row*W+Column][0] = m_Image[(Y+Row)*m_Width+X+Column][0];
      CroppedImage[Row*W+Column][1] = m_Image[(Y+Row)*m_Width+X+Column][1];
      CroppedImage[Row*W+Column][2] = m_Image[(Y+Row)*m_Width+X+Column][2];
    }
  }

  // The image worked finally upon is 'this' or a new created one.
  dlImage* WorkImage = InPlace ? this : new (dlImage);

  if (InPlace) {
    FREE(m_Image); // FREE the old image.
  }

  WorkImage->m_Image  = CroppedImage;
  WorkImage->m_Width  = W;
  WorkImage->m_Height = H;
  WorkImage->m_Colors = m_Colors;
  WorkImage->m_Depth  = m_Depth;
  return WorkImage;
}


////////////////////////////////////////////////////////////////////////////////
//
// Bin
//
////////////////////////////////////////////////////////////////////////////////

dlImage* dlImage::Bin(const short ScaleFactor) {

  if (ScaleFactor == 0) return this;

  uint16_t NewHeight = m_Height >> ScaleFactor;
  uint16_t NewWidth = m_Width >> ScaleFactor;

  short Step = 1 << ScaleFactor;
  int Average = 2 * ScaleFactor;

  uint16_t (*NewImage)[3] =
    (uint16_t (*)[3]) CALLOC(NewWidth*NewHeight,sizeof(*m_Image));
  dlMemoryError(NewImage,__FILE__,__LINE__);

#pragma omp parallel for schedule(static)
  for (uint16_t Row=0; Row < NewHeight*Step; Row+=Step) {
    for (uint16_t Col=0; Col < NewWidth*Step; Col+=Step) {
      uint32_t  PixelValue[3] = {0,0,0};
      for (uint8_t sRow=0; sRow < Step; sRow++) {
        for (uint8_t sCol=0; sCol < Step; sCol++) {
          int32_t index = (Row+sRow)*m_Width+Col+sCol;
          for (short c=0; c < 3; c++) {
            PixelValue[c] += m_Image[index][c];
          }
        }
      }
      for (short c=0; c < 3; c++) {
        NewImage[Row/Step*NewWidth+Col/Step][c]
          = PixelValue[c] >> Average;
      }
    }
  }

  FREE(m_Image);
  m_Height = NewHeight;
  m_Width = NewWidth;
  m_Image = NewImage;

  return this;
}

////////////////////////////////////////////////////////////////////////////////
//
// LabtoRGB simple
//
////////////////////////////////////////////////////////////////////////////////

dlImage* dlImage::lcmsLabToRGBSimple() {

  cmsHPROFILE InProfile = cmsCreateLab4Profile(NULL);
  cmsHPROFILE OutProfile = cmsCreate_sRGBProfile();

  cmsHTRANSFORM Transform;
  Transform = cmsCreateTransform(InProfile,
                                 TYPE_Lab_16,
                                 OutProfile,
                                 TYPE_RGB_16,
                                 INTENT_PERCEPTUAL,
                                 cmsFLAGS_BLACKPOINTCOMPENSATION);

  int32_t Size = m_Width*m_Height;
  int32_t Step = 100000;
#pragma omp parallel for schedule(static)
  for (int32_t i = 0; i < Size; i+=Step) {
    int32_t Length = (i+Step)<Size ? Step : Size - i;
    uint16_t* Image = &m_Image[i][0];
    cmsDoTransform(Transform,Image,Image,Length);
  }

  cmsDeleteTransform(Transform);
  cmsCloseProfile(InProfile);
  cmsCloseProfile(OutProfile);
  m_ColorSpace = dlSpace_sRGB_D65;
  return this;
}

////////////////////////////////////////////////////////////////////////////////
//
// ViewLAB
//
////////////////////////////////////////////////////////////////////////////////

dlImage* dlImage::ViewLAB(const short Channel) {

  assert (m_ColorSpace == dlSpace_Lab);

  switch(Channel) {

    case dlViewLAB_L:
#pragma omp parallel for schedule(static)
      for (uint32_t i=0; i<(uint32_t) m_Height*m_Width; i++) {
        m_Image[i][1]=0x8080;
        m_Image[i][2]=0x8080;
      }
      break;

    case dlViewLAB_A:
#pragma omp parallel for schedule(static)
      for (uint32_t i=0; i<(uint32_t) m_Height*m_Width; i++) {
        m_Image[i][0]=m_Image[i][1];
        m_Image[i][1]=0x8080;
        m_Image[i][2]=0x8080;
      }
      break;

    case dlViewLAB_B:
#pragma omp parallel for schedule(static)
      for (uint32_t i=0; i<(uint32_t) m_Height*m_Width; i++) {
        m_Image[i][0]=m_Image[i][2];
        m_Image[i][1]=0x8080;
        m_Image[i][2]=0x8080;
      }
      break;

    default:
      break;
  }

  return this;
}

////////////////////////////////////////////////////////////////////////////////
