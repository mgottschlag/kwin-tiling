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

#include "bgmonitor.h"

#include <klocale.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kurl.h>

#include <QApplication>
#include <QDesktopWidget>

#include "kworkspace/screenpreviewwidget.h"

// Constants used (should they be placed somewhere?)
// Size of monitor image: 200x186
// Geometry of "display" part of monitor image: (23,14)-[151x115]

//BEGIN class BGMonitorArrangement
BGMonitorArrangement::BGMonitorArrangement(QWidget *parent)
    : QWidget(parent)
{
    m_pBGMonitor.resize(QApplication::desktop()->numScreens());

    int numScreens = QApplication::desktop()->numScreens();
    for (int screen = 0; screen < numScreens; ++screen) {
        ScreenPreviewWidget *previewWidget = new ScreenPreviewWidget(this);
        m_pBGMonitor[screen] = previewWidget;
        previewWidget->setWhatsThis(i18n("This picture of a monitor contains a preview of what the current settings will look like on your desktop."));

        connect(previewWidget, SIGNAL(imageDropped(QString)), this, SIGNAL(imageDropped(QString)));
        previewWidget->setFixedSize(180, 180);
    }

    parent->setFixedSize(210 * numScreens, 200);
    setFixedSize(210 * numScreens, 200);
    updateArrangement();
}


ScreenPreviewWidget *BGMonitorArrangement::monitor(unsigned screen) const
{
    return m_pBGMonitor[screen];
}


QRect BGMonitorArrangement::expandToPreview(const QRect &r) const
{
    double scaleX = 200.0 / 151.0;
    double scaleY = 186.0 / 115.0;
    return QRect(int(r.x() * scaleX), int(r.y() * scaleY), int(r.width() * scaleX), int(r.height() * scaleY));
}


QSize BGMonitorArrangement::expandToPreview(const QSize &s) const
{
    double scaleX = 200.0 / 151.0;
    double scaleY = 186.0 / 115.0;
    return QSize(int(s.width() * scaleX), int(s.height() * scaleY));
}


QPoint BGMonitorArrangement::expandToPreview(const QPoint &p) const
{
    double scaleX = 200.0 / 151.0;
    double scaleY = 186.0 / 115.0;
    return QPoint(int(p.x() * scaleX), int(p.y() * scaleY));
}


void BGMonitorArrangement::updateArrangement()
{
    // In this function, sizes, etc have a normal value, and their "expanded"
    // value. The expanded value is used for setting the size of the monitor
    // image that contains the preview of the background. The monitor image
    // will set the background preview back to the normal value.

    QRect overallGeometry;
    for (int screen = 0; screen < QApplication::desktop()->numScreens(); ++screen)
        overallGeometry |= QApplication::desktop()->screenGeometry(screen);

    QRect expandedOverallGeometry = expandToPreview(overallGeometry);

    double scale = qMin(
                       double(width()) / double(expandedOverallGeometry.width()),
                       double(height()) / double(expandedOverallGeometry.height())
                   );

    m_combinedPreviewSize = overallGeometry.size() * scale;

    m_maxPreviewSize = QSize(0, 0);
    int previousMax = 0;

    for (int screen = 0; screen < QApplication::desktop()->numScreens(); ++screen) {
        QPoint topLeft = (QApplication::desktop()->screenGeometry(screen).topLeft() - overallGeometry.topLeft()) * scale;
        QPoint expandedTopLeft = expandToPreview(topLeft);

        QSize previewSize = QApplication::desktop()->screenGeometry(screen).size() * scale;
        QSize expandedPreviewSize = expandToPreview(previewSize);

        if ((previewSize.width() * previewSize.height()) > previousMax) {
            previousMax = previewSize.width() * previewSize.height();
            m_maxPreviewSize = previewSize;
        }


        m_pBGMonitor[screen]->setGeometry(QRect(expandedTopLeft, expandedPreviewSize));
        m_pBGMonitor[screen]->setRatio((qreal)previewSize.width() / (qreal)previewSize.height());
    }
}


void BGMonitorArrangement::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    updateArrangement();
}


void BGMonitorArrangement::setPixmap(const QPixmap &pm)
{
    for (int screen = 0; screen < m_pBGMonitor.size(); ++screen) {
        m_pBGMonitor[screen]->setPreview(pm);
    }
}
//END class BGMonitorArrangement

#include "bgmonitor.moc"
