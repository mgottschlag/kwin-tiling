/*
 *   Copyright (C) 2008 Petri Damsten <damu@iki.fi>
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

#include "ram.h"
#include <Plasma/SignalPlotter>
#include <Plasma/Theme>
#include <Plasma/ToolTipManager>
#include <KConfigDialog>
#include <QTimer>
#include <QGraphicsLinearLayout>

SM::Ram::Ram(QObject *parent, const QVariantList &args)
    : SM::Applet(parent, args)
{
    setHasConfigurationInterface(true);
    resize(234 + 20 + 23, 135 + 20 + 25);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(themeChanged()));
}

SM::Ram::~Ram()
{
}

void SM::Ram::init()
{
    KConfigGroup cg = config();
    setEngine(dataEngine("systemmonitor"));
    setInterval(cg.readEntry("interval", 2) * 1000);
    setTitle(i18n("RAM"));
    
    Plasma::Theme* theme = Plasma::Theme::defaultTheme();
    m_showTopBar = cg.readEntry("showTopBar", true);
    m_showBackground = cg.readEntry("showBackground", true);
    m_graphColor = cg.readEntry("graphColor", QColor(theme->color(Plasma::Theme::TextColor)));

    if (engine()->sources().count() == 0) {
        connect(engine(), SIGNAL(sourceAdded(QString)), this, SLOT(initLater(const QString)));
    } else {
        parseSources();
    }
}

void SM::Ram::parseSources()
{
    m_memories << "mem/physical/application" << "mem/swap/used";
    KConfigGroup cg = config();
    setItems(cg.readEntry("memories", QStringList() << m_memories));
    connectToEngine();
}

void SM::Ram::initLater(const QString &name)
{
    // How we know all (ram) sources are ready???
    if (name == "system/uptime") {
        QTimer::singleShot(0, this, SLOT(parseSources()));
    }
}

bool SM::Ram::addMeter(const QString& source)
{
    QStringList l = source.split('/');
    if (l.count() < 3) {
        return false;
    }
    QString ram = l[1];
    Plasma::Theme* theme = Plasma::Theme::defaultTheme();
    Plasma::SignalPlotter *plotter = new Plasma::SignalPlotter(this);
    plotter->addPlot(theme->color(Plasma::Theme::TextColor));
    plotter->setUseAutoRange(false);
    plotter->setThinFrame(false);
    plotter->setShowLabels(false);
    plotter->setShowTopBar(m_showTopBar);
    plotter->setShowVerticalLines(false);
    plotter->setShowHorizontalLines(false);
    plotter->setFontColor(theme->color(Plasma::Theme::HighlightColor));
    QFont font = theme->font(Plasma::Theme::DefaultFont);
    font.setPointSize(8);
    plotter->setFont(font);
    plotter->setHorizontalLinesColor(theme->color(Plasma::Theme::HighlightColor));
    plotter->setVerticalLinesColor(theme->color(Plasma::Theme::HighlightColor));
    plotter->setHorizontalLinesCount(4);
    if (m_showBackground) {
        plotter->setSvgBackground("widgets/plot-background");
    } else {
        plotter->setSvgBackground(QString());
        plotter->setBackgroundColor(Qt::transparent);
    }
    plotter->setTitle(ram);
    plotter->setUnit("MB");
    appendPlotter(source, plotter);
    mainLayout()->addItem(plotter);
    setPreferredItemHeight(80);
    return true;
}

void SM::Ram::themeChanged()
{
    Plasma::Theme* theme = Plasma::Theme::defaultTheme();
    foreach (Plasma::SignalPlotter *plotter, plotters().values()) {
        plotter->setFontColor(theme->color(Plasma::Theme::HighlightColor));
        plotter->setHorizontalLinesColor(theme->color(Plasma::Theme::HighlightColor));
        plotter->setVerticalLinesColor(theme->color(Plasma::Theme::HighlightColor));
    }
}

void SM::Ram::dataUpdated(const QString& source, const Plasma::DataEngine::Data &data)
{
    Plasma::SignalPlotter *plotter = plotters()[source];
    if (plotter) {
        if (data["value"].toDouble() > m_max[source]) {
            QString tmp = source.left(source.lastIndexOf('/') + 1);
            m_max[source] = engine()->query(tmp + "used")["value"].toDouble() +
                            engine()->query(tmp + "free")["value"].toDouble() +
                            engine()->query(tmp + "application")["value"].toDouble();
            plotter->setVerticalRange(0.0, m_max[source] / 1024.0);
        }

        plotter->addSample(QList<double>() << data["value"].toDouble() / 1024.0);
        if (mode() == SM::Applet::Panel) {
            m_html[source] = QString("<tr><td>%1</td><td>%2</td><td>%3</td></tr>")
                    .arg(plotter->title())
                    .arg(KGlobal::locale()->formatByteSize(data["value"].toDouble()))
                    .arg(KGlobal::locale()->formatByteSize(m_max[source]));
            QString html = "<table>";
            foreach (const QString& s, m_html.keys()) {
                html += m_html[s];
            }
            html += "</table>";
            Plasma::ToolTipContent data(title(), html);
            Plasma::ToolTipManager::self()->setContent(this, data);
        }
    }
}

void SM::Ram::createConfigurationInterface(KConfigDialog *parent)
{
    QWidget *widget = new QWidget();
    ui.setupUi(widget);
    m_model.clear();
    m_model.setHorizontalHeaderLabels(QStringList() << i18n("RAM"));
    QStandardItem *parentItem = m_model.invisibleRootItem();
    QRegExp rx("mem/(\\w+)/.*");

    foreach (const QString& ram, m_memories) {
        if (rx.indexIn(ram) != -1) {
            QStandardItem *item1 = new QStandardItem(rx.cap(1));
            item1->setEditable(false);
            item1->setCheckable(true);
            item1->setData(ram);
            if (items().contains(ram)) {
                item1->setCheckState(Qt::Checked);
            }
            parentItem->appendRow(QList<QStandardItem *>() << item1);
        }
    }
    ui.treeView->setModel(&m_model);
    ui.treeView->resizeColumnToContents(0);
    ui.intervalSpinBox->setValue(interval() / 1000);
    connect(ui.intervalSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateSpinBoxSuffix(int)));
    parent->addPage(widget, i18n("RAM"), "ram");

    widget = new QWidget();
    uiAdv.setupUi(widget);
    uiAdv.showTopBarCheckBox->setChecked(m_showTopBar);
    uiAdv.showBackgroundCheckBox->setChecked(m_showBackground);
    uiAdv.graphColorCombo->setColor(m_graphColor);
    parent->addPage(widget, i18n("Advanced"), "preferences-other");

    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
}

void SM::Ram::updateSpinBoxSuffix(int interval)
{
    ui.intervalSpinBox->setSuffix(QString(" ") + i18np("second", "seconds", interval));
}

void SM::Ram::configAccepted()
{
    KConfigGroup cg = config();
    QStandardItem *parentItem = m_model.invisibleRootItem();

    clearItems();
    for (int i = 0; i < parentItem->rowCount(); ++i) {
        QStandardItem *item = parentItem->child(i, 0);
        if (item) {
            if (item->checkState() == Qt::Checked) {
                appendItem(item->data().toString());
            }
        }
    }
    cg.writeEntry("memories", items());

    uint interval = ui.intervalSpinBox->value();
    cg.writeEntry("interval", interval);
    interval *= 1000;
    setInterval(interval);

    cg.writeEntry("showTopBar", m_showTopBar = uiAdv.showTopBarCheckBox->isChecked());
    cg.writeEntry("showBackground", m_showBackground = uiAdv.showBackgroundCheckBox->isChecked());
    cg.writeEntry("graphColor", m_graphColor = uiAdv.graphColorCombo->color());
    
    emit configNeedsSaving();
    connectToEngine();
}

void SM::Ram::setDetail(Detail detail)
{
    foreach (const QString& key, plotters().keys()) {
        plotters().value(key)->setShowLabels(detail == SM::Applet::High);
        plotters().value(key)->setShowHorizontalLines(detail == SM::Applet::High);
    }
}

#include "ram.moc"
