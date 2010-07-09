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

#include "dlViewWindow.h"
#include "dlSettings.h"

#include <QPen>
#include <QMessageBox>

// A prototype we need
void UpdateSettings();
void CB_InputChanged(const QString,const QVariant);
void CB_ZoomFitButton();


////////////////////////////////////////////////////////////////////////////////
//
// dlViewWindow constructor.
//
////////////////////////////////////////////////////////////////////////////////

dlViewWindow::dlViewWindow(const dlImage* RelatedImage,
                                 QWidget* Parent)

  : QAbstractScrollArea(Parent) {

  m_RelatedImage = RelatedImage; // don't delete that at cleanup !
  m_ZoomFactor   = 1.0;
  m_PreviousZoomFactor   = -1.0;

  // Some other dynamic members we want to have clean.
  m_QImage           = NULL;
  m_QImageZoomed     = NULL;
  m_QImageCut        = NULL;
  // With respect to event handling.
  m_StartDragX       = 0;
  m_StartDragY       = 0;
  m_SelectionAllowed = 0;
  m_SelectionOngoing = 0;
  m_Grid             = 0;
  m_GridX            = 0;
  m_GridY            = 0;
  m_DrawRectangle    = 0;
  m_FixedAspectRatio = 0;

  //Avoiding tricky blacks at zoom fit.
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  QVBoxLayout *Layout = new QVBoxLayout;
  Layout->setContentsMargins(0,0,0,0);
  Layout->setSpacing(0);
  Layout->addWidget(this);
  Parent->setLayout(Layout);
  setStyleSheet("QAbstractScrollArea {Border: none;}");

  // OSD for the zoom size
  m_SizeReportText = "";
  QString SizeReportStyleSheet;
  SizeReportStyleSheet = "QLabel {border: 8px solid rgb(75,150,255);"
  "border-radius: 25px; padding: 8px; color: rgb(75,150,255);"
  "background: rgb(190,220,255);}";
  m_SizeReport = new QLabel();
  m_SizeReport->setText("<h1>"+m_SizeReportText+"%</h1>");
  m_SizeReport->setTextFormat(Qt::RichText);
  m_SizeReport->setAlignment(Qt::AlignCenter);
  m_SizeReport->setTextInteractionFlags(Qt::NoTextInteraction);
  m_SizeReport->setParent(this);
  m_SizeReport->setStyleSheet(SizeReportStyleSheet);
  m_SizeReport->setVisible(0);

  // A timer for report of the size
  m_SizeReportTimeOut = 1000;
  m_SizeReportTimer = new QTimer(m_SizeReport);
  m_SizeReportTimer->setSingleShot(1);

  connect(m_SizeReportTimer,SIGNAL(timeout()),
          this,SLOT(SizeReportTimerExpired()));

  // OSD for the pipe status
  m_StatusReport = new QLabel();
  m_StatusReport->setTextFormat(Qt::RichText);
  m_StatusReport->setAlignment(Qt::AlignCenter);
  m_StatusReport->setTextInteractionFlags(Qt::NoTextInteraction);
  m_StatusReport->setParent(this);
  m_StatusReport->setVisible(0);

  // A timer for report of the size
  m_StatusReportTimeOut = 1500;
  m_StatusReportTimer = new QTimer(m_StatusReport);
  m_StatusReportTimer->setSingleShot(1);

  connect(m_StatusReportTimer,SIGNAL(timeout()),
          this,SLOT(StatusReportTimerExpired()));

  // A timer for the resize with mousewheel
  m_ResizeTimeOut = 300;
  m_ResizeTimer = new QTimer(this);
  m_ResizeTimer->setSingleShot(1);

  connect(m_ResizeTimer,SIGNAL(timeout()),
          this,SLOT(ResizeTimerExpired()));

  // for resize with mousewheel
  m_NewSize = 0;

  // Create actions for context menu
  m_AtnZoomFit = new QAction(QObject::tr("Zoom fit"), this);
  connect(m_AtnZoomFit, SIGNAL(triggered()), this, SLOT(MenuZoomFit()));
  QIcon ZoomFitIcon;
  ZoomFitIcon.addPixmap(QPixmap(
    QString::fromUtf8(":/LabCurves/Icons/viewmag_h.svg")));
  m_AtnZoomFit->setIcon(ZoomFitIcon);
  m_AtnZoomFit->setIconVisibleInMenu(true);

  m_AtnZoom100 = new QAction(QObject::tr("Zoom 100%"), this);
  connect(m_AtnZoom100, SIGNAL(triggered()), this, SLOT(MenuZoom100()));
  QIcon Zoom100Icon;
  Zoom100Icon.addPixmap(QPixmap(
    QString::fromUtf8(":/LabCurves/Icons/viewmag1.svg")));
  m_AtnZoom100->setIcon(Zoom100Icon);
  m_AtnZoom100->setIconVisibleInMenu(true);

}

////////////////////////////////////////////////////////////////////////////////
//
// dlViewWindow destructor.
//
////////////////////////////////////////////////////////////////////////////////

dlViewWindow::~dlViewWindow() {
  //printf("(%s,%d) %s\n",__FILE__,__LINE__,__PRETTY_FUNCTION__);
  delete m_QImage;
  delete m_QImageZoomed;
  delete m_QImageCut;
}

////////////////////////////////////////////////////////////////////////////////
//
// Methods for setting and/or determining if a selection is ongoing.
// The few calculations are for ofsetting against what is in the
// viewport versus what is in the image + zoomfactor.
//
////////////////////////////////////////////////////////////////////////////////

void dlViewWindow::AllowSelection(const short  Allow,
                                  const short  FixedAspectRatio,
                                  const double HOverW) {
  m_SelectionAllowed = Allow;
  m_SelectionOngoing = Allow;
  m_FixedAspectRatio = FixedAspectRatio;
  m_HOverW           = HOverW;
}

short dlViewWindow::SelectionOngoing() {
  return m_SelectionOngoing;
}

uint16_t dlViewWindow::GetSelectionX() {
  uint16_t X = MIN(m_StartDragX,m_EndDragX);
  X -= m_XOffsetInVP;
  X += m_StartX;
  X = (uint16_t)(X/m_ZoomFactor+0.5);
  return X;
}

uint16_t dlViewWindow::GetSelectionY() {
  uint16_t Y = MIN(m_StartDragY,m_EndDragY);
  Y -= m_YOffsetInVP;
  Y += m_StartY;
  Y = (uint16_t)(Y/m_ZoomFactor+0.5);
  return Y;
}

uint16_t dlViewWindow::GetSelectionWidth() {
  uint16_t W = abs(m_StartDragX-m_EndDragX);
  W = (uint16_t)(W/m_ZoomFactor+0.5);
  return W;
}

uint16_t dlViewWindow::GetSelectionHeight() {
  uint16_t H = abs(m_StartDragY-m_EndDragY);
  H = (uint16_t)(H/m_ZoomFactor+0.5);
  return H;
}

////////////////////////////////////////////////////////////////////////////////
//
// Grid
//
////////////////////////////////////////////////////////////////////////////////

void dlViewWindow::Grid(const short Enabled, const short GridX, const short GridY) {
  if (Enabled) {
    m_Grid = 1;
    m_GridX = GridX;
    m_GridY = GridY;
  } else {
    m_Grid = 0;
  }
}

////////////////////////////////////////////////////////////////////////////////
//
// ZoomFit/Zoom
//
////////////////////////////////////////////////////////////////////////////////

short dlViewWindow::ZoomFit() {
  // Startup condition.
  if (!m_RelatedImage) {
    double Factor1 =(double)
      (viewport()->size().width())/m_QImage->width();
    double Factor2 =(double)
      (viewport()->size().height())/m_QImage->height();
    m_ZoomFactor = MIN(Factor1,Factor2);
    UpdateView(NULL,1);
    return (short)(m_ZoomFactor*100+0.5);
  }
  // Normal condition.
  double Factor1 =(double)
    (viewport()->size().width())/m_RelatedImage->m_Width;
  double Factor2 =(double)
    (viewport()->size().height())/m_RelatedImage->m_Height;
  m_ZoomFactor = MIN(Factor1,Factor2);
  UpdateView();
  return (short)(m_ZoomFactor*100+0.5);
}

void dlViewWindow::Zoom(const short Factor) {
  m_ZoomFactor = Factor/100.0;
  UpdateView();
}

////////////////////////////////////////////////////////////////////////////////
//
// UpdateView.
//
////////////////////////////////////////////////////////////////////////////////

void dlViewWindow::UpdateView(const dlImage* NewRelatedImage, const short StartUp) {

  if (StartUp == 0) {
    if (NewRelatedImage) m_RelatedImage = NewRelatedImage;
    if (!m_RelatedImage) return;

    if (Settings->GetInt("ZoomMode")==dlZoomMode_Fit) {
      double Factor1 =(double)
        (viewport()->size().width())/m_RelatedImage->m_Width;
      double Factor2 =(double)
        (viewport()->size().height())/m_RelatedImage->m_Height;
      m_ZoomFactor = MIN(Factor1,Factor2);
      Settings->SetValue("Zoom",(int)(m_ZoomFactor*100+0.5));
    }

    // Convert the dlImage to a QImage. Mind R<->B and 16->8
    if (NewRelatedImage) {
      delete m_QImage;
      m_QImage = new QImage(m_RelatedImage->m_Width,
                            m_RelatedImage->m_Height,
                            QImage::Format_RGB32);
      for (uint16_t Row=0; Row<m_RelatedImage->m_Height; Row++) {
        for (uint16_t Col=0; Col<m_RelatedImage->m_Width; Col++) {
          uint32_t PixelInQFormat;
          uint8_t* Pixel = (uint8_t*) &PixelInQFormat;
          for (short c=0; c<3; c++) {
            // Mind the R<->B swap !
            Pixel[2-c] =
              m_RelatedImage->m_Image[Row*m_RelatedImage->m_Width+Col][c]>>8;
          }
          Pixel[3] = 0xff;
          m_QImage->setPixel(Col,Row,PixelInQFormat);
        }
      }
    }
    // Size of zoomed image.
    m_ZoomWidth  = (uint16_t)(m_RelatedImage->m_Width*m_ZoomFactor+.5);
    m_ZoomHeight = (uint16_t)(m_RelatedImage->m_Height*m_ZoomFactor+.5);

  } else {
    delete m_QImage;
    m_QImage = new QImage(":/LabCurves/Splash.png");
    // Size of zoomed image.
    m_ZoomWidth  = (uint16_t)(m_QImage->width()*m_ZoomFactor+.5);
    m_ZoomHeight = (uint16_t)(m_QImage->height()*m_ZoomFactor+.5);
  }


  // Size of viewport.
  uint16_t VP_Width  = viewport()->size().width();
  uint16_t VP_Height = viewport()->size().height();

  // Set the scrollBars acccordingly.
  verticalScrollBar()->setPageStep(VP_Height);
  verticalScrollBar()->setRange(0,m_ZoomHeight-VP_Height);
  horizontalScrollBar()->setPageStep(VP_Width);
  horizontalScrollBar()->setRange(0,m_ZoomWidth-VP_Width);

  // This correspond to the current (X,Y) offset of the cut image
  // versus the zoomed image.
  int32_t CurrentStartX = horizontalScrollBar()->value();
  int32_t CurrentStartY = verticalScrollBar()->value();

  if (NewRelatedImage || m_PreviousZoomFactor != m_ZoomFactor) {

    // Recalculate CurrentStartX/Y such that centrum stays centrum.
    CurrentStartX = (int32_t)
      ( (CurrentStartX+VP_Width/2)*m_ZoomFactor/m_PreviousZoomFactor -
        VP_Width/2 );
    CurrentStartY = (int32_t)
      ( (CurrentStartY+VP_Height/2)*m_ZoomFactor/m_PreviousZoomFactor -
        VP_Height/2 );
    CurrentStartX = MAX(0,CurrentStartX);
    CurrentStartY = MAX(0,CurrentStartY);

    m_PreviousZoomFactor = m_ZoomFactor;

    // Recalculate the zoomed image.
    delete m_QImageZoomed;
    m_QImageZoomed = new QImage(m_QImage->scaled(m_ZoomWidth,
                                                 m_ZoomHeight,
                                                 Qt::IgnoreAspectRatio,
                                                 Qt::SmoothTransformation));

  }

  // Maybe move scrollbars such that centrum stays centrum during zoom.
  // block signals as to avoid this programmatic update generates events.
  // TODO Correct way of doing things ?

  horizontalScrollBar()->blockSignals(1);
  horizontalScrollBar()->setValue(CurrentStartX);
  horizontalScrollBar()->blockSignals(0);

  verticalScrollBar()->blockSignals(1);
  verticalScrollBar()->setValue(CurrentStartY);
  verticalScrollBar()->blockSignals(0);

  // Recalculate the image cut out of m_QImageZoomed.
  RecalculateCut();

  // Update view.
  viewport()->update();

}

////////////////////////////////////////////////////////////////////////////////
//
// RecalculateCut()
//
// Recalculates m_QImageCut when the selection
// has been changed, i.e. for scrollbar movements or a resizing.
//
////////////////////////////////////////////////////////////////////////////////

void dlViewWindow::RecalculateCut() {

  if (!m_QImageZoomed) return;

  // Following are coordinates in a zoomed image.
  m_StartX = horizontalScrollBar()->value();
  uint16_t Width  = MIN(horizontalScrollBar()->pageStep(),
                        m_QImageZoomed->width());
  m_StartY = verticalScrollBar()->value();
  uint16_t Height = MIN(verticalScrollBar()->pageStep(),
                        m_QImageZoomed->height());

  // Make a new cut out of our zoomed image
  delete m_QImageCut;
  m_QImageCut = new QImage(m_QImageZoomed->copy(m_StartX,
                                                m_StartY,
                                                Width,
                                                Height));
}

////////////////////////////////////////////////////////////////////////////////
//
// scrollContentsBy()
//
// Overloaded virtual from QAbstractScrollArea to handle scrolling.
// Only recalculates the image cut in our case.
//
////////////////////////////////////////////////////////////////////////////////

void dlViewWindow::scrollContentsBy(int,int) {
  // Make a new cut out of our zoomed image.
  RecalculateCut();
  // And update the view.
  viewport()->repaint();
}

////////////////////////////////////////////////////////////////////////////////
//
// viewportEvent()
//
// Overloaded virtual from QAbstractScrollArea.
// Handles all events into the viewport :
//      - Paint : Draw the pixmap and maybe the 'overlay' rectangle
//            used during selection.
//      - Resize: Basically recalculate the cut and update.
//      - mousePress/Move/Release for doing a selection.
//                (and only if m_SelectionAllowed)
//
////////////////////////////////////////////////////////////////////////////////

bool dlViewWindow::viewportEvent(QEvent* Event) {

  if (Event->type() == QEvent::Paint) {

    // PaintEvent : Draw the pixmap and the 'overlay' rectangle if there.

    uint16_t VP_Width  = viewport()->size().width();
    uint16_t VP_Height = viewport()->size().height();

    m_XOffsetInVP = 0;
    if (VP_Width > m_QImageCut->width()) {
      m_XOffsetInVP = (VP_Width - m_QImageCut->width())/2;
    }

    m_YOffsetInVP = 0;
    if (VP_Height > m_QImageCut->height()) {
      m_YOffsetInVP = (VP_Height - m_QImageCut->height())/2;
    }

    QPainter Painter(viewport());
    Painter.save();
    Painter.fillRect(0,0,VP_Width,VP_Height,palette().color(QPalette::Window));
    if (m_QImageCut) {
      Painter.drawImage(m_XOffsetInVP,m_YOffsetInVP,*m_QImageCut);
    }
    if (m_DrawRectangle) {
      int16_t FrameX0 = m_XOffsetInVP;
      int16_t FrameY0 = m_YOffsetInVP;
      int16_t FrameX1 = m_XOffsetInVP + m_QImageCut->width();
      int16_t FrameY1 = m_YOffsetInVP + m_QImageCut->height();
      int16_t RectX0 = MIN(m_StartDragX,m_EndDragX);
      int16_t RectY0 = MIN(m_StartDragY,m_EndDragY);
      int16_t RectX1 = MAX(m_StartDragX,m_EndDragX);
      int16_t RectY1 = MAX(m_StartDragY,m_EndDragY);
      QBrush Brush(QColor(20, 20, 20, 200));
      if (RectY0 > FrameY0) { // Top
        Painter.fillRect(FrameX0,FrameY0,
             FrameX1-FrameX0,MIN(FrameY1-FrameY0,RectY0-FrameY0),Brush);
      }
      if (RectY1 < FrameY1) { // Bottom
        Painter.fillRect(FrameX0,MAX(RectY1+1,FrameY0),
             FrameX1-FrameX0,FrameY1-MAX(RectY1+1,FrameY0),Brush);
      }
      if ((RectX0 > FrameX0) &&
          !((RectY0 < FrameY0) && (RectY1 < FrameY0)) &&
          !((RectY0 > FrameY1) && (RectY1 > FrameY1))) { // Left
        Painter.fillRect(FrameX0,MAX(FrameY0,RectY0),
             MIN(FrameX1-FrameX0,RectX0-FrameX0),MIN(FrameY1,RectY1)-MAX(FrameY0,RectY0)+1,Brush);
      }
      if ((RectX1 < FrameX1) &&
          !((RectY0 < FrameY0) && (RectY1 < FrameY0)) &&
          !((RectY0 > FrameY1) && (RectY1 > FrameY1))) { // Right
        Painter.fillRect(MAX(RectX1+1,FrameX0),MAX(FrameY0,RectY0),
             FrameX1-MAX(RectX1+1,FrameX0),MIN(FrameY1,RectY1)-MAX(FrameY0,RectY0)+1,Brush);
      }
      QPen Pen(QColor(150, 150, 150),1);
      Painter.setPen(Pen);
      Painter.drawRect(m_StartDragX, m_StartDragY,
                       m_EndDragX-m_StartDragX,m_EndDragY-m_StartDragY);
    }
    Painter.restore();

    Painter.restore();

  } else if (Event->type() == QEvent::Resize) {

   // ResizeEvent : Make a new cut on the appropriate place.

    uint16_t VP_Width  = viewport()->size().width();
    uint16_t VP_Height = viewport()->size().height();

    // Adapt scrollbars to new viewport size
    verticalScrollBar()->setPageStep(VP_Height);
    verticalScrollBar()->setRange(0,m_ZoomHeight-VP_Height);
    horizontalScrollBar()->setPageStep(VP_Width);
    horizontalScrollBar()->setRange(0,m_ZoomWidth-VP_Width);

    // Recalculat the cut.
    RecalculateCut();
    if (Settings->GetInt("ZoomMode") == dlZoomMode_Fit)
      ::CB_ZoomFitButton();


  } else if (Event->type() == QEvent::MouseButtonPress && m_SelectionAllowed) {

    // Start selection.

    m_DrawRectangle = 0;

    m_StartDragX = ((QMouseEvent*) Event)->x();
    m_StartDragY = ((QMouseEvent*) Event)->y();

    viewport()->repaint();

  } else if (Event->type() == QEvent::MouseMove && m_SelectionAllowed) {

    // Drag selection.

    uint16_t Y;

    if (m_FixedAspectRatio) {
      if (((QMouseEvent*) Event)->x()>m_StartDragX) {
        if (((QMouseEvent*) Event)->y()>m_StartDragY) {
          Y = m_StartDragY +
              (int) (m_HOverW*(((QMouseEvent*)Event)->x()-m_StartDragX));
        } else {
          Y = m_StartDragY -
              (int) (m_HOverW*(((QMouseEvent*)Event)->x()-m_StartDragX));
        }
      } else {
        if (((QMouseEvent*) Event)->y()>m_StartDragY) {
          Y = m_StartDragY -
              (int) (m_HOverW*(((QMouseEvent*)Event)->x()-m_StartDragX));
        } else {
          Y = m_StartDragY +
              (int) (m_HOverW*(((QMouseEvent*)Event)->x()-m_StartDragX));
        }
      }
    } else {
      Y = ((QMouseEvent*) Event)->y();
    }

    m_DrawRectangle = 1;

    m_EndDragX = ((QMouseEvent*) Event)->x();
    m_EndDragY = Y;

    viewport()->repaint();

  } else if (Event->type() == QEvent::MouseButtonRelease && m_SelectionAllowed){

    // End selection.

    m_DrawRectangle = 0;

    uint16_t Y;

    if (m_FixedAspectRatio) {
      if (((QMouseEvent*) Event)->x()>m_StartDragX) {
        if (((QMouseEvent*) Event)->y()>m_StartDragY) {
          Y = m_StartDragY +
              (int) (m_HOverW*(((QMouseEvent*)Event)->x()-m_StartDragX));
        } else {
          Y = m_StartDragY -
              (int) (m_HOverW*(((QMouseEvent*)Event)->x()-m_StartDragX));
        }
      } else {
        if (((QMouseEvent*) Event)->y()>m_StartDragY) {
          Y = m_StartDragY -
              (int) (m_HOverW*(((QMouseEvent*)Event)->x()-m_StartDragX));
        } else {
          Y = m_StartDragY +
              (int) (m_HOverW*(((QMouseEvent*)Event)->x()-m_StartDragX));
        }
      }
    } else {
      Y = ((QMouseEvent*) Event)->y();
    }

    m_EndDragX = ((QMouseEvent*) Event)->x();
    m_EndDragY = Y;

    m_SelectionOngoing = 0;

    viewport()->repaint();

  } else if (Event->type() == QEvent::MouseButtonPress && !m_SelectionAllowed) {

    // A mouse button press, without selection being allowed, is going
    // to be interpreted as a move.

    m_StartDragX = ((QMouseEvent*) Event)->x();
    m_StartDragY = ((QMouseEvent*) Event)->y();


  } else if (Event->type() == QEvent::MouseMove && !m_SelectionAllowed) {

    // On the move with the image.
    //
    m_EndDragX = ((QMouseEvent*) Event)->x();
    m_EndDragY = ((QMouseEvent*) Event)->y();

    int32_t CurrentStartX = horizontalScrollBar()->value();
    int32_t CurrentStartY = verticalScrollBar()->value();

    horizontalScrollBar()->setValue(CurrentStartX-m_EndDragX+m_StartDragX);
    verticalScrollBar()->setValue(CurrentStartY-m_EndDragY+m_StartDragY);

    m_StartDragX = m_EndDragX;
    m_StartDragY = m_EndDragY;

    viewport()->repaint();

  } else if (Event->type() == QEvent::ContextMenu) {
    ContextMenu(Event);
  } else if (Event->type() == QEvent::Wheel) {
    m_SizeReportTimer->start(m_SizeReportTimeOut);

    QList<int> Scales;
    Scales << 5 << 8 << 10 << 15 << 20 << 25 << 33 << 50 << 66 << 100 << 150 << 200 << 300 << 400;

    Settings->SetValue("ZoomMode",dlZoomMode_NonFit);
    int TempZoom = m_NewSize?m_NewSize:Settings->GetInt("Zoom");
    int Choice = 0;
    if (((QWheelEvent*)Event)->delta() < 0) {
      for (int i=0; i<Scales.size(); i++) {
        if (Scales.at(i) < TempZoom) Choice = i;
      }
    } else {
      for (int i=0; i<Scales.size(); i++) {
        if (Scales.at(Scales.size()-1-i) > TempZoom) Choice = Scales.size()-1-i;
      }
      if (TempZoom == Scales.at(Scales.size()-1)) Choice = Scales.size()-1;
    }
    m_NewSize = Scales.at(Choice);
    m_ResizeTimer->start(m_ResizeTimeOut);
    m_SizeReportText = QString::number(m_NewSize);
    m_SizeReport->setGeometry(width()-170,20,150,70);
    m_SizeReport->setText("<h1>"+m_SizeReportText+"%</h1>");
    m_SizeReport->update();
    m_SizeReport->setVisible(1);
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
// Call for status report
//
////////////////////////////////////////////////////////////////////////////////

void dlViewWindow::StatusReport(short State) {
  if (State == 0) { // Done
    QString StatusReportStyleSheet;
    StatusReportStyleSheet = "QLabel {border: 8px solid rgb(0,130,0);"
    "border-radius: 25px; padding: 8px; color: rgb(0,130,0);"
    "background: rgb(120,170,120);}";
    m_StatusReport->setStyleSheet(StatusReportStyleSheet);
    m_StatusReport->setText("<h1> Done </h1>");
    m_StatusReportTimer->start(m_StatusReportTimeOut);
    m_StatusReport->setGeometry(20,20,150,70);
    m_StatusReport->update();
    m_StatusReport->setVisible(1);
  } else if (State == 1) { // Updating
    m_StatusReportTimer->stop();
    QString StatusReportStyleSheet;
    StatusReportStyleSheet = "QLabel {border: 8px solid rgb(255,140,0);"
    "border-radius: 25px; padding: 8px; color: rgb(255,140,0);"
    "background: rgb(255,200,120);}";
    m_StatusReport->setStyleSheet(StatusReportStyleSheet);
    m_StatusReport->setText("<h1> Updating </h1>");
    m_StatusReport->setGeometry(20,20,200,70);
    m_StatusReport->update();
    m_StatusReport->setVisible(1);
  } else if (State == 2) { // Processing
    m_StatusReportTimer->stop();
    QString StatusReportStyleSheet;
    StatusReportStyleSheet = "QLabel {border: 8px solid rgb(255,75,75);"
    "border-radius: 25px; padding: 8px; color: rgb(255,75,75);"
    "background: rgb(255,190,190);}";
    m_StatusReport->setStyleSheet(StatusReportStyleSheet);
    m_StatusReport->setText("<h1> Processing </h1>");
    m_StatusReport->setGeometry(20,20,220,70);
    m_StatusReport->update();
    m_StatusReport->setVisible(1);
  } else { // should not happen, clean up
    m_StatusReportTimer->stop();
    m_StatusReport->setVisible(0);
  }
}

////////////////////////////////////////////////////////////////////////////////
//
// slots for the timers
//
////////////////////////////////////////////////////////////////////////////////

void dlViewWindow::SizeReportTimerExpired() {
  m_SizeReport->setVisible(0);
}

void dlViewWindow::StatusReportTimerExpired() {
  m_StatusReport->setVisible(0);
}

void dlViewWindow::ResizeTimerExpired() {
  if (m_NewSize != Settings->GetInt("Zoom"))
    CB_InputChanged("ZoomInput",m_NewSize);
  m_NewSize = 0;
}

////////////////////////////////////////////////////////////////////////////////
//
// generate context menu
//
////////////////////////////////////////////////////////////////////////////////

void dlViewWindow::ContextMenu(QEvent* Event) {
  QMenu Menu(this);
  Menu.addAction(m_AtnZoomFit);
  Menu.addAction(m_AtnZoom100);
  Menu.exec(((QMouseEvent*)Event)->globalPos());
}

////////////////////////////////////////////////////////////////////////////////
//
// slots for the context menu
//
////////////////////////////////////////////////////////////////////////////////

void UpdatePreviewImage(const dlImage* ForcedImage   /* = NULL  */,
                        const short    OnlyHistogram /* = false */,
                        const short    ForceRun      /* = 0     */);

void dlViewWindow::MenuZoomFit() {
  ::CB_ZoomFitButton();
}

void CB_ZoomFullButton();
void dlViewWindow::MenuZoom100() {
  ::CB_ZoomFullButton();
}

////////////////////////////////////////////////////////////////////////////////
