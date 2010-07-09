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

#ifndef DLVIEWWINDOW_H
#define DLVIEWWINDOW_H

#include <QtGui>

#include "dlImage.h"


class dlViewWindow : public QAbstractScrollArea {

Q_OBJECT

public :

// Constructor.
dlViewWindow(const dlImage*       RelatedImage,
                   QWidget*       Parent);
// Destructor.
~dlViewWindow();

// NewRelatedImage to associate anonter dlImage with this window.
void UpdateView(const dlImage* NewRelatedImage = NULL,
                const short StartUp = 0);


// Allow to select in the image. (push/drag/release events).
// Argument is 0 or 1.
// If FixedAspectRatio, then the selection box has the HOverW ratio.
void AllowSelection(const short  Allow,
                    const short  FixedAspectRatio = 0,
                    const double HOverW = 2.0/3);

// Returns 1 if selection process is ongoing.
short SelectionOngoing();

// Results of selection.
// Expressed in terms of the RelatedImage.
uint16_t GetSelectionX();
uint16_t GetSelectionY();
uint16_t GetSelectionWidth();
uint16_t GetSelectionHeight();

// Grid
void Grid(const short Enabled, const short GridX, const short GridY);

// Zoom functions. Fit returns the factor in %.
short ZoomFit();
void  Zoom(const short Factor); // Expressed in %

// Status report
void StatusReport (short State);

const dlImage*       m_RelatedImage;

short                m_SelectionAllowed;
short                m_SelectionOngoing;
int16_t              m_StartDragX;
int16_t              m_StartDragY;
int16_t              m_EndDragX;
int16_t              m_EndDragY;
short                m_FixedAspectRatio;
double               m_HOverW;
uint16_t             m_StartX; // Offset of the shown part into the image.
uint16_t             m_StartY;
uint16_t             m_XOffsetInVP; // For images smaller than viewport
uint16_t             m_YOffsetInVP;
double               m_ZoomFactor;

// Order reflects also order into the pipe :
// The original->Zoom->Cut (to visible) ->pixmap for acceleration.
QImage*              m_QImage;
QImage*              m_QImageZoomed;
QImage*              m_QImageCut;

protected:
// overloaded virtual ones.
bool  viewportEvent(QEvent*);
void  scrollContentsBy ( int dx, int dy );

private slots:
void MenuZoomFit();
void MenuZoom100();
void SizeReportTimerExpired();
void StatusReportTimerExpired();
void ResizeTimerExpired();

private:
void        RecalculateCut();
void        ContextMenu(QEvent* Event);

uint16_t    m_ZoomWidth;
uint16_t    m_ZoomHeight;
double      m_PreviousZoomFactor;
short       m_Grid;
short       m_GridX;
short       m_GridY;
short       m_DrawRectangle;

QAction*    m_AtnZoomFit;
QAction*    m_AtnZoom100;

QLabel*     m_SizeReport;
QString     m_SizeReportText;
int         m_SizeReportTimeOut;
QTimer*     m_SizeReportTimer;
QLabel*     m_StatusReport;
int         m_StatusReportTimeOut;
QTimer*     m_StatusReportTimer;
int         m_ResizeTimeOut;
QTimer*     m_ResizeTimer;
int         m_NewSize;

public:
};

#endif

////////////////////////////////////////////////////////////////////////////////
