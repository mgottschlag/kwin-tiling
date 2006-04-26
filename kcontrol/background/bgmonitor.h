/* vi: ts=8 sts=4 sw=4
   kate: space-indent on; indent-width 4; indent-mode cstyle;

   This file is part of the KDE project, module kcmbackground.

   Copyright (C) 2002 Laurent Montel <montell@club-internet.fr>
   Copyright (C) 2003 Waldo Bastian <bastian@kde.org>
   Copyright (C) 2005 David Saxton <david@bluehaze.org>
  
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License 
   version 2 as published by the Free Software Foundation.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
   
   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */

#ifndef _BGMONITOR_H_
#define _BGMONITOR_H_

#include <qwidget.h>
#include <qlabel.h>
//Added by qt3to4:
#include <QDragEnterEvent>
#include <QDropEvent>

class BGMonitor;
class BGMonitorLabel;
class QPixmap;

  /**
 * This class arranges and resizes a set of monitor images according to the
 * monitor geometries.
 */
class BGMonitorArrangement : public QWidget
{
    Q_OBJECT
public:
    BGMonitorArrangement(QWidget *parent, const char *name=0L);
    
    /**
     * Splits up the pixmap according to monitor geometries and sets each
     * BGMonitor pixmap accordingly.
     */
    void setPixmap( const QPixmap & pm );
    QSize combinedPreviewSize() const { return m_combinedPreviewSize; }
    QSize maxPreviewSize() const { return m_maxPreviewSize; }
    unsigned numMonitors() const { return m_pBGMonitor.size(); }
    
    BGMonitor * monitor( unsigned screen ) const;
    void updateArrangement();

signals:
    void imageDropped(const QString &);
    
protected:
    virtual void resizeEvent( QResizeEvent * );
    QRect expandToPreview( QRect r ) const;
    QSize expandToPreview( QSize s ) const;
    QPoint expandToPreview( QPoint p ) const;
    
    QVector<BGMonitorLabel*> m_pBGMonitor;
    QSize m_combinedPreviewSize;
    QSize m_maxPreviewSize;
};

/**
 * Contains a BGMonitor.
 */
class BGMonitorLabel : public QLabel
{
public:
    BGMonitorLabel(QWidget *parent, const char *name=0L);
    
    BGMonitor * monitor() const { return m_pBGMonitor; }
    void updateMonitorGeometry();
    
    void setPreviewPosition( QRect r ) { m_previewPosition = r; }
    QRect previewPosition() const { return m_previewPosition; }
    
protected:
    virtual void resizeEvent( QResizeEvent * );
    BGMonitor * m_pBGMonitor;
    QRect m_previewPosition;
};

/**
 * This class handles drops on the preview monitor.
 */
class BGMonitor : public QLabel
{
    Q_OBJECT
public:

    BGMonitor(QWidget *parent, const char *name=0L);

Q_SIGNALS:
    void imageDropped(const QString &);

protected:
    virtual void dropEvent(QDropEvent *);
    virtual void dragEnterEvent(QDragEnterEvent *);
};


#endif
