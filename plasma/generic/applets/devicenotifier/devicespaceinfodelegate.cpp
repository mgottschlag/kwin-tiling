/*  Copyright 2009 by Martin Klapetek <martin.klapetek@gmail.com>
    Copyright 2009 by Giulio Camuffo <giuliocamuffo@gmail.com>

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

#include "devicespaceinfodelegate.h"

#include <kcapacitybar.h>
#include <KLocale>
#include <KGlobal>
#include <KDebug>

#include <plasma/paintutils.h>

DeviceSpaceInfoDelegate::DeviceSpaceInfoDelegate(QObject* parent)
    : Plasma::Delegate(parent),
    m_capacityBar(0)
{
    m_capacityBar = new KCapacityBar(KCapacityBar::DrawTextInline);
    m_capacityBar->setMaximumWidth(200);
    m_capacityBar->setMinimumWidth(200);

    m_svg = new Plasma::FrameSvg(this);
    m_svg->setImagePath("widgets/viewitem");
    m_svg->setElementPrefix("hover");

}

DeviceSpaceInfoDelegate::~DeviceSpaceInfoDelegate()
{
    delete m_capacityBar;
}

void DeviceSpaceInfoDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    painter->save();

    QVariantList sizes = index.data(Qt::UserRole + 12).value<QVariantList>(); // DeviceFreeSpaceRole =  Qt::UserRole + 12
    qulonglong size = 0;
    if (sizes.count() == 2) {
        size = qulonglong(sizes[0].toULongLong());
        if (size) {
            qulonglong freeSpace = qulonglong(sizes[1].toULongLong());
            qulonglong usedSpace = size - freeSpace;

            m_capacityBar->setText(i18nc("@info:status Free disk space", "%1  free", KGlobal::locale()->formatByteSize(freeSpace)));
            m_capacityBar->setUpdatesEnabled(false);
            m_capacityBar->setValue(size > 0 ? (usedSpace * 100) / size : 0);
            m_capacityBar->setUpdatesEnabled(true);
            m_capacityBar->drawCapacityBar(painter, QRect(QPoint(option.rect.x()+48, Plasma::Delegate::rectAfterTitle(option, index).bottom()+4),
                                                        QSize(option.rect.width() - 50, 14)));
        }
    }

    const bool hover = option.state & (QStyle::State_MouseOver | QStyle::State_Selected);

    if (hover && (index.data(Qt::UserRole + 7).isValid() || size)) {
        QStyleOptionViewItem opt = option;
        opt.state = QStyle::State_None;

        Plasma::Delegate::paint(painter, opt, index);

        bool mouseOver;
        if (size) {
            mouseOver = hover;
        } else {
            mouseOver = option.state & QStyle::State_MouseOver;
        }

        if (mouseOver) {
            QColor backgroundColor = option.palette.color(QPalette::Highlight);
            backgroundColor.setAlphaF(0.2);
            QColor backgroundColor2 = option.palette.color(QPalette::Highlight);
            backgroundColor2.setAlphaF(0.5);

            QRect highlightRect = option.rect;

            QPen outlinePen(backgroundColor, 2);

            const int columns = index.model()->columnCount();
            const int column = index.column();
            int roundedRadius = 5;
            if (size) {
                if (columns > 1) {
                    roundedRadius = m_svg->marginSize(Plasma::RightMargin);
                    painter->setClipRect(option.rect);
                    highlightRect.adjust(0, 0, roundedRadius, 0);

                //last column, clip left (right for rtl)
                } else if (column == columns-1) {
                    roundedRadius = m_svg->marginSize(Plasma::LeftMargin);
                    painter->setClipRect(option.rect);
                    highlightRect.adjust(-roundedRadius, 0, 0, 0);

                //column < columns-1; clip both ways
                } else {
                    roundedRadius = m_svg->marginSize(Plasma::LeftMargin);
                    painter->setClipRect(option.rect);
                    highlightRect.adjust(-roundedRadius, 0, +roundedRadius, 0);
                }
            }

            QLinearGradient gradient(highlightRect.topLeft(), highlightRect.topRight());

            //reverse the gradient
            if (option.direction == Qt::RightToLeft) {
                gradient.setStart(highlightRect.topRight());
                gradient.setFinalStop(highlightRect.topLeft());
            }

            QRect titleRect = QRect(option.rect.x()+20, option.rect.y(), option.rect.width()-20, option.rect.height()/2);;
            gradient.setColorAt(0, backgroundColor);
            gradient.setColorAt(((qreal)titleRect.width()/3.0) / (qreal)highlightRect.width(), backgroundColor2);
            gradient.setColorAt(0.7, backgroundColor);
            outlinePen.setBrush(gradient);

            m_svg->resizeFrame(highlightRect.size());
            m_svg->paintFrame(painter, highlightRect.topLeft());
        }
    } else {
        Plasma::Delegate::paint(painter, option, index);
    }

    painter->restore();
}

#include "devicespaceinfodelegate.moc"