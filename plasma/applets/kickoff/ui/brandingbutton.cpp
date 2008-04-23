/*
    Copyright 2008 Aaron Seigo <aseigo@kde.org>

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

#include "brandingbutton.h"

#include <QtGui/QPainter>

#include <KDebug>

#include "plasma/svg.h"


#include "plasma/theme.h"
#include <KStandardDirs>
namespace Kickoff
{

BrandingButton::BrandingButton(QWidget *parent)
    : QToolButton(parent),
      m_svg(new Plasma::Svg())
{
    m_svg->setImagePath("widgets/branding");
    m_svg->resize();
    setCursor(Qt::PointingHandCursor);
}

QSize BrandingButton::minimumSizeHint() const
{
    return sizeHint();
}

QSize BrandingButton::sizeHint() const
{
    return m_svg->elementSize("brilliant");
}

void BrandingButton::paintEvent(QPaintEvent *event)
{
    if (!m_svg->isValid()) {
        kDebug() << "bad branding svg!";
        return;
    }

    QPainter p(this);
    QSize s = m_svg->elementSize("brilliant");
    QRect r = rect();

    // center ourselves in the full rect
    if (r.width() > s.width()) {
        r.setX(r.x() + (r.width() - s.width()) / 2);
    }

    if (r.height() > s.height()) {
        r.setY(r.y() + (r.height() - s.height()) / 2);
    }

    m_svg->paint(&p, rect(), "brilliant");
}

} // namespace KickOff

