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
#include <Plasma/ToolTipManager>
#include <KConfigDialog>
#include <KUnitConversion/Converter>
#include <KUnitConversion/Value>
#include <QGraphicsLinearLayout>
#include <QTimer>
#include <cmath>

using namespace KUnitConversion;

Temperature::Temperature(QObject *parent, const QVariantList &args)
    : SM::Applet(parent, args)
    , m_tempModel(0)
    , m_rx(".*temp.*", Qt::CaseInsensitive)
{
    setHasConfigurationInterface(true);
    resize(215 + 20 + 23, 109 + 20 + 25);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(themeChanged()));
    m_sourceTimer.setSingleShot(true);
    connect(&m_sourceTimer, SIGNAL(timeout()), this, SLOT(sourcesAdded()));
}

Temperature::~Temperature()
{
}

void Temperature::init()
{
    KGlobal::locale()->insertCatalog("plasma_applet_system-monitor");
    KConfigGroup cg = config();
    setEngine(dataEngine("systemmonitor"));
    setInterval(cg.readEntry("interval", 2) * 1000);
    setTitle(i18n("Temperature"));

    /* At the time this method is running, not all source may be connected. */
    connect(engine(), SIGNAL(sourceAdded(QString)), this, SLOT(sourceAdded(const QString)));
    foreach (const QString& source, engine()->sources()) {
        sourceAdded(source);
    }
}

void Temperature::sourceAdded(const QString& name)
{
    if (m_rx.indexIn(name) != -1) {
        //kDebug() << m_rx.cap(1);
        m_sources << name;
        if (!m_sourceTimer.isActive()) {
            m_sourceTimer.start(0);
        }
    }
}

void Temperature::sourcesAdded()
{
    KConfigGroup cg = config();
    setItems(cg.readEntry("temps", m_sources.mid(0, 5)));
    connectToEngine();
}

void Temperature::createConfigurationInterface(KConfigDialog *parent)
{
    QWidget *widget = new QWidget();
    ui.setupUi(widget);
    m_tempModel.clear();
    m_tempModel.setHorizontalHeaderLabels(QStringList() << i18n("Sensor")
                                                        << i18n("Name"));

    QStandardItem *parentItem = m_tempModel.invisibleRootItem();
    foreach (const QString& temp, m_sources) {
        QStandardItem *item1 = new QStandardItem(temp);
        item1->setEditable(false);
        item1->setCheckable(true);
        if (items().contains(temp)) {
            item1->setCheckState(Qt::Checked);
        }
        QStandardItem *item2 = new QStandardItem(temperatureTitle(temp));
        item2->setEditable(true);
        parentItem->appendRow(QList<QStandardItem *>() << item1 << item2);
    }
    ui.treeView->setModel(&m_tempModel);
    ui.treeView->resizeColumnToContents(0);

    ui.intervalSpinBox->setValue(interval() / 1000);
    ui.intervalSpinBox->setSuffix(ki18np(" second", " seconds"));
    parent->setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Apply);
    parent->addPage(widget, i18n("Temperature"), "cpu");

    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
}

void Temperature::configAccepted()
{
    KConfigGroup cg = config();
    KConfigGroup cgGlobal = globalConfig();
    QStandardItem *parentItem = m_tempModel.invisibleRootItem();

    clearItems();
    for (int i = 0; i < parentItem->rowCount(); ++i) {
        QStandardItem *item = parentItem->child(i, 0);
        if (item) {
            cgGlobal.writeEntry(item->text(),
                                parentItem->child(i, 1)->text());
            if (item->checkState() == Qt::Checked) {
                appendItem(item->text());
            }
        }
    }
    cg.writeEntry("temps", items());
    uint interval = ui.intervalSpinBox->value();
    cg.writeEntry("interval", interval);
    interval *= 1000;
    setInterval(interval);

    emit configNeedsSaving();
    connectToEngine();
}

QString Temperature::temperatureTitle(const QString& source)
{
    KConfigGroup cg = globalConfig();
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
    QString title = temperatureTitle(source);

    if (mode() != SM::Applet::Panel) {
        Plasma::Meter *meter = new Plasma::Meter(this);
        meter->setMeterType(Plasma::Meter::AnalogMeter);
        if (mode() != SM::Applet::Panel) {
            meter->setLabel(0, title);
            meter->setLabelColor(0, theme->color(Plasma::Theme::TextColor));
            meter->setLabel(1, QString());
            meter->setLabelColor(1, QColor("#000"));
            meter->setLabelAlignment(1, Qt::AlignCenter);
            QFont font = theme->font(Plasma::Theme::DefaultFont);
            font.setPointSize(7);
            meter->setLabelFont(0, font);
            meter->setLabelFont(1, font);
        }
        meter->setMinimum(0);
        meter->setMaximum(110);
        layout->addItem(meter);
        appendMeter(source, meter);
        appendKeepRatio(meter);
        setMinimumWidth(24);
    }
    Plasma::SignalPlotter *plotter = new Plasma::SignalPlotter(this);
    plotter->addPlot(theme->color(Plasma::Theme::TextColor));
    plotter->setThinFrame(false);
    plotter->setShowLabels(false);
    plotter->setShowTopBar(true);
    plotter->setShowVerticalLines(false);
    plotter->setShowHorizontalLines(false);
    plotter->setTitle(title);
    plotter->setUseAutoRange(false);
    plotter->setVerticalRange(0.0, 110.0);
    plotter->setFontColor(theme->color(Plasma::Theme::TextColor));
    QFont font = theme->font(Plasma::Theme::DefaultFont);
    font.setPointSize(8);
    plotter->setFont(font);
    plotter->setSvgBackground("widgets/plot-background");
    layout->addItem(plotter);
    appendPlotter(source, plotter);
    setRatioOrientation(Qt::Horizontal);
    mainLayout()->addItem(layout);

    dataUpdated(source, data);
    setPreferredItemHeight(80);
    return true;
}

void Temperature::themeChanged()
{
    Plasma::Theme* theme = Plasma::Theme::defaultTheme();
    foreach (Plasma::Meter *w, meters()) {
        w->setLabelColor(0, theme->color(Plasma::Theme::TextColor));
        QFont font = theme->font(Plasma::Theme::DefaultFont);
        font.setPointSize(7);
        w->setLabelFont(0, font);
        w->setLabelFont(1, font);
    }
}

void Temperature::dataUpdated(const QString& source,
                              const Plasma::DataEngine::Data &data)
{
    if (!items().contains(source)) {
        return;
    }
    Plasma::Meter *w = meters().value(source);
    Plasma::SignalPlotter *plotter = plotters().value(source);
    QString temp;
    qreal value = (qreal)data["value"].toDouble();

    if (data["units"].toString() == "F" &&
        KGlobal::locale()->measureSystem() == KLocale::Metric) {
        value = Value(value, Fahrenheit).convertTo(Celsius).number();
    } else if (data["units"].toString() == "C" &&
        KGlobal::locale()->measureSystem() != KLocale::Metric) {
        value = Value(value, Celsius).convertTo(Fahrenheit).number();
    }

    value = int(value * 10) / 10.0;

    if (KGlobal::locale()->measureSystem() == KLocale::Metric) {
        temp = i18n("%1 °C", value);
    } else {
        temp = i18n("%1 °F", value);
    }

    if (w) {
        w->setValue(data["value"].toDouble());
        if (mode() != SM::Applet::Panel) {
            w->setLabel(1, temp);
        }
    }
    if (plotter) {
        plotter->addSample(QList<double>() << data["value"].toDouble());
    }

    if (mode() == SM::Applet::Panel) {
        m_html[source] = QString("<tr><td>%1</td><td>%2</td></tr>")
                .arg(temperatureTitle(source)).arg(temp);
        QString html = "<table>";
        foreach (const QString& s, m_html.keys()) {
            html += m_html[s];
        }
        html += "</table>";
        Plasma::ToolTipContent data(title(), html);
        Plasma::ToolTipManager::self()->setContent(this, data);
    }
}

#include "temperature.moc"
