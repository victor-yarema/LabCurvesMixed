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

#include "dlDefines.h"

#ifndef DLERROR_H
#define DLERROR_H

// Fatal memory error.
void dlMemoryError(void *Ptr,const char *FileName,const int Line);

// Other errors logged.
void dlLogError(const short ErrorCode,const char* Format, ... );

void dlLogWarning(const short WarningCode,const char* Format, ... );

// Access to error.
extern int  dlErrNo;
extern char dlErrorMessage[1024];

#endif

////////////////////////////////////////////////////////////////////////////////
