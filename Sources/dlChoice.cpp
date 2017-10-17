////////////////////////////////////////////////////////////////////////////////
//
// LabCurves
//
// Copyright (C) 2008 Jos De Laender
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

#include <assert.h>

#include "dlChoice.h"
#include "dlSettings.h"

////////////////////////////////////////////////////////////////////////////////
//
// Constructor.
//
////////////////////////////////////////////////////////////////////////////////

dlChoice::dlChoice(const QWidget*          MainWindow,
                   const QString           ObjectName,
                   const QString           ParentName,
                   const short             HasDefaultValue,
                   const QVariant          Default,
                   const dlGuiOptionsItem* InitialOptions,
                   const QString           ToolTip,
       const int               TimeOut)
  :QObject() {

  if (Default.type() != QVariant::Int) {
    printf("(%s,%d) this : %s\n"
           "Default.type() : %d\n",
           __FILE__,__LINE__,
           ObjectName.toAscii().data(),
           Default.type());
    assert (Default.type() == QVariant::Int);
 }

  m_HaveDefault = HasDefaultValue;

  setObjectName(ObjectName);

  QWidget* Parent = MainWindow->findChild <QWidget*> (ParentName);

  if (!Parent) {
    fprintf(stderr,"(%s,%d) Could not find '%s'. Aborting\n",
           __FILE__,__LINE__,ParentName.toAscii().data());
    assert(Parent);
  }
  setParent(Parent);

  QHBoxLayout *Layout = new QHBoxLayout(Parent);

  //~ Layout->setContentsMargins(2,2,2,2);
  //~ Layout->setMargin(2);
  Parent->setLayout(Layout);

  m_ComboBox = new QComboBox(Parent);
  m_ComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  m_ComboBox->setToolTip(ToolTip);
  m_ComboBox->setFixedHeight(24);
  //~ m_ComboBox->setLayoutDirection(Qt::RightToLeft);
  for (short i=0; InitialOptions && InitialOptions[i].Value !=-1; i++ ) {
    m_ComboBox->addItem(InitialOptions[i].Text,InitialOptions[i].Value);
  }
  m_ComboBox->installEventFilter(this);

  //~ m_Button  = new QToolButton(Parent);
  //~ QIcon ButtonIcon;
  //~ ButtonIcon.addPixmap(QPixmap(
    //~ QString::fromUtf8(":/LabCurves/Icons/reload.svg")),
    //~ QIcon::Normal, QIcon::Off);
  //~ m_Button->setIcon(ButtonIcon);
  //~ m_Button->setIconSize(QSize(14,14));
  //~ m_Button->setText(QObject::tr("Reset"));
  //~ m_Button->setToolTip(QObject::tr("Reset to defaults"));
  //~ if (!m_HaveDefault) m_Button->hide();

  //~ Layout->addWidget(m_Button);
  Layout->addWidget(m_ComboBox);
  Layout->setContentsMargins(0,0,0,0);
  Layout->setMargin(0);
  Layout->addStretch(1);
  Layout->setSpacing(2);

  // A timer for time filtering signals going outside.
  m_TimeOut = TimeOut;
  m_Timer = new QTimer(this);
  m_Timer->setSingleShot(1);

  connect(m_Timer,SIGNAL(timeout()),
          this,SLOT(OnValueChangedTimerExpired()));

  // Connect the button
  //~ connect(m_Button,SIGNAL(clicked()),
          //~ this,SLOT(OnButtonClicked()));

  // Connect the combo
  connect(m_ComboBox,SIGNAL(activated(int)),
          this,SLOT(OnValueChanged(int)));

  // Set the default Value (and remember for later).
  m_DefaultValue = Default;
  SetValue(Default,1);
}

////////////////////////////////////////////////////////////////////////////////
//
// SetValue
//
////////////////////////////////////////////////////////////////////////////////

void dlChoice::SetValue(const QVariant Value,
                        const short    BlockSignal) {
  m_Value = Value;
  for (int i=0; i<m_ComboBox->count(); i++) {
   if (Value == m_ComboBox->itemData(i)) {
     m_ComboBox->setCurrentIndex(i);
     // setCurrentIndex does not generate signals !
     if (!BlockSignal) OnValueChanged(i);
     break;
   }
  }
}

////////////////////////////////////////////////////////////////////////////////
//
// AddOrReplaceItem
// Keeps Data unique, i.e. replace if exist.
//
////////////////////////////////////////////////////////////////////////////////

void dlChoice::AddOrReplaceItem(const QString Text,const QVariant Data) {
  for (int i=0; i<m_ComboBox->count(); i++) {
   if (Data == m_ComboBox->itemData(i)) {
     m_ComboBox->setItemText(i,Text);
     return;
   }
  }
  m_ComboBox->addItem(Text,Data);
}

////////////////////////////////////////////////////////////////////////////////
//
// Clear
//
////////////////////////////////////////////////////////////////////////////////

void dlChoice::Clear(void) {
  m_ComboBox->clear();
  m_Value.clear();
}

////////////////////////////////////////////////////////////////////////////////
//
// Show
//
////////////////////////////////////////////////////////////////////////////////

void dlChoice::Show(const short Show) {
  if (Show) {
    m_ComboBox->show();
    //~ if (m_HaveDefault) m_Button->show();
  } else {
    m_ComboBox->hide();
    //~ m_Button->hide();
  }
}

////////////////////////////////////////////////////////////////////////////////
//
// SetEnabled
//
////////////////////////////////////////////////////////////////////////////////

void dlChoice::SetEnabled(const short Enabled) {
  m_ComboBox->setEnabled(Enabled);
  //~ m_Button->setEnabled(Enabled);
}

////////////////////////////////////////////////////////////////////////////////
//
// OnButtonClicked
// Reset to the defaults.
//
////////////////////////////////////////////////////////////////////////////////

void dlChoice::OnButtonClicked() {
  SetValue(m_DefaultValue,0); // Generate signal !
}

////////////////////////////////////////////////////////////////////////////////
//
// OnValueChanged
// Translate to an 'external' signal
//
////////////////////////////////////////////////////////////////////////////////

void dlChoice::OnValueChanged(int Value) {
  m_Value = m_ComboBox->itemData(Value);
  if (m_TimeOut) {
    m_Timer->start(m_TimeOut);
  } else {
    OnValueChangedTimerExpired();
  }
}

void dlChoice::OnValueChangedTimerExpired() {
  //printf("(%s,%d) emiting signal(%d)\n",__FILE__,__LINE__,m_Value.toInt());
  emit(valueChanged(m_Value));
}

////////////////////////////////////////////////////////////////////////////////
//
// Event filter
//
////////////////////////////////////////////////////////////////////////////////

bool dlChoice::eventFilter(QObject *obj, QEvent *event)
{
  if (event->type() == QEvent::ContextMenu) {
    if (m_HaveDefault)
      OnButtonClicked();
    return true;
  } else {
    // pass the event on to the parent class
    return QObject::eventFilter(obj, event);
  }
}

////////////////////////////////////////////////////////////////////////////////
//
// Destructor.
//
////////////////////////////////////////////////////////////////////////////////

dlChoice::~dlChoice() {
  //printf("(%s,%d) %s this:%p name:%s\n",
  //       __FILE__,__LINE__,__PRETTY_FUNCTION__,
  //       this,objectName().toAscii().data());
  // Do not remove what is handled by Parent !
}

////////////////////////////////////////////////////////////////////////////////
