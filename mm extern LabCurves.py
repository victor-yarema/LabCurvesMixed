#!/usr/bin/env python

'''
mm extern LabCurves.py
Use 16 bit L*a*b* curves with external binary.

Author:
Michael Munzert (mail mm-log com)

Version:
2010.06.18 Inital release.

this script is modelled after the trace plugin
(lloyd konneker, lkk, bootch at nc.rr.com)

License:

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

The GNU Public License is available at
http://www.gnu.org/copyleft/gpl.html

'''

from gimpfu import *
import subprocess
import os, sys

def plugin_main(image, drawable, visible):
  pdb.gimp_image_undo_group_start(image)
  # Copy so the save operations doesn't affect the original
  if visible == 0:
    # Save in temporary.  Note: empty user entered file name
    temp = pdb.gimp_image_get_active_drawable(image)
    pdb.gimp_edit_named_copy(temp, "Temp")
  else:
    # Get the current visible
    temp = pdb.gimp_layer_new_from_visible(image, image, "Lab curves")
    image.add_layer(temp, 0)
    pdb.gimp_edit_named_copy(temp, "Temp")

  tempimage = pdb.gimp_edit_named_paste_as_new("Temp")
  pdb.gimp_buffer_delete("Temp")
  if not tempimage:
    raise RuntimeError
  pdb.gimp_image_undo_disable(tempimage)

  tempdrawable = pdb.gimp_image_get_active_layer(tempimage)

  # Use temp file names from gimp, it reflects the user's choices in gimp.rc
  tempfilename = pdb.gimp_temp_name("tif")

  # !!! Note no run-mode first parameter, and user entered filename is empty string
  pdb.gimp_progress_set_text ("Saving a copy")
  # pdb.gimp_file_save(tempimage, tempdrawable, tempfilename, "")
  pdb.file_tiff_save2(tempimage, tempdrawable, tempfilename, tempfilename, 0, 1)

  # Command line
  path = sys.path[0]
  command = path + "/LabCurves/LabCurves " + "\"" + tempfilename + "\""

  # Invoke external command
  pdb.gimp_progress_set_text ("run LabCurves...")
  pdb.gimp_progress_pulse()
  child = subprocess.Popen(command, shell=True)
  child.communicate()

  # put it as a new layer in the opened image
  try:
    newlayer2 = pdb.gimp_file_load_layer(tempimage, tempfilename)
  except:
    RuntimeError
  tempimage.add_layer(newlayer2,-1)
  pdb.gimp_edit_named_copy(newlayer2, "ImgVisible")

  if visible == 0:
    sel = pdb.gimp_edit_named_paste(drawable, "ImgVisible", 0)
  else:
    sel = pdb.gimp_edit_named_paste(temp, "ImgVisible", 0)
  pdb.gimp_buffer_delete("ImgVisible")
  pdb.gimp_floating_sel_anchor(sel)

  # cleanup
  os.remove(tempfilename)  # delete the temporary file
  gimp.delete(tempimage)   # delete the temporary image

  # Note the new image is dirty in Gimp and the user will be asked to save before closing.
  pdb.gimp_image_undo_group_end(image)
  gimp.displays_flush()


register(
        "python_fu_mm_extern_LabCurves",
        "L*a*b* curves.",
        "l*a*b* curves.",
        "Michael Munzert (mail mm-log com)",
        "Copyright 2010 Michael Munzert",
        "2010",
        "<Image>/Filters/MM-Filters/_Lab curves...", # menuitem with accelerator key
        "*", # image types
        [ (PF_RADIO, "visible", "Layer:", 1, (("new from visible", 1),("current layer",0)))
        ],
        [],
        plugin_main,
        # menu="<Image>/Filters", # really the menupath less menuitem.
        # Enables plugin regardless of image open, passes less params
        # domain=("gimp20-python", gimp.locale_directory))
        )

main()


