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

#include <QFileInfo>
#include <QApplication>

#include "assert.h"

#include "dlConstants.h"
#include "dlError.h"
#include "dlSettings.h"
#include "dlCurve.h"

#include "dlProcessor.h"

////////////////////////////////////////////////////////////////////////////////
//
// Constructor
//
////////////////////////////////////////////////////////////////////////////////

dlProcessor::dlProcessor(void (*ReportProgress)(const QString Message)) {

  // We work with a callback to avoid dependency on dlMainWindow
  m_ReportProgress = ReportProgress;

  // Cached version at different points.
  m_Image_AfterOpen        = NULL;
  m_Image_AfterScale       = NULL;
  m_Image_AfterLab         = NULL;

  //
  m_ProfileSize       = 0;
  m_ProfileBuffer     = NULL;
}

// Prototype for status report in Viewwindow
void ViewWindowStatusReport(short State);

////////////////////////////////////////////////////////////////////////////////
//
// Open
//
////////////////////////////////////////////////////////////////////////////////

int dlProcessor::Open() {
  m_ReportProgress(QObject::tr("Opening the image..."));
  int Success = 0;
  QTime Timer;
  Timer.start();
  m_Image_AfterOpen = new dlImage();
  m_Image_AfterOpen->dlGMOpenImage(
    Settings->GetString("InputFileName").toAscii().data(),
    m_ProfileSize, m_ProfileBuffer, Success);

  TRACEMAIN("opened image at %d ms.",Timer.elapsed());

  return Success;
}

////////////////////////////////////////////////////////////////////////////////
//
// Main Graphical Pipe.
// Look here for all operations and all possible future extensions.
// As well Gui mode as JobMode are sharing this part of the code.
// The idea is to have an image object and operating on it.
// Run the graphical pipe from a certain point on.
//
////////////////////////////////////////////////////////////////////////////////

void dlProcessor::Run(short Phase,
                      short SubPhase,
                      short WithIdentify,
                      short ProcessorMode) {

  printf("(%s,%d) %s\n",__FILE__,__LINE__,__PRETTY_FUNCTION__);

  // Purposes of timing the lenghty operations.
  QTime Timer;
  Timer.start();

  // Status report
  ::ViewWindowStatusReport(2);

  switch(Phase) {
    case dlProcessorPhase_Scale :

      if (Settings->GetInt("JobMode")) {
        m_Image_AfterScale = m_Image_AfterOpen; // Job mode -> no cache
      } else {
        if (!m_Image_AfterScale) m_Image_AfterScale = new dlImage();
        m_Image_AfterScale->Set(m_Image_AfterOpen);
      }

      if (Settings->GetInt("JobMode")==0) {
        m_ReportProgress(QObject::tr("Scaling"));

        m_Image_AfterScale->Bin(Settings->GetInt("PipeSize"));

        TRACEMAIN("Done scaling at %d ms.",Timer.elapsed());
      }

    case dlProcessorPhase_Lab :

      if (Settings->GetInt("JobMode")) {
        m_Image_AfterLab = m_Image_AfterScale; // Job mode -> no cache
      } else {
        if (!m_Image_AfterLab) m_Image_AfterLab = new dlImage();
        m_Image_AfterLab->Set(m_Image_AfterScale);
      }

      // L Curve

      if (Settings->GetInt("CurveL")) {
        m_ReportProgress(QObject::tr("Applying L curve"));

        m_Image_AfterLab->ApplyCurve(Curve[dlCurveChannel_L],1);

        TRACEMAIN("Done L Curve at %d ms.",Timer.elapsed());
      }

      // a Curve

      if (Settings->GetInt("CurveLa")) {
        m_ReportProgress(QObject::tr("Applying a curve"));

        m_Image_AfterLab->ApplyCurve(Curve[dlCurveChannel_a],2);

        TRACEMAIN("Done a Curve at %d ms.",Timer.elapsed());
      }

      // b Curve

      if (Settings->GetInt("CurveLb")) {
        m_ReportProgress(QObject::tr("Applying b curve"));

        m_Image_AfterLab->ApplyCurve(Curve[dlCurveChannel_b],4);

        TRACEMAIN("Done b Curve at %d ms.",Timer.elapsed());
      }

      // Saturation Curve

      if (Settings->GetInt("CurveSaturation")) {
        m_ReportProgress(QObject::tr("Applying saturation curve"));

        m_Image_AfterLab->ApplySaturationCurve(Curve[dlCurveChannel_Saturation],
                                               Settings->GetInt("SatCurveMode"),
                                               Settings->GetInt("SatCurveType"));

        TRACEMAIN("Done saturation Curve at %d ms.",Timer.elapsed());
      }

      Settings->SetValue("PipeImageW",m_Image_AfterLab->m_Width);
      Settings->SetValue("PipeImageH",m_Image_AfterLab->m_Height);

    case dlProcessorPhase_Output : // Run Output.


Exit:

      TRACEMAIN("Done pipe processing at %d ms.",Timer.elapsed());

      break;

    default : // Should not happen.
      assert(0);
  }

  m_ReportProgress(QObject::tr("Ready"));
}

////////////////////////////////////////////////////////////////////////////////
//
// Destructor
//
////////////////////////////////////////////////////////////////////////////////

dlProcessor::~dlProcessor() {
  // Tricky delete stuff as some pointer might be shared.
  QList <dlImage*> PointerList;
  PointerList << m_Image_AfterOpen
              << m_Image_AfterScale
              << m_Image_AfterLab;
  while(PointerList.size()) {
    dlImage* CurrentPointer = PointerList[0];
    delete CurrentPointer;
    // Remove all elements equal to CurrentPointer.
    short Index=0;
    while (Index<PointerList.size()) {
      if (CurrentPointer == PointerList[Index]) {
        PointerList.removeAt(Index);
      } else {
        Index++;
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
