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
#include <QGraphicsLinearLayout>
#include <QTimer>
#include <cmath>

Temperature::Temperature(QObject *parent, const QVariantList &args)
    : SM::Applet(parent, args), m_tempModel(0), m_showPlotters(false)
{
    setHasConfigurationInterface(true);
    resize(215 + 20 + 23, 109 + 20 + 25);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(themeChanged()));
}

Temperature::~Temperature()
{
}

void Temperature::init()
{
    KConfigGroup cg = config();
    setEngine(dataEngine("systemmonitor"));
    setInterval(cg.readEntry("interval", 2) * 1000);
    setTitle(i18n("Temperature"));

    Plasma::Theme* theme = Plasma::Theme::defaultTheme();
    m_showTopBar = cg.readEntry("showTopBar", true);
    m_showPlotters = cg.readEntry("showPlotters", true);
    m_showBackground = cg.readEntry("showBackground", true);
    m_graphColor = cg.readEntry("graphColor", QColor(theme->color(Plasma::Theme::TextColor)));

    if (engine()->sources().count() == 0) {
        connect(engine(), SIGNAL(sourceAdded(QString)), this, SLOT(initLater(const QString)));
    } else {
        parseSources();
    }
}

void Temperature::parseSources()
{
    KConfigGroup cg = config();
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
        QStandardItem *item2 = new QStandardItem(temperatureTitle(temp));
        item2->setEditable(true);
        parentItem->appendRow(QList<QStandardItem *>() << item1 << item2);
    }
    ui.treeView->setModel(&m_tempModel);
    ui.treeView->resizeColumnToContents(0);

    ui.intervalSpinBox->setValue(interval() / 1000);
    updateSpinBoxSuffix(interval() / 1000);
    parent->setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Apply);
    connect(ui.intervalSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateSpinBoxSuffix(int)));
    parent->addPage(widget, i18n("Temperature"), "cpu");

    widget = new QWidget();
    uiAdv.setupUi(widget);
    uiAdv.showTopBarCheckBox->setChecked(m_showTopBar);
    uiAdv.showPlotters->setChecked(m_showPlotters);
    uiAdv.showBackgroundCheckBox->setChecked(m_showBackground);
    uiAdv.graphColorCombo->setColor(m_graphColor);
    parent->addPage(widget, i18n("Advanced"), "preferences-other");

    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
}

void Temperature::updateSpinBoxSuffix(int interval)
{
    ui.intervalSpinBox->setSuffix(QString(" ") + i18np("second", "seconds", interval));
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

    cg.writeEntry("showTopBar", m_showTopBar = uiAdv.showTopBarCheckBox->isChecked());
    cg.writeEntry("showPlotters", m_showPlotters = uiAdv.showPlotters->isChecked());
    cg.writeEntry("showBackground", m_showBackground = uiAdv.showBackgroundCheckBox->isChecked());
    cg.writeEntry("graphColor", m_graphColor = uiAdv.graphColorCombo->color());

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

    if (mode() != SM::Applet::Panel || (mode() == SM::Applet::Panel && !m_showPlotters)) {
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
    if (m_showPlotters) {
        Plasma::SignalPlotter *plotter = new Plasma::SignalPlotter(this);
        plotter->addPlot(m_graphColor);
        plotter->setUseAutoRange(true);
        plotter->setThinFrame(false);
        plotter->setShowLabels(false);
        plotter->setShowTopBar(m_showTopBar);
        plotter->setShowVerticalLines(false);
        plotter->setShowHorizontalLines(false);
        plotter->setTitle(title);
        plotter->setFontColor(theme->color(Plasma::Theme::HighlightColor));
        QFont font = theme->font(Plasma::Theme::DefaultFont);
        font.setPointSize(8);
        plotter->setFont(font);
        if (m_showBackground) {
            plotter->setSvgBackground("widgets/plot-background");
        } else {
            plotter->setSvgBackground(QString());
            plotter->setBackgroundColor(Qt::transparent);
        }
        layout->addItem(plotter);
        appendPlotter(source, plotter);
        setRatioOrientation(Qt::Horizontal);
    }
    mainLayout()->addItem(layout);

    dataUpdated(source, data);
    setPreferredItemHeight(80);
    return true;
}

void Temperature::themeChanged()
{
    Plasma::Theme* theme = Plasma::Theme::defaultTheme();
    foreach (Plasma::Meter *w, meters().values()) {
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
    qreal celsius = ((qreal)data["value"].toDouble());

    if (KGlobal::locale()->measureSystem() == KLocale::Metric) {
        temp = i18n("%1 °C", celsius);
    } else {
        temp = i18n("%1 °F", (celsius * 1.8) + 32);
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
