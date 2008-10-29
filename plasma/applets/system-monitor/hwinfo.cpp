/*
 *   Copyright (C) 2007 Petri Damsten <damu@iki.fi>
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

#include "hwinfo.h"
#include <plasma/widgets/webview.h>
#include <plasma/widgets/iconwidget.h>
#include <plasma/containment.h>
#include <plasma/tooltipmanager.h>
#include <plasma/theme.h>
#include <KStandardDirs>
#include <KIcon>
#include <KTextEdit>
#include <QTextDocument>
#include <QGraphicsLinearLayout>

#define START "<html><head><style type=\"text/css\">\
body { background-color: %1; } \
td { vertical-align: top; font-size:7pt; font-weight:normal; font-style:normal; color: %2; } \
</style></head><body>"
#define START_BASIC "<html><head></head><body>"
#define START_TABLE "<table>"
#define INFO_ROW "<tr><td>%1:</td><td>%2</td></tr>"
#define END_TABLE "</table>"
#define END "</body><html>"

HWInfo::HWInfo(QObject *parent, const QVariantList &args)
    : SM::Applet(parent, args), m_info(0), m_icon(0)
{
    setHasConfigurationInterface(false);
    resize(234 + 20 + 23, 135 + 20 + 25);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

HWInfo::~HWInfo()
{
}

void HWInfo::init()
{
    setTitle(i18n("Hardware Info"));
    appendItem("info");
    connectToEngine();
}

bool HWInfo::addMeter(const QString&)
{
    if (mode() != SM::Applet::Panel) {
        m_info = new Plasma::WebView(this);
        m_info->setHtml(QString(START + i18n("Getting hardware information...") + END));
        m_icon = 0;
        mainLayout()->addItem(m_info);
        //m_info->nativeWidget()->document()->setTextWidth(contentsRect().width());
        //setPreferredItemHeight(m_info->nativeWidget()->document()->size().height());
        setPreferredItemHeight(135);
    } else {
        m_icon = new Plasma::IconWidget(KIcon("hwinfo"), QString(), this);
        m_info = 0;
        mainLayout()->addItem(m_icon);
    }
    return false;
}

void HWInfo::connectToEngine()
{
    Applet::connectToEngine();
    setEngine(dataEngine("soliddevice"));
    QStringList ids;

    ids = engine()->query("IS Processor")["IS Processor"].toStringList();
    foreach (const QString& id, ids) {
        engine()->connectSource(id, this);
    }
    ids = engine()->query("IS NetworkInterface")["IS NetworkInterface"].toStringList();
    foreach (const QString& id, ids) {
        engine()->connectSource(id, this);
    }
    ids = engine()->query("IS AudioInterface")["IS AudioInterface"].toStringList();
    foreach (const QString& id, ids) {
        engine()->connectSource(id, this);
    }
    // TODO: get this from soliddevice
    Plasma::DataEngine* engine = dataEngine("executable");
    QString path = QString::fromLocal8Bit(getenv("PATH")) + QString::fromLatin1(":/usr/sbin:/sbin/");
    QString exe = KStandardDirs::findExe( "lspci", path );
    if (exe.isEmpty())
       kError()  << "lspci not found in " << path << endl;
    else
    {
       QString tmp = exe + " | grep VGA | sed 's/.*: //g'";
       engine->connectSource(tmp, this);
    }
}

void HWInfo::dataUpdated(const QString& source,
                     const Plasma::DataEngine::Data &data)
{
    // TODO: Handle multiple devices
    if (source.indexOf("playback") > -1) {
        //kDebug() << "audio" << source;
        m_audio = data["Name"].toString();
    } else if (source.indexOf("VGA") > -1) {
        //kDebug() << "gpu" << source;
        m_gpu = data["stdout"].toString().trimmed();
    } else if (source.indexOf("net") > -1) {
        //kDebug() << "net" << source;
        m_net = data["Product"].toString();
    } else if (source.indexOf("CPU") > -1) {
        //kDebug() << "cpu" << source;
        m_cpu = data["Product"].toString();
    }
    QString html;
    html += QString(INFO_ROW).arg(i18n("CPU")).arg(m_cpu);
    html += QString(INFO_ROW).arg(i18n("GPU")).arg(m_gpu);
    html += QString(INFO_ROW).arg(i18n("Audio")).arg(m_audio);
    html += QString(INFO_ROW).arg(i18n("Net")).arg(m_net);
    html += END_TABLE END;
    if (m_info) {
        Plasma::Theme* theme = Plasma::Theme::defaultTheme();
        html = QString(START START_TABLE)
                .arg(theme->color(Plasma::Theme::BackgroundColor).name())
                .arg(theme->color(Plasma::Theme::TextColor).name()) + html;
        m_info->setHtml(html);
    } else if (m_icon) {
        html = START_BASIC START_TABLE + html;
        Plasma::ToolTipManager::Content data;
        data.mainText = i18n("Hardware Info");
        data.subText = html;
        Plasma::ToolTipManager::self()->setContent(m_icon, data);
    }
}

#include "hwinfo.moc"
