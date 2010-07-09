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

#include <QMessageBox>
#include "dlImage.h"
#include "dlConstants.h"
#include "dlError.h"
#include "dlSettings.h"

#include <Magick++.h>

using namespace Magick;

#ifdef _OPENMP
  #include <omp.h>
#endif

#include <lcms2.h>

// Open Image
dlImage* dlImage::dlGMOpenImage(const char* FileName,
                                long& ProfileSize,
                                uint8_t* &ProfileBuffer,
                                int& Success) {

  Magick::Image image;
  try {
    image.read(FileName);
  } catch (Exception &Error) {
    return this;
  }
  Success = 1;

  if (image.depth() == 16 )
    m_Depth = 16;
  else
    m_Depth = 8;

  image.type(TrueColorType);
  image.magick( "RGB" );
  image.depth( 16 );

  uint16_t NewWidth = image.columns();
  uint16_t NewHeight = image.rows();

  // Get the embedded profile
  Magick::Blob Profile = image.iccColorProfile();

  ProfileSize = Profile.length();

  FREE(ProfileBuffer);

  ProfileBuffer = (uint8_t*) CALLOC(ProfileSize,sizeof(uint8_t));
  dlMemoryError(ProfileBuffer,__FILE__,__LINE__);

  memcpy(ProfileBuffer, Profile.data(), ProfileSize);

  cmsHPROFILE InProfile = NULL;

  if (ProfileSize > 0) {
    InProfile = cmsOpenProfileFromMem(ProfileBuffer, ProfileSize);
  } else {
    InProfile = cmsCreate_sRGBProfile();
    FREE(ProfileBuffer);
  }
  if (!InProfile) {
    InProfile = cmsCreate_sRGBProfile();
  }

  // the next hast to be double for lcms
  float (*ImageBuffer)[3] =
    (float (*)[3]) CALLOC(NewWidth*NewHeight,sizeof(*ImageBuffer));
  dlMemoryError(ImageBuffer,__FILE__,__LINE__);

  image.write(0,0,NewWidth,NewHeight,"RGB",FloatPixel,ImageBuffer);

  FREE(m_Image);
  m_Width  = NewWidth;
  m_Height = NewHeight;
  m_Colors = 3;
  m_ColorSpace = dlSpace_Lab;
  m_Image = (uint16_t (*)[3]) CALLOC(m_Width*m_Height,sizeof(*m_Image));
  dlMemoryError(m_Image,__FILE__,__LINE__);

  cmsHPROFILE OutProfile = cmsCreateLab4Profile(NULL);

  cmsHTRANSFORM Transform;
  Transform = cmsCreateTransform(InProfile,
                                 TYPE_RGB_FLT,
                                 OutProfile,
                                 TYPE_Lab_16,
                                 INTENT_PERCEPTUAL,
                                 cmsFLAGS_BLACKPOINTCOMPENSATION);


  int32_t Size = m_Width*m_Height;
  int32_t Step = 100000;
#pragma omp parallel for schedule(static)
  for (int32_t i = 0; i < Size; i+=Step) {
    int32_t Length = (i+Step)<Size ? Step : Size - i;
    float* Buffer = &ImageBuffer[i][0];
    uint16_t* Image = &m_Image[i][0];
    cmsDoTransform(Transform,
                   Buffer,
                   Image,
                   Length);
  }

  cmsDeleteTransform(Transform);
  cmsCloseProfile(InProfile);
  cmsCloseProfile(OutProfile);

  FREE(ImageBuffer);

  return this;
}
