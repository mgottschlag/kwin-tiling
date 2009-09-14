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

#include "net.h"
#include <Plasma/SignalPlotter>
#include <Plasma/Theme>
#include <Plasma/ToolTipManager>
#include <KConfigDialog>
#include <QGraphicsLinearLayout>

SM::Net::Net(QObject *parent, const QVariantList &args)
    : SM::Applet(parent, args)
    , m_rx("^network/interfaces/(\\w+)/transmitter/data$")
{
    setHasConfigurationInterface(true);
    resize(234 + 20 + 23, 135 + 20 + 25);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(themeChanged()));
    m_sourceTimer.setSingleShot(true);
    connect(&m_sourceTimer, SIGNAL(timeout()), this, SLOT(sourcesAdded()));
}

SM::Net::~Net()
{
}

void SM::Net::init()
{
    KGlobal::locale()->insertCatalog("plasma_applet_system-monitor");
    KConfigGroup cg = config();
    setEngine(dataEngine("systemmonitor"));
    setInterval(cg.readEntry("interval", 2) * 1000);
    setTitle(i18n("Network"));

    connect(engine(), SIGNAL(sourceAdded(const QString&)),
            this, SLOT(sourceAdded(const QString&)));
    connect(engine(), SIGNAL(sourceRemoved(const QString&)),
            this, SLOT(sourceRemoved(const QString&)));
    if (!engine()->sources().isEmpty()) {
        foreach (const QString& source, engine()->sources()) {
            sourceAdded(source);
        }
    }
}

void SM::Net::sourceAdded(const QString& name)
{
    if (m_rx.indexIn(name) != -1) {
        //kDebug() << m_rx.cap(1);
        if (m_rx.cap(1) != "lo") {
            m_interfaces << name;
            if (!m_sourceTimer.isActive()) {
                m_sourceTimer.start(0);
            }
        }
    }
}

void SM::Net::sourcesAdded()
{
    //kDebug() << m_interfaces;
    KConfigGroup cg = config();
    setItems(cg.readEntry("interfaces", m_interfaces));
    connectToEngine();
}

void SM::Net::sourceRemoved(const QString& name)
{
    m_interfaces.removeAll(name);
}

bool SM::Net::addMeter(const QString& source)
{
    QStringList l = source.split('/');
    if (l.count() < 3) {
        return false;
    }
    QString interface = l[2];
    Plasma::Theme* theme = Plasma::Theme::defaultTheme();
    Plasma::SignalPlotter *plotter = new Plasma::SignalPlotter(this);
    plotter->addPlot(adjustColor(theme->color(Plasma::Theme::TextColor), 40));
    plotter->addPlot(adjustColor(theme->color(Plasma::Theme::BackgroundColor), 40));
    plotter->setUseAutoRange(true);
    plotter->setThinFrame(false);
    plotter->setShowLabels(false);
    plotter->setShowTopBar(true);
    plotter->setShowVerticalLines(false);
    plotter->setShowHorizontalLines(false);
    plotter->setStackPlots(true);
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
    plotter->setTitle(interface);
    plotter->setUnit("KiB/s");
    appendPlotter(interface, plotter);
    mainLayout()->addItem(plotter);
    connectSource("network/interfaces/" + interface + "/receiver/data");
    setPreferredItemHeight(80);
    return true;
}

void SM::Net::themeChanged()
{
    Plasma::Theme* theme = Plasma::Theme::defaultTheme();
    foreach (Plasma::SignalPlotter *plotter, plotters()) {
        plotter->setFontColor(theme->color(Plasma::Theme::TextColor));
        plotter->setHorizontalLinesColor(theme->color(Plasma::Theme::TextColor));
        plotter->setVerticalLinesColor(theme->color(Plasma::Theme::TextColor));
    }
}

void SM::Net::dataUpdated(const QString& source,
                          const Plasma::DataEngine::Data &data)
{
    QStringList splitted = source.split('/');
    if (splitted.length() < 4) {
        return;
    }
    QString interface = splitted[2];
    int index = (splitted[3] == "receiver") ? 0 : 1;
    if (!m_data.contains(interface)) {
        m_data[interface] = QList<double>() << -1 << -1;
    }
    m_data[interface][index] = data["value"].toDouble();
    if (!m_data[interface].contains(-1)) {
       Plasma::SignalPlotter *plotter = plotters()[interface];
        if (plotter) {
            plotter->addSample(m_data[interface]);
            if (mode() == SM::Applet::Panel) {
                m_html[source] = QString("%1&nbsp;&nbsp;in %2&nbsp;&nbsp;out %3</br>")
                        .arg(plotter->title())
                        .arg(m_data[interface][0])
                        .arg(m_data[interface][1]);
                QString html;
                foreach (const QString& s, m_html.keys()) {
                    html += m_html[s];
                }
                Plasma::ToolTipContent data(title(), html);
                Plasma::ToolTipManager::self()->setContent(this, data);
            }
        }
        m_data[interface] = QList<double>() << -1 << -1;
    }
}

void SM::Net::createConfigurationInterface(KConfigDialog *parent)
{
    QWidget *widget = new QWidget();
    ui.setupUi(widget);
    m_model.clear();
    m_model.setHorizontalHeaderLabels(QStringList() << i18n("Network Interface"));
    QStandardItem *parentItem = m_model.invisibleRootItem();

    foreach (const QString& interface, m_interfaces) {
        QString ifname = interface.split('/')[2];
        QStandardItem *item1 = new QStandardItem(ifname);
        item1->setEditable(false);
        item1->setCheckable(true);
        item1->setData(interface);
        if (items().contains(interface)) {
            item1->setCheckState(Qt::Checked);
        }
        parentItem->appendRow(QList<QStandardItem *>() << item1);
    }
    ui.treeView->setModel(&m_model);
    ui.treeView->resizeColumnToContents(0);
    ui.intervalSpinBox->setValue(interval() / 1000.0);
    ui.intervalSpinBox->setSuffix(i18n("s"));
    parent->addPage(widget, i18n("Interfaces"), "network-workgroup");

    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
}

void SM::Net::configAccepted()
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
    cg.writeEntry("interfaces", items());

    double interval = ui.intervalSpinBox->value();
    cg.writeEntry("interval", interval);
    setInterval(interval * 1000.0);

    emit configNeedsSaving();
    connectToEngine();
}

void SM::Net::setDetail(Detail detail)
{
    foreach (const QString& key, plotters().keys()) {
        plotters().value(key)->setShowLabels(detail == SM::Applet::High);
        //plotters().value(key)->setShowTopBar(detail == SM::Applet::High);
        //plotters().value(key)->setShowVerticalLines(detail == SM::Applet::High);
        plotters().value(key)->setShowHorizontalLines(detail == SM::Applet::High);
    }
}

#include "net.moc"
