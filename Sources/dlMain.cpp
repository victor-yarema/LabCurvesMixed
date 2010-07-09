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

#define QT_CLEAN_NAMESPACE

#include <QtGui>
#include <QtCore>
#include <string>
#include <cassert>

#include "dlProcessor.h"
#include "dlMainWindow.h"
#include "dlViewWindow.h"
#include "dlCurveWindow.h"
#include "dlHistogramWindow.h"
#include "dlGuiOptions.h"
#include "dlSettings.h"
#include "dlError.h"
#include "dlCurve.h"

#include <Magick++.h>
#include <lcms2.h>

using namespace std;

////////////////////////////////////////////////////////////////////////////////
//
// This is the file where everything is started.
// It starts not only the gui but it is *also* the starting point
// for guiless jobmode. The design is such that the pipe runs
// independent whether or not gui thingies are attached to it.
//
////////////////////////////////////////////////////////////////////////////////

dlProcessor* TheProcessor    = NULL;

// L,a,b
dlCurve*  Curve[4]        = {NULL,NULL,NULL,NULL};
dlCurve*  BackupCurve[4]  = {NULL,NULL,NULL,NULL};
// I don't manage to init statically following ones. Done in InitCurves.
QStringList CurveKeys, CurveBackupKeys;
QStringList CurveFileNamesKeys;

cmsHPROFILE PreviewColorProfile = NULL;

dlImage*  PreviewImage     = NULL;
dlImage*  HistogramImage   = NULL;

// The main windows of the application.
dlMainWindow*      MainWindow      = NULL;
dlViewWindow*      ViewWindow      = NULL;
dlHistogramWindow* HistogramWindow = NULL;
dlCurveWindow*     CurveWindow[4]  = {NULL,NULL,NULL,NULL};

// Gui options and settings.
dlGuiOptions  *GuiOptions  = NULL;
dlSettings    *Settings = NULL;

// Screen position
QPoint MainWindowPos;
QSize  MainWindowSize;

// uint16_t (0,0xffff) to float (0.0, 1.0)
float ToFloatTable[0x10000];

// Filter patterns for the filechooser.
const QString CurveFilePattern =
  QObject::tr("Curve File (*.dlc);;All files (*.*)");

////////////////////////////////////////////////////////////////////////////////
//
// Some function prototypes.
//
////////////////////////////////////////////////////////////////////////////////

void   RunJob(const QString FileName);
short  ReadJobFile(const QString FileName);
void   WriteOut();
void   UpdatePreviewImage(const dlImage* ForcedImage   = NULL,
                          const short    OnlyHistogram = 0,
        const short    ForceRun = 0);
void   InitCurves();
void   CB_CurveChoice(const int Channel, const int Choice);
void   CB_ZoomFitButton();
void   CB_MenuFileExit(const short);

int    LabCurvesMain(int Argc, char *Argv[]);

////////////////////////////////////////////////////////////////////////////////
//
// Progress function in the GUI.
// Can later also in job.
//
////////////////////////////////////////////////////////////////////////////////

void ReportProgress(const QString Message) {
  printf("Progress : %s\n",Message.toAscii().data());
  if (!MainWindow) return;
  MainWindow->StatusLabel->setText(Message);
  MainWindow->StatusLabel->repaint();
  // Workaround to keep the GUI responsive
  // during pipe processing...
  QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
}

////////////////////////////////////////////////////////////////////////////////
//
// Update
//
////////////////////////////////////////////////////////////////////////////////

void Update(short Phase,
            short SubPhase      = -1,
            short WithIdentify  = 1,
            short ProcessorMode = dlProcessorMode_Preview) {
  MainWindow->UpdateSettings();
  TheProcessor->Run(Phase,SubPhase,WithIdentify, ProcessorMode);
  UpdatePreviewImage();
}

////////////////////////////////////////////////////////////////////////////////
//
// Main : instantiating toplevel windows, settings and options ..
//
////////////////////////////////////////////////////////////////////////////////

short   InStartup  = 1;

QString ImageFileToOpen;

int main(int Argc, char *Argv[]) {
  int RV = LabCurvesMain(Argc,Argv);
  return RV;
}

QApplication* TheApplication;

int LabCurvesMain(int Argc, char *Argv[]) {
  Magick::InitializeMagick(*Argv);

  // TextCodec
  QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());

  //QApplication TheApplication(Argc,Argv);
  TheApplication = new QApplication(Argc,Argv);

  if (Argc == 1 || Argc > 3) {
    QString ErrorMessage = QObject::tr("Usage : LabCurves Input Output");
    fprintf(stderr,"%s\n",ErrorMessage.toAscii().data());
    exit(EXIT_FAILURE);
  }

  // Some QStringLists to be initialized.
  CurveKeys << "CurveL"
            << "CurveLa"
            << "CurveLb"
            << "CurveSaturation";

  CurveBackupKeys = CurveKeys;

  CurveFileNamesKeys << "CurveFileNamesL"
                     << "CurveFileNamesLa"
                     << "CurveFileNamesLb"
                     << "CurveFileNamesSaturation";

  // Persistent settings.
  QCoreApplication::setOrganizationName(CompanyName);
  QCoreApplication::setOrganizationDomain("");
  QCoreApplication::setApplicationName(ProgramName);
  // I strongly prefer ini files above register values as they
  // are readable and editeable (think of debug)
  // We don't want something in a windows registry, do we ?
  QSettings::setDefaultFormat(QSettings::IniFormat);

  // Load the Settings (are also partly used in JobMode)
  Settings = new dlSettings(2);

  // FileNames
  Settings->SetValue("InputFileName", Argv[1]);
  Settings->SetValue("OutputFileName", Argv[2]);

  if (Settings->GetString("OutputFileName")=="")
    Settings->SetValue("OutputFileName",Settings->GetString("InputFileName"));

  // String corrections
  if (Settings->GetString("MainDirectory")!=QCoreApplication::applicationDirPath().append("/")) {
    QString OldMainDirectory = Settings->GetString("MainDirectory");
    OldMainDirectory.chop(1);
    QStringList Locations;
    Locations << "CurvesDirectory";
    Settings->SetValue("MainDirectory",QCoreApplication::applicationDirPath().append("/"));
    int LeftPart = OldMainDirectory.length();
    for (int i = 0; i < Locations.size(); i++) {
      if (Settings->GetString(Locations.at(i)).left(LeftPart)==OldMainDirectory) {
        QString TmpStr = Settings->GetString(Locations.at(i));
        TmpStr.remove(OldMainDirectory);
        TmpStr.prepend(QCoreApplication::applicationDirPath());
        Settings->SetValue(Locations.at(i),TmpStr);
      }
    }
    QStringList Files;
    Files << "CurveFileNamesL"
      << "CurveFileNamesLa"
      << "CurveFileNamesLb"
      << "CurveFileNamesSaturation";

    for (int i = 0; i < Files.size(); i++) {
      QStringList FilesList;
      FilesList = Settings->GetStringList(Files.at(i));
      FilesList.replaceInStrings(OldMainDirectory,QCoreApplication::applicationDirPath());
      Settings->SetValue(Files.at(i),FilesList);
    }
  }

  // Instantiate the processor.
  TheProcessor = new dlProcessor(ReportProgress);

  GuiOptions  = new dlGuiOptions();

  // Open and keep open the profile for previewing.
  PreviewColorProfile = cmsCreate_sRGBProfile();

  MainWindow =
    new dlMainWindow(QObject::tr("Lab curves"));

  ViewWindow =
    new dlViewWindow(NULL,MainWindow->ViewFrameCentralWidget);

  HistogramWindow =
    new dlHistogramWindow(NULL,MainWindow->HistogramFrameCentralWidget);

  QPalette BGPal;
  BGPal.setColor(QPalette::Background, QColor(0,0,0));
  ViewWindow->setPalette(BGPal);

  // Different curvewindows.
  QWidget* ParentWidget[] = {MainWindow->LCurveCentralWidget,
                             MainWindow->aCurveCentralWidget,
                             MainWindow->bCurveCentralWidget,
                             MainWindow->SaturationCurveCentralWidget};

  for (short Channel=0; Channel <= dlCurveChannel_Saturation; Channel++) {
    Curve[Channel] = new dlCurve(Channel); // Automatically a null curve.
    CurveWindow[Channel] =
      new dlCurveWindow(Curve[Channel],Channel,ParentWidget[Channel]);
  }

  // Calculate a nice position.
  // Persistent settings.

  QRect DesktopRect = (QApplication::desktop())->screenGeometry(MainWindow);

  MainWindowPos = Settings->m_IniSettings->
          value("MainWindowPos",
                      QPoint(DesktopRect.width()/20,
                             DesktopRect.height()/20)
                     ).toPoint();
  MainWindowSize = Settings->m_IniSettings->
          value("MainWindowSize",
                      QSize(DesktopRect.width()*9/10,
                            DesktopRect.height()*9/10)
                     ).toSize();

  MainWindow->MainSplitter->
    restoreState(Settings->m_IniSettings->
     value("MainSplitter").toByteArray());
  MainWindow->ControlSplitter->
    restoreState(Settings->m_IniSettings->
     value("ControlSplitter").toByteArray());

  MainWindow->resize(MainWindowSize);
  MainWindow->move(MainWindowPos);

  UpdatePreviewImage(NULL,0,1);
  MainWindow->show();

  // Start event loops.
  return TheApplication->exec();
}

////////////////////////////////////////////////////////////////////////////////
//
// Hack : called at t=0 after the event loop started.
//
////////////////////////////////////////////////////////////////////////////////

void CB_Event0() {
  // Init Curves : supposed to be in event loop indeed.
  // (f.i. for progress reporting)

  QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

  // uint16_t (0,0xffff) to float (0.0, 1.0)
#pragma omp parallel for
  for (uint32_t i=0; i<0x10000; i++) {
    ToFloatTable[i] = (float)i/(float)0xffff;
  }

  InitCurves();

  // Open file
  int OpenError = 0;
  uint16_t InputWidth = 0;
  uint16_t InputHeight = 0;
  try {
    Magick::Image image;

    image.ping(Settings->GetString("InputFileName").toAscii().data());

    InputWidth = image.columns();
    InputHeight = image.rows();
  } catch (Magick::Exception &Error) {
    OpenError = 1;
  }

  if (OpenError == 1) {
    QString ErrorMessage = QObject::tr("Cannot decode")
                         + " '"
                         + Settings->GetString("InputFileName")
                         + "'" ;
    QMessageBox::critical(MainWindow,"Decode error",ErrorMessage);
    exit(EXIT_FAILURE);
  } else if (InputWidth < 16 || InputHeight < 16) {
    QMessageBox::critical(MainWindow,"Error","Image too small!");
    exit(EXIT_FAILURE);
  }
  if (TheProcessor->Open() == 0) {
    QMessageBox::critical(MainWindow,"Error","Could not open!");
    exit(EXIT_FAILURE);
  }
  uint16_t LongerSide = InputWidth>InputHeight?InputWidth:InputHeight;
  if (LongerSide > 4800) Settings->SetValue("PipeSize", 3);
  else if (LongerSide > 2400) Settings->SetValue("PipeSize", 2);
  else if (LongerSide > 1200) Settings->SetValue("PipeSize", 1);
  else Settings->SetValue("PipeSize", 0);

  Update(dlProcessorPhase_Scale);
  InStartup = 0;
}

////////////////////////////////////////////////////////////////////////////////
//
// InitCurves
// Bring the curves comboboxes in sync and read the correct curve
// just after initialization.
// (This part needed to be added after the persist settings in QSetting
// where the m_CurveFileNames are inited, but not the associated comboboxes
// nor the reading of the curve)
// In fact we mimick here the one after one reading of the curves
// which also ensures no unreadable curves are there, for instance because
// they were meanwhile removed.
//
////////////////////////////////////////////////////////////////////////////////

void InitCurves() {

  QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

  for (short Channel=0; Channel<=dlCurveChannel_Saturation; Channel++) {

    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

    // All curvefilenames with this channel.
    QStringList CurveFileNames =
      Settings->GetStringList(CurveFileNamesKeys[Channel]);
    // Curve set for this particular channel.
    short SettingsCurve = Settings->GetInt(CurveKeys[Channel]);
    ReportProgress(QObject::tr("Loading curves (") + CurveKeys[Channel] + ")");

    // Start adding for this channel.
    for (short Idx = 0; Idx<CurveFileNames.count(); Idx++) {
      if (!Curve[Channel]) Curve[Channel] = new dlCurve(Channel);
      if (Curve[Channel]->ReadCurve(CurveFileNames[Idx].toAscii().data())) {
        QString ErrorMessage = QObject::tr("Cannot read curve ")
                           + " '"
                           + CurveFileNames[Idx]
                           + "'" ;
        QMessageBox::warning(MainWindow,
                         QObject::tr("Curve read error"),
                         ErrorMessage);

        // Remove this invalid and continue.
        // Some househoding due to removal.
        if (SettingsCurve > dlCurveChoice_File+Idx) {
          SettingsCurve--;
        } else if (SettingsCurve == dlCurveChoice_File+Idx) {
          SettingsCurve=0;
        }
        CurveFileNames.removeAt(Idx);
        Idx--;
        continue;
      }

      // Small routine that lets Shortfilename point to the basename.
      QFileInfo PathInfo(CurveFileNames[Idx]);
      QString ShortFileName = PathInfo.fileName();
      Settings->AddOrReplaceOption(CurveKeys[Channel],
                                   ShortFileName,
                                   dlCurveChoice_File+Idx);
    }

    // We have to write back some stuff to the settins.
    Settings->SetValue(CurveKeys[Channel],SettingsCurve);
    Settings->SetValue(CurveFileNamesKeys[Channel],CurveFileNames);
    // And process now as if just chosen
    // TODO JDLA : Wouldn't this be implied by the setCurrentIndex signal ?
    // Probably not : we're not yet in eventloop.
    CB_CurveChoice(Channel,SettingsCurve);
  }
  ReportProgress(QObject::tr("Ready"));
}

////////////////////////////////////////////////////////////////////////////////
//
// Histogram
//
////////////////////////////////////////////////////////////////////////////////

void HistogramGetCrop() {
  // Get the crop for the histogram
  if (Settings->GetInt("HistogramCrop")) {
      // Allow to be selected in the view window. And deactivate main.
      ViewWindow->AllowSelection(1);
      MainWindow->ControlFrame->setEnabled(0);
      while (ViewWindow->SelectionOngoing()) QApplication::processEvents();
      // Selection is done at this point. Disallow it further and activate main.
      ViewWindow->AllowSelection(0);
      MainWindow->ControlFrame->setEnabled(1);
      short XScale = 1<<Settings->GetInt("PipeSize");
      short YScale = 1<<Settings->GetInt("PipeSize");
      Settings->SetValue("HistogramCropX",
                         ViewWindow->GetSelectionX()*XScale);
      Settings->SetValue("HistogramCropY",
                         ViewWindow->GetSelectionY()*YScale);
      Settings->SetValue("HistogramCropW",
                         ViewWindow->GetSelectionWidth()*XScale);
      Settings->SetValue("HistogramCropH",
                         ViewWindow->GetSelectionHeight()*YScale);
      // Check if the chosen area is large enough
      if (Settings->GetInt("HistogramCropW") < 50 || Settings->GetInt("HistogramCropH") < 50) {
        QMessageBox::information(0,
          QObject::tr("Crop too small"),
          QObject::tr("Crop rectangle too small.\nNo crop, try again."));
        Settings->SetValue("HistogramCropX",0);
        Settings->SetValue("HistogramCropY",0);
        Settings->SetValue("HistogramCropW",0);
        Settings->SetValue("HistogramCropH",0);
        Settings->SetValue("HistogramCrop",0);
      }
  }

  ReportProgress(QObject::tr("Updating histogram"));
  UpdatePreviewImage(NULL,0,1);
  ReportProgress(QObject::tr("Ready"));
}

////////////////////////////////////////////////////////////////////////////////
//
// Status report in Viewwindow
//
////////////////////////////////////////////////////////////////////////////////

void ViewWindowStatusReport(short State) {
  ViewWindow->StatusReport(State);
}

////////////////////////////////////////////////////////////////////////////////
//
// Determine and update the preview image.
//
////////////////////////////////////////////////////////////////////////////////

void UpdatePreviewImage(const dlImage* ForcedImage   /* = NULL  */,
                        const short    OnlyHistogram /* = false */,
                        const short    ForceRun      /* = 0     */) {

  if (!TheProcessor->m_Image_AfterLab) {
    ViewWindow->UpdateView(NULL,1);
    // The splash we want to fit always, but not loosing the
    // m_ZoomMode or m_Zoom setting due to that process.
    int StoredZoom     = Settings->GetInt("Zoom");
    int StoredZoomMode = Settings->GetInt("ZoomMode");
    CB_ZoomFitButton();
    Settings->SetValue("Zoom",StoredZoom);
    Settings->SetValue("ZoomMode",StoredZoomMode);
    return;
  }

  ViewWindow->StatusReport(1);
  ReportProgress(QObject::tr("Updating preview image"));

  // Create PreviewImage if needed and it's not yet there.
  if (!PreviewImage && !OnlyHistogram) PreviewImage = new (dlImage);

  if (!HistogramImage) HistogramImage = new (dlImage);

  // Determine first what is the current image.
  if (!OnlyHistogram) {
    if (Settings->GetInt("PreviewMode") == dlPreviewMode_Tab)
      PreviewImage->Set(TheProcessor->m_Image_AfterScale);
    else
      PreviewImage->Set(TheProcessor->m_Image_AfterLab);
  }

  // View LAB
  if (Settings->GetInt("ViewLAB")) {
    ReportProgress(QObject::tr("View LAB"));
    PreviewImage->ViewLAB(Settings->GetInt("ViewLAB"));
  }

  ReportProgress(QObject::tr("Converting to screen space"));

  PreviewImage->lcmsLabToRGBSimple();

  ReportProgress(QObject::tr("Updating Histogram"));
  HistogramImage->Set(PreviewImage);

  uint16_t Width = 0;
  uint16_t Height = 0;
  uint16_t TempCropX = 0;
  uint16_t TempCropY = 0;
  uint16_t TempCropW = 0;
  uint16_t TempCropH = 0;

  HistogramWindow->UpdateView(HistogramImage);
  // In case of histogram update only, we're done.
  if (OnlyHistogram) {
    Settings->SetValue("PipeIsRunning",0);
    ViewWindow->StatusReport(0);
    return;
  }

  if (Settings->GetInt("HistogramCrop")) {
    short TmpScaled = Settings->GetInt("PipeSize");
    Width = HistogramImage->m_Width;
    Height = HistogramImage->m_Height;
    TempCropX = Settings->GetInt("HistogramCropX")>>TmpScaled;
    TempCropY = Settings->GetInt("HistogramCropY")>>TmpScaled;
    TempCropW = Settings->GetInt("HistogramCropW")>>TmpScaled;
    TempCropH = Settings->GetInt("HistogramCropH")>>TmpScaled;
    if ((((TempCropX) + (TempCropW)) >  Width) ||
        (((TempCropY) + (TempCropH)) >  Height)) {
      QMessageBox::information(MainWindow,
        QObject::tr("Crop outside the image"),
        QObject::tr("Crop rectangle too large.\nNo crop, try again."));
      Settings->SetValue("HistogramCropX",0);
      Settings->SetValue("HistogramCropY",0);
      Settings->SetValue("HistogramCropW",0);
      Settings->SetValue("HistogramCropH",0);
      Settings->SetValue("HistogramCrop",0);
    } else {
      HistogramImage->Crop(TempCropX, TempCropY, TempCropW, TempCropH);
    }
  }
  HistogramWindow->UpdateView(HistogramImage);

  ViewWindow->UpdateView(PreviewImage);
  ViewWindow->StatusReport(0);
  ReportProgress(QObject::tr("Ready"));
}

////////////////////////////////////////////////////////////////////////////////
//
// WriteOut
// Write out in one of the output formats (after applying output profile).
//
////////////////////////////////////////////////////////////////////////////////

void WriteOut() {
  ReportProgress(QObject::tr("Converting to output profile"));
  dlImage* OutImage = TheProcessor->m_Image_AfterLab;

  cmsHPROFILE InProfile = cmsCreateLab4Profile(NULL);

  cmsHPROFILE OutProfile = NULL;
  if (TheProcessor->m_ProfileSize > 0) {
    OutProfile = cmsOpenProfileFromMem(
      TheProcessor->m_ProfileBuffer,
      TheProcessor->m_ProfileSize);
  } else {
    OutProfile = cmsCreate_sRGBProfile();
  }

  if (!OutProfile) {
    OutProfile = cmsCreate_sRGBProfile();
  }

  cmsHTRANSFORM Transform;
  Transform = cmsCreateTransform(InProfile,
                                 TYPE_Lab_16,
                                 OutProfile,
                                 TYPE_RGB_16,
                                 INTENT_PERCEPTUAL,
                                 cmsFLAGS_BLACKPOINTCOMPENSATION);

  int32_t Size = OutImage->m_Width*OutImage->m_Height;
  int32_t Step = 100000;
#pragma omp parallel for schedule(static)
  for (int32_t i = 0; i < Size; i+=Step) {
    int32_t Length = (i+Step)<Size ? Step : Size - i;
    uint16_t* Image = &(OutImage->m_Image[i][0]);
    cmsDoTransform(Transform,Image,Image,Length);
  }

  cmsDeleteTransform(Transform);
  cmsCloseProfile(InProfile);
  cmsCloseProfile(OutProfile);

  ReportProgress(QObject::tr("Writing output"));

  OutImage->dlGMCWriteImage(
      Settings->GetString("OutputFileName").toAscii().data(),
      TheProcessor->m_ProfileBuffer,
      TheProcessor->m_ProfileSize);

  delete OutImage;

  ReportProgress(QObject::tr("Ready"));
}

////////////////////////////////////////////////////////////////////////////////
//
// For convenience...
//
////////////////////////////////////////////////////////////////////////////////

void UpdateSettings() {
  MainWindow->UpdateSettings();
}

////////////////////////////////////////////////////////////////////////////////
//
// Callbacks pertaining to the Menu structure.
// (inclusive 'X' pressing).
//
////////////////////////////////////////////////////////////////////////////////

void CB_MenuFileSaveOutput(const short) {
  if (Settings->GetInt("PipeSize")!=0) {
    Settings->SetValue("JobMode",1);
    TheProcessor->Run(dlProcessorPhase_Scale);
  }

  WriteOut();
  CB_MenuFileExit(1);
}

void CB_MenuFileExit(const short) {
  // TODO Do we need some blabla before exiting ?
  printf("That's al folks ...\n");

  // Disable manual curves when closing
  for (int i = 0; i < CurveKeys.size(); i++) {
    if (Settings->GetInt(CurveKeys.at(i))==dlCurveChoice_Manual)
      Settings->SetValue(CurveKeys.at(i),dlCurveChoice_None);
  }

  Settings->SetValue("HistogramCropX",0);
  Settings->SetValue("HistogramCropY",0);
  Settings->SetValue("HistogramCropW",0);
  Settings->SetValue("HistogramCropH",0);
  Settings->SetValue("HistogramCrop",0);

  // Store the position of the splitter and main window
  Settings->m_IniSettings->
    setValue("MainSplitter",MainWindow->MainSplitter->saveState());
  Settings->m_IniSettings->
    setValue("ControlSplitter",MainWindow->ControlSplitter->saveState());
  Settings->m_IniSettings->setValue("MainWindowPos",MainWindow->pos());
  Settings->m_IniSettings->setValue("MainWindowSize",MainWindow->size());

  // Explicitly. The destructor of it cares for persistent settings.
  delete Settings;

  ALLOCATED(10000000);

  QCoreApplication::exit(EXIT_SUCCESS);
}


////////////////////////////////////////////////////////////////////////////////
//
// Callbacks pertaining to the Zoom function
//
////////////////////////////////////////////////////////////////////////////////

void CB_ZoomFitButton() {

  Settings->SetValue("ZoomMode",dlZoomMode_Fit);
  Settings->SetValue("Zoom",ViewWindow->ZoomFit());

  MainWindow->UpdateSettings(); // To reflect maybe new zoom

  return ;
}

void CB_InputChanged(const QString,const QVariant);
void CB_ZoomFullButton() {
  Settings->SetValue("ZoomMode",dlZoomMode_NonFit);
  CB_InputChanged("ZoomInput",100);
}


void CB_ToGimpButton() {
  CB_MenuFileExit(1);
}

void CB_PipeSizeChoice(const QVariant Choice) {

  short PreviousPipeSize = Settings->GetInt("PipeSize");

  if (Choice == dlPipeSize_Full) {
    if (QMessageBox::question(MainWindow,
      QObject::tr("Are you sure?"),
      QObject::tr("Setting to 1:1 pipe will increase the used ressources.\nAre you sure?"),
        QMessageBox::Ok,QMessageBox::Cancel)==QMessageBox::Cancel){;
      Settings->SetValue("PipeSize",PreviousPipeSize);
      return;
    }
  }

  Settings->SetValue("PipeSize",Choice);
  short PipeSize = Settings->GetInt("PipeSize");
  short Expansion = PreviousPipeSize-PipeSize;

  // Following adaptation is needed for the case spot WB is in place.
  if (Expansion > 0) {
    Settings->SetValue("VisualSelectionX",
                       Settings->GetInt("VisualSelectionX")<<Expansion);
    Settings->SetValue("VisualSelectionY",
                       Settings->GetInt("VisualSelectionY")<<Expansion);
    Settings->SetValue("VisualSelectionWidth",
                       Settings->GetInt("VisualSelectionWidth")<<Expansion);
    Settings->SetValue("VisualSelectionHeight",
                       Settings->GetInt("VisualSelectionHeight")<<Expansion);
  } else {
    Expansion = -Expansion;
    Settings->SetValue("VisualSelectionX",
                       Settings->GetInt("VisualSelectionX")>>Expansion);
    Settings->SetValue("VisualSelectionY",
                       Settings->GetInt("VisualSelectionY")>>Expansion);
    Settings->SetValue("VisualSelectionWidth",
                       Settings->GetInt("VisualSelectionWidth")>>Expansion);
    Settings->SetValue("VisualSelectionHeight",
                       Settings->GetInt("VisualSelectionHeight")>>Expansion);
  }

  Update(dlProcessorPhase_Scale);
  if (Settings->GetInt("ZoomMode") == dlZoomMode_Fit) {
    CB_ZoomFitButton();
  }
}

void CB_PreviewModeButton(const QVariant State) {
  Settings->SetValue("PreviewTabMode",State);
  if (Settings->GetInt("PreviewTabMode")) {
    Settings->SetValue("PreviewMode",dlPreviewMode_Tab);
  } else {
    Settings->SetValue("PreviewMode",dlPreviewMode_End);
  }
  Update(dlProcessorPhase_Output);
}

void CB_RunModeCheck(const QVariant Check) {
  Settings->SetValue("RunMode",Check);
  MainWindow->UpdateSettings();
  if (Settings->GetInt("RunMode")==0) {
    Update(dlProcessorPhase_Output);
  }
}

void CB_RunButton() {
  short OldRunMode = Settings->GetInt("RunMode");
  Settings->SetValue("RunMode",0);
  Update(dlProcessorPhase_Output);
  Settings->SetValue("RunMode",OldRunMode);
}

////////////////////////////////////////////////////////////////////////////////
//
// Callbacks pertaining to the Curves
//
////////////////////////////////////////////////////////////////////////////////

void CB_CurveOpenButton(const int Channel) {

  QStringList CurveFileNames =
    Settings->GetStringList(CurveFileNamesKeys[Channel]);
  int Index = CurveFileNames.count();

  QString CurveFileName = QFileDialog::getOpenFileName(
                            NULL,
                            QObject::tr("Open Curve"),
                            Settings->GetString("CurvesDirectory"),
                            CurveFilePattern);

  if (0 == CurveFileName.size() ) {
    return;
  } else {
    QFileInfo PathInfo(CurveFileName);
    Settings->SetValue("CurvesDirectory",PathInfo.absolutePath());
    CurveFileNames.append(PathInfo.absoluteFilePath());
  }
  if (!Curve[Channel]) Curve[Channel] = new(dlCurve);
  if (Curve[Channel]-> ReadCurve(CurveFileNames[Index].toAscii().data())) {
    QString ErrorMessage = QObject::tr("Cannot read curve ")
                           + " '"
                           + CurveFileNames[Index]
                           + "'" ;
    QMessageBox::warning(MainWindow,
                         QObject::tr("Curve read error"),
                         ErrorMessage);
    // Remove last invalid and return.
    CurveFileNames.removeLast();
    // Write it back to our settings.
    Settings->SetValue(CurveFileNamesKeys[Channel],CurveFileNames);
    return;
  }

  // Remember the maybe changed CurveFileNames list.
  Settings->SetValue(CurveFileNamesKeys[Channel],CurveFileNames);

  // At this moment we have a valid CurveFileName. We have to add it now
  // to the list of possible selections.

  // Small routine that lets Shortfilename point to the basename.
  QFileInfo PathInfo(CurveFileNames[Index]);
  QString ShortFileName = PathInfo.fileName();
  Settings->AddOrReplaceOption(CurveKeys[Channel],
                               ShortFileName,
                               dlCurveChoice_File+Index);
  Settings->SetValue(CurveKeys[Channel],dlCurveChoice_File+Index);

  // And process now as if just chosen
  CB_CurveChoice(Channel,dlCurveChoice_File+Index);
}

void CB_CurveLOpenButton() {
  CB_CurveOpenButton(dlCurveChannel_L);
}

void CB_CurveaOpenButton() {
  CB_CurveOpenButton(dlCurveChannel_a);
}

void CB_CurvebOpenButton() {
  CB_CurveOpenButton(dlCurveChannel_b);
}

void CB_CurveSaturationOpenButton() {
  CB_CurveOpenButton(dlCurveChannel_Saturation);
}

void CB_CurveSaveButton(const int Channel) {
  QString CurveFileName = QFileDialog::getSaveFileName(
                            NULL,
                            QObject::tr("Save Curve"),
                            Settings->GetString("CurvesDirectory"),
                            CurveFilePattern);
  if (0 == CurveFileName.size() ) return;

  QString Header =
    ";\n"
    "; LabCurves Curve File\n"
    ";\n"
    "; This curve was written from within LabCurves\n"
    ";\n"
    "; ";

  bool Success;
  QString Explanation =
    QInputDialog::getText(NULL,
                          QObject::tr("Save Curve"),
                          QObject::tr("Give a description"),
                          QLineEdit::Normal,
                          NULL,
                          &Success);
  if (Success && !Explanation.isEmpty()) {
    Header += Explanation + "\n;\n";
  }

  Curve[Channel]->WriteCurve(CurveFileName.toAscii().data(),
                             Header.toAscii().data());
}

void CB_CurveLSaveButton() {
  CB_CurveSaveButton(dlCurveChannel_L);
}

void CB_CurveaSaveButton() {
  CB_CurveSaveButton(dlCurveChannel_a);
}

void CB_CurvebSaveButton() {
  CB_CurveSaveButton(dlCurveChannel_b);
}

void CB_CurveSaturationSaveButton() {
  CB_CurveSaveButton(dlCurveChannel_Saturation);
}

void CB_CurveChoice(const int Channel, const int Choice) {
  // Save the old Curve
  if (Settings->GetInt(CurveKeys.at(Channel))==dlCurveChoice_Manual) {
    if (!BackupCurve[Channel]) BackupCurve[Channel] = new dlCurve();
    BackupCurve[Channel]->Set(Curve[Channel]);
  }

  // Restore the saved curve
  if (Settings->GetInt(CurveKeys.at(Channel))!=dlCurveChoice_Manual &&
      Choice == dlCurveChoice_Manual) {
    if (BackupCurve[Channel])
      Curve[Channel]->Set(BackupCurve[Channel]);
    else
      Curve[Channel]->SetNullCurve(Channel);
  }

  // Set the newly choosen curve.
  Settings->SetValue(CurveKeys[Channel],Choice);

  QStringList CurveFileNames =
    Settings->GetStringList(CurveFileNamesKeys[Channel]);

  // If we have a curve, go for reading it.
  if (Choice >= dlCurveChoice_File) {
    if (!Curve[Channel]) Curve[Channel] = new(dlCurve);
    // At this stage, as we have checked on loading the curves
    // we assume the curve can be read. OK, if user has meanwhile
    // removed it this might go wrong, but then we simply die.
    if (Curve[Channel]->
        ReadCurve(CurveFileNames[Choice-dlCurveChoice_File].
                  toAscii().data())){
      assert(0);
    }
  }

  if (Choice == dlCurveChoice_None) {
    Curve[Channel]->SetNullCurve(Channel);
  }

  // Update the View.
  QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
  CurveWindow[Channel]->UpdateView(Curve[Channel]);

  // Run the graphical pipe according to a changed curve.
  if (!InStartup)
    switch(Channel) {
      case dlCurveChannel_L :
      case dlCurveChannel_a :
      case dlCurveChannel_b :
      case dlCurveChannel_Saturation :
        Update(dlProcessorPhase_Lab);
        break;
      default :
        assert(0);
    }
  if (InStartup) MainWindow->UpdateSettings();
}

void CB_CurveLChoice(const QVariant Choice) {
  CB_CurveChoice(dlCurveChannel_L,Choice.toInt());
}

void CB_CurveLaChoice(const QVariant Choice) {
  CB_CurveChoice(dlCurveChannel_a,Choice.toInt());
}

void CB_CurveLbChoice(const QVariant Choice) {
  CB_CurveChoice(dlCurveChannel_b,Choice.toInt());
}

void CB_CurveSaturationChoice(const QVariant Choice) {
  CB_CurveChoice(dlCurveChannel_Saturation,Choice.toInt());
}

void CB_CurveWindowRecalc(const short Channel) {

  // Run the graphical pipe according to a changed curve.
  switch(Channel) {
    case dlCurveChannel_L :
    case dlCurveChannel_a :
    case dlCurveChannel_b :
    case dlCurveChannel_Saturation :
      Update(dlProcessorPhase_Lab);
      break;
    default :
      assert(0);
  }
}

void CB_CurveWindowManuallyChanged(const short Channel) {

  // Combobox and curve choice has to be adapted to manual.
  Settings->SetValue(CurveKeys[Channel],dlCurveChoice_Manual);
  // Run the graphical pipe according to a changed curve.
  CB_CurveWindowRecalc(Channel);
}

void CB_WritePipeButton() {
  CB_MenuFileSaveOutput(1);
}

////////////////////////////////////////////////////////////////////////////////
//
// Callback for view LAB
//
////////////////////////////////////////////////////////////////////////////////

void CB_ViewLABChoice(const QVariant Choice) {
  Settings->SetValue("ViewLAB",Choice);
  Update(dlProcessorPhase_Output);
}

////////////////////////////////////////////////////////////////////////////////
//
// Callback dispatcher
//
////////////////////////////////////////////////////////////////////////////////

void CB_InputChanged(const QString ObjectName, const QVariant Value) {

  // No CB processing while in startup phase. Too much
  // noise events otherwise.
  if (InStartup) return;

  if (ObjectName == "ZoomInput") {
    Settings->SetValue("ZoomMode",dlZoomMode_NonFit);
    Settings->SetValue("Zoom",Value.toInt());
    ViewWindow->Zoom(Value.toInt());
    MainWindow->UpdateSettings(); // To reflect maybe new zoom

  #define M_Dispatch(Name)\
  } else if (ObjectName == #Name) { CB_ ## Name (Value);

  M_Dispatch(PipeSizeChoice)
  M_Dispatch(RunModeCheck)

  M_Dispatch(CurveLChoice)
  M_Dispatch(CurveLaChoice)
  M_Dispatch(CurveLbChoice)
  M_Dispatch(CurveSaturationChoice)

  M_Dispatch(ViewLABChoice)

  } else {
    fprintf(stderr,"(%s,%d) Unexpected ObjectName '%s'\n",
            __FILE__,__LINE__,ObjectName.toAscii().data());
    assert(0);
  }
}

////////////////////////////////////////////////////////////////////////////////
