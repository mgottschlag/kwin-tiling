/*
 *   Copyright 2010 Chani Armitage <chani@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2,
 *   or (at your option) any later version.
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


#include "plasma-shell-desktop.h"
#include "kactivitycontroller.h"
#include "kactivityinfo.h"

#include <QPixmap>
#include <QString>
#include <QSize>
#include <QFile>

#include <KIcon>
#include <KMessageBox>
#include <KWindowSystem>
#include <kephal/screens.h>

#include <Plasma/Containment>
#include <Plasma/Context>
#include <Plasma/Corona>

#include "plasmaapp.h"

#include "activity.h"

Activity::Activity(const QString &id, QObject *parent)
    : QObject(parent),
      m_id(id),
      m_info(new KActivityInfo(id, this))
{
    connect(m_info, SIGNAL(nameChanged(QString)), this, SLOT(setName(QString)));

    if (m_info) {
        m_name = m_info->name();
    } else {
        m_name = m_id;
        kDebug() << "nepomuk is probably broken :(";
    }

    Plasma::Corona *corona = PlasmaApp::self()->corona();

    //find your containments
    foreach (Plasma::Containment *cont, corona->containments()) {
        if ((cont->containmentType() == Plasma::Containment::DesktopContainment ||
            cont->containmentType() == Plasma::Containment::CustomContainment) &&
                !corona->offscreenWidgets().contains(cont) && cont->context()->currentActivityId() == id) {
            m_containments << cont;
            break;
        }
    }
    kDebug() << m_containments.size();

}

Activity::~Activity()
{
}


QString Activity::id()
{
    return m_id;
}

QString Activity::name()
{
    return m_name;
}

QPixmap Activity::thumbnail(const QSize &size)
{
    //TODO
    return KIcon("plasma").pixmap(size);
}

bool Activity::isActive()
{
    KActivityConsumer c;
    return m_id == c.currentActivity();
    //TODO maybe plasmaapp should cache the current activity to reduce dbus calls?
}

bool Activity::isRunning()
{
    return ! m_containments.isEmpty();
}

void Activity::destroy()
{
    if (KMessageBox::warningContinueCancel(
                0, //FIXME pass a view in
                i18nc("%1 is the name of the activity", "Do you really want to remove %1?", name()),
                i18nc("@title:window %1 is the name of the activity", "Remove %1", name()), KStandardGuiItem::remove()) == KMessageBox::Continue) {
        foreach (Plasma::Containment *c, m_containments) {
            c->destroy(false);
        }
        KActivityController controller;
        controller.removeActivity(m_id);
    }
}

void Activity::activate()
{
    if (m_containments.isEmpty()) {
        open();
        if (m_containments.isEmpty()) {
            kDebug() << "open failed??";
            return;
        }
    }
    //FIXME also ensure there's a containment for every screen. it's possible numscreens changed
    //since we were opened.

    //figure out where we are
    int currentScreen = Kephal::ScreenUtils::screenId(QCursor::pos());
    int currentDesktop = -1;
    if (AppSettings::perVirtualDesktopViews()) {
        currentDesktop = KWindowSystem::currentDesktop()-1;
    }
    //and bring the containment to where we are
    m_containments.first()->setScreen(currentScreen, currentDesktop);
    //TODO handle other screens

    KActivityController c;
    c.setCurrentActivity(m_id);
}

void Activity::setName(const QString &name)
{
    if (m_name == name) {
        return;
    }

    m_name = name;
    emit nameChanged(name);
    //FIXME right now I'm assuming the name change came *from* nepomuk
    //so I'm not trying to set the activity name in nepomuk.
    //nor am I propogating the change to my containments; they need to be able to react to such
    //things when this class isn't around anyways.
}

void Activity::close()
{
    QString name = "activities/";
    name += m_id;
    KConfig external(name, KConfig::SimpleConfig, "appdata");
    foreach (const QString &group, external.groupList()) {
        KConfigGroup cg(&external, group);
        cg.deleteGroup();
    }

    //TODO: multi-screen saving/restoring, where each screen can be
    // independently restored: put each screen's containments into a 
    // different group, e.g. [Screens][0][Containments], [Screens][1][Containments], etc
    KConfigGroup dest(&external, "Containments");
    KConfigGroup dummy;
    foreach (Plasma::Containment *c, m_containments) {
        c->save(dummy);
        c->config().reparent(&dest);
        c->destroy(false);
    }

    external.sync();
    m_containments.clear();
    emit closed();
    kDebug() << "attempting to write to" << external.name();
    //FIXME only destroy it if nothing went wrong
    //TODO save a thumbnail to a file too
}

void Activity::open()
{
    QString fileName = "activities/";
    fileName += m_id;
    KConfig external(fileName, KConfig::SimpleConfig, "appdata");

    foreach (Plasma::Containment *newContainment, PlasmaApp::self()->corona()->importLayout(external)) {
        m_containments << newContainment;
    }

    KConfigGroup configs(&external, "Containments");
    configs.deleteGroup();

    PlasmaApp::self()->corona()->requireConfigSync();
    external.sync();
    emit opened();
}

#include "activity.moc"

// vim: sw=4 sts=4 et tw=100
