/*
 *   Copyright (C) 2007 Petri Damsten <damu@iki.fi>
 *   Copyright (C) 2008 Marco Martin <notmart@gmail.com>
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

#include "temperature.h"
#include <Plasma/SignalPlotter>
#include <Plasma/Meter>
#include <Plasma/Containment>
#include <Plasma/Theme>
#include <KConfigDialog>
#include <QGraphicsLinearLayout>
#include <QTimer>
#include <cmath>

Temperature::Temperature(QObject *parent, const QVariantList &args)
    : SM::Applet(parent, args), m_showPlotters(false), m_tempModel(0)
{
    setHasConfigurationInterface(true);
    resize(215 + 20 + 23, 109 + 20 + 25);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

Temperature::~Temperature()
{
}

void Temperature::init()
{
    KConfigGroup cg = persistentConfig();
    setEngine(dataEngine("systemmonitor"));
    m_showPlotters = cg.readEntry("showPlotters", true);
    setInterval(cg.readEntry("interval", 2) * 1000);

    setTitle(i18n("Temperature"));
    if (engine()->sources().count() == 0) {
        connect(engine(), SIGNAL(sourceAdded(QString)), this, SLOT(initLater(const QString)));
    } else {
        parseSources();
    }
}

void Temperature::parseSources()
{
    KConfigGroup cg = persistentConfig();
    setItems(cg.readEntry("temps", engine()->sources().filter(QRegExp(".*temp.*", Qt::CaseInsensitive))));
    connectToEngine();
}

void Temperature::initLater(const QString &name)
{
    // How we know all (cpu) sources are ready???
    if (name == "system/uptime") {
        QTimer::singleShot(0, this, SLOT(parseSources()));
    }
}

void Temperature::createConfigurationInterface(KConfigDialog *parent)
{
    QWidget *widget = new QWidget();
    ui.setupUi(widget);
    m_tempModel.clear();
    m_tempModel.setHorizontalHeaderLabels(QStringList() << i18n("Sensor")
                                                        << i18n("Name"));
    Plasma::DataEngine *engine = dataEngine("systemmonitor");
    QStringList temps = engine->sources().filter(QRegExp(".*temp.*", Qt::CaseInsensitive));

    QStandardItem *parentItem = m_tempModel.invisibleRootItem();
    foreach (const QString& temp, temps) {
        QStandardItem *item1 = new QStandardItem(temp);
        item1->setEditable(false);
        item1->setCheckable(true);
        if (items().contains(temp)) {
            item1->setCheckState(Qt::Checked);
        }
        QStandardItem *item2 = new QStandardItem(title(temp));
        item2->setEditable(true);
        parentItem->appendRow(QList<QStandardItem *>() << item1 << item2);
    }
    ui.treeView->setModel(&m_tempModel);
    ui.treeView->resizeColumnToContents(0);

    ui.showPlotters->setChecked(m_showPlotters);
    ui.intervalSpinBox->setValue(interval() / 1000);

    parent->setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Apply);
    parent->addPage(widget, i18n("Temperature"), "cpu");
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
}

void Temperature::configAccepted()
{
    KConfigGroup cg = persistentConfig();
    QStandardItem *parentItem = m_tempModel.invisibleRootItem();

    clearItems();
    for (int i = 0; i < parentItem->rowCount(); ++i) {
        QStandardItem *item = parentItem->child(i, 0);
        if (item) {
            cg.writeEntry(item->text(),
                        parentItem->child(i, 1)->text());
            if (item->checkState() == Qt::Checked) {
                appendItem(item->text());
            }
        }
    }
    cg.writeEntry("temps", items());

    m_showPlotters = ui.showPlotters->isChecked();
    cg.writeEntry("showPlotters", m_showPlotters);
    uint interval = ui.intervalSpinBox->value();
    cg.writeEntry("interval", interval);
    interval *= 1000;
    setInterval(interval);

    emit configNeedsSaving();
    connectToEngine();
}

QString Temperature::title(const QString& source)
{
    KConfigGroup cg = persistentConfig();
    return cg.readEntry(source, source.mid(source.lastIndexOf('/') + 1));
}

bool Temperature::addMeter(const QString& source)
{
    Plasma::DataEngine *engine = dataEngine("systemmonitor");
    Plasma::DataEngine::Data data;
    Plasma::Theme* theme = Plasma::Theme::defaultTheme();

    if (!engine) {
        return false;
    }
    data = engine->query(source);
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(Qt::Horizontal);
    layout->setContentsMargins(3, 3, 3, 3);
    layout->setSpacing(5);

    Plasma::Meter *meter = new Plasma::Meter(this);
    meter->setMeterType(Plasma::Meter::AnalogMeter);
    meter->setLabel(0, title(source));
    meter->setLabelColor(0, theme->color(Plasma::Theme::TextColor));
    meter->setLabel(1, QString());
    meter->setLabelColor(1, QColor("#000"));
    meter->setLabelAlignment(1, Qt::AlignCenter);
    QFont font = theme->font(Plasma::Theme::DefaultFont);
    font.setPointSize(7);
    meter->setLabelFont(0, font);
    meter->setLabelFont(1, font);
    meter->setMinimum(0);
    meter->setMaximum(110);
    layout->addItem(meter);
    appendMeter(source, meter);
    appendKeepRatio(meter);

    if (mode() != SM::Applet::Panel && m_showPlotters) {
        Plasma::SignalPlotter *plotter = new Plasma::SignalPlotter(this);
        plotter->addPlot(Qt::white);
        plotter->setUseAutoRange(true);
        plotter->setThinFrame(false);
        plotter->setShowLabels(false);
        plotter->setShowTopBar(false);
        plotter->setShowVerticalLines(false);
        plotter->setShowHorizontalLines(false);
        plotter->setSvgBackground("widgets/plot-background");
        layout->addItem(plotter);
        appendPlotter(source, plotter);
        setRatioOrientation(Qt::Horizontal);
    } else {
        setMinimumWidth(24);
    }
    mainLayout()->addItem(layout);

    dataUpdated(source, data);
    //setPreferredItemHeight(layout->preferredSize().height());
    setPreferredItemHeight(80);
    return true;
}

void Temperature::dataUpdated(const QString& source,
                              const Plasma::DataEngine::Data &data)
{
    if (!items().contains(source)) {
        return;
    }
    Plasma::Meter *w = meters().value(source);
    if (!w) {
        return;
    }
    w->setValue(data["value"].toDouble());

    qreal celsius = ((qreal)data["value"].toDouble());

    if (KGlobal::locale()->measureSystem() == KLocale::Metric) {
        w->setLabel(1, i18n("%1 °C", celsius));
    } else {
        w->setLabel(1, i18n("%1 °F", (celsius * 1.8) + 32));
    }

    if (m_showPlotters) {
        Plasma::SignalPlotter *plotter = plotters().value(source);
        if (plotter) {
            plotter->addSample(QList<double>() << data["value"].toDouble());
        }
    }
}

#include "temperature.moc"
