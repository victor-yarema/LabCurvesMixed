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

#ifndef DLGUIOPTIONS_H
#define DLGUIOPTIONS_H

#include "QtCore"
#include "dlConstants.h"

////////////////////////////////////////////////////////////////////////////////
//
// dlGuiOptions
//
// Bunch of structured options for the Gui choice elements;
//
////////////////////////////////////////////////////////////////////////////////

struct dlGuiOptionsItem {
QVariant Value;
QString  Text;
};

// Attention : heavy use of static, dlGuiOptions are
// obviously only meant to be instantiated once.

class dlGuiOptions {
public:
static const dlGuiOptionsItem PipeSize[];
static const dlGuiOptionsItem CurveL[];
static const dlGuiOptionsItem Curvea[];
static const dlGuiOptionsItem Curveb[];
static const dlGuiOptionsItem CurveSaturation[];
static const dlGuiOptionsItem ViewLAB[];
};

extern dlGuiOptions* GuiOptions;

#endif

////////////////////////////////////////////////////////////////////////////////
