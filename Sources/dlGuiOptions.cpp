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

#include <stdlib.h>

#include "QObject"

#include "dlGuiOptions.h"
#include "dlDefines.h"

////////////////////////////////////////////////////////////////////////////////
//
// Bunch of structured options for the Gui choice elements;
//
////////////////////////////////////////////////////////////////////////////////

const dlGuiOptionsItem dlGuiOptions::PipeSize[] = {
  {dlPipeSize_Eighth,  QObject::tr("1:8") },
  {dlPipeSize_Quarter, QObject::tr("1:4") },
  {dlPipeSize_Half,    QObject::tr("1:2") },
  {dlPipeSize_Full,    QObject::tr("1:1") },
  {-1,NULL}};

const dlGuiOptionsItem dlGuiOptions::CurveL[] = {
  {dlCurveChoice_None,    QObject::tr("None") },
  {dlCurveChoice_Manual,  QObject::tr("Manual") },
  {-1,NULL}};

const dlGuiOptionsItem dlGuiOptions::Curvea[] = {
  {dlCurveChoice_None,    QObject::tr("None") },
  {dlCurveChoice_Manual,  QObject::tr("Manual") },
  {-1,NULL}};

const dlGuiOptionsItem dlGuiOptions::Curveb[] = {
  {dlCurveChoice_None,    QObject::tr("None") },
  {dlCurveChoice_Manual,  QObject::tr("Manual") },
  {-1,NULL}};

const dlGuiOptionsItem dlGuiOptions::CurveSaturation[] = {
  {dlCurveChoice_None,    QObject::tr("None") },
  {dlCurveChoice_Manual,  QObject::tr("Manual") },
  {-1,NULL}};

const dlGuiOptionsItem dlGuiOptions::ViewLAB[] = {
  {dlViewLAB_LAB,         QObject::tr("LAB") },
  {dlViewLAB_L,           QObject::tr("L") },
  {dlViewLAB_A,           QObject::tr("A") },
  {dlViewLAB_B,           QObject::tr("B") },
  {-1,NULL}};

////////////////////////////////////////////////////////////////////////////////
