######################################################################
##
## LabCurves
##
## This file is part of LabCurves.
##
## LabCurves is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, version 3 of the License.
##
## LabCurves is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with LabCurves.  If not, see <http:/www.gnu.org/licenses/>.
##
######################################################################

######################################################################
#
# This is the Qt project file for LabCurves.
# Don't let it overwrite by qmake -project !
# A number of settings is tuned.
#
# qmake will make a platform dependent makefile of it.
#
######################################################################

CONFIG += release silent
#CONFIG += debug
TEMPLATE = app
TARGET = LabCurves
DEPENDPATH += .
DESTDIR = ..
OBJECTS_DIR = ../Objects
MOC_DIR = ../Objects
UI_HEADERS_DIR = ../Objects
RCC_DIR = ../Objects
# Bit funky for the glib.
QMAKE_CXXFLAGS_DEBUG += -ffast-math -O0 -g
QMAKE_CXXFLAGS_RELEASE += -O3 -fopenmp
QMAKE_CXXFLAGS_RELEASE += -ffast-math
QMAKE_CFLAGS_DEBUG += -ffast-math -O0 -g
QMAKE_CFLAGS_RELEASE += -O3 -fopenmp
QMAKE_CFLAGS_RELEASE += -ffast-math
QMAKE_LFLAGS_RELEASE += -fopenmp
QMAKE_LFLAGS_DEBUG += -rdynamic
LIBS += -lGraphicsMagick++ -lGraphicsMagickWand -lGraphicsMagick
LIBS += -lgomp -lpthread -llcms2
unix {
  QMAKE_CC = ccache /usr/bin/gcc
  QMAKE_CXX = ccache /usr/bin/g++
  INCLUDEPATH += /usr/include/GraphicsMagick
}
win32 {
  LIBS += -lwsock32 -lexpat -lregex -lgdi32
  INCLUDEPATH += /mingw/include/GraphicsMagick
}


# Input
HEADERS += ../Sources/dlConstants.h
HEADERS += ../Sources/dlCurve.h
HEADERS += ../Sources/dlDefines.h
HEADERS += ../Sources/dlError.h
HEADERS += ../Sources/dlGuiOptions.h
HEADERS += ../Sources/dlSettings.h
HEADERS += ../Sources/dlGuiItems.i
HEADERS += ../Sources/dlItems.i
HEADERS += ../Sources/dlImage.h
HEADERS += ../Sources/dlImage8.h
HEADERS += ../Sources/dlMainWindow.h
HEADERS += ../Sources/dlCurveWindow.h
HEADERS += ../Sources/dlHistogramWindow.h
HEADERS += ../Sources/dlViewWindow.h
HEADERS += ../Sources/dlProcessor.h
HEADERS += ../Sources/dlInput.h
HEADERS += ../Sources/dlChoice.h
HEADERS += ../Sources/dlCheck.h
HEADERS += ../Sources/dlCalloc.h
HEADERS += ../Sources/dlGroupBox.h
FORMS +=   ../Sources/dlMainWindow.ui
SOURCES += ../Sources/dlCurve.cpp
SOURCES += ../Sources/dlError.cpp
SOURCES += ../Sources/dlGuiOptions.cpp
SOURCES += ../Sources/dlSettings.cpp
SOURCES += ../Sources/dlImage.cpp
SOURCES += ../Sources/dlImage8.cpp
SOURCES += ../Sources/dlImage_GM.cpp
SOURCES += ../Sources/dlImage_GMC.cpp
SOURCES += ../Sources/dlMain.cpp
SOURCES += ../Sources/dlMainWindow.cpp
SOURCES += ../Sources/dlCurveWindow.cpp
SOURCES += ../Sources/dlHistogramWindow.cpp
SOURCES += ../Sources/dlViewWindow.cpp
SOURCES += ../Sources/dlProcessor.cpp
SOURCES += ../Sources/dlInput.cpp
SOURCES += ../Sources/dlChoice.cpp
SOURCES += ../Sources/dlCheck.cpp
SOURCES += ../Sources/dlCalloc.cpp
SOURCES += ../Sources/dlGroupBox.cpp
RESOURCES = ../LabCurves.qrc

###############################################################################
