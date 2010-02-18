/*
 *   Copyright (C) 2010 Petri Damsten <damu@iki.fi>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#include "plotter.h"
#include <Plasma/SignalPlotter>
#include <Plasma/Meter>
#include <Plasma/Theme>
#include <Plasma/Frame>
#include <QGraphicsLinearLayout>

namespace SM {

Plotter::Plotter(QGraphicsItem* parent, Qt::WindowFlags wFlags)
    : QGraphicsWidget(parent, wFlags)
    , m_layout(0)
    , m_plotter(0)
    , m_meter(0)
    , m_plotCount(1)
    , m_min(0.0)
    , m_max(0.0)
    , m_overlayFrame(0)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    createWidgets();
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(themeChanged()));
}

Plotter::~Plotter()
{
}

void Plotter::setAnalog(bool analog)
{
    if (analog && m_layout->count() < 2) {
        m_meter = new Plasma::Meter(this);
        m_meter->setMeterType(Plasma::Meter::AnalogMeter);
        m_meter->setLabel(0, m_title);
        m_meter->setLabel(1, QString());
        m_meter->setLabelAlignment(1, Qt::AlignCenter);
        m_layout->insertItem(0, m_meter);
        m_meter->setMinimum(m_min);
        m_meter->setMaximum(m_max);
        m_meter->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        themeChanged();
        resizeEvent(0);
    } else if (m_layout->count() > 1) {
        m_layout->removeAt(0);
        delete m_meter;
        m_meter = 0;
    }
}

void Plotter::setMinMax(double min, double max)
{
    if (m_meter) {
        m_meter->setMinimum(min);
        m_meter->setMaximum(max);
    }
    m_plotter->setVerticalRange(min, max);
    m_min = min;
    m_max = max;
}

const QString& Plotter::title()
{
    return m_title;
}

void Plotter::setTitle(const QString& title)
{
    m_plotter->setTitle(title);
    if (m_meter) {
        m_meter->setLabel(0, title);
    }
    m_title = title;
}

void Plotter::setUnit(const QString& unit)
{
    m_plotter->setUnit(unit);
    m_unit = unit;
}

void Plotter::setScale(qreal scale)
{
    m_plotter->scale(scale);
}

void Plotter::setPlotCount(int count)
{
    for (int i = 0; i < m_plotCount; ++i) {
        m_plotter->removePlot(0);
    }
    m_plotCount = count;
    for (int i = 0; i < m_plotCount; ++i) {
        // TODO color adjust
        Plasma::Theme* theme = Plasma::Theme::defaultTheme();
        m_plotter->addPlot(theme->color(Plasma::Theme::TextColor));
    }
}

void Plotter::createWidgets()
{
    m_layout = new QGraphicsLinearLayout(Qt::Horizontal);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(5);
    setLayout(m_layout);

    m_plotter = new Plasma::SignalPlotter(this);
    m_plotter->setThinFrame(false);
    m_plotter->setShowLabels(false);
    m_plotter->setShowTopBar(true);
    m_plotter->setShowVerticalLines(false);
    m_plotter->setShowHorizontalLines(false);
    m_plotter->setUseAutoRange(false);
    m_plotter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_layout->addItem(m_plotter);
    themeChanged();
    resizeEvent(0);
}

void Plotter::themeChanged()
{
    Plasma::Theme* theme = Plasma::Theme::defaultTheme();
    if (m_meter) {
        m_meter->setLabelColor(0, theme->color(Plasma::Theme::TextColor));
        QFont font = theme->font(Plasma::Theme::DefaultFont);
        font.setPointSize(7);
        m_meter->setLabelFont(0, font);
        m_meter->setLabelFont(1, font);
        m_meter->setLabelColor(0, theme->color(Plasma::Theme::TextColor));
        m_meter->setLabelColor(1, QColor("#000"));
    }
    QFont font = theme->font(Plasma::Theme::DefaultFont);
    font.setPointSize(8);
    m_plotter->setFont(font);
    m_plotter->setFontColor(theme->color(Plasma::Theme::TextColor));
    m_plotter->setSvgBackground("widgets/plot-background");
    QColor linesColor = theme->color(Plasma::Theme::TextColor);
    linesColor.setAlphaF(0.4);
    m_plotter->setHorizontalLinesColor(linesColor);
    m_plotter->setVerticalLinesColor(linesColor);
    setPlotCount(m_plotCount);
}

void Plotter::addSample(const QList<double>& values)
{
    m_plotter->addSample(values);
    QStringList list;
    foreach (double value, values) {
        list << QString("%1 %2").arg(value, 0, 'f', (value > 1000.0) ? 0 : 1).arg(m_unit);
    }
    setOverlayText(list.join(" / "));
}

void Plotter::setOverlayText(const QString& text)
{
    if (!m_overlayFrame) {
        QGraphicsLinearLayout* layout = new QGraphicsLinearLayout(Qt::Vertical, m_plotter);
        m_plotter->setLayout(layout);
        m_overlayFrame = new Plasma::Frame(m_plotter);
        m_overlayFrame->setZValue(10);
        m_overlayFrame->resize(m_overlayFrame->size().height() * 2.5,
                               m_overlayFrame->size().height());
        layout->addStretch();
        QGraphicsLinearLayout* layout2 = new QGraphicsLinearLayout(Qt::Horizontal, layout);
        layout2->addStretch();
        layout2->addItem(m_overlayFrame);
        layout2->addStretch();
        layout->addItem(layout2);
    }
    m_overlayFrame->setText(text);
    if (m_meter) {
        m_meter->setLabel(1, text);
    }
}

void Plotter::resizeEvent(QGraphicsSceneResizeEvent* event)
{
    Q_UNUSED(event)
    qreal h = size().height();
    QFontMetrics metrics(m_plotter->font());
    bool showTopBar = (metrics.height() < m_plotter->size().height() / 3);

    m_plotter->setShowTopBar(showTopBar);

    bool show = false;

    if (size().width() > 250 && h > 150) {
        show = true;
    }
    m_plotter->setShowLabels(show);
    m_plotter->setShowHorizontalLines(show);
    // TODO show/hide overlay text

    if (m_meter) {
        m_meter->setMinimumSize(h, h);
    }
}

} // namespace

#include "plotter.moc"
