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

#ifndef DLSETTINGS_H
#define DLSETTINGS_H

#include <QtCore>

#include "dlConstants.h"
#include "dlGuiOptions.h"
#include "dlInput.h"
#include "dlChoice.h"
#include "dlCheck.h"

////////////////////////////////////////////////////////////////////////////////
//
// dlSettingItem
// Description of any setting.
// Gui elements will be attached and handled via the setting.
// Remark that one never will use this class directly but only
// via its friend class dlSettings. So nothing public.
//
////////////////////////////////////////////////////////////////////////////////

// Some forward declarations.
struct dlGuiInputItem;
struct dlGuiChoiceItem;
struct dlGuiCheckItem;
struct dlItem;

class dlSettingItem {

// All will be accessed via dlSettings
friend class dlSettings;

short    InitLevel;       // the smaller, the longer coming from ini fle
QVariant DefaultValue;    // Give always one. Maybe nonsensical.
QVariant Value;
short    InJobFile;       // Should be in job file ?
short    GuiType;         // Associated gui type
short    HasDefaultValue; // For gui : is the default sensical.
// Remainder is gui related stuff.
QVariant MinimumValue;
QVariant MaximumValue;
QVariant Step;
short    NrDecimals;
QString  Label;
QString  ToolTip;
const dlGuiOptionsItem * InitialOptions; // For choice (combo) gui elements.

// Reference to the associated Gui stuff
dlInput*  GuiInput;
dlChoice* GuiChoice;
dlCheck*  GuiCheck;

// Constructor of an item :
// Make sure non QVariants all have sensible defaults.
void dlSettingsItem() {
  InitLevel       = 9; // Never reached.
  InJobFile       = 0;
  GuiType         = dlGT_None;
  HasDefaultValue = 0;
  NrDecimals      = 0;
  InitialOptions  = NULL;
  GuiInput        = NULL;
  GuiCheck        = NULL;
  GuiChoice       = NULL;
  }
}; // End dlSettingItem class.

////////////////////////////////////////////////////////////////////////////////
//
// dlSettings
// This class finally works on underlying dlSettingItem, which
// are shielded. All access and changes via this one.
//
////////////////////////////////////////////////////////////////////////////////

class dlSettings {

private:
// We have this private section on top for being able
// referring to m_Hash already during declaration of class.
// Hash with the items.
QHash <QString,dlSettingItem*> m_Hash;

public :

// First of all a number of simple accessors to the dlSettingItem
// characterized by 'Key'

const QStringList GetKeys()
  { return m_Hash.keys(); };
short GetGuiType(const QString Key)
  { return m_Hash[Key]->GuiType;};
short GetHasDefaultValue(const QString Key)
  { return m_Hash[Key]->HasDefaultValue;};
const QVariant GetDefaultValue(const QString Key)
  { return m_Hash[Key]->DefaultValue;};
const QVariant GetMinimumValue(const QString Key)
  { return m_Hash[Key]->MinimumValue;};
const QVariant GetMaximumValue(const QString Key)
  { return m_Hash[Key]->MaximumValue;};
const QVariant GetStep(const QString Key)
  { return m_Hash[Key]->Step;};
short GetNrDecimals(const QString Key)
  { return m_Hash[Key]->NrDecimals;};
const QString GetLabel(const QString Key)
  { return m_Hash[Key]->Label;};
const QString GetToolTip(const QString Key)
  { return m_Hash[Key]->ToolTip;};
const dlGuiOptionsItem* GetInitialOptions(const QString Key)
  { return m_Hash[Key]->InitialOptions;};
short GetInJobFile(const QString Key)
  { return m_Hash[Key]->InJobFile;};

// Constructor
// The InitLevel determines how much of the .ini information is preserved.
// If InitLevel > InitLevelOfItem the initialization of that Item is
// preserved from the .ini.
// So dlSettings(0) would keep no setting at all. As InitLevelOfItem > 0.
// If InitLevelOfItem = 9 then the item will never be initialized from
// the .ini as InitLevel<9 (asserted).
dlSettings(const short InitLevel);

// Destructor
~dlSettings();

// Accessors to the Value part of a setting.
// Selfexplaining.
int    GetInt(const QString Key);
double GetDouble(const QString Key);
const QString     GetString(const QString Key);
const QStringList GetStringList(const QString Key);
// This should be avoided as much as possible and replaced by above.
// It is needed though, for instance while writing a job file.
const QVariant GetValue(const QString Key);

// Setting of value. Implies update for gui element, if one.
void  SetValue(const QString Key, const QVariant Value);
// Other selfexplaining settings. Make only sense for gui elements.
void  SetEnabled(const QString Key, const short Enabled);
void  SetMaximum(const QString Key, const QVariant Maximum);
void  Show(const QString Key, const short Show);

// Methods specific for choice (combo) type of gui elements.

// Keeps Value unique in the choice.
void  AddOrReplaceOption(const QString  Key,
                         const QString  Text,
                         const QVariant Value);
void  ClearOptions(const QString Key);
int   GetNrOptions(const QString Key);
// Value of the combobox at a certain index position.
const QVariant    GetOptionsValue(const QString Key, const int Index);
// Current text of the combobox.
const QString     GetCurrentText(const QString Key);

// Following are only used by dlMainWindow when inserting
// the gui elements in the dlMainWindow. They provide an access
// to one of the underlying gui elements.
void SetGuiInput(const QString Key, dlInput* Input);
void SetGuiChoice(const QString Key, dlChoice* Choice);
void SetGuiCheck(const QString Key, dlCheck* Check);

// Persistent Settings. Initialization code in dlMain
// will access it directly.
QSettings* m_IniSettings;
}; // end class dlSettings

// And we will instantiate one toplevel.
extern dlSettings* Settings;

////////////////////////////////////////////////////////////////////////////////
//
// dlGui*Item
// Initial description (from .i files) of items.
//
////////////////////////////////////////////////////////////////////////////////

// Initial description of a numerical input gui element.
struct dlGuiInputItem {
QString  KeyName;
short    GuiType;
short    InitLevel;
short    InJobFile;
short    HasDefaultValue;
QVariant DefaultValue;
QVariant MinimumValue;
QVariant MaximumValue;
QVariant Step;
short    NrDecimals;
QString  Label;
QString  ToolTip;
};

// Initial description of a choice (combobox) input gui element.
struct dlGuiChoiceItem {
QString                 KeyName;
short                   GuiType;
short                   InitLevel;
short                   InJobFile;
short                   HasDefaultValue;
QVariant                DefaultValue;
const dlGuiOptionsItem* InitialOptions;
QString                 ToolTip;
};

// Initial description of a check input gui element.
struct dlGuiCheckItem {
QString  KeyName;
short    GuiType;
short    InitLevel;
short    InJobFile;
QVariant DefaultValue;
QString  Label;
QString  ToolTip;
};

// Initial description of a simple setting element.
struct dlItem {
QString  KeyName;
short    InitLevel;
QVariant DefaultValue;
short    InJobFile;
};

#endif

////////////////////////////////////////////////////////////////////////////////
