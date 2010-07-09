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
TEMPLATE = subdirs

SUBDIRS += LabCurvesProject

###############################################################################
