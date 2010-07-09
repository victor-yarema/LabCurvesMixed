LabCurves
=========

About
-----
This program is meant as an example for an external GIMP plugin with
preview and internal 16 bit calculation. In this case a conversion to
L*a*b* color space is implemented and on each channel there is a curve
for adjusting the values.

Installation
------------
Prerequisites:
* GraphicsMagick Q16 (http://www.graphicsmagick.org/)
* Lcms 2 (http://www.littlecms.com/)
* QT (http://qt.nokia.com/)
* ccache
Compile:
* qmake
* make

Copy the python script to your GIMP plugins directory
and alter line 66 appropriately for the location of 
your compiled version.

Copyright
---------
LabCurves is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, version 3 of the License.

LabCurves is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

(C) 2010 Michael Munzert <mail@mm-log.com>

LabCurves uses code from dlRaw (http://dlraw.sourceforge.net/).
Copyright Jos De Laender and contributors.
