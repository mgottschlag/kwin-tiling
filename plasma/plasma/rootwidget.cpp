/*
 *   Copyright (C) 2006 Aaron Seigo <aseigo@kde.org>
 *   Copyright (C) 2007 Matt Broadstone <mbroadst@gmail.com>
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

#include "rootwidget.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QVBoxLayout>

#include <KWindowSystem>

#include "plasma/corona.h"
#include "plasma/plasma.h"
#include "plasma/svg.h"

#include "controlbox.h"
#include "desktopview.h"
#include "plasmaapp.h"

//#define ICON_DEMO
//#define SUPERKARAMBA_DEMO
//#define CONFIGXML_DEMO

#ifdef CONFIGXML_DEMO
#include <QFile>
#include <KStandardDirs>
#include "plasma/configxml.h"
#endif

#ifdef ICON_DEMO
#include "plasma/widgets/icon.h"
#endif

RootWidget::RootWidget()
    : QWidget(0, Qt::FramelessWindowHint)
{
    setFocusPolicy( Qt::NoFocus );
    setGeometry( QApplication::desktop()->geometry() );
    lower();

    KWindowSystem::setOnAllDesktops(winId(), true);
    KWindowSystem::setType(winId(), NET::Desktop);

    QVBoxLayout* rootLayout = new QVBoxLayout(this);
    rootLayout->setMargin(0);
    rootLayout->setSpacing(0);

    m_desktop = new DesktopView(this);
    Plasma::Corona* corona = PlasmaApp::self()->corona();
    //FIXME: form factors need to move out of Corona

    corona->setFormFactor(Plasma::Planar);
    corona->setLocation(Plasma::Desktop);
    rootLayout->addWidget(m_desktop);
    m_desktop->show();

    connect(QApplication::desktop(), SIGNAL(resized(int)), SLOT(adjustSize()));
    m_controlBox = new ControlBox(this);
    m_controlBox->show();

    connect(m_controlBox, SIGNAL(zoomIn()), m_desktop, SLOT(zoomIn()));
    connect(m_controlBox, SIGNAL(zoomOut()), m_desktop, SLOT(zoomOut()));
    connect(m_controlBox, SIGNAL(addApplet(const QString&)), corona, SLOT(addApplet(const QString&)));
    connect(m_controlBox, SIGNAL(setFormFactor(Plasma::FormFactor)), corona, SLOT(setFormFactor(Plasma::FormFactor)));
    connect(m_controlBox, SIGNAL(lockInterface(bool)), corona, SLOT(setImmutable(bool)));

#ifdef ICON_DEMO
    Plasma::Icon* icon = new Plasma::Icon();
    icon->setIcon("user-home");
    icon->setIconSize(64, 64);
//    icon->setFlags(QGraphicsItem::ItemIsMovable);
    corona->addItem(icon);

    icon = new Plasma::Icon(icon);
    icon->setIcon("plasmagik");
//    icon->setFlags(QGraphicsItem::ItemIsMovable);
    corona->addItem(icon);

#endif

#ifdef SUPERKARAMBA_DEMO
    corona->addKaramba(KUrl("~/themes/aero_aio.skz"));
#endif

#ifdef CONFIGXML_DEMO
    QFile file(KStandardDirs::locate("kcfg", "kickerSettings.kcfg"));
    Plasma::ConfigXml appletConfig(KStandardDirs::locateLocal("config", "kickerrc"), &file);
    foreach (KConfigSkeletonItem* item, appletConfig.items()) {
        kDebug() << "item " << item->name() << " in " <<  item->group();
    }
#endif
}

RootWidget::~RootWidget()
{
}

DesktopView* RootWidget::desktop()
{
    return m_desktop;
}

void RootWidget::adjustSize()
{
    setGeometry( QApplication::desktop()->geometry() );
}

#include "rootwidget.moc"

