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

#ifndef DLGROUPBOX_H
#define DLGROUPBOX_H

#include <QtGui>

////////////////////////////////////////////////////////////////////////////////
//
// dlGroupBox is a modified groupbox
//
////////////////////////////////////////////////////////////////////////////////

class dlGroupBox : public QWidget {

Q_OBJECT

public :

dlGroupBox(const QString Title,
     QWidget* Parent,
     int i,
     int j);

// Destructor.
~dlGroupBox();

QWidget*   m_Widget;

private slots:

signals :

protected:
void mousePressEvent(QMouseEvent *event);
void paintEvent(QPaintEvent *);

private:
bool      m_Folded;
QPixmap   RightArrow;
QPixmap   DownArrow;
QLabel*   m_Icon;
QLabel*   m_Title;
QString   m_Name;
QLabel*   test;
};

#endif

////////////////////////////////////////////////////////////////////////////////

