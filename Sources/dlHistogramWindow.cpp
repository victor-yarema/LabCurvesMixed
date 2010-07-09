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

#include "dlSettings.h"
#include "dlHistogramWindow.h"

#include <iostream>

#ifdef _OPENMP
  #include <omp.h>
#endif

extern float ToFloatTable[0x10000];

using namespace std;

////////////////////////////////////////////////////////////////////////////////
//
// Constructor.
//
// Instantiates a (also here defined) HistogramWidget,
// which acts as a central widget where the operations are finally done upon.
//
////////////////////////////////////////////////////////////////////////////////

dlHistogramWindow::dlHistogramWindow(const dlImage* RelatedImage,
                                           QWidget* Parent)
  :QWidget(NULL) {

  m_RelatedImage = RelatedImage; // don't delete that at cleanup !
  // Some other dynamic members we want to have clean.
  m_QPixmap      = NULL;
  m_Image8       = NULL;

  m_PreviousHistogramGamma = -1;
  m_PreviousHistogramLogX  = -1;
  m_PreviousHistogramLogY  = -1;
  m_PreviousFileName       = "";

  // Sizing and layout related.
  QSizePolicy Policy(QSizePolicy::Minimum, QSizePolicy::Preferred);
  Policy.setHeightForWidth(1);
  setSizePolicy(Policy);

  QVBoxLayout *Layout = new QVBoxLayout;
  Layout->setContentsMargins(0,0,0,0);
  Layout->setSpacing(0);
  Layout->addWidget(this);
  Parent->setLayout(Layout);

  // Timer to delay on resize operations.
  // (avoiding excessive calculations)
  m_ResizeTimer = new QTimer(this);
  m_ResizeTimer->setSingleShot(1);
  connect(m_ResizeTimer,
          SIGNAL(timeout()),
          this,
          SLOT(ResizeTimerExpired()));

  // Create actions for context menu
  m_AtnLnX = new QAction(QObject::tr("LnX"), this);
  m_AtnLnX->setStatusTip(QObject::tr("X axis logarithmic"));
  m_AtnLnX->setCheckable(true);
  m_AtnLnX->setChecked(Settings->GetInt("HistogramLogX"));
  connect(m_AtnLnX, SIGNAL(triggered()), this, SLOT(MenuLnX()));

  m_AtnLnY = new QAction(QObject::tr("LnY"), this);
  m_AtnLnY->setStatusTip(QObject::tr("Y axis logarithmic"));
  m_AtnLnY->setCheckable(true);
  m_AtnLnY->setChecked(Settings->GetInt("HistogramLogY"));
  connect(m_AtnLnY, SIGNAL(triggered()), this, SLOT(MenuLnY()));

  m_AtnCrop = new QAction(QObject::tr("Crop"), this);
  m_AtnCrop->setStatusTip(QObject::tr("Histogram only on a part of the image"));
  m_AtnCrop->setCheckable(true);
  if (Settings->GetInt("HistogramCrop"))
    m_AtnCrop->setChecked(1);
  else
    m_AtnCrop->setChecked(0);
  connect(m_AtnCrop, SIGNAL(triggered()), this, SLOT(MenuCrop()));

  m_AtnRGB = new QAction(QObject::tr("RGB"), this);
  m_AtnRGB->setStatusTip(QObject::tr("RGB"));
  m_AtnRGB->setCheckable(true);
  connect(m_AtnRGB, SIGNAL(triggered()), this, SLOT(MenuChannel()));

  m_AtnR = new QAction(QObject::tr("R"), this);
  m_AtnR->setStatusTip(QObject::tr("R"));
  m_AtnR->setCheckable(true);
  connect(m_AtnR, SIGNAL(triggered()), this, SLOT(MenuChannel()));

  m_AtnG = new QAction(QObject::tr("G"), this);
  m_AtnG->setStatusTip(QObject::tr("G"));
  m_AtnG->setCheckable(true);
  connect(m_AtnG, SIGNAL(triggered()), this, SLOT(MenuChannel()));

  m_AtnB = new QAction(QObject::tr("B"), this);
  m_AtnB->setStatusTip(QObject::tr("B"));
  m_AtnB->setCheckable(true);
  connect(m_AtnB, SIGNAL(triggered()), this, SLOT(MenuChannel()));

  m_ChannelGroup = new QActionGroup(this);
  m_ChannelGroup->addAction(m_AtnRGB);
  m_ChannelGroup->addAction(m_AtnR);
  m_ChannelGroup->addAction(m_AtnG);
  m_ChannelGroup->addAction(m_AtnB);

  if (Settings->GetInt("HistogramChannel")==dlHistogramChannel_RGB)
    m_AtnRGB->setChecked(true);
  else if (Settings->GetInt("HistogramChannel")==dlHistogramChannel_R)
    m_AtnR->setChecked(true);
  else if (Settings->GetInt("HistogramChannel")==dlHistogramChannel_G)
    m_AtnG->setChecked(true);
  else
    m_AtnB->setChecked(true);

}

////////////////////////////////////////////////////////////////////////////////
//
// Destructor.
//
////////////////////////////////////////////////////////////////////////////////

dlHistogramWindow::~dlHistogramWindow() {
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

void dlHistogramWindow::resizeEvent(QResizeEvent*) {
  // Schedule the action 500ms from here to avoid multiple rescaling actions
  // during multiple resizeEvents from a window resized by the user.
  m_ResizeTimer->start(500); // 500 ms.
}

void dlHistogramWindow::ResizeTimerExpired() {
  // Create side effect for recalibrating the maximum
  m_PreviousHistogramGamma = -1;
  // m_RelatedImage enforces update, even if it is the same image.
  UpdateView(m_RelatedImage);
}

////////////////////////////////////////////////////////////////////////////////
//
// CalculateHistogram.
//
// Calculates the histogram into an m_Image8.
//
////////////////////////////////////////////////////////////////////////////////

void dlHistogramWindow::CalculateHistogram() {

  //QTime Timer;
  //Timer.start();

  int WidgetWidth  = width();
  int WidgetHeight = height();

  const uint16_t HistogramMargin = 5;
  const uint16_t HistogramWidth  = WidgetWidth-2*HistogramMargin;

  uint32_t Histogram[3][HistogramWidth];
  // Zero the Histogram
  memset(Histogram,0,sizeof(Histogram));

  // MaxColor (We want only the luminance in LAB).
  short MaxColor = (m_RelatedImage->m_ColorSpace==dlSpace_Lab)?1:3;

  // Average of ideal linear histogram.
  uint32_t HistoAverage =
    m_RelatedImage->m_Width*m_RelatedImage->m_Height/HistogramWidth;

  //printf("(%s,%d) %d\n",__FILE__,__LINE__,Timer.elapsed());
  // Calculate Histogram
  const uint16_t Width  = m_RelatedImage->m_Width;
  const uint16_t Height = m_RelatedImage->m_Height;
  const int32_t Size   = Width*Height;
  const short HistogramGamma = 0;
  float r = 0;
  uint16_t HistogramPoint = 0;
#pragma omp parallel default(shared) private (r, HistogramPoint)
    {
#ifdef _OPENMP
      // We need a thread-private copy.
      int TpHistogram[3][HistogramWidth];
      memset (TpHistogram, 0, sizeof Histogram);
#endif
#pragma omp for
    for (int32_t i=0; i<(int32_t) Size; i++) {
      for (short c=0;c<MaxColor;c++) {
        r = ToFloatTable[m_RelatedImage->m_Image[i][c]];
        HistogramPoint = (uint16_t)(r*HistogramWidth);
#ifdef _OPENMP
          TpHistogram[c][HistogramPoint]++;
#else
          Histogram[c][HistogramPoint]++;
#endif
      }
    }
#ifdef _OPENMP
#pragma omp critical
      for(int c=0; c<3*HistogramWidth; c++) {
        Histogram[0][c]+=TpHistogram[0][c];
      }
#endif
    } // End omp parallel zone.

  // Logaritmic variants.
  const short HistogramLogX = Settings->GetInt("HistogramLogX");
  if (HistogramLogX) {
    uint32_t LogHistogram[3][HistogramWidth];
    // Zero the Histogram
    memset(LogHistogram,0,sizeof(LogHistogram));
    for (uint16_t k=1;k<HistogramWidth;k++) {
      for (short c=0; c<MaxColor; c++ ) {
        short Index = (short)(log(k)/log(HistogramWidth)*HistogramWidth);
        LogHistogram[c][Index] += Histogram[c][k];
      }
    }
    memcpy(Histogram,LogHistogram,sizeof(LogHistogram));
  }

  // Logaritmic variants.
  const short HistogramLogY = Settings->GetInt("HistogramLogY");
  if (HistogramLogY) {
    for (uint16_t k=0;k<HistogramWidth;k++) {
      for (short c=0; c<MaxColor; c++ ) {
        // sqrt(10)-1 such that average arrives on 50%
        Histogram[c][k] = (uint32_t)
          //4096 is rather random scaler. Later rescaled.
          (4096* log10(1+2.16227766/HistoAverage*Histogram[c][k]));
      }
    }
    // On 50% per construction.
    HistoAverage = 2048;
  }

  // Maximum : only recalculated in a few cases : changed setting or file.
  if ((HistogramGamma != m_PreviousHistogramGamma)  ||
      (HistogramLogX  != m_PreviousHistogramLogX)   ||
      (HistogramLogY  != m_PreviousHistogramLogY)) {

    m_PreviousHistogramGamma = HistogramGamma;
    m_PreviousHistogramLogX  = HistogramLogX;
    m_PreviousHistogramLogY  = HistogramLogY;

    m_HistoMax = 0;
    // Remark that we don't take the extremes of the x-axis
    // into account. Typical clipping region.
    uint16_t LeftEnd = -1;
    uint16_t Value = 0;
    while (Value == 0) {
      LeftEnd += 1;
      for (short c=0; c<MaxColor; c++)
        Value += Histogram[c][LeftEnd];
      if (LeftEnd==HistogramWidth-1) Value = 1;
    }
    uint16_t RightEnd = HistogramWidth;
    Value = 0;
    while (Value == 0) {
      RightEnd -= 1;
      for (short c=0; c<MaxColor; c++)
        Value += Histogram[c][RightEnd];
      if (RightEnd==LeftEnd) Value = 1;
    }

    for (uint16_t k=LeftEnd+1;k<RightEnd;k++) {
      for (short c=0; c<MaxColor; c++ ) {
        if (Histogram[c][k] > m_HistoMax) {
          m_HistoMax = Histogram[c][k];
        }
      }
    }
    // Create some headroom for changes.
    m_HistoMax = m_HistoMax*4/3;
  }

  // Instantiate an Image8 and put the histogram in it.
  delete m_Image8;
  m_Image8 = new dlImage8(WidgetWidth,WidgetHeight,3);

  uint16_t RowLimit = WidgetHeight-1;

  for (short c=0; c<MaxColor; c++ ) {
    if (!(MaxColor==1) && !((1<<c) & Settings->GetInt("HistogramChannel"))) {
      continue;
    }
    for (uint16_t i=0; i<HistogramWidth; i++) {
      double r = Histogram[c][i]/(double)(m_HistoMax);
      if (r>=0.99) r=0.99; // Safety.
      uint16_t Row = RowLimit-(uint16_t)(r*WidgetHeight);
      // if (Row<0) Row = 0;
      if (Row >= WidgetHeight) Row=WidgetHeight-1;
      for (uint16_t k=Row;k<RowLimit;k++) {
        // 2- ! Image8[0]=B for QT !
        for (short z=0; z<3; z++) {
          m_Image8->m_Image[k*m_Image8->m_Width+i+HistogramMargin][z] +=
            ((z==(2-c)) || (MaxColor ==1))?0xff:0;
        }
      }
      // baselines. A grey colour.
      m_Image8->m_Image[RowLimit*m_Image8->m_Width+i+HistogramMargin][0] = 0x80;
      m_Image8->m_Image[RowLimit*m_Image8->m_Width+i+HistogramMargin][1] = 0x80;
      m_Image8->m_Image[RowLimit*m_Image8->m_Width+i+HistogramMargin][2] = 0x80;
      // Average line.
      r = HistoAverage/(double)(m_HistoMax);
      if (r>=0.99) r=0.99; // Safety.
      Row = RowLimit-(uint16_t)(r*WidgetHeight);
      // if (Row<0) Row = 0;
      if (Row >= WidgetHeight) Row=WidgetHeight-1;
      m_Image8->m_Image[Row*m_Image8->m_Width+i+HistogramMargin][0] = 0xa0;
      m_Image8->m_Image[Row*m_Image8->m_Width+i+HistogramMargin][1] = 0xa0;
      m_Image8->m_Image[Row*m_Image8->m_Width+i+HistogramMargin][2] = 0xa0;

    }
  }

  // Grid
  int Sections = 5;
  int Step = (int) ((double)HistogramWidth/(double)Sections);
  for (short i=1; i<Sections; i++) {
    uint16_t Col=i*Step+HistogramMargin;
    for (uint16_t Row=0; Row<WidgetHeight; Row++) {
      int value = (int) MIN(MAX(Row-15,0), 0x60);
      if (!m_Image8->m_Image[Row*WidgetWidth+Col][0] &&
          !m_Image8->m_Image[Row*WidgetWidth+Col][1] &&
          !m_Image8->m_Image[Row*WidgetWidth+Col][2]) {
        m_Image8->m_Image[Row*WidgetWidth+Col][0] = value;
        m_Image8->m_Image[Row*WidgetWidth+Col][1] = value;
        m_Image8->m_Image[Row*WidgetWidth+Col][2] = value;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
//
// UpdateView.
//
////////////////////////////////////////////////////////////////////////////////

void dlHistogramWindow::UpdateView(const dlImage* NewRelatedImage) {

  if (NewRelatedImage) m_RelatedImage = NewRelatedImage;
  if (!m_RelatedImage) return;

  CalculateHistogram();

  // The detour QImage=>QPixmap is needed to enjoy
  // HW acceleration of QPixmap.
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

void dlHistogramWindow::paintEvent(QPaintEvent*) {
  //printf("(%s,%d) %s - Size : (%d,%d)\n",
  //       __FILE__,__LINE__,__PRETTY_FUNCTION__,width(),height());
  QPainter Painter(this);
  Painter.save();
  if (!m_QPixmap) {
    int WidgetWidth  = width();
    int WidgetHeight = height();

    const uint16_t HistogramMargin = 5;
    const uint16_t HistogramWidth  = WidgetWidth-2*HistogramMargin;

    delete m_Image8;
    m_Image8 = new dlImage8(WidgetWidth,WidgetHeight,3);

    uint16_t RowLimit = WidgetHeight-1;

    for (uint16_t i=0; i<HistogramWidth; i++) {
      // baselines. A grey colour.
      m_Image8->m_Image[RowLimit*m_Image8->m_Width+i+HistogramMargin][0] = 0x80;
      m_Image8->m_Image[RowLimit*m_Image8->m_Width+i+HistogramMargin][1] = 0x80;
      m_Image8->m_Image[RowLimit*m_Image8->m_Width+i+HistogramMargin][2] = 0x80;
    }

    // Grid
    int Sections = 5;
    int Step = (int) ((double)HistogramWidth/(double)Sections);
    for (short i=1; i<Sections; i++) {
      uint16_t Col=i*Step+HistogramMargin;
      for (uint16_t Row=0; Row<WidgetHeight; Row++) {
        int value = (int) MIN(MAX(Row-15,0), 0x60);
        m_Image8->m_Image[Row*WidgetWidth+Col][0] = value;
        m_Image8->m_Image[Row*WidgetWidth+Col][1] = value;
        m_Image8->m_Image[Row*WidgetWidth+Col][2] = value;
      }
    }
    m_QPixmap = new QPixmap(
    QPixmap::fromImage(QImage((const uchar*) m_Image8->m_Image,
                              m_Image8->m_Width,
                              m_Image8->m_Height,
                              QImage::Format_RGB32)));
  }
  Painter.drawPixmap(0,0,*m_QPixmap);
  Painter.restore();
}

////////////////////////////////////////////////////////////////////////////////
//
// contextMenuEvent handler.
//
////////////////////////////////////////////////////////////////////////////////

void dlHistogramWindow::contextMenuEvent(QContextMenuEvent *event)
{
  QMenu Menu(this);
  QMenu ChannelMenu(this);
  ChannelMenu.addAction(m_AtnRGB);
  ChannelMenu.addAction(m_AtnR);
  ChannelMenu.addAction(m_AtnG);
  ChannelMenu.addAction(m_AtnB);
  ChannelMenu.setTitle(QObject::tr("Channel"));
  Menu.addMenu(&ChannelMenu);
  Menu.addSeparator();
  Menu.addAction(m_AtnLnX);
  Menu.addAction(m_AtnLnY);
  Menu.addSeparator();
  if (Settings->GetInt("HistogramCrop"))
    m_AtnCrop->setChecked(1);
  else
    m_AtnCrop->setChecked(0);
  Menu.addAction(m_AtnCrop);

  Menu.exec(event->globalPos());
}

////////////////////////////////////////////////////////////////////////////////
//
// slots for the context menu
//
////////////////////////////////////////////////////////////////////////////////

void dlHistogramWindow::MenuLnX() {
  Settings->SetValue("HistogramLogX",(int)m_AtnLnX->isChecked());
  UpdateView();
}

void dlHistogramWindow::MenuLnY() {
  Settings->SetValue("HistogramLogY",(int)m_AtnLnY->isChecked());
  UpdateView();
}

void HistogramGetCrop();
void dlHistogramWindow::MenuCrop() {
  if (m_AtnCrop->isChecked())
    Settings->SetValue("HistogramCrop", 1);
  else
    Settings->SetValue("HistogramCrop", 0);

  ::HistogramGetCrop();
}

void dlHistogramWindow::MenuChannel() {
  if (m_AtnRGB->isChecked())
    Settings->SetValue("HistogramChannel", dlHistogramChannel_RGB);
  else if (m_AtnR->isChecked())
    Settings->SetValue("HistogramChannel", dlHistogramChannel_R);
  else if (m_AtnG->isChecked())
    Settings->SetValue("HistogramChannel", dlHistogramChannel_G);
  else if (m_AtnB->isChecked())
    Settings->SetValue("HistogramChannel", dlHistogramChannel_B);

  UpdateView();
}

////////////////////////////////////////////////////////////////////////////////
