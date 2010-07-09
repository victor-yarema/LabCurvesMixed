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

#ifndef DLCALLOC_H
#define DLCALLOC_H

#include <cstddef>

void* dlCalloc(size_t num,
               size_t size,
               const char*  FileName,
               const int    LineNumber,
               const void*  ObjectPointer);

void  dlFree(void* Ptr,
             const char* FileName,
             const int   LineNumber,
             const void* ObjectPointer);

void  dlAllocated(const int   MinimumToShow,
                  const char* FileName,
                  const int   LineNumber);
#endif
