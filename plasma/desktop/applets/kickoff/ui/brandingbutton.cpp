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

#include <KConfigGroup>
#include <KDebug>
#include <KStandardDirs>
#include <KRun>

#include <Plasma/Svg>
#include <Plasma/Theme>

namespace Kickoff
{

BrandingButton::BrandingButton(QWidget *parent)
        : QToolButton(parent),
        m_svg(new Plasma::Svg(this))
{
    m_svg->setImagePath("widgets/branding");
    m_svg->resize();
    checkBranding();
    connect(m_svg, SIGNAL(repaintNeeded()), this, SLOT(checkBranding()));
    connect(this, SIGNAL(clicked()), SLOT(openHomepage()));
    setCursor(Qt::PointingHandCursor);
}

QSize BrandingButton::minimumSizeHint() const
{
    return sizeHint();
}

QSize BrandingButton::sizeHint() const
{
    return m_size;
}

void BrandingButton::checkBranding()
{
    m_doingBranding = m_svg->isValid() && m_svg->hasElement("brilliant");

    if (!m_doingBranding) {
        m_size = QSize();
        return;
    }

    m_size = m_svg->elementSize("brilliant");
}

void BrandingButton::openHomepage()
{
    new KRun(Plasma::Theme::defaultTheme()->homepage(), topLevelWidget(), false, false);
}

void BrandingButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    if (!m_doingBranding) {
        //kDebug() << "bad branding svg!";
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

#include "brandingbutton.moc"


