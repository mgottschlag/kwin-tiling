/*
 * Copyright 2007 Frerich Raabe <raabe@kde.org>
 * Copyright 2007 Aaron Seigo <aseigo@kde.org
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "fullview.h"

#include <plasma/containment.h>
#include <KStandardDirs>
#include <KIconLoader>
#include <QIcon>
#include <QResizeEvent>

using namespace Plasma;

FullView::FullView(const QString &ff, QWidget *parent)
    : QGraphicsView(parent),
      m_formfactor(Plasma::Planar),
      m_containment(0),
      m_applet(0)
{
    setFrameStyle(QFrame::NoFrame);
    QString formfactor = ff.toLower();
    if (formfactor.isEmpty() || formfactor == "planar") {
        m_formfactor = Plasma::Planar;
    } else if (formfactor == "vertical") {
        m_formfactor = Plasma::Vertical;
    } else if (formfactor == "horizontal") {
        m_formfactor = Plasma::Horizontal;
    } else if (formfactor == "mediacenter") {
        m_formfactor = Plasma::MediaCenter;
    }

    setScene(&m_corona);
    connect(&m_corona, SIGNAL(sceneRectChanged(QRectF)),
            this, SLOT(sceneRectChanged(QRectF)));
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setAlignment(Qt::AlignLeft | Qt::AlignTop);

    QPixmap bg(KStandardDirs::locate("appdata", "checker.png"));
    m_corona.setBackgroundBrush(bg);
}

void FullView::addApplet(const QString &a)
{
    m_containment = m_corona.addContainment("null");
    m_containment->setFormFactor(m_formfactor);
    m_applet = m_containment->addApplet(a, QVariantList(), 0, QRectF(0, 0, -1, -1));
    m_applet->setFlag(QGraphicsItem::ItemIsMovable, false);

    setSceneRect(m_corona.sceneRect());
    setWindowTitle(m_applet->name());
    setWindowIcon(SmallIcon(m_applet->icon()));
}

void FullView::resizeEvent(QResizeEvent *event)
{
    if (!m_applet) {
        kDebug() << "no applet";
        return;
    }

    // The applet always keeps its aspect ratio, so let's respect it.
    float ratio = event->oldSize().width() / event->oldSize().height();
    float newPossibleWidth = size().height()*ratio;
    int newWidth;
    int newHeight;
    if (newPossibleWidth > size().width()) {
        newHeight = (int)(size().width()/ratio);
        newWidth = (int)(newHeight*ratio);
    } else {
        newWidth = (int)newPossibleWidth;
        newHeight = (int)(newWidth/ratio);
    }
    m_containment->resize(QSize(newWidth, newHeight));
    m_applet->setGeometry(QRectF(QPoint(0, 0), QSize(newWidth, newHeight)));
//    kDebug() << "oooooooooooooooooo";

    event->accept();

//     m_containment->resize(size());
//     m_applet->setGeometry(QRectF(QPoint(0, 0), size()));
}

void FullView::sceneRectChanged(const QRectF &rect)
{
    setSceneRect(rect);
}

#include "fullview.moc"

