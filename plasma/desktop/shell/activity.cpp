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
#include "activitymanager/kidenticongenerator.h"

#include <QPixmap>
#include <QString>
#include <QSize>
#include <QFile>

#include <KConfig>
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
            insertContainment(cont);
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

QPixmap Activity::pixmap(const QSize &size)
{
    if (m_info->isValid() && !m_info->icon().isEmpty()) {
        return KIcon(m_info->icon()).pixmap(size);
    } else {
        return KIdenticonGenerator::self()->generate(size.width(), m_id);
    }
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
        KActivityController().removeActivity(m_id);
        foreach (Plasma::Containment *c, m_containments) {
            c->destroy(false);
        }
    }
}

Plasma::Containment* Activity::containmentForScreen(int screen, int desktop)
{
    return m_containments.value(QPair<int,int>(screen,desktop));
}

void Activity::activateContainment(int screen, int desktop)
{
    Plasma::Containment *c = m_containments.value(QPair<int,int>(screen, desktop));
    if (!c) {
        //TODO check if there are saved containments once we start saving them
        c = PlasmaApp::self()->addContainment(m_id);
        m_containments.insert(QPair<int,int>(screen, desktop), c);
    }
    c->setScreen(screen, desktop);
}

void Activity::activate()
{
    if (m_containments.isEmpty()) {
        open();
    }

    KActivityController().setCurrentActivity(m_id);

    //ensure there's a containment for every screen & desktop.
    int numScreens = Kephal::ScreenUtils::numScreens();
    int numDesktops = 0;
    if (AppSettings::perVirtualDesktopViews()) {
        numDesktops = KWindowSystem::numberOfDesktops();
    }
    for (int screen = 0; screen < numScreens; ++screen) {
        if (numDesktops) {
            for (int desktop = 0; desktop < numDesktops; ++desktop) {
                activateContainment(screen, desktop);
            }
        } else {
            activateContainment(screen, -1);
        }
    }
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

void Activity::save(KConfig &external)
{
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
        KConfigGroup group(&dest, QString::number(c->id()));
        c->config().copyTo(&group);
    }

    external.sync();
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

void Activity::insertContainment(Plasma::Containment* cont)
{
    int screen = cont->lastScreen();
    int desktop = cont->lastDesktop();
    kDebug() << screen << desktop;
    if (screen == -1) {
        //the migration can't set lastScreen, so maybe we need to assign the containment here
        kDebug() << "found a lost one";
        screen = 0;
    }
    QPair<int,int> key(screen, desktop);
    if (m_containments.contains(key)) {
        kDebug() << "@!@!@!@!@!@@@@rejecting containment!!!";
        return;
    }
    m_containments.insert(key, cont);
}

void Activity::open()
{
    QString fileName = "activities/";
    fileName += m_id;
    KConfig external(fileName, KConfig::SimpleConfig, "appdata");

    //TODO only load existing screens
    foreach (Plasma::Containment *newContainment, PlasmaApp::self()->corona()->importLayout(external)) {
        insertContainment(newContainment);
        //ensure it's hooked up (if something odd happened we don't want orphan containments)
        Plasma::Context *context = newContainment->context();
        context->setCurrentActivityId(m_id);
        connect(context, SIGNAL(activityChanged(Plasma::Context*)), this, SLOT(updateActivityName(Plasma::Context*)), Qt::UniqueConnection);
    }

    KConfigGroup configs(&external, "Containments");
    configs.deleteGroup();

    if (m_containments.isEmpty()) {
        //TODO check if we need more for screens/desktops
        kDebug() << "open failed (bad file?). creating new containment";
        Plasma::Containment *newContainment = PlasmaApp::self()->addContainment(m_id);
        m_containments.insert(QPair<int,int>(0, -1), newContainment);
    }

    PlasmaApp::self()->corona()->requireConfigSync();
    external.sync();
    emit opened();
}

#include "activity.moc"

// vim: sw=4 sts=4 et tw=100
