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
#include <stdarg.h>
#include "dlError.h"

////////////////////////////////////////////////////////////////////////////////
//
// dlMemoryError
//
// Checks if the memory allocation for Ptr succeeded and abort otherwise.
//
////////////////////////////////////////////////////////////////////////////////

void dlMemoryError(void        *Ptr,
                   const char  *FileName,
                   const int   Line) {
  if (Ptr) return;
  fprintf(stderr,"Memory allocation error at %s : %d\n",FileName,Line);
  exit(EXIT_FAILURE);
}

// Global ErrNo available for application.
int dlErrNo;
int dlWarNo;
// Global ErrorMessage.
char dlErrorMessage[1024];
char dlWarningMessage[1024];

////////////////////////////////////////////////////////////////////////////////
//
// dlLogError
//
// Log an error, set the dlErrno accordingly but dont bail out.
// Format,.. is printf like.
//
////////////////////////////////////////////////////////////////////////////////

void dlLogError(const short ErrorCode,
                const char* Format,
                ... ) {
  va_list ArgPtr;
  va_start(ArgPtr,Format);
  vfprintf(stderr,Format,ArgPtr);
  vsnprintf(dlErrorMessage,1024,Format,ArgPtr);
  va_end(ArgPtr);
  fprintf(stderr,"\n");
  dlErrNo = ErrorCode;
}

////////////////////////////////////////////////////////////////////////////////
//
// dlLogWarning
//
// Log an warning, set the dlWarNo accordingly but dont bail out.
// Format,.. is printf like.
//
////////////////////////////////////////////////////////////////////////////////

void dlLogWarning(const short WarningCode,
                  const char* Format,
                  ... ) {
  va_list ArgPtr;
  va_start(ArgPtr,Format);
  vfprintf(stderr,Format,ArgPtr);
  vsnprintf(dlWarningMessage,1024,Format,ArgPtr);
  va_end(ArgPtr);
  dlWarNo = WarningCode;
}

////////////////////////////////////////////////////////////////////////////////
