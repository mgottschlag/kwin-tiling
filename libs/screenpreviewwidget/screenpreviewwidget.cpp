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


class ScreenPreviewWidgetPrivate
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

    void updateScreenGraphics()
    {
        QRect bounds(QPoint(0,0), QSize(q->size().width(), q->height() - screenGraphics->elementSize("base").height() + screenGraphics->marginSize(Plasma::BottomMargin)));

        QSize monitorSize(q->size().width(), q->size().width()/ratio);

        monitorSize.scale(bounds.size(), Qt::KeepAspectRatio);

        screenGraphics->resizeFrame(monitorRect.size());

        previewRect = screenGraphics->contentsRect().toRect();
        previewRect.moveCenter(bounds.center());
        monitorRect = QRect(QPoint(0,0), monitorSize);
        monitorRect.moveCenter(bounds.center());

        if (wallpaper) {
            wallpaper->setBoundingRect(QRect(QPoint(0,0), previewRect.size()));
        }
    }

    ScreenPreviewWidget *q;
    Plasma::Wallpaper* wallpaper;
    Plasma::FrameSvg *screenGraphics;
    QPixmap preview;
    QRect monitorRect;
    qreal ratio;
    QRect previewRect;
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
    d->updateScreenGraphics();
}

qreal ScreenPreviewWidget::ratio() const
{
    return d->ratio;
}

QRect ScreenPreviewWidget::previewRect() const
{
    return d->previewRect;
}

void ScreenPreviewWidget::resizeEvent(QResizeEvent *e)
{
    Q_UNUSED(e)

    d->updateScreenGraphics();
}

void ScreenPreviewWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QPoint standPosition(d->monitorRect.center().x() - d->screenGraphics->elementSize("base").width()/2, d->screenGraphics->contentsRect().bottom());

    d->screenGraphics->paint(&painter, QRect(standPosition, d->screenGraphics->elementSize("base")), "base");
    d->screenGraphics->paintFrame(&painter, d->monitorRect.topLeft());

    painter.save();
    if (d->wallpaper) {
        //FIXME: seems to not be other ways beside translating the painter
        painter.translate(previewRect().topLeft());
        d->wallpaper->paint(&painter, event->rect().translated(-previewRect().topLeft()));
    } else if (!d->preview.isNull()) {
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        painter.drawPixmap(d->previewRect, d->preview, d->preview.rect());
    }
    painter.restore();

    d->screenGraphics->paint(&painter, d->previewRect, "glass");
}

void ScreenPreviewWidget::dropEvent(QDropEvent *e)
{
    if (!KUrl::List::canDecode(e->mimeData()))
        return;

    const KUrl::List uris(KUrl::List::fromMimeData(e->mimeData()));
    if (uris.count() > 0) {
        // TODO: Download remote file
        if (uris.first().isLocalFile())
           emit imageDropped(uris.first().path());
    }
}

#include "screenpreviewwidget.moc"
