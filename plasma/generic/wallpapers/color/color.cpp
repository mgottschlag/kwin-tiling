/*
 *   Copyright 2008 by Petri Damsten <damu@iki.fi>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "color.h"

#include <QPainter>
#include <QLinearGradient>
#include <QRadialGradient>
#include <KDebug>

enum BackgroundMode {
    SOLID,
    HORIZONTAL,
    VERTICAL,
    RECTANGULAR,
    RADIAL,
    TOP_LEFT_DIAGONAL,
    TOP_RIGHT_DIAGONAL
};

Color::Color(QObject *parent, const QVariantList &args)
    : Plasma::Wallpaper(parent, args)
{
}

void Color::init(const KConfigGroup &config)
{
    m_color1 = config.readEntry("color1", QColor(Qt::white));
    m_color2 = config.readEntry("color2", QColor(Qt::black));
    m_backgroundMode = config.readEntry("backgroundMode", (int)SOLID);
    emit update(boundingRect());
}

void Color::paint(QPainter *painter, const QRectF& exposedRect)
{
    switch (m_backgroundMode) {
    case SOLID: {
            painter->fillRect(exposedRect, m_color1);
            break;
        }

    case HORIZONTAL: {
            QLinearGradient gradient = QLinearGradient(boundingRect().topLeft(),
                                                       boundingRect().topRight());
            gradient.setColorAt(0, m_color1);
            gradient.setColorAt(1, m_color2);
            painter->fillRect(exposedRect, gradient);
            break;
        }

    case VERTICAL: {
            QLinearGradient gradient = QLinearGradient(boundingRect().topLeft(),
                                                       boundingRect().bottomLeft());
            gradient.setColorAt(0, m_color1);
            gradient.setColorAt(1, m_color2);
            painter->fillRect(exposedRect, gradient);
            break;
        }

    case RECTANGULAR: {
            // First draw a horizontal gradient covering the whole view/screen
            QLinearGradient horizontalGradient = QLinearGradient(boundingRect().topLeft(),
                                                                 boundingRect().topRight());
            horizontalGradient.setColorAt(0, m_color2);
            horizontalGradient.setColorAt(0.5, m_color1);
            horizontalGradient.setColorAt(1, m_color2);

            painter->fillRect(exposedRect, horizontalGradient);

            // Then draw two triangles with vertical gradient
            QLinearGradient verticalGradient = QLinearGradient(boundingRect().topLeft(),
                                                               boundingRect().bottomLeft());
            verticalGradient.setColorAt(0, m_color2);
            verticalGradient.setColorAt(0.5, m_color1);
            verticalGradient.setColorAt(1, m_color2);
            painter->setBrush(verticalGradient);
            painter->setPen(Qt::NoPen);

            QPolygon triangle = QPolygon(3);

            // Draw a triangle which starts from the top edge to the center
            triangle.append(boundingRect().topLeft().toPoint());
            triangle.append(boundingRect().topRight().toPoint());
            triangle.append(boundingRect().center().toPoint());
            painter->drawPolygon(triangle);

            triangle.clear();

            // Draw a triangle which starts from the bottom edge to the center
            triangle.append(boundingRect().bottomLeft().toPoint());
            triangle.append(boundingRect().bottomRight().toPoint());
            triangle.append(boundingRect().center().toPoint());
            painter->drawPolygon(triangle);

            break;
        }

    case RADIAL: {
            // The diameter of the gradient will be the max screen dimension
            int maxDimension = qMax(boundingRect().height(), boundingRect().width());

            QRadialGradient gradient = QRadialGradient(boundingRect().center(),
                                                       maxDimension / 2,
                                                       boundingRect().center());
            gradient.setColorAt(0, m_color1);
            gradient.setColorAt(1, m_color2);
            painter->fillRect(exposedRect, gradient);
            break;
        }

    case TOP_LEFT_DIAGONAL: {
            QLinearGradient gradient = QLinearGradient(boundingRect().topLeft(),
                                                       boundingRect().bottomRight());
            gradient.setColorAt(0, m_color1);
            gradient.setColorAt(1, m_color2);
            painter->fillRect(exposedRect, gradient);
            break;
        }

    case TOP_RIGHT_DIAGONAL: {
            QLinearGradient gradient = QLinearGradient(boundingRect().topRight(),
                                                       boundingRect().bottomLeft());
            gradient.setColorAt(0, m_color1);
            gradient.setColorAt(1, m_color2);
            painter->fillRect(exposedRect, gradient);
            break;
        }

    }
}

QWidget* Color::createConfigurationInterface(QWidget* parent)
{
    QWidget *widget = new QWidget(parent);
    m_ui.setupUi(widget);

    m_ui.m_color1->setColor(m_color1);
    m_ui.m_color2->setColor(m_color2);
    m_ui.m_backgroundMode->setCurrentIndex(m_backgroundMode);

    if (m_backgroundMode == SOLID) {
        m_ui.m_color2->setEnabled(false);
    } else {
        m_ui.m_color2->setEnabled(true);
    }

    connect(m_ui.m_color1, SIGNAL(changed(QColor)), this, SLOT(settingsModified()));
    connect(m_ui.m_color2, SIGNAL(changed(QColor)), this, SLOT(settingsModified()));
    connect(m_ui.m_backgroundMode, SIGNAL(currentIndexChanged(int)), this, SLOT(settingsModified()));

    connect(this, SIGNAL(settingsChanged(bool)), parent, SLOT(settingsChanged(bool)));

    return widget;
}

void Color::save(KConfigGroup &config)
{
    config.writeEntry("color1", m_color1);
    config.writeEntry("color2", m_color2);
    config.writeEntry("backgroundMode", m_backgroundMode);
}

void Color::settingsModified()
{
    m_color1 = m_ui.m_color1->color();
    m_color2 = m_ui.m_color2->color();
    m_backgroundMode = m_ui.m_backgroundMode->currentIndex();

    if (m_backgroundMode == SOLID) {
        m_ui.m_color2->setEnabled(false);
    } else {
        m_ui.m_color2->setEnabled(true);
    }

    emit settingsChanged(true);
    emit update(boundingRect());
}

#include "color.moc"
