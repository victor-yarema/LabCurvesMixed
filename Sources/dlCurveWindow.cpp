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

#include "dlCurveWindow.h"
#include "dlSettings.h"
#include <assert.h>

#include <iostream>
using namespace std;

void CB_CurveWindowManuallyChanged(const short Channel);
void CB_CurveWindowRecalc(const short Channel);

////////////////////////////////////////////////////////////////////////////////
//
// Constructor.
//
// Instantiates a (also here defined) CurveWidget,
// which acts as a central widget where the operations are finally done upon.
//
////////////////////////////////////////////////////////////////////////////////

dlCurveWindow::dlCurveWindow(dlCurve*    RelatedCurve,
                             const short Channel,
                             QWidget*    Parent)

  :QWidget(NULL) {

  m_RelatedCurve = RelatedCurve;
  m_Channel      = Channel;

  // Sizing and layout related.

  QSizePolicy Policy(QSizePolicy::Minimum, QSizePolicy::Preferred);
  Policy.setHeightForWidth(1);
  setSizePolicy(Policy);

  QVBoxLayout *Layout = new QVBoxLayout;
  Layout->setContentsMargins(0,0,0,0);
  Layout->setSpacing(0);
  Layout->addWidget(this);
  Parent->setLayout(Layout);
  m_Parent = Parent;

  // Some other dynamic members we want to have clean.
  m_OverlayAnchorX = 0;
  m_OverlayAnchorY = 0;
  m_QPixmap        = NULL;
  m_Image8         = NULL;
  // Nothing moving. (Event handling)
  m_MovingAnchor   = -1;
  m_BlockEvents    = 0;
  m_RecalcNeeded   = 0;

  // Timer to delay on resize operations.
  // (avoiding excessive calculations)
  m_ResizeTimer = new QTimer(this);
  m_ResizeTimer->setSingleShot(1);
  connect(m_ResizeTimer,
          SIGNAL(timeout()),
          this,
          SLOT(ResizeTimerExpired()));

  m_AtnAdaptive = new QAction(QObject::tr("Adaptive"), this);
  m_AtnAdaptive->setStatusTip(QObject::tr("Adaptive saturation"));
  m_AtnAdaptive->setCheckable(true);
  connect(m_AtnAdaptive, SIGNAL(triggered()), this, SLOT(SetSatMode()));
  m_AtnAbsolute = new QAction(QObject::tr("Absolute"), this);
  m_AtnAbsolute->setStatusTip(QObject::tr("Absolute saturation"));
  m_AtnAbsolute->setCheckable(true);
  connect(m_AtnAbsolute, SIGNAL(triggered()), this, SLOT(SetSatMode()));

  m_SatModeGroup = new QActionGroup(this);
  m_SatModeGroup->addAction(m_AtnAdaptive);
  m_SatModeGroup->addAction(m_AtnAbsolute);
  m_AtnAdaptive->setChecked(Settings->GetInt("SatCurveMode")>0?true:false);
  m_AtnAbsolute->setChecked(Settings->GetInt("SatCurveMode")>0?false:true);

  m_AtnByLuma = new QAction(QObject::tr("By luminance"), this);
  m_AtnByLuma->setStatusTip(QObject::tr("Saturation by luminance"));
  m_AtnByLuma->setCheckable(true);
  connect(m_AtnByLuma, SIGNAL(triggered()), this, SLOT(SetSatType()));
  m_AtnByChroma = new QAction(QObject::tr("By color"), this);
  m_AtnByChroma->setStatusTip(QObject::tr("Saturation by color"));
  m_AtnByChroma->setCheckable(true);
  connect(m_AtnByChroma, SIGNAL(triggered()), this, SLOT(SetSatType()));

  m_SatTypeGroup = new QActionGroup(this);
  m_SatTypeGroup->addAction(m_AtnByLuma);
  m_SatTypeGroup->addAction(m_AtnByChroma);
  m_AtnByLuma->setChecked(Settings->GetInt("SatCurveType")>0?true:false);
  m_AtnByChroma->setChecked(Settings->GetInt("SatCurveType")>0?false:true);

  m_AtnITLinear = new QAction(QObject::tr("Linear"), this);
  m_AtnITLinear->setStatusTip(QObject::tr("Linear interpolation"));
  m_AtnITLinear->setCheckable(true);
  connect(m_AtnITLinear, SIGNAL(triggered()), this, SLOT(SetInterpolationType()));
  m_AtnITSpline = new QAction(QObject::tr("Spline"), this);
  m_AtnITSpline->setStatusTip(QObject::tr("Spline interpolation"));
  m_AtnITSpline->setCheckable(true);
  connect(m_AtnITSpline, SIGNAL(triggered()), this, SLOT(SetInterpolationType()));

  m_ITGroup = new QActionGroup(this);
  m_ITGroup->addAction(m_AtnITLinear);
  m_ITGroup->addAction(m_AtnITSpline);
  m_AtnITSpline->setChecked(
    m_RelatedCurve->m_IntType==dlCurveIT_Spline?true:false);
  m_AtnITLinear->setChecked(
    m_RelatedCurve->m_IntType==dlCurveIT_Linear?true:false);
}

////////////////////////////////////////////////////////////////////////////////
//
// Destructor.
//
////////////////////////////////////////////////////////////////////////////////

dlCurveWindow::~dlCurveWindow() {
  //printf("(%s,%d) %s\n",__FILE__,__LINE__,__PRETTY_FUNCTION__);
  delete m_QPixmap;
  delete m_Image8;
}

////////////////////////////////////////////////////////////////////////////////
//
// resizeEvent.
// Delay a resize via a timer.
// ResizeTimerExpired upon expiration.
//
////////////////////////////////////////////////////////////////////////////////

void dlCurveWindow::resizeEvent(QResizeEvent*) {
  // Schedule the action 500ms from here to avoid multiple rescaling actions
  // during multiple resizeEvents from a window resized by the user.
  m_ResizeTimer->start(200); // 500 ms.
}

void dlCurveWindow::ResizeTimerExpired() {
  // m_RelatedCurve enforces update, even if it is the same image.
  UpdateView(m_RelatedCurve);
}

////////////////////////////////////////////////////////////////////////////////
//
// Context menu.
//
////////////////////////////////////////////////////////////////////////////////

void dlCurveWindow::ContextMenu(QMouseEvent* event) {
  short TempSetting = 0;
  switch (m_Channel) {
    case dlCurveChannel_L :
      TempSetting = Settings->GetInt("CurveL"); break;
    case dlCurveChannel_a :
      TempSetting = Settings->GetInt("CurveLa"); break;
    case dlCurveChannel_b :
      TempSetting = Settings->GetInt("CurveLb"); break;
    case dlCurveChannel_Saturation :
      TempSetting = Settings->GetInt("CurveSaturation"); break;
    default :
      assert(0);
  }
  if (m_Channel != dlCurveChannel_Saturation &&
      !(TempSetting==dlCurveChoice_Manual ||
        TempSetting==dlCurveChoice_None) )
    return;

  QMenu Menu(this);
  if ((TempSetting==dlCurveChoice_Manual ||
        TempSetting==dlCurveChoice_None)) {
    m_AtnITSpline->setChecked(
      m_RelatedCurve->m_IntType==dlCurveIT_Spline?true:false);
    Menu.addAction(m_AtnITLinear);
    Menu.addAction(m_AtnITSpline);
  }
  if (m_Channel == dlCurveChannel_Saturation) {
    m_AtnAdaptive->setChecked(Settings->GetInt("SatCurveMode")>0?true:false);
    //~ m_AtnAbsolute->setChecked(Settings->GetInt("SatCurveMode")>0?false:true);
    if ((TempSetting==dlCurveChoice_Manual ||
        TempSetting==dlCurveChoice_None))
      Menu.addSeparator();
    Menu.addAction(m_AtnAbsolute);
    Menu.addAction(m_AtnAdaptive);
    Menu.addSeparator();
    Menu.addAction(m_AtnByLuma);
    Menu.addAction(m_AtnByChroma);
  }

  Menu.exec(event->globalPos());
}

void dlCurveWindow::SetSatMode() {
  if (Settings->GetInt("SatCurveMode") == (int)m_AtnAdaptive->isChecked())
    return;
  Settings->SetValue("SatCurveMode",(int)m_AtnAdaptive->isChecked());

  if (Settings->GetInt("CurveSaturation")==dlCurveChoice_None) return;
  if (m_BlockEvents) return;

  m_BlockEvents  = 1;
  CB_CurveWindowRecalc(m_Channel);
  m_RecalcNeeded = 0;
  m_BlockEvents  = 0;

  return;
}

void dlCurveWindow::SetSatType() {
  if (Settings->GetInt("SatCurveType") == (int)m_AtnByLuma->isChecked())
    return;
  Settings->SetValue("SatCurveType",(int)m_AtnByLuma->isChecked());

  CalculateCurve();
  UpdateView();

  if (Settings->GetInt("CurveSaturation")==dlCurveChoice_None) return;
  if (m_BlockEvents) return;

  m_BlockEvents  = 1;
  CB_CurveWindowRecalc(m_Channel);
  m_RecalcNeeded = 0;
  m_BlockEvents  = 0;

  return;
}

void dlCurveWindow::SetInterpolationType() {
  short Temp = 0;
  if ((int)m_AtnITLinear->isChecked())
    Temp = dlCurveIT_Linear;
  if ((int)m_AtnITSpline->isChecked())
    Temp = dlCurveIT_Spline;
  if (m_RelatedCurve->m_IntType==Temp) return;

  m_RelatedCurve->m_IntType = Temp;

  if (m_BlockEvents) return;

  m_BlockEvents  = 1;
  m_RelatedCurve->SetCurveFromAnchors();
  UpdateView();
  CB_CurveWindowRecalc(m_Channel);
  m_RecalcNeeded = 0;
  m_BlockEvents  = 0;

  return;
}

////////////////////////////////////////////////////////////////////////////////
//
// CalculateCurve.
//
// Calculates the curve into an m_Image8.
//
////////////////////////////////////////////////////////////////////////////////

void dlCurveWindow::CalculateCurve() {

  if (!m_RelatedCurve) return;

  int Width  = width();
  int Height = height();

  uint16_t LocalCurve[Width];

  // Zero the LocalCurve
  memset(LocalCurve,0,sizeof(LocalCurve));

  // Compute it. Take already the Y axis going down into account.
  for (uint16_t x=0; x<Width; x++) {
    uint16_t CurveX = (uint16_t)( 0.5 + (double)x/(Width-1) * 0xffff );
    LocalCurve[x] = Height-1- (uint16_t)
     ( 0.5 + (double) m_RelatedCurve->m_Curve[CurveX]/0xffff * (Height-1));
  }

  delete m_Image8;
  m_Image8 = new dlImage8(Width,Height,3);

  // Colors from the palette should give more consistent results.
  QColor FGColor = QColor(200,200,200);//palette().color(QPalette::WindowText);
  QColor BGColor = QColor(0,0,0);//palette().color(QPalette::Window);
  QColor MColor  = QColor(53,53,53);//palette().color(QPalette::Mid);

  // Gradient for saturation curve
  if (m_Channel == dlCurveChannel_Saturation) {
    if ((int)m_AtnByLuma->isChecked() == 1) {
      for (uint16_t i=0;i<Width;i++) {
        int Value = (int)(i/(float)Width*255);
        for (uint16_t Row = Height-Height/20;
             Row <= Height-2;
             Row++) {
          m_Image8->m_Image[Row*Width+i][0] = Value;
          m_Image8->m_Image[Row*Width+i][1] = Value;
          m_Image8->m_Image[Row*Width+i][2] = Value;
        }
      }
    } else {
      for (uint16_t i=0;i<Width;i++) {
        int ValueR = 0;
        int ValueG = 0;
        int ValueB = 0;
        if (i < Width/4) {
          ValueR = 255;
          ValueG = (int) (255*i/(Width/4));
        } else if (i < Width/2) {
          ValueR = 255-(int) (255*(i-Width/4)/(Width/4));
          ValueG = 255;
        } else if (i < 3*Width/4) {
          ValueG = 255-(int) (255*(i-Width/2)/(Width/4));
          ValueB = (int) (255*(i-Width/2)/(Width/4));
        } else if (i < Width) {
          ValueR = (int) (255*(i-3*Width/4)/(Width/4));
          ValueB = 255-(int) (255*(i-3*Width/4)/(Width/4));
        }
        for (uint16_t Row = Height-Height/20;
             Row <= Height-2;
             Row++) {
          m_Image8->m_Image[Row*Width+i][0] = ValueB;
          m_Image8->m_Image[Row*Width+i][1] = ValueG;
          m_Image8->m_Image[Row*Width+i][2] = ValueR;
        }
      }
    }
  }

  // Grid lines.
  for (uint16_t Count = 0, Row = Height-1;
       Count <= 10;
       Count++, Row = Height-1-Count*(Height-1)/10) {
    for (uint16_t i=0;i<Width;i++) {
      m_Image8->m_Image[Row*Width+i][0] = MColor.blue();
      m_Image8->m_Image[Row*Width+i][1] = MColor.green();
      m_Image8->m_Image[Row*Width+i][2] = MColor.red();
    }
  }
  for (uint16_t Count = 0, Column = 0;
       Count <= 10;
       Count++, Column = Count*(Width-1)/10) {
    for (uint16_t i=0;i<Height;i++) {
      m_Image8->m_Image[i*Width+Column][0] = MColor.blue();
      m_Image8->m_Image[i*Width+Column][1] = MColor.green();
      m_Image8->m_Image[i*Width+Column][2] = MColor.red();
    }
  }

  // The curve
  for (uint16_t i=0; i<Width; i++) {
    int32_t Row      = LocalCurve[i];
    int32_t NextRow  = LocalCurve[(i<(Width-1))?i+1:i];
    uint16_t kStart = MIN(Row,NextRow);
    uint16_t kEnd   = MAX(Row,NextRow);
    for(uint16_t k=kStart;k<=kEnd;k++) {
      m_Image8->m_Image[k*Width+i][0] = FGColor.blue();
      m_Image8->m_Image[k*Width+i][1] = FGColor.green();
      m_Image8->m_Image[k*Width+i][2] = FGColor.red();
    }
  }

  // Anchors in case of anchored curve.
  if (m_RelatedCurve->m_Type == dlCurveType_Anchor) {
    for (short Anchor=0;Anchor<m_RelatedCurve->m_NrAnchors;Anchor++) {
      int32_t XSpot =
        (uint16_t) (.5 + m_RelatedCurve->m_XAnchor[Anchor]*(Width-1));
      int32_t YSpot =
        (uint16_t) (.5 + Height-1-m_RelatedCurve->m_YAnchor[Anchor]*(Height-1));
      // Remember it for faster event detection.
      m_XSpot[Anchor] = XSpot;
      m_YSpot[Anchor] = YSpot;
      for (int32_t Row=YSpot-3; Row<YSpot+4 ; Row++) {
        if (Row>=Height) continue;
        if (Row<0) continue;
        for (int32_t Column=XSpot-3; Column<XSpot+4; Column++) {
           if (Column>=Width) continue;
           if (Column<0) continue;
           m_Image8->m_Image[Row*Width+Column][0] = FGColor.blue();
           m_Image8->m_Image[Row*Width+Column][1] = FGColor.green();
           m_Image8->m_Image[Row*Width+Column][2] = FGColor.red();
        }
      }
    }
  }

  // If we have an anchor moving around, show it too.
  if (m_MovingAnchor != -1) {
    for (int32_t Row=m_OverlayAnchorY-3; Row<m_OverlayAnchorY+4 ; Row++) {
      if (Row>=Height) continue;
      if (Row<0) continue;
      for (int32_t Column=m_OverlayAnchorX-3; Column<m_OverlayAnchorX+4;
           Column++) {
        if (Column>=Width) continue;
        if (Column<0) continue;
        m_Image8->m_Image[Row*Width+Column][0] = FGColor.blue();
        m_Image8->m_Image[Row*Width+Column][1] = FGColor.green();
        m_Image8->m_Image[Row*Width+Column][2] = FGColor.red();
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
//
// UpdateView.
//
////////////////////////////////////////////////////////////////////////////////

void dlCurveWindow::UpdateView(dlCurve* NewRelatedCurve) {

  if (NewRelatedCurve) m_RelatedCurve = NewRelatedCurve;
  if (!m_RelatedCurve) return;

  CalculateCurve();

  // The detour QImage=>QPixmap is needed to enjoy
  // HW acceleration of QPixmap.
  if (!m_Image8) return;
  delete m_QPixmap;
  m_QPixmap = new QPixmap(
   QPixmap::fromImage(QImage((const uchar*) m_Image8->m_Image,
                             m_Image8->m_Width,
                             m_Image8->m_Height,
                             QImage::Format_RGB32)));
  repaint();
}

////////////////////////////////////////////////////////////////////////////////
//
// paintEvent handler.
// Just draw the previously constructed m_QPixmap.
//
////////////////////////////////////////////////////////////////////////////////

void dlCurveWindow::paintEvent(QPaintEvent*) {
  QPainter Painter(this);
  Painter.save();
  if (m_QPixmap) Painter.drawPixmap(0,0,*m_QPixmap);
  Painter.restore();
}

// How many pixels will be considered as 'bingo' for having the anchor ?
const short SnapDelta = 6;
// Percentage to be close to a curve to get a new Anchor
const double CurveDelta = 0.12;

////////////////////////////////////////////////////////////////////////////////
//
// mousePressEvent handler.
// Implements part of the anchors creation/deletion/moving.
//
////////////////////////////////////////////////////////////////////////////////

void dlCurveWindow::mousePressEvent(QMouseEvent *Event) {

  if (m_BlockEvents) return;

  // Do only hanlde curves with minimum 2 anchors.
  if (m_RelatedCurve->m_Type != dlCurveType_Anchor) return;
  if (m_RelatedCurve->m_NrAnchors < 2) return;

  short Xd;
  short Yd;
  short i;
  short Snapped;

  Snapped = 0;
  // Did we snap one of the anchors ?
  for (i=0; i<m_RelatedCurve->m_NrAnchors; i++) {
    Xd =  abs(m_XSpot[i]-Event->x());
    Yd =  abs(m_YSpot[i]-Event->y());
    if ((Xd<SnapDelta) && (Yd<SnapDelta)) {
      Snapped = 1;
      if (Event->buttons() == 1) { // Left mouse. Start moving.
        m_MovingAnchor = i;
        m_RecalcNeeded = 1;
      }
      if (Event->buttons() == 2) { // Right mouse. Delete.
        // Delete indeed if still more than 2 anchors.
        if (m_RelatedCurve->m_NrAnchors > 2) {
          short j;
          for (j=i;j<m_RelatedCurve->m_NrAnchors-1;j++) {
            m_RelatedCurve->m_XAnchor[j]=m_RelatedCurve->m_XAnchor[j+1];
            m_RelatedCurve->m_YAnchor[j]=m_RelatedCurve->m_YAnchor[j+1];
          }
          m_RelatedCurve->m_NrAnchors--;
          m_RelatedCurve->SetCurveFromAnchors();
          // Notify we have a manual curve now ...
          switch (m_Channel) {
            case dlCurveChannel_L :
              Settings->SetValue("CurveL",dlCurveChoice_Manual); break;
            case dlCurveChannel_a :
              Settings->SetValue("CurveLa",dlCurveChoice_Manual); break;
            case dlCurveChannel_b :
              Settings->SetValue("CurveLb",dlCurveChoice_Manual); break;
            case dlCurveChannel_Saturation :
              Settings->SetValue("CurveSaturation",dlCurveChoice_Manual); break;
            default :
              assert(0);
          }
          UpdateView();
          m_RecalcNeeded = 1;
        }
      }
      break;
    }
  }

  // Insert a new anchor ? (Right mouse but not on an anchor).
  if (m_RelatedCurve->m_NrAnchors < dlMaxAnchors &&
      !Snapped && Event->buttons() == 2 &&
      // Close to the curve or far away?
      fabs(((double)m_RelatedCurve->
            m_Curve[(int32_t)((double)Event->x()/(double)width()*0xffff)]
             /(double)0xffff) -
            ((height()-Event->y())/(double)height())) < CurveDelta) {
    // Find out where to insert. (Initially the average of the
    // neighbouring anchors).
    if (Event->x() < m_XSpot[0]) {
      for (short j=m_RelatedCurve->m_NrAnchors; j>0 ; j--) {
        m_RelatedCurve->m_XAnchor[j] = m_RelatedCurve->m_XAnchor[j-1];
        m_RelatedCurve->m_YAnchor[j] = m_RelatedCurve->m_YAnchor[j-1];
      }
      m_RelatedCurve->m_XAnchor[0] = (double)Event->x()/(double)width();
      m_RelatedCurve->m_YAnchor[0] =
        m_RelatedCurve->m_Curve[
          (int32_t)(m_RelatedCurve->m_XAnchor[0]*0xffff)]/(double)0xffff;
      m_RecalcNeeded = 1;
    } else if (Event->x() > m_XSpot[m_RelatedCurve->m_NrAnchors-1]) {
      m_RelatedCurve->m_XAnchor[m_RelatedCurve->m_NrAnchors] =
        (double)Event->x()/(double)width();
      m_RelatedCurve->m_YAnchor[m_RelatedCurve->m_NrAnchors] =
        m_RelatedCurve->m_Curve[(int32_t)(m_RelatedCurve->m_XAnchor[
          m_RelatedCurve->m_NrAnchors]*0xffff)]/(double)0xffff;
      m_RecalcNeeded = 1;
    } else {
      for (i=0; i<m_RelatedCurve->m_NrAnchors-1; i++) {
        if (Event->x()>m_XSpot[i] && Event->x()<m_XSpot[i+1]) {
          for (short j=m_RelatedCurve->m_NrAnchors; j>i+1 ; j--) {
            m_RelatedCurve->m_XAnchor[j] = m_RelatedCurve->m_XAnchor[j-1];
            m_RelatedCurve->m_YAnchor[j] = m_RelatedCurve->m_YAnchor[j-1];
          }
          m_RelatedCurve->m_XAnchor[i+1] =
              (double)Event->x()/(double)width();
          m_RelatedCurve->m_YAnchor[i+1] =
            m_RelatedCurve->m_Curve[
              (int32_t)(m_RelatedCurve->m_XAnchor[i+1]*0xffff)]/(double)0xffff;
          break;
        }
      }
    }
    m_RelatedCurve->m_NrAnchors++;
    m_RelatedCurve->SetCurveFromAnchors();
    // Notify we have a manual curve now ...
    switch (m_Channel) {
      case dlCurveChannel_L :
        Settings->SetValue("CurveL",dlCurveChoice_Manual); break;
      case dlCurveChannel_a :
        Settings->SetValue("CurveLa",dlCurveChoice_Manual); break;
      case dlCurveChannel_b :
        Settings->SetValue("CurveLb",dlCurveChoice_Manual); break;
      case dlCurveChannel_Saturation :
        Settings->SetValue("CurveSaturation",dlCurveChoice_Manual); break;
      default :
        assert(0);
    }
    UpdateView();
  } else if (!Snapped && Event->buttons() == 2) {
    // ContextMenu
    ContextMenu((QMouseEvent*)Event);
  }
  return;
}

////////////////////////////////////////////////////////////////////////////////
//
// mouseMoveEvent handler.
// Move anchor around.
//
////////////////////////////////////////////////////////////////////////////////

const float Delta = 0.005;

void dlCurveWindow::mouseMoveEvent(QMouseEvent *Event) {

  if (m_BlockEvents) return;

  // Do only hanlde curves with minimum 2 anchors.
  if (m_RelatedCurve->m_Type != dlCurveType_Anchor) return;
  if (m_RelatedCurve->m_NrAnchors < 2) return;

  int Width  = width();
  int Height = height();

  if (m_MovingAnchor != -1) {
    double X =  Event->x()/(double) (Width-1);
    double Y = 1.0 - Event->y()/(double)(Height-1);

    // Handle mouse out of range X coordinate
    if (m_MovingAnchor == 0) {
      if (Event->x() >= m_XSpot[1]) {
        X = (m_XSpot[1]/(double) (Width-1)) - Delta;
      }
      X = MAX(0.0, X);
    } else if (m_MovingAnchor == m_RelatedCurve->m_NrAnchors-1)  {
      if (Event->x()<=m_XSpot[m_RelatedCurve->m_NrAnchors-2]) {
        X = (m_XSpot[m_RelatedCurve->m_NrAnchors-2]/(double) (Width-1)) + Delta;
      }
      X=MIN(1.0,X);
    } else if (Event->x()>=m_XSpot[m_MovingAnchor+1]) {
      X = (m_XSpot[m_MovingAnchor+1]/(double) (Width-1)) - Delta;
    } else if (Event->x()<=m_XSpot[m_MovingAnchor-1]) {
      X = (m_XSpot[m_MovingAnchor-1]/(double) (Width-1)) + Delta;
    }
    Y = MAX(0.0, Y);  // Handle mouse out of range Y coordinate
    Y = MIN(1.0, Y);

    m_RelatedCurve->m_XAnchor[m_MovingAnchor] = X;
    m_RelatedCurve->m_YAnchor[m_MovingAnchor] = Y;
    m_RelatedCurve->SetCurveFromAnchors();

    // Notify we have a manual curve now ...
    switch (m_Channel) {
      case dlCurveChannel_L :
        Settings->SetValue("CurveL",dlCurveChoice_Manual); break;
      case dlCurveChannel_a :
        Settings->SetValue("CurveLa",dlCurveChoice_Manual); break;
      case dlCurveChannel_b :
        Settings->SetValue("CurveLb",dlCurveChoice_Manual); break;
      case dlCurveChannel_Saturation :
        Settings->SetValue("CurveSaturation",dlCurveChoice_Manual); break;
      default :
        assert(0);
    }

    m_OverlayAnchorX = (int32_t) (X*(Width-1));
    m_OverlayAnchorY = (int32_t) ((1.0 - Y) * (Height-1));
    UpdateView();
  }
  return;
}

////////////////////////////////////////////////////////////////////////////////
//
// mouseReleaseEvent handler.
// Install the newly placed anchor and finalize.
//
////////////////////////////////////////////////////////////////////////////////

void dlCurveWindow::mouseReleaseEvent(QMouseEvent*) {

  if (m_BlockEvents) return;

  m_MovingAnchor = -1;
  // This recalculates the image at release of the button.
  // As this takes time we block further events on this one
  // at least.
  if (m_RecalcNeeded) {
    m_BlockEvents  = 1;
    CB_CurveWindowManuallyChanged(m_Channel);
    m_RecalcNeeded = 0;
    m_BlockEvents  = 0;
  }
  return;
}

////////////////////////////////////////////////////////////////////////////////
