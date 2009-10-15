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

/* All sources we are interested in. */
static const char *phys_source = "mem/physical/application";
static const char *swap_source = "mem/swap/used";

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
    KGlobal::locale()->insertCatalog("plasma_applet_system-monitor");
    KConfigGroup cg = config();
    setEngine(dataEngine("systemmonitor"));
    setInterval(cg.readEntry("interval", 2) * 1000);
    setTitle(i18n("RAM"));

    /* At the time this method is running, not all source may be connected. */
    connect(engine(), SIGNAL(sourceAdded(const QString&)),
            this, SLOT(sourceAdded(const QString&)));
    foreach (const QString& source, engine()->sources()) {
        sourceAdded(source);
    }
}

void SM::Ram::sourceAdded(const QString& name)
{
    if ((name == phys_source || name == swap_source) && !m_memories.contains(name)) {
        m_memories << name;
        if (m_memories.count() == 2) {
            // all sources are ready
            QTimer::singleShot(0, this, SLOT(sourcesAdded()));
        }
    }
}

void SM::Ram::sourcesAdded()
{
    KConfigGroup cg = config();
    setItems(cg.readEntry("memories", m_memories));
    connectToEngine();
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
    plotter->addPlot(adjustColor(theme->color(Plasma::Theme::TextColor), 40));
    plotter->setUseAutoRange(false);
    plotter->setThinFrame(false);
    plotter->setShowLabels(false);
    plotter->setShowTopBar(true);
    plotter->setShowVerticalLines(false);
    plotter->setShowHorizontalLines(false);
    plotter->setFontColor(theme->color(Plasma::Theme::TextColor));
    QFont font = theme->font(Plasma::Theme::DefaultFont);
    font.setPointSize(8);
    plotter->setFont(font);
    QColor linesColor = theme->color(Plasma::Theme::TextColor);
    linesColor.setAlphaF(0.4);
    plotter->setHorizontalLinesColor(linesColor);
    plotter->setVerticalLinesColor(linesColor);
    plotter->setHorizontalLinesCount(4);
    plotter->setSvgBackground("widgets/plot-background");
    plotter->setTitle(ram);
    plotter->setUnit("B");
    appendPlotter(source, plotter);
    mainLayout()->addItem(plotter);
    setPreferredItemHeight(80);
    return true;
}

void SM::Ram::themeChanged()
{
    Plasma::Theme* theme = Plasma::Theme::defaultTheme();
    foreach (Plasma::SignalPlotter *plotter, plotters()) {
        plotter->setFontColor(theme->color(Plasma::Theme::TextColor));
        plotter->setHorizontalLinesColor(theme->color(Plasma::Theme::TextColor));
        plotter->setVerticalLinesColor(theme->color(Plasma::Theme::TextColor));
    }
}

void SM::Ram::dataUpdated(const QString& source, const Plasma::DataEngine::Data &data)
{
    Plasma::SignalPlotter *plotter = plotters()[source];
    if (plotter) {
        /* A factor to convert from default units to bytes.
         * If units is not "KB", assume it is bytes. */
        const double factor = (data["units"].toString() == "KB") ? 1024.0 : 1.0;
        const double value_b = data["value"].toDouble() * factor;
        const double max_b = data["max"].toDouble() * factor;
        static const QStringList units = QStringList() << "B" << "KiB" << "MiB" << "GiB" << "TiB";
        if (value_b > m_max[source]) {
            m_max[source] = max_b;
            plotter->setVerticalRange(0.0, max_b);
            qreal scale = 1.0;
            int i = 0;
            while (max_b / scale > 1024.0 && i < units.size()) {
                scale *= 1024.0;
                ++i;
            }
            plotter->setUnit(units[i]);
            plotter->scale(scale);
        }

        plotter->addSample(QList<double>() << value_b);
        if (mode() == SM::Applet::Panel) {
            setToolTip(source, QString("<tr><td>%1</td><td>%2</td><td>of</td><td>%3</td></tr>")
                                      .arg(plotter->title())
                                      .arg(KGlobal::locale()->formatByteSize(value_b))
                                      .arg(KGlobal::locale()->formatByteSize(m_max[source])));
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
    ui.intervalSpinBox->setValue(interval() / 1000.0);
    ui.intervalSpinBox->setSuffix(i18n("s"));
    parent->addPage(widget, i18n("RAM"), "ram");

    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
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

    double interval = ui.intervalSpinBox->value();
    cg.writeEntry("interval", interval);
    setInterval(interval * 1000.0);

    m_max.clear();
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
