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

#ifndef DLINPUT_H
#define DLINPUT_H

#include <QtGui>

////////////////////////////////////////////////////////////////////////////////
//
// dlInput is a object showing a input element with associated widgets
//
////////////////////////////////////////////////////////////////////////////////

class dlInput : public QObject {

Q_OBJECT

public :

// Constructor.
dlInput(const QWidget*   MainWindow,
        const QString    ObjectName,
        const QString    ParentName,
        const short      HasSlider,
        const short      ColorSetting,
        const short      HasDefaultValue,
        const QVariant   Default,
        const QVariant   Minimum,
        const QVariant   Maximum,
        const QVariant   Step,
        const int        Decimals,
        const QString    LabelText,
        const QString    ToolTip,
        const int        TimeOut);
// Destructor.
~dlInput();

// BlockSignal avoids a signal emitted on programmatic update.
void SetValue(const QVariant Value, const short BlockSignal = 1);
void SetMaximum(const QVariant Value);
void SetMinimum(const QVariant Value);
void SetEnabled(const short Enabled);
void Show(const short Show);

private slots:
void OnSpinBoxChanged(int Value);
void OnSpinBoxChanged(double Value);
void OnSliderChanged(int Value);
void OnButtonClicked();
void OnValueChanged(int Value);
void OnValueChanged(double Value);
void OnValueChangedTimerExpired();

signals :
void valueChanged(QVariant Value);

protected:
bool eventFilter(QObject *obj, QEvent *event);

private:
QVariant m_Value;
QVariant m_DefaultValue;
int      m_TimeOut;
short    m_HaveDefault;

QVariant::Type    m_Type; // All values (and determines spinbox f.i.)
QAbstractSpinBox* m_SpinBox; // Common base for int and double.
QSlider*          m_Slider;
QToolButton*      m_Button;
QLabel*           m_Label;
QTimer*           m_Timer;
};

#endif

////////////////////////////////////////////////////////////////////////////////
