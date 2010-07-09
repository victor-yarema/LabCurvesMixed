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

#ifndef DLCONSTANTS_H
#define DLCONSTANTS_H

////////////////////////////////////////////////////////////////////////////////
//
// Program wide constants definition
//
////////////////////////////////////////////////////////////////////////////////

const char ProgramName[] = "LabCurves";
const char CompanyName[] = "mm";

// Mathematical constants.
const double dlPI     = 3.14159265358979323846264338327950288419716939937510;
const double dlSQ2PI  = 2.50662827463100024161235523934010416269302368164062;

// Some program limits.
const short dlMaxAnchors    = 50; // Curve anchors.
const short dlMaxInputFiles = 2048;

// Don't mess with the numbers of any of those constants.
// Often there is relied upon , for instance as index in an array.
// Or the numbers are assumptions from dcraw.

// Processor phases.

const short dlProcessorPhase_Scale       = 1;
const short dlProcessorPhase_Lab         = 2;
const short dlProcessorPhase_Output      = 3;

// Processor modes.

const short dlProcessorMode_Preview     = 0;
const short dlProcessorMode_Full        = 1;
const short dlProcessorMode_Thumb       = 2;

// Color spaces.

const short dlSpace_sRGB_D65         = 1;
const short dlSpace_AdobeRGB_D65     = 2;
const short dlSpace_WideGamutRGB_D50 = 3;
const short dlSpace_ProPhotoRGB_D50  = 4;
const short dlSpace_Lab              = 10;
const short dlSpace_XYZ              = 11;
const short dlSpace_Profiled         = 20;

// Color profiles.

const short dlCameraColor_Flat              = 0;
const short dlCameraColor_Adobe_Matrix      = 1;
const short dlCameraColor_Adobe_Profile     = 2;
const short dlCameraColor_Embedded          = 3;
const short dlCameraColor_Profile           = 4;

const short dlCameraColorGamma_None     = 0;
const short dlCameraColorGamma_sRGB     = 1;
const short dlCameraColorGamma_BT709    = 2;
const short dlCameraColorGamma_Pure22   = 3;

const short dlScreenColor_Profile    = 20;/* Avoid collision with RGB or LAB */
const short dlOutputColor_Profile    = 21;/* Avoid collision with RGB or LAB */

// Size of the pipe.

const short dlPipeSize_Eighth        = 3;
const short dlPipeSize_Quarter       = 2; // Relying on this values for shift.
const short dlPipeSize_Half          = 1;
const short dlPipeSize_Full          = 0;

// Way of viewing.

const short dlPreviewMode_End        = 0;
const short dlPreviewMode_Tab        = 1;

// Histogram Channels

const short dlHistogramChannel_R     = 1; // Be aware : encoded values !
const short dlHistogramChannel_G     = 2;
const short dlHistogramChannel_B     = 4;
const short dlHistogramChannel_RGB   = 7;

// Curves.

const short dlCurveChannel_L           = 0;
const short dlCurveChannel_a           = 1;
const short dlCurveChannel_b           = 2;
const short dlCurveChannel_Saturation  = 3;

const short dlCurveType_Full         = 0;
const short dlCurveType_Anchor       = 1;

const short dlCurveChoice_None       = 0;
const short dlCurveChoice_Manual     = 1;
const short dlCurveChoice_File       = 2;

// Curve Interpolation Type

const short dlCurveIT_Spline = 0;
const short dlCurveIT_Linear = 1;

// ViewLAB

const short dlViewLAB_LAB    = 0;
const short dlViewLAB_L      = 1;
const short dlViewLAB_A      = 2;
const short dlViewLAB_B      = 3;

// Resize filters

const short dlResizeFilter_Box              = 0;
const short dlResizeFilter_Triangle         = 1;
const short dlResizeFilter_Quadratic        = 2;
const short dlResizeFilter_CubicBSpline     = 3;
const short dlResizeFilter_QuadraticBSpline = 4;
const short dlResizeFilter_CubicConvolution = 5;
const short dlResizeFilter_Lanczos3         = 6;
const short dlResizeFilter_Mitchell         = 7;
const short dlResizeFilter_CatmullRom       = 8;
const short dlResizeFilter_Cosine           = 9;
const short dlResizeFilter_Bell             = 10;
const short dlResizeFilter_Hermite          = 11;

// Resize filters for Imagemagick

const short dlIMFilter_Point     = 0;
const short dlIMFilter_Box       = 1;
const short dlIMFilter_Triangle  = 2;
const short dlIMFilter_Hermite   = 3;
const short dlIMFilter_Hanning   = 4;
const short dlIMFilter_Hamming   = 5;
const short dlIMFilter_Blackman  = 6;
const short dlIMFilter_Gaussian  = 7;
const short dlIMFilter_Quadratic = 8;
const short dlIMFilter_Cubic     = 9;
const short dlIMFilter_Catrom    = 10;
const short dlIMFilter_Mitchell  = 11;
const short dlIMFilter_Lanczos   = 12;
//const short dlIMFilter_Bessel    = 13;
//const short dlIMFilter_Sinc      = 14;


// Zoom modes
const short dlZoomMode_Fit    = 0;
const short dlZoomMode_NonFit = 1;

// Gui events : timeout filter
// 1s after releasing input arrows, processing will be triggered.
// Should be working also for sufficiently fast typing :)
const short dlTimeout_Input = 500;

// Gui Elements
const short dlGT_None            = 0;
const short dlGT_Input           = 1;
const short dlGT_InputSlider     = 2;
const short dlGT_InputSliderHue  = 3;
const short dlGT_Choice          = 4;
const short dlGT_Check           = 5;

// Warning values;
const short dlWarning_Argument      = 2;
const short dlWarning_Exiv2         = 8;

// Error values.
const short dlError_FileOpen        = 1;
const short dlError_Argument        = 2;
const short dlError_Spline          = 3;
const short dlError_FileFormat      = 4;
const short dlError_Profile         = 5;
const short dlError_NotForeseen     = 6;
const short dlError_lcms            = 7;
const short dlError_Lensfun         = 8;

// Matrices to transform from XYZ to RGB space.
// For different RGB spaces.
// XYZ = Matrix * RGB
// Dummy : Index 0 : don't use => 0
// dlSpace_sRGB_D65=1
// dlSpace_AdobeRGB_D65 = 2
// dlSpace_WideGamutRGB_D50 = 3
// dlSpace_ProPhotoRGB_D50 = 4

const double MatrixRGBToXYZ [5][3][3] =
  {
  // Dummy : Index 0 : don't use => 0
  {
  {0.0,0.0,0.0},
  {0.0,0.0,0.0},
  {0.0,0.0,0.0}
  },

  //dlSpace_sRGB_D65=1

  {
  {0.412424  , 0.357579  , 0.180464},
  {0.212656  , 0.715158  , 0.0721856},
  {0.0193324 , 0.119193  , 0.950444}
  },

  // dlSpace_AdobeRGB_D65 = 2
  {
  {0.576700  , 0.185556  , 0.188212},
  {0.297361  , 0.627355  , 0.0752847},
  {0.0270328 , 0.0706879 , 0.991248}
  },

  // dlSpace_WideGamutRGB_D50 = 3
  {
  {0.716105  , 0.100930  , 0.147186},
  {0.258187  , 0.724938  , 0.0168748},
  {0.000000  , 0.0517813 , 0.773429}
  },

  // dlSpace_ProPhotoRGB_D50 = 4
  {
  {0.797675  , 0.135192  , 0.0313534},
  {0.288040  , 0.711874  , 0.000086},
  {0.000000  , 0.000000  , 0.825210}
  }
  };

// Matrices to transform from RGB to XYZ space.
// For different RGB spaces.
// RGB = Matrix * XYZ

const double MatrixXYZToRGB [5][3][3] =
  {
  // Dummy : Index 0 : don't use => 0
  {
  {0.0,0.0,0.0},
  {0.0,0.0,0.0},
  {0.0,0.0,0.0}
  },

  //dlSpace_sRGB_D65=1
  {
  { 3.24071   ,-1.53726   ,-0.498571},
  {-0.969258  , 1.87599   , 0.0415557},
  { 0.0556352 ,-0.203996  , 1.05707}
  },

  // dlSpace_AdobeRGB_D65 = 2
  {
  { 2.04148   ,-0.564977  ,-0.344713},
  {-0.969258  , 1.87599   , 0.0415557},
  { 0.0134455 ,-0.118373  , 1.01527}
  },

  // dlSpace_WideGamutRGB_D50 = 3
  {
  { 1.46281   ,-0.184062  ,-0.274361},
  {-0.521793  , 1.44724   , 0.0677228},
  { 0.0349342 ,-0.0968931 , 1.28841}
  },

  // dlSpace_ProPhotoRGB_D50 = 4
  {
  { 1.34594   ,-0.255608  ,-0.0511118},
  {-0.544599  , 1.50817   , 0.0205351},
  { 0.000000  , 0.000000  , 1.21181}
  }
  };

const double MatrixBradfordD50ToD65[3][3] =
  {
  { 0.955577  , -0.023039 , 0.063164},
  {-0.028290  ,  1.009941 , 0.021007},
  { 0.012298  , -0.020483 , 1.329910}
  };

#endif

////////////////////////////////////////////////////////////////////////////////
