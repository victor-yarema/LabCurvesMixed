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

#ifndef DLHISTOGRAMWINDOW_H
#define DLHISTOGRAMWINDOW_H

#include <QtGui>

#include "dlImage.h"
#include "dlImage8.h"

////////////////////////////////////////////////////////////////////////////////
//
// dlHistogramWindow is a Gui element showing a histogram.
//
////////////////////////////////////////////////////////////////////////////////

class dlHistogramWindow : public QWidget {

Q_OBJECT

public :

const dlImage*      m_RelatedImage;
QTimer*             m_ResizeTimer; // To circumvent multi resize events.

// Constructor.
dlHistogramWindow(const dlImage* RelatedImage,
                        QWidget* Parent);
// Destructor.
~dlHistogramWindow();

// NewRelatedImage to associate anonter dlImage with this window.
void UpdateView(const dlImage* NewRelatedImage = NULL);

protected:
void resizeEvent(QResizeEvent*);
void paintEvent(QPaintEvent*);
QSize sizeHint() const { return QSize(200,200); };
QSize minimumSizeHint() const { return QSize(100,100); };
int  heightForWidth(int w) const { return MIN(150,w);};
void contextMenuEvent(QContextMenuEvent *event);

private slots:
void ResizeTimerExpired();
void MenuLnX();
void MenuLnY();
void MenuCrop();
void MenuChannel();

private:
const dlImage8* m_Image8;
QPixmap*        m_QPixmap;
short           m_RecalcNeeded;
uint32_t        m_HistoMax;
short           m_PreviousHistogramGamma;
short           m_PreviousHistogramLogX;
short           m_PreviousHistogramLogY;
QString         m_PreviousFileName;
QAction*        m_AtnLnX;
QAction*        m_AtnLnY;
QAction*        m_AtnCrop;
QActionGroup*   m_ChannelGroup;
QAction*        m_AtnRGB;
QAction*        m_AtnR;
QAction*        m_AtnG;
QAction*        m_AtnB;

void CalculateHistogram();
};

#endif

////////////////////////////////////////////////////////////////////////////////
