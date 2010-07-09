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

#include "dlSettings.h"
#include "dlError.h"
#include "dlGuiOptions.h"

#include <assert.h>

#ifdef _
  #define LabCurves_OLD__ _
  #undef _
#endif
// Define a simple one character _ for translation.
#define _ QObject::tr

// Load in the gui input elements
const dlGuiInputItem GuiInputItems[] = {
#define LabCurves_GUI_INPUT_ITEM
#include "dlGuiItems.i"
#undef  LabCurves_GUI_INPUT_ITEM
};

// Load in the gui choice (combo) elements
const dlGuiChoiceItem GuiChoiceItems[] = {
#define LabCurves_GUI_CHOICE_ITEM
#include "dlGuiItems.i"
#undef  LabCurves_GUI_CHOICE_ITEM
};

// Load in the gui check elements
const dlGuiCheckItem GuiCheckItems[] = {
#define LabCurves_GUI_CHECK_ITEM
#include "dlGuiItems.i"
#undef  LabCurves_GUI_CHECK_ITEM
};

// Load in the non gui elements
const dlItem Items[] = {
#include "dlItems.i"
};

#ifdef LabCurves_OLD__
  #undef _
  #define _ LabCurves_OLD__
#endif

// Macro for inserting a key into the hash and checking it is a new one.
#define M_InsertKeyIntoHash(Key,Item)                      \
  if (m_Hash.contains(Key)) {                              \
    dlLogError(dlError_Argument,                           \
               "Inserting an existing key (%s)",           \
               Key.toAscii().data());                      \
    assert (!m_Hash.contains(Key));                        \
  }                                                        \
  m_Hash[Key] = Item;

///////////////////////////////////////////////////////////////////////////////
//
// Constructor
//
///////////////////////////////////////////////////////////////////////////////

dlSettings::dlSettings(const short InitLevel) {

   assert(InitLevel<9); // 9 reserved for never to be remembered.

   // Gui Numerical inputs. Copy them from the const array in dlSettingItem.
  short NrSettings = sizeof(GuiInputItems)/sizeof(dlGuiInputItem);
  for (short i=0; i<NrSettings; i++) {
    dlGuiInputItem Description = GuiInputItems[i];
    dlSettingItem* SettingItem = new dlSettingItem;
    SettingItem->GuiType         = Description.GuiType;
    SettingItem->InitLevel       = Description.InitLevel;
    SettingItem->InJobFile       = Description.InJobFile;
    SettingItem->HasDefaultValue = Description.HasDefaultValue;
    SettingItem->DefaultValue    = Description.DefaultValue;
    SettingItem->MinimumValue    = Description.MinimumValue;
    SettingItem->MaximumValue    = Description.MaximumValue;
    SettingItem->Step            = Description.Step;
    SettingItem->NrDecimals      = Description.NrDecimals;
    SettingItem->Label           = Description.Label;
    SettingItem->ToolTip         = Description.ToolTip;
    M_InsertKeyIntoHash(Description.KeyName,SettingItem);
  }
  // Gui Choice inputs. Copy them from the const array in dlSettingItem.
  NrSettings = sizeof(GuiChoiceItems)/sizeof(dlGuiChoiceItem);
  for (short i=0; i<NrSettings; i++) {
    dlGuiChoiceItem Description = GuiChoiceItems[i];
    dlSettingItem* SettingItem = new dlSettingItem;
    SettingItem->GuiType         = Description.GuiType;
    SettingItem->InitLevel       = Description.InitLevel;
    SettingItem->InJobFile       = Description.InJobFile;
    SettingItem->HasDefaultValue = Description.HasDefaultValue;
    SettingItem->DefaultValue    = Description.DefaultValue;
    SettingItem->Value           = Description.DefaultValue;
    SettingItem->ToolTip         = Description.ToolTip;
    SettingItem->InitialOptions  = Description.InitialOptions;
    M_InsertKeyIntoHash(Description.KeyName,SettingItem);
  }
  // Gui Check inputs. Copy them from the const array in dlSettingItem.
  NrSettings = sizeof(GuiCheckItems)/sizeof(dlGuiCheckItem);
  for (short i=0; i<NrSettings; i++) {
    dlGuiCheckItem Description = GuiCheckItems[i];
    dlSettingItem* SettingItem = new dlSettingItem;
    SettingItem->GuiType      = Description.GuiType;
    SettingItem->InitLevel    = Description.InitLevel;
    SettingItem->InJobFile    = Description.InJobFile;
    SettingItem->DefaultValue = Description.DefaultValue;
    SettingItem->Value        = Description.DefaultValue;
    SettingItem->Label        = Description.Label;
    SettingItem->ToolTip      = Description.ToolTip;
    M_InsertKeyIntoHash(Description.KeyName,SettingItem);
  }
  // Non gui elements
  NrSettings = sizeof(Items)/sizeof(dlItem);
  for (short i=0; i<NrSettings; i++) {
    dlItem Description = Items[i];
    dlSettingItem* SettingItem = new dlSettingItem;
    SettingItem->GuiType      = dlGT_None;
    SettingItem->InitLevel    = Description.InitLevel;
    if (Description.DefaultValue.type() == QVariant::String) {
      QString Tmp = Description.DefaultValue.toString();
      Tmp.replace(QString("@INSTALL@"),QCoreApplication::applicationDirPath());
      Description.DefaultValue = Tmp;
    }
    SettingItem->DefaultValue = Description.DefaultValue;
    SettingItem->Value        = Description.DefaultValue;
    SettingItem->InJobFile    = Description.InJobFile;
    M_InsertKeyIntoHash(Description.KeyName,SettingItem);
  }

  // Now we have initialized from static values.
  // In the second round we overwrite now with what's coming from the ini
  // files.

  // Persistent settings.
  QCoreApplication::setOrganizationName(CompanyName);
  QCoreApplication::setOrganizationDomain("LabCurves.sourceforge.net");
  QCoreApplication::setApplicationName(ProgramName);
  // I strongly prefer ini files above register values as they
  // are readable and editeable (think of debug)
  // We don't want something in a windows registry, do we ?
  QSettings::setDefaultFormat(QSettings::IniFormat);
  m_IniSettings = new QSettings;

  QStringList Keys = m_Hash.keys();
  for (int i=0; i<Keys.size(); i++) {
    QString Key = Keys[i];
    dlSettingItem* Setting = m_Hash[Key];

    if (InitLevel > Setting->InitLevel) {
      // Default needs to be overwritten by something coming from ini file.
      Setting->Value = m_IniSettings->value(Key,Setting->DefaultValue);
      // Correction needed as the values coming from the ini file are
      // often interpreted as strings even if they could be int or so.
      const QVariant::Type TargetType = Setting->DefaultValue.type();
      if (Setting->Value.type() != TargetType) {
        switch (TargetType) {
          case QVariant::Int:
          case QVariant::UInt:
            Setting->Value = Setting->Value.toInt();
            break;
          case QVariant::Double:
    case QMetaType::Float:
            Setting->Value = Setting->Value.toDouble();
            break;
          case QVariant::StringList:
            Setting->Value = Setting->Value.toStringList();
            break;
          default:
            dlLogError(dlError_Argument,"Unexpected type %d",TargetType);
            assert(0);
        }
      }
      // Seen above this shouldn't happen, but better safe then sorry.
      if (Setting->Value.type() != Setting->DefaultValue.type()) {
        dlLogError(dlError_Argument,
                   "Type conversion error from ini file. Should : %d Is : %d\n",
                   Setting->DefaultValue.type(),Setting->Value.type());
        assert(Setting->Value.type() == Setting->DefaultValue.type());
      }
    } else {
      Setting->Value = Setting->DefaultValue;
    }
  }

  // Some ad-hoc corrections
  SetValue("Scaled",GetValue("PipeSize"));
};

////////////////////////////////////////////////////////////////////////////////
//
// Destructor
// Basically dumping the whole Settings hash to ini files.
//
////////////////////////////////////////////////////////////////////////////////

dlSettings::~dlSettings() {
  QStringList Keys = m_Hash.keys();
  for (int i=0; i<Keys.size(); i++) {
    QString Key = Keys[i];
    dlSettingItem* Setting = m_Hash[Key];
    m_IniSettings->setValue(Key,Setting->Value);
  }
  // Explicit call destructor (such that synced to disk)
  delete m_IniSettings;
}

////////////////////////////////////////////////////////////////////////////////
//
// GetValue
// Access to the hash, but with protection on non existing key.
//
////////////////////////////////////////////////////////////////////////////////

const QVariant dlSettings::GetValue(const QString Key) {
  if (!m_Hash.contains(Key)) {
    dlLogError(dlError_Argument,
               "(%s,%d) Could not find key '%s'\n",
               __FILE__,__LINE__,Key.toAscii().data());
    assert (m_Hash.contains(Key));
  }
  return m_Hash[Key]->Value;
}

////////////////////////////////////////////////////////////////////////////////
//
// GetInt
//
////////////////////////////////////////////////////////////////////////////////

int dlSettings::GetInt(const QString Key) {
  // Remark : UInt and Int are mixed here.
  // The only settings related type where u is important is uint16_t
  // (dimensions). uint16_t fits in an integer which is 32 bit.
  QVariant Tmp = GetValue(Key);
  if (Tmp.type() != QVariant::Int && Tmp.type() != QVariant::UInt) {
    dlLogError(dlError_Argument,
               "Expected 'QVariant::(U)Int' but got '%d' for key '%s'\n",
               Tmp.type(),Key.toAscii().data());
    if (Tmp.type() == QVariant::String) {
      dlLogError(dlError_Argument,
                 "Additionally : it's a string '%s'\n",
                 Tmp.toString().toAscii().data());
    }
    assert(Tmp.type() == QVariant::Int);
  }
  return Tmp.toInt();
}

////////////////////////////////////////////////////////////////////////////////
//
// GetDouble
//
////////////////////////////////////////////////////////////////////////////////

double dlSettings::GetDouble(const QString Key) {
  QVariant Tmp = GetValue(Key);
  if (static_cast<QMetaType::Type>(Tmp.type()) == QMetaType::Float) Tmp.convert(QVariant::Double);
  if (Tmp.type() != QVariant::Double) {
    dlLogError(dlError_Argument,
               "Expected 'QVariant::Double' but got '%d' for key '%s'\n",
               Tmp.type(),Key.toAscii().data());
    assert(Tmp.type() == QVariant::Double);
  }
  return Tmp.toDouble();
}

////////////////////////////////////////////////////////////////////////////////
//
// GetString
//
////////////////////////////////////////////////////////////////////////////////

const QString dlSettings::GetString(const QString Key) {
  QVariant Tmp = GetValue(Key);
  if (Tmp.type() != QVariant::String) {
    dlLogError(dlError_Argument,
               "Expected 'QVariant::String' but got '%d' for key '%s'\n",
               Tmp.type(),Key.toAscii().data());
    assert(Tmp.type() == QVariant::String);
  }
  return Tmp.toString();
}

////////////////////////////////////////////////////////////////////////////////
//
// GetStringList
//
////////////////////////////////////////////////////////////////////////////////

const QStringList dlSettings::GetStringList(const QString Key) {
  QVariant Tmp = GetValue(Key);
  if (Tmp.type() != QVariant::StringList) {
    dlLogError(dlError_Argument,
               "Expected 'QVariant::StringList' but got '%d' for key '%s'\n",
               Tmp.type(),Key.toAscii().data());
    assert(Tmp.type() == QVariant::StringList);
  }
  return Tmp.toStringList();
}

////////////////////////////////////////////////////////////////////////////////
//
// SetValue
// Access to the hash, but with protection on non existing key.
//
////////////////////////////////////////////////////////////////////////////////

void dlSettings::SetValue(const QString Key, const QVariant Value) {
  if (!m_Hash.contains(Key)) {
    dlLogError(dlError_Argument,
               "(%s,%d) Could not find key '%s'\n",
               __FILE__,__LINE__,Key.toAscii().data());
    assert (m_Hash.contains(Key));
  }
  m_Hash[Key]->Value = Value;
  // In job mode there are no gui elements and we have to return.
  if (GetInt("JobMode")) return;
  // If it's a gui element, we have to update it at once for consistency.
  switch (m_Hash[Key]->GuiType) {
    case dlGT_Input :
    case dlGT_InputSlider :
    case dlGT_InputSliderHue :
      m_Hash[Key]->GuiInput->SetValue(Value);
      break;
    case dlGT_Choice :
      m_Hash[Key]->GuiChoice->SetValue(Value);
      break;
    case dlGT_Check :
      m_Hash[Key]->GuiCheck->SetValue(Value);
      break;
    default:
      assert(m_Hash[Key]->GuiType == dlGT_None); // Else we missed a gui one
      break;
  }
}

////////////////////////////////////////////////////////////////////////////////
//
// SetEnabled
// Enable the underlying gui element (if one)
//
////////////////////////////////////////////////////////////////////////////////

void dlSettings::SetEnabled(const QString Key, const short Enabled) {
  if (!m_Hash.contains(Key)) {
    dlLogError(dlError_Argument,
               "(%s,%d) Could not find key '%s'\n",
               __FILE__,__LINE__,Key.toAscii().data());
    assert (m_Hash.contains(Key));
  }
  switch (m_Hash[Key]->GuiType) {
    case dlGT_Input :
    case dlGT_InputSlider :
    case dlGT_InputSliderHue :
      m_Hash[Key]->GuiInput->SetEnabled(Enabled);
      break;
    case dlGT_Choice :
      m_Hash[Key]->GuiChoice->SetEnabled(Enabled);
      break;
    case dlGT_Check :
      m_Hash[Key]->GuiCheck->SetEnabled(Enabled);
      break;
    default:
      dlLogError(dlError_Argument,
                 "%s is no (expected) gui element.",
                 Key.toAscii().data());
      assert(m_Hash[Key]->GuiType); // Should have gui type !
      break;
  }
}

////////////////////////////////////////////////////////////////////////////////
//
// SetMaximum
// Makes only sense for gui input element, which is asserted.
//
////////////////////////////////////////////////////////////////////////////////

void dlSettings::SetMaximum(const QString Key, const QVariant Maximum) {
  if (!m_Hash.contains(Key)) {
    dlLogError(dlError_Argument,
               "Could not find key '%s'\n",
               Key.toAscii().data());
    assert (m_Hash.contains(Key));
  }
  if (!m_Hash[Key]->GuiInput) {
    dlLogError(dlError_Argument,
               "Key '%s' has no initialized GuiInput\n",
               Key.toAscii().data());
    assert (m_Hash[Key]->GuiChoice);
  }
  return m_Hash[Key]->GuiInput->SetMaximum(Maximum);
}

////////////////////////////////////////////////////////////////////////////////
//
// Show (or hide)
// Makes only sense for gui element, which is asserted.
//
////////////////////////////////////////////////////////////////////////////////

void  dlSettings::Show(const QString Key, const short Show) {
  if (!m_Hash.contains(Key)) {
    dlLogError(dlError_Argument,
               "(%s,%d) Could not find key '%s'\n",
               __FILE__,__LINE__,Key.toAscii().data());
    assert (m_Hash.contains(Key));
  }
  switch (m_Hash[Key]->GuiType) {
    case dlGT_Input :
    case dlGT_InputSlider :
    case dlGT_InputSliderHue :
      m_Hash[Key]->GuiInput->Show(Show);
      break;
    case dlGT_Choice :
      m_Hash[Key]->GuiChoice->Show(Show);
      break;
    case dlGT_Check :
      m_Hash[Key]->GuiCheck->Show(Show);
      break;
    default:
      dlLogError(dlError_Argument,
                 "%s is no (expected) gui element.",
                 Key.toAscii().data());
      assert(m_Hash[Key]->GuiType);
      break;
  }
}

////////////////////////////////////////////////////////////////////////////////
//
// AddOrReplaceOption
// Makes only sense for gui choice (combo) element, which is asserted.
//
////////////////////////////////////////////////////////////////////////////////

void dlSettings::AddOrReplaceOption(const QString  Key,
                                    const QString  Text,
                                    const QVariant Value) {
  if (!m_Hash.contains(Key)) {
    dlLogError(dlError_Argument,
               "Could not find key '%s'\n",
               Key.toAscii().data());
    assert (m_Hash.contains(Key));
  }
  if (!m_Hash[Key]->GuiChoice) {
    dlLogError(dlError_Argument,
               "Key '%s' has no initialized GuiChoice\n",
               Key.toAscii().data());
    assert (m_Hash[Key]->GuiChoice);
  }
  m_Hash[Key]->GuiChoice->AddOrReplaceItem(Text,Value);
}

////////////////////////////////////////////////////////////////////////////////
//
// ClearOptions
// Makes only sense for gui choice (combo) element, which is asserted.
//
////////////////////////////////////////////////////////////////////////////////

void dlSettings::ClearOptions(const QString  Key) {
  if (!m_Hash.contains(Key)) {
    dlLogError(dlError_Argument,
               "Could not find key '%s'\n",
               Key.toAscii().data());
    assert (m_Hash.contains(Key));
  }
  if (!m_Hash[Key]->GuiChoice) {
    dlLogError(dlError_Argument,
               "Key '%s' has no initialized GuiChoice\n",
               Key.toAscii().data());
    assert (m_Hash[Key]->GuiChoice);
  }
  m_Hash[Key]->GuiChoice->Clear();
}

////////////////////////////////////////////////////////////////////////////////
//
// GetNrOptions
// Makes only sense for gui choice (combo) element, which is asserted.
//
////////////////////////////////////////////////////////////////////////////////

int dlSettings::GetNrOptions(const QString Key) {
  if (!m_Hash.contains(Key)) {
    dlLogError(dlError_Argument,
               "Could not find key '%s'\n",
               Key.toAscii().data());
    assert (m_Hash.contains(Key));
  }
  if (!m_Hash[Key]->GuiChoice) {
    dlLogError(dlError_Argument,
               "Key '%s' has no initialized GuiChoice\n",
               Key.toAscii().data());
    assert (m_Hash[Key]->GuiChoice);
  }
  return m_Hash[Key]->GuiChoice->Count();
}

////////////////////////////////////////////////////////////////////////////////
//
// GetOptionsValue (at index Index)
// Makes only sense for gui choice (combo) element, which is asserted.
//
////////////////////////////////////////////////////////////////////////////////

const QVariant dlSettings::GetOptionsValue(const QString Key,const int Index){
  if (!m_Hash.contains(Key)) {
    dlLogError(dlError_Argument,
               "Could not find key '%s'\n",
               Key.toAscii().data());
    assert (m_Hash.contains(Key));
  }
  if (!m_Hash[Key]->GuiChoice) {
    dlLogError(dlError_Argument,
               "Key '%s' has no initialized GuiChoice\n",
               Key.toAscii().data());
    assert (m_Hash[Key]->GuiChoice);
  }
  return m_Hash[Key]->GuiChoice->GetItemData(Index);
}

////////////////////////////////////////////////////////////////////////////////
//
// GetCurrentText
// Makes only sense for gui choice (combo) element, which is asserted.
//
////////////////////////////////////////////////////////////////////////////////

const QString dlSettings::GetCurrentText(const QString Key){
  if (!m_Hash.contains(Key)) {
    dlLogError(dlError_Argument,
               "Could not find key '%s'\n",
               Key.toAscii().data());
    assert (m_Hash.contains(Key));
  }
  if (!m_Hash[Key]->GuiChoice) {
    dlLogError(dlError_Argument,
               "Key '%s' has no initialized GuiChoice\n",
               Key.toAscii().data());
    assert (m_Hash[Key]->GuiChoice);
  }
  return m_Hash[Key]->GuiChoice->CurrentText();
}

////////////////////////////////////////////////////////////////////////////////
//
// SetGuiInput
//
////////////////////////////////////////////////////////////////////////////////

void dlSettings::SetGuiInput(const QString Key, dlInput* Value) {
  if (!m_Hash.contains(Key)) {
    dlLogError(dlError_Argument,
               "(%s,%d) Could not find key '%s'\n",
               __FILE__,__LINE__,Key.toAscii().data());
    assert (m_Hash.contains(Key));
  }
  m_Hash[Key]->GuiInput = Value;
}

////////////////////////////////////////////////////////////////////////////////
//
// SetGuiChoice
//
////////////////////////////////////////////////////////////////////////////////

void dlSettings::SetGuiChoice(const QString Key, dlChoice* Value) {
  if (!m_Hash.contains(Key)) {
    dlLogError(dlError_Argument,
               "(%s,%d) Could not find key '%s'\n",
               __FILE__,__LINE__,Key.toAscii().data());
    assert (m_Hash.contains(Key));
  }
  m_Hash[Key]->GuiChoice = Value;
}

////////////////////////////////////////////////////////////////////////////////
//
// SetGuiCheck
//
////////////////////////////////////////////////////////////////////////////////

void dlSettings::SetGuiCheck(const QString Key, dlCheck* Value) {
  if (!m_Hash.contains(Key)) {
    dlLogError(dlError_Argument,
               "(%s,%d) Could not find key '%s'\n",
               __FILE__,__LINE__,Key.toAscii().data());
    assert (m_Hash.contains(Key));
  }
  m_Hash[Key]->GuiCheck = Value;
}

///////////////////////////////////////////////////////////////////////////////
