/*
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

#ifndef FULLVIEW_H
#define FULLVIEW_H

#include <plasma/corona.h>

#include <QGraphicsView>

class FullView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit FullView(const QString &formfactor = "planar", const QString &location = "floating", QWidget *parent = 0);

    void addApplet(const QString &a, const QVariantList &args = QVariantList());

protected slots:
    void sceneRectChanged(const QRectF &rect);
    void resizeEvent(QResizeEvent *event);

private:
    Plasma::Corona m_corona;
    Plasma::FormFactor m_formfactor;
    Plasma::Location m_location;
    Plasma::Containment *m_containment;
    Plasma::Applet *m_applet;
};

#endif

