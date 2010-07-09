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

#ifndef DLPROCESSOR_H
#define DLPROCESSOR_H

#include <QString>
#include <QTime>

#include "dlImage.h"

class dlProcessor {

public:

// Cached image versions at different points.
dlImage*  m_Image_AfterOpen;
dlImage*  m_Image_AfterScale;
dlImage*  m_Image_AfterLab;

// Reporting back
void (*m_ReportProgress)(const QString Message);
void (*m_UpdateGUI)();

// Constructor
dlProcessor(void (*ReportProgress)(const QString Message));
// Destructor
~dlProcessor();

// Run mode
short NextPhase;
short NextSubPhase;

// Open the image
int Open();

// The real processing.
void Run(short Phase,
         short SubPhase      = -1,
         short WithIdentify  = 1,
         short ProcessorMode = dlProcessorMode_Preview);

// Reporting
void ReportProgress(const QString Message);

// Color Profile
long m_ProfileSize;
uint8_t* m_ProfileBuffer;

};

#endif

///////////////////////////////////////////////////////////////////////////////
