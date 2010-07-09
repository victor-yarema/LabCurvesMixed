////////////////////////////////////////////////////////////////////////////////
//
// LabCurves
//
// Copyright (C) 2010 Michael Munzert <mail@mm-log.com> (mike)
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

#include "dlGroupBox.h"
#include "dlSettings.h"

#include <QMessageBox>

////////////////////////////////////////////////////////////////////////////////
//
// Constructor.
//
////////////////////////////////////////////////////////////////////////////////

dlGroupBox::dlGroupBox(const QString Title,
           QWidget* Parent,
           int i,
           int j) {

  QVBoxLayout *Layout = new QVBoxLayout(this);

  setParent(Parent);

  RightArrow = QPixmap(QString::fromUtf8(":/LabCurves/Icons/rightarrow.svg"));
  RightArrow = RightArrow.scaled(QSize(14,14));
  DownArrow = QPixmap(QString::fromUtf8(":/LabCurves/Icons/downarrow.svg"));
  DownArrow = DownArrow.scaled(QSize(14,14));
  setObjectName("Box");
  m_Widget = new QWidget();
  m_Widget->setContentsMargins(5,5,0,5);
  m_Widget->setObjectName("GroupBox");

  m_Icon = new QLabel();
  m_Icon->setPixmap(DownArrow);

  m_Title = new QLabel();
  m_Title->setObjectName("Title");
  m_Title->setText("<b>"+Title+"</b>");
  m_Title->setTextFormat(Qt::RichText);
  m_Title->setTextInteractionFlags(Qt::NoTextInteraction);

  QHBoxLayout *ButtonLayout = new QHBoxLayout();

  ButtonLayout->addWidget(m_Icon);
  ButtonLayout->addWidget(m_Title);
  ButtonLayout->addStretch();
  ButtonLayout->setContentsMargins(0,0,0,0);
  ButtonLayout->setSpacing(4);
  ButtonLayout->setMargin(0);

  Layout->addLayout(ButtonLayout);
  Layout->addWidget(m_Widget);
  Layout->setContentsMargins(0,0,0,0);
  Layout->setSpacing(0);
  Layout->setMargin(0);
  Layout->setAlignment(Qt::AlignTop);

  m_Name = "i" + QString::number(i) + "j" + QString::number(j) + "Widget";

  m_Folded = Settings->m_IniSettings->value(m_Name,1).toBool();
  if (m_Folded==1) {
    m_Widget->setVisible(false);
    m_Icon->clear();
    m_Icon->setPixmap(RightArrow);
  } else {
    m_Widget->setVisible(true);
    m_Icon->clear();
    m_Icon->setPixmap(DownArrow);
  }
}

////////////////////////////////////////////////////////////////////////////////
//
// MousePress
//
////////////////////////////////////////////////////////////////////////////////

void dlGroupBox::mousePressEvent(QMouseEvent *event) {
  if (event->y()<20 && event->x()<250) {
    if (m_Folded==0) {
      m_Widget->setVisible(false);
      m_Icon->clear();
      m_Icon->setPixmap(RightArrow);
      m_Folded = 1;
    } else {
      m_Widget->setVisible(true);
      m_Icon->clear();
      m_Icon->setPixmap(DownArrow);
      m_Folded = 0;
    }
    Settings->m_IniSettings->setValue(m_Name,m_Folded);
  }
}

 void dlGroupBox::paintEvent(QPaintEvent *)
 {
     QStyleOption opt;
     opt.init(this);
     QPainter p(this);
     style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
 }

////////////////////////////////////////////////////////////////////////////////
//
// Destructor.
//
////////////////////////////////////////////////////////////////////////////////

dlGroupBox::~dlGroupBox() {
}

////////////////////////////////////////////////////////////////////////////////


