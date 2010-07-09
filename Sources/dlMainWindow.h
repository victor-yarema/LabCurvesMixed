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

#ifndef DLMAINWINDOW_H
#define DLMAINWINDOW_H

#include <QTimer>

#include <exiv2/exif.hpp>
#include <exiv2/image.hpp>

#include "ui_dlMainWindow.h"

#include "dlInput.h"
#include "dlChoice.h"
#include "dlCheck.h"
#include "dlGroupBox.h"


////////////////////////////////////////////////////////////////////////////////
//
// dlMainWindow is the main gui element, showing all menus and controls.
//
////////////////////////////////////////////////////////////////////////////////

class dlMainWindow : public QMainWindow, public Ui::dlMainWindow {

Q_OBJECT

public:

// Constructor.
dlMainWindow(const QString         Title);
// Destructor.
~dlMainWindow();

// Update some settings from the GuiSettings.
// (after a dcraw calculation for instance).
void UpdateSettings(void);

// Make and model are remembered because in
// case of change the choice of white balances must be redone.
QString              m_CameraMake;
QString              m_CameraModel;

// Resize timer
QTimer* m_ResizeTimer;
// Event0 timer (create event at t=0)
QTimer* m_Event0Timer;

// Desktop
QDesktopWidget* m_DesktopWidget;

QDockWidget* ControlsDockWidget;

protected:
void closeEvent(QCloseEvent * Event);
void resizeEvent(QResizeEvent * Event);
private :
dlGroupBox* m_LBox;
dlGroupBox* m_ABox;
dlGroupBox* m_BBox;
dlGroupBox* m_SaturationBox;
dlGroupBox* m_VLABBox;

private slots:
void ResizeTimerExpired();
void Event0TimerExpired();

// The generic catchall input change.
//~ void OnTagsEditTextChanged();

void OnInputChanged(QVariant Value);

void OnToGimpButtonClicked();

void OnPreviewModeButtonClicked();

void OnRunButtonClicked();

void OnCurveLOpenButtonClicked();
void OnCurveLSaveButtonClicked();

void OnCurveaOpenButtonClicked();
void OnCurveaSaveButtonClicked();

void OnCurvebOpenButtonClicked();
void OnCurvebSaveButtonClicked();

void OnCurveSaturationOpenButtonClicked();
void OnCurveSaturationSaveButtonClicked();

void OnWritePipeButtonClicked();
};

#endif

////////////////////////////////////////////////////////////////////////////////
