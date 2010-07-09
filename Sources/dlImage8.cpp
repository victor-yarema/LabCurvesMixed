////////////////////////////////////////////////////////////////////////////////
//
// LabCurves
//
// Copyright (C) 2008 Jos De Laender
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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "dlError.h"
#include "dlImage8.h"
#include "dlImage.h"
#include "cmath"

////////////////////////////////////////////////////////////////////////////////
//
// Constructor.
//
////////////////////////////////////////////////////////////////////////////////

dlImage8::dlImage8() {
m_Width              = 0;
m_Height             = 0;
m_Image              = NULL;
m_Colors             = 0;
m_ColorSpace         = dlSpace_sRGB_D65;
};

////////////////////////////////////////////////////////////////////////////////
//
// Constructor.
//
////////////////////////////////////////////////////////////////////////////////

dlImage8::dlImage8(const uint16_t Width,
                   const uint16_t Height,
                   const short    NrColors) {
  m_Width              = Width;
  m_Height             = Height;
  m_Image              = NULL;
  m_Colors             = NrColors;
  m_ColorSpace         = dlSpace_sRGB_D65;

  m_Image = (uint8_t (*)[4]) CALLOC(m_Width*m_Height,sizeof(*m_Image));
  dlMemoryError(m_Image,__FILE__,__LINE__);
};

////////////////////////////////////////////////////////////////////////////////
//
// Destructor.
//
////////////////////////////////////////////////////////////////////////////////

dlImage8::~dlImage8() {
  FREE(m_Image);
};

////////////////////////////////////////////////////////////////////////////////
//
// Set
//
////////////////////////////////////////////////////////////////////////////////

dlImage8* dlImage8::Set(const dlImage *Origin) { // Always deep

  assert(NULL != Origin);
  assert(dlSpace_Lab != Origin->m_ColorSpace);

  m_Width      = Origin->m_Width;
  m_Height     = Origin->m_Height;
  m_Colors     = Origin->m_Colors;
  m_ColorSpace = Origin->m_ColorSpace;

  // Free maybe preexisting.
  FREE(m_Image);

  m_Image = (uint8_t (*)[4]) CALLOC(m_Width*m_Height,sizeof(*m_Image));
  for (uint32_t i=0 ; i<(uint32_t)m_Width*m_Height; i++) {
    for (short c=0; c<3; c++) {
      // Mind the R<->B swap !
      m_Image[i][2-c] = Origin->m_Image[i][c]>>8;
    }
    m_Image[i][3] = 0xff;
  }

  return this;
}

////////////////////////////////////////////////////////////////////////////////
//
// SimpleScale
//
////////////////////////////////////////////////////////////////////////////////

dlImage8* dlImage8::SimpleScale(const float Factor) {

  assert(m_Colors ==3);
  assert (Factor>0.0 && Factor<=1.0);

  if (fabs(Factor-1.0) < 0.01 ) return this;

  uint16_t   Divider = MAX(m_Height,m_Width);
  uint16_t   Multiplier = (uint16_t)(Divider*Factor);
  uint32_t   Normalizer = Divider * Divider;

  uint16_t NewHeight = m_Height * Multiplier / Divider;
  uint16_t NewWidth  = m_Width  * Multiplier / Divider;

  uint64_t (*Image64Bit)[3] =
    (uint64_t (*)[3]) CALLOC(NewWidth*NewHeight,sizeof(*Image64Bit));
  dlMemoryError(Image64Bit,__FILE__,__LINE__);

  for(uint16_t r=0; r<m_Height; r++) {
    /* r should be divided between ri and rii */
    uint16_t ri  = r * Multiplier / Divider;
    uint16_t rii = (r+1) * Multiplier / Divider;
    /* with weights riw and riiw (riw+riiw==Multiplier) */
    int64_t riw  = rii * Divider - r * Multiplier;
    int64_t riiw = (r+1) * Multiplier - rii * Divider;
    if (rii>=NewHeight) {
      rii  = NewHeight-1;
      riiw = 0;
    }
    if (ri>=NewHeight) {
      ri  = NewHeight-1;
      riw = 0;
    }
    for(uint16_t c=0; c<m_Width; c++) {
      uint16_t ci   = c * Multiplier / Divider;
      uint16_t cii  = (c+1) * Multiplier / Divider;
      int64_t  ciw  = cii * Divider - c * Multiplier;
      int64_t  ciiw = (c+1) * Multiplier - cii * Divider;
      if (cii>=NewWidth) {
        cii  = NewWidth-1;
        ciiw = 0;
      }
      if (ci>=NewWidth) {
        ci  = NewWidth-1;
        ciw = 0;
      }
      for (short cl=0; cl<3; cl++) {
        Image64Bit[ri *NewWidth+ci ][cl] += m_Image[r*m_Width+c][cl]*riw *ciw ;
        Image64Bit[ri *NewWidth+cii][cl] += m_Image[r*m_Width+c][cl]*riw *ciiw;
        Image64Bit[rii*NewWidth+ci ][cl] += m_Image[r*m_Width+c][cl]*riiw*ciw ;
        Image64Bit[rii*NewWidth+cii][cl] += m_Image[r*m_Width+c][cl]*riiw*ciiw;
      }
    }
  }

  FREE(m_Image); // free the old image.
  m_Image = NULL;

  m_Image =
    (uint8_t (*)[4]) CALLOC(NewWidth*NewHeight,sizeof(*m_Image));
  dlMemoryError(m_Image,__FILE__,__LINE__);

  // Fill the image from the Image64Bit.
  for (uint32_t c=0; c<(uint32_t)NewHeight*NewWidth; c++) {
    for (short cl=0; cl<3; cl++) {
      m_Image[c][cl] = Image64Bit[c][cl]/Normalizer;
    }
    m_Image[c][3] = 0xff;
  }

  FREE(Image64Bit);

  m_Width  = NewWidth;
  m_Height = NewHeight;

  return this;
}

////////////////////////////////////////////////////////////////////////////////
//
// WriteAsPpm
//
////////////////////////////////////////////////////////////////////////////////

short dlImage8::WriteAsPpm(const char*  FileName) {

  FILE *OutputFile = fopen(FileName,"wb");
  if (!OutputFile) {
    dlLogError(dlError_FileOpen,FileName);
    return dlError_FileOpen;
  }

  fprintf(OutputFile,"P%d\n%d %d\n%d\n",m_Colors/2+5, m_Width, m_Height, 0xff);

  uint8_t*  PpmRow = (uint8_t *) CALLOC(m_Width,m_Colors);
  dlMemoryError(PpmRow,__FILE__,__LINE__);

  for (uint16_t Row=0; Row<m_Height; Row++) {
    for (uint16_t Col=0; Col<m_Width; Col++) {
      for (short c=0;c<3;c++) {
        // Mind the R<->B swap !
        PpmRow [Col*m_Colors+c] = m_Image[Row*m_Width+Col][2-c];
      }
    }
    assert ( m_Width == fwrite(PpmRow,m_Colors,m_Width,OutputFile) );
  }

  FREE(PpmRow);
  FCLOSE(OutputFile);
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
