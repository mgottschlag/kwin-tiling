/* This file is part of the KDE libraries

   Copyright (C) 2009 Marco Martin <notmart@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "screenpreviewwidget.h"

#include <QResizeEvent>
#include <QPaintEvent>
#include <QPainter>


#include "plasma/framesvg.h"
#include "plasma/wallpaper.h"


class ScreenPreviewWidgetPrivate : public QWidget
{
public:
    ScreenPreviewWidgetPrivate(ScreenPreviewWidget *screen)
          : q(screen),
            wallpaper(0),
            ratio(1)
    {}
    ~ScreenPreviewWidgetPrivate()
    {}

    void updateRect(const QRectF& rect)
    {
        q->update(rect.toRect().translated(q->previewRect().topLeft()));
    }

    ScreenPreviewWidget *q;
    Plasma::Wallpaper* wallpaper;
    Plasma::FrameSvg *screenGraphics;
    QPixmap preview;
    QSize monitorSize;
    qreal ratio;
};

ScreenPreviewWidget::ScreenPreviewWidget(QWidget *parent)
    : QWidget(parent),
      d(new ScreenPreviewWidgetPrivate(this))
{
    d->screenGraphics = new Plasma::FrameSvg(this);
    d->screenGraphics->setImagePath("widgets/monitor");
}

ScreenPreviewWidget::~ScreenPreviewWidget()
{
}

void ScreenPreviewWidget::setPreview(const QPixmap &preview)
{
    d->preview = preview;
}

const QPixmap ScreenPreviewWidget::preview() const
{
    return d->preview;
}

void ScreenPreviewWidget::setPreview(Plasma::Wallpaper* wallpaper)
{
    d->preview = QPixmap();
    d->wallpaper = wallpaper;
    if (d->wallpaper) {
        connect(d->wallpaper, SIGNAL(update(const QRectF &)),
                this, SLOT(updateRect(const QRectF &)));
        resizeEvent(0);
    }
}

void ScreenPreviewWidget::setRatio(const qreal ratio)
{
    d->ratio = ratio;
}

qreal ScreenPreviewWidget::ratio() const
{
    return d->ratio;
}

QRect ScreenPreviewWidget::previewRect() const
{
    return d->screenGraphics->contentsRect().toRect();
}

void ScreenPreviewWidget::resizeEvent(QResizeEvent *e)
{
    Q_UNUSED(e)

    d->monitorSize = QSize(size().width(), size().width()/d->ratio);

    d->screenGraphics->resizeFrame(d->monitorSize);

    if (d->wallpaper) {
        d->wallpaper->setBoundingRect(QRect(QPoint(0,0), previewRect().size()));
    }
}

void ScreenPreviewWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QPoint standPosition(d->monitorSize.width()/2 - d->screenGraphics->elementSize("base").width()/2, d->screenGraphics->contentsRect().bottom());

    d->screenGraphics->paint(&painter, QRect(standPosition, d->screenGraphics->elementSize("base")), "base");
    d->screenGraphics->paintFrame(&painter);

    painter.save();
    if (d->wallpaper) {
        //FIXME: seems to not be other ways beside translating the painter
        painter.translate(previewRect().topLeft());
        d->wallpaper->paint(&painter, event->rect().translated(-previewRect().topLeft()));
    } else if (!d->preview.isNull()) {
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        painter.drawPixmap(d->screenGraphics->contentsRect(), d->preview, d->preview.rect());
    }
    painter.restore();

    d->screenGraphics->paint(&painter, d->screenGraphics->contentsRect(), "glass");
}

#include "screenpreviewwidget.moc"
