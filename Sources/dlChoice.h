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

#ifndef DLCHOICE_H
#define DLCHOICE_H

#include <QtGui>
#include "dlGuiOptions.h"

////////////////////////////////////////////////////////////////////////////////
//
// dlChoice is a object showing a choice element with associated widgets
//
////////////////////////////////////////////////////////////////////////////////

class dlChoice : public QObject {

Q_OBJECT

public :

dlChoice(const QWidget*          MainWindow,
         const QString           ObjectName,
         const QString           ParentName,
         const short             HasDefaultValue,
         const QVariant          Default,
         const dlGuiOptionsItem* InitialOptions,
         const QString           ToolTip,
   const int     TimeOut);
// Destructor.
~dlChoice();

// Matches Value with a value in combobox.
void SetValue(const QVariant Value, const short BlockSignal = 1);
void AddOrReplaceItem(const QString Text,const QVariant Data);
int  Count(void) { return m_ComboBox->count(); };
void SetEnabled(const short Enabled);
void Clear(void);
void Show(const short Show);
QVariant GetItemData(const int Index) { return m_ComboBox->itemData(Index); };
QString  CurrentText(void) { return m_ComboBox->currentText(); };

private slots:
void OnValueChanged(int Value);
void OnButtonClicked();
void OnValueChangedTimerExpired();

signals :
void valueChanged(QVariant Value);

protected:
bool eventFilter(QObject *obj, QEvent *event);

private:
QVariant m_Value;
QVariant m_DefaultValue;
short    m_HaveDefault;
int      m_TimeOut;

QComboBox*   m_ComboBox;
QToolButton* m_Button;
QTimer*      m_Timer;
};

#endif

////////////////////////////////////////////////////////////////////////////////
