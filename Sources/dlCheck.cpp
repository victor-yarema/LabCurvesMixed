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

#include <assert.h>

#include "dlCheck.h"

////////////////////////////////////////////////////////////////////////////////
//
// Constructor.
//
////////////////////////////////////////////////////////////////////////////////

dlCheck::dlCheck(const QWidget* MainWindow,
                 const QString  ObjectName,
                 const QString  ParentName,
                 const QString  Label,
                 const QString  ToolTip)
  :QObject() {

  setObjectName(ObjectName);

  QWidget* Parent = MainWindow->findChild <QWidget*> (ParentName);

  if (!Parent) {
    fprintf(stderr,"(%s,%d) Could not find '%s'. Aborting\n",
           __FILE__,__LINE__,ParentName.toAscii().data());
    assert(Parent);
  }
  setParent(Parent);

  QHBoxLayout *Layout = new QHBoxLayout(Parent);

  Layout->setContentsMargins(2,2,2,2);
  Layout->setMargin(2);
  Parent->setLayout(Layout);

  m_CheckBox = new QCheckBox(Parent);
  m_CheckBox->setToolTip(ToolTip);
  m_CheckBox->setText(Label);
  m_CheckBox->setFocusPolicy(Qt::NoFocus);

  Layout->addWidget(m_CheckBox);
  Layout->setContentsMargins(0,0,0,0);
  Layout->setMargin(0);
  Layout->addStretch(1);

  // Connect the check
  connect(m_CheckBox,SIGNAL(stateChanged(int)),
          this,SLOT(OnValueChanged(int)));

}

////////////////////////////////////////////////////////////////////////////////
//
// SetValue
//
////////////////////////////////////////////////////////////////////////////////

void dlCheck::SetValue(const QVariant Value,
                       const short    BlockSignal) {
  if (Value.type() != QVariant::Int) {
    printf("(%s,%d) this : %s Value.type() : %d\n",
           __FILE__,__LINE__,
           this->objectName().toAscii().data(),
           Value.type());
    assert(Value.type() == QVariant::Int);
  }

  m_Value = Value;

  m_CheckBox->blockSignals(BlockSignal);
  m_CheckBox->setChecked(Value.toInt());
  m_CheckBox->blockSignals(0);
}

////////////////////////////////////////////////////////////////////////////////
//
// SetEnabled
//
////////////////////////////////////////////////////////////////////////////////

void dlCheck::SetEnabled(const short Enabled) {
  m_CheckBox->setEnabled(Enabled);
}

////////////////////////////////////////////////////////////////////////////////
//
// Show
//
////////////////////////////////////////////////////////////////////////////////

void dlCheck::Show(const short Show) {
  if (Show) m_CheckBox->show();
  if (!Show) m_CheckBox->hide();
  return;
}

////////////////////////////////////////////////////////////////////////////////
//
// OnValueChanged
// Translate to an 'external' signal
//
////////////////////////////////////////////////////////////////////////////////

void dlCheck::OnValueChanged(int) {
  // We don't want bools in our variant system. Complex enough without.
  m_Value = (int) m_CheckBox->isChecked();
  //printf("(%s,%d) emiting signal(%d)\n",__FILE__,__LINE__,m_Value.toInt());
  emit(valueChanged(m_Value));
}

////////////////////////////////////////////////////////////////////////////////
//
// Destructor.
//
////////////////////////////////////////////////////////////////////////////////

dlCheck::~dlCheck() {
  //printf("(%s,%d) %s this:%p name:%s\n",
  //       __FILE__,__LINE__,__PRETTY_FUNCTION__,
  //       this,objectName().toAscii().data());
  // Do not remove what is handled by Parent !
}

////////////////////////////////////////////////////////////////////////////////
