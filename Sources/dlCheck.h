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

#ifndef DLCHECK_H
#define DLCHECK_H

#include <QtGui>

////////////////////////////////////////////////////////////////////////////////
//
// dlCheck is a object showing a checkbox
//
////////////////////////////////////////////////////////////////////////////////

class dlCheck : public QObject {

Q_OBJECT

public :

dlCheck(const QWidget* MainWindow,
        const QString  ObjectName,
        const QString  ParentName,
        const QString  Label,
        const QString  ToolTip);
// Destructor.
~dlCheck();

void SetValue(const QVariant Value, const short BlockSignal = 1);
void SetEnabled(const short Enabled);
void Show(const short Show);

protected:

private slots:
void OnValueChanged(int Value);

signals :
void valueChanged(QVariant Value);

private:
QVariant   m_Value;
QCheckBox* m_CheckBox;
};

#endif

////////////////////////////////////////////////////////////////////////////////
