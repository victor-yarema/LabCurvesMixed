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

#include "dlMainWindow.h"
#include "dlError.h"
#include "dlGuiOptions.h"
#include "dlSettings.h"

#include <iostream>
#include <iomanip>

#include <iostream>
#include <QMessageBox>
using namespace std;

////////////////////////////////////////////////////////////////////////////////
//
// Here follow a bunch of macro's to avoid repetitive code for lots of
// the Gui elements.
//
////////////////////////////////////////////////////////////////////////////////

// Connect a menu action.

// Must be bool for the checkable menu items !
#define Macro_ConnectSomeMenu(Some)                                    \
  connect( action ## Some,SIGNAL(triggered(bool)),                     \
          this,SLOT(On ## Some ## Triggered(bool)));

// Connect a button.

#define Macro_ConnectSomeButton(Some)                                  \
  connect(Some ## Button,SIGNAL(clicked()),                            \
          this,SLOT(On ## Some ## ButtonClicked()));

// Callback into main for a menu item activated.

#define Macro_OnSomeMenuActivated(Some)                           \
void CB_Menu ## Some(const short Enabled);                        \
void dlMainWindow::On ## Some ## Triggered(const bool  Enabled) { \
  ::CB_Menu ## Some(Enabled);                                     \
}

// Leftover from removing the menu
void CB_MenuFileExit(const short);

////////////////////////////////////////////////////////////////////////////////
//
// dlMainWindow constructor.
//
////////////////////////////////////////////////////////////////////////////////

dlMainWindow::dlMainWindow(const QString Title)
  : QMainWindow(NULL) {

  // Setup from the Gui builder.
  setupUi(this);
  setWindowTitle(Title);

  // Arrange sizes
  MainSplitter->setStretchFactor(1,1);
  ControlSplitter->setStretchFactor(1,1);

  // Go and construct all the input,choice .. gui elements.
  const QStringList Keys = Settings->GetKeys();
  for (int i=0; i<Keys.size(); i++) {
    const QString Key = Keys[i];
    //printf("(%s,%d) '%s'\n",__FILE__,__LINE__,Key.toAscii().data());
    switch (Settings->GetGuiType(Key)) {

      case dlGT_InputSlider :
      case dlGT_Input       :
        //printf("(%s,%d) Creating Input for '%s'\n",
        //       __FILE__,__LINE__,Key.toAscii().data());
        {
        QString ParentName = Key + "Widget";
        QString ObjectName = Key + "Input";
        dlInput* Input =
          new dlInput(this,
                      ObjectName,
                      ParentName,
                      Settings->GetGuiType(Key) == dlGT_InputSlider,
                      0,
                      Settings->GetHasDefaultValue(Key),
                      Settings->GetDefaultValue(Key),
                      Settings->GetMinimumValue(Key),
                      Settings->GetMaximumValue(Key),
                      Settings->GetStep(Key),
                      Settings->GetNrDecimals(Key),
                      Settings->GetLabel(Key),
                      Settings->GetToolTip(Key),
                      dlTimeout_Input);
        connect(Input,SIGNAL(valueChanged(QVariant)),
                this,SLOT(OnInputChanged(QVariant)));
        //m_GuiInputs[ObjectName] = Input;
        Settings->SetGuiInput(Key,Input);
        }
        break;
      case dlGT_InputSliderHue :
        //printf("(%s,%d) Creating Input for '%s'\n",
        //       __FILE__,__LINE__,Key.toAscii().data());
        {
        QString ParentName = Key + "Widget";
        QString ObjectName = Key + "Input";
        dlInput* Input =
          new dlInput(this,
                      ObjectName,
                      ParentName,
                      1,
                      1,
                      Settings->GetHasDefaultValue(Key),
                      Settings->GetDefaultValue(Key),
                      Settings->GetMinimumValue(Key),
                      Settings->GetMaximumValue(Key),
                      Settings->GetStep(Key),
                      Settings->GetNrDecimals(Key),
                      Settings->GetLabel(Key),
                      Settings->GetToolTip(Key),
                      dlTimeout_Input);
        connect(Input,SIGNAL(valueChanged(QVariant)),
                this,SLOT(OnInputChanged(QVariant)));
        //m_GuiInputs[ObjectName] = Input;
        Settings->SetGuiInput(Key,Input);
        }
        break;
      case dlGT_Choice :
        //printf("(%s,%d) Creating Choice for '%s'\n",
        //       __FILE__,__LINE__,Key.toAscii().data());
        {
        QString ParentName = Key + "Widget";
        QString ObjectName = Key + "Choice";
        dlChoice* Choice =
          new dlChoice(this,
                       ObjectName,
                       ParentName,
                       Settings->GetHasDefaultValue(Key),
                       Settings->GetDefaultValue(Key),
                       Settings->GetInitialOptions(Key),
                       Settings->GetToolTip(Key),
                       dlTimeout_Input);
        connect(Choice,SIGNAL(valueChanged(QVariant)),
                this,SLOT(OnInputChanged(QVariant)));
        //m_GuiChoices[ObjectName] = Choice;
        Settings->SetGuiChoice(Key,Choice);
        }
        break;

      case dlGT_Check :
        //printf("(%s,%d) Creating Check for '%s'\n",
        //       __FILE__,__LINE__,Key.toAscii().data());
        {
        QString ParentName = Key + "Widget";
        QString ObjectName = Key + "Check";
        dlCheck* Check =
          new dlCheck(this,
                      ObjectName,
                      ParentName,
                      Settings->GetLabel(Key),
                      Settings->GetToolTip(Key));
        connect(Check,SIGNAL(valueChanged(QVariant)),
                this,SLOT(OnInputChanged(QVariant)));
        //m_GuiChecks[ObjectName] = Check;
        Settings->SetGuiCheck(Key,Check);
        }
        break;

      default :
        //printf("(%s,%d) No widget for '%s'\n",
        //       __FILE__,__LINE__,Key.toAscii().data());
        continue;
    };
    // To sync the state of the now created gui element we have
    // to set the value. As the setting now has the gui element
    // attached, it will be updated.
    Settings->SetValue(Key,Settings->GetValue(Key));
  }

  // GroupBoxes
  m_LBox = new dlGroupBox("L* Curve", CurvesContent, 0, 0);
  m_ABox = new dlGroupBox("a* Curve", CurvesContent, 0, 1);
  m_BBox = new dlGroupBox("b* Curve", CurvesContent, 0, 2);
  m_SaturationBox = new dlGroupBox("Saturation Curve", CurvesContent, 0, 3);
  m_VLABBox = new dlGroupBox("View L*a*b*", CurvesContent, 0, 4);

  while(CurvesContent->layout()->count())
    CurvesContent->layout()->removeItem(CurvesContent->layout()->itemAt(0));

  CurvesContent->layout()->addWidget(m_LBox);
  CurvesContent->layout()->addWidget(m_ABox);
  CurvesContent->layout()->addWidget(m_BBox);
  CurvesContent->layout()->addWidget(m_SaturationBox);
  CurvesContent->layout()->addWidget(m_VLABBox);
  (qobject_cast <QBoxLayout*> (CurvesContent->layout()))->addSpacerItem(CurveSpacer);
  (qobject_cast <QBoxLayout*> (CurvesContent->layout()))->addSpacerItem(CurveSpacer2);
  LCurve->setParent(m_LBox->m_Widget);
  aCurve->setParent(m_ABox->m_Widget);
  bCurve->setParent(m_BBox->m_Widget);
  SaturationCurve->setParent(m_SaturationBox->m_Widget);
  ViewLAB->setParent(m_VLABBox->m_Widget);
  m_LBox->m_Widget->setLayout(LCurve->layout());
  m_ABox->m_Widget->setLayout(aCurve->layout());
  m_BBox->m_Widget->setLayout(bCurve->layout());
  m_SaturationBox->m_Widget->setLayout(SaturationCurve->layout());
  m_VLABBox->m_Widget->setLayout(ViewLAB->layout());

  //
  // Run pipe related
  //

  Macro_ConnectSomeButton(PreviewMode);
  PreviewModeButton->setChecked(Settings->GetInt("PreviewMode"));

  Macro_ConnectSomeButton(Run);
  RunButton->setEnabled(Settings->GetInt("RunMode"));
  RunButton->setVisible(false);
  RunModeWidget->setVisible(false);

  Macro_ConnectSomeButton(ToGimp);

  Macro_ConnectSomeButton(CurveLOpen);
  Macro_ConnectSomeButton(CurveLSave);
  Macro_ConnectSomeButton(CurveaOpen);
  Macro_ConnectSomeButton(CurveaSave);
  Macro_ConnectSomeButton(CurvebOpen);
  Macro_ConnectSomeButton(CurvebSave);
  Macro_ConnectSomeButton(CurveSaturationOpen);
  Macro_ConnectSomeButton(CurveSaturationSave);

  Macro_ConnectSomeButton(WritePipe);

  // Timer to delay on resize operations.
  // (avoiding excessive calculations and loops in the ZoomFit approach.)
  m_ResizeTimer = new QTimer(this);
  m_ResizeTimer->setSingleShot(1);
  connect(m_ResizeTimer,
          SIGNAL(timeout()),
          this,
          SLOT(ResizeTimerExpired()));
  // This qualifies for an ugly hack :)
  // Need an event at t=0 at toplevel.
  m_Event0Timer = new QTimer(this);
  m_Event0Timer->setSingleShot(1);
  m_Event0Timer->start(0);
  connect(m_Event0Timer,
          SIGNAL(timeout()),
          this,
          SLOT(Event0TimerExpired()));
}

void CB_Event0();
void dlMainWindow::Event0TimerExpired() {
  ::CB_Event0();
}

////////////////////////////////////////////////////////////////////////////////
//
// All kind of Gui events.
// Translated back to a CB_ function into dlMain
//
////////////////////////////////////////////////////////////////////////////////

//
// Gimp
//

void CB_ToGimpButton();
void dlMainWindow::OnToGimpButtonClicked() {
  ::CB_ToGimpButton();
}

//
// PreviewMode
//

void CB_PreviewModeButton(const QVariant State);
void dlMainWindow::OnPreviewModeButtonClicked() {
  if (PreviewModeButton->isChecked())
    ::CB_PreviewModeButton(1);
  else
    ::CB_PreviewModeButton(0);
}

//
// Run
//

void CB_RunButton();
void dlMainWindow::OnRunButtonClicked() {
  ::CB_RunButton();
}

void CB_WritePipeButton();
void dlMainWindow::OnWritePipeButtonClicked() {
  ::CB_WritePipeButton();
}

void CB_InputChanged(const QString ObjectName, const QVariant Value);
void dlMainWindow::OnInputChanged(const QVariant Value) {
  QObject* Sender = sender();
  printf("(%s,%d) Sender : '%s'\n",
         __FILE__,__LINE__,Sender->objectName().toAscii().data());
  CB_InputChanged(Sender->objectName(),Value);

}

void CB_CurveLOpenButton();
void dlMainWindow::OnCurveLOpenButtonClicked() {
  ::CB_CurveLOpenButton();
}

void CB_CurveLSaveButton();
void dlMainWindow::OnCurveLSaveButtonClicked() {
  ::CB_CurveLSaveButton();
}

void CB_CurveaOpenButton();
void dlMainWindow::OnCurveaOpenButtonClicked() {
  ::CB_CurveaOpenButton();
}

void CB_CurveaSaveButton();
void dlMainWindow::OnCurveaSaveButtonClicked() {
  ::CB_CurveaSaveButton();
}

void CB_CurvebOpenButton();
void dlMainWindow::OnCurvebOpenButtonClicked() {
  ::CB_CurvebOpenButton();
}

void CB_CurvebSaveButton();
void dlMainWindow::OnCurvebSaveButtonClicked() {
  ::CB_CurvebSaveButton();
}

void CB_CurveSaturationOpenButton();
void dlMainWindow::OnCurveSaturationOpenButtonClicked() {
  ::CB_CurveaOpenButton();
}

void CB_CurveSaturationSaveButton();
void dlMainWindow::OnCurveSaturationSaveButtonClicked() {
  ::CB_CurveaSaveButton();
}

// Intercept close event and translate to a FileExit.
void dlMainWindow::closeEvent(QCloseEvent *Event) {
  Event->ignore();
  ::CB_MenuFileExit(1);
}

// Fit the image again after a resize event.

void dlMainWindow::resizeEvent(QResizeEvent*) {
  // Avoiding excessive calculations and ZoomFit loops.
  m_ResizeTimer->start(500); // 500 ms.
}

void CB_ZoomFitButton();
void dlMainWindow::ResizeTimerExpired() {
  // 250 would be the size for the progress label then.
  if (Settings->GetInt("ZoomMode") == dlZoomMode_Fit) {
    ::CB_ZoomFitButton();
  }
}

////////////////////////////////////////////////////////////////////////////////
//
// UpdateSettings()
//
// Adapt the Gui elements according to GuiSettings
// (for those gui elements that should be updated after a programmatic
// recalculation)
//
////////////////////////////////////////////////////////////////////////////////

void dlMainWindow::UpdateSettings() {
  // Preview Mode
  PreviewModeButton->setChecked(Settings->GetInt("PreviewMode"));

  // Run mode
  RunButton->setEnabled(Settings->GetInt("RunMode"));

}

////////////////////////////////////////////////////////////////////////////////
//
// Destructor
//
////////////////////////////////////////////////////////////////////////////////

dlMainWindow::~dlMainWindow() {
  //printf("(%s,%d) %s\n",__FILE__,__LINE__,__PRETTY_FUNCTION__);
}

////////////////////////////////////////////////////////////////////////////////
