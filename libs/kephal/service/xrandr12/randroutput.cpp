/*
 * Copyright (c) 2007      Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>
 * Copyright (c) 2007, 2008 Harry Bock <hbock@providence.edu>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "randroutput.h"

#include <QX11Info>
#include <KDebug>

#include "randrscreen.h"
#include "randrcrtc.h"
#include "randrmode.h"



RandROutput::RandROutput(RandRScreen *parent, RROutput id)
: QObject(parent)
{
    m_screen = parent;
    Q_ASSERT(m_screen);

    m_id = id;
    m_crtc = 0;
    m_rotations = 0;
    m_connected = false;

    queryOutputInfo();

    m_proposedRotation = m_originalRotation;
    m_proposedRate = m_originalRate;
    m_proposedRect = m_originalRect;
}

RandROutput::~RandROutput()
{
}

RROutput RandROutput::id() const
{
    return m_id;
}

RandRScreen *RandROutput::screen() const
{
    return m_screen;
}

bool RandROutput::queryOutputInfo(void)
{
    XRROutputInfo *info = XRRGetOutputInfo(QX11Info::display(), m_screen->resources(), m_id);
    Q_ASSERT(info);

        bool changes = false;
    if (RandR::timestamp != info->timestamp) {
        RandR::timestamp = info->timestamp;
                //changes = true;
        }

    // Set up the output's connection status, name, and current
    // CRT controller.
        bool pConn = m_connected;
    m_connected = (info->connection == RR_Connected);
        if (pConn != m_connected) {
            changes = true;
        }
    m_name = info->name;

    setCrtc(m_screen->crtc(info->crtc));
    m_crtc->loadSettings(false);

    for(int i = 0; i < info->ncrtc; ++i)
        m_possibleCrtcs.append(info->crtcs[i]);

    //TODO: is it worth notifying changes on mode list changing?
    m_modes.clear();
    m_preferredMode = m_screen->mode(info->modes[info->npreferred]);

    for (int i = 0; i < info->nmode; ++i)
        m_modes.append(info->modes[i]);

    //get all possible rotations
    m_rotations = 0;
    for (int i = 0; i < m_possibleCrtcs.count(); ++i)
    {
        RandRCrtc *crtc = m_screen->crtc(m_possibleCrtcs.at(i));
        Q_ASSERT(crtc);
        m_rotations |= crtc->rotations();
    }
    m_originalRotation = m_crtc->rotation();
    m_originalRate     = m_crtc->refreshRate();
    m_originalRect     = m_crtc->rect();

    if(isConnected()) {
        kDebug() << "Output name:" << m_name;
        kDebug() << "Output refresh rate:" << m_originalRate;
        kDebug() << "Output rect:" << m_originalRect;
        kDebug() << "Output rotation:" << m_originalRotation;
    }

    XRRFreeOutputInfo(info);

    return changes;
}

void RandROutput::loadSettings(bool notify)
{
    Q_UNUSED(notify);
    queryOutputInfo();

    kDebug() << "STUB: calling queryOutputInfo instead. Check if this has "
             << "any undesired effects. ";
}

void RandROutput::handleEvent(XRROutputChangeNotifyEvent *event)
{
    int changed = 0;

    kDebug() << "[OUTPUT" << m_id << "] Got event for " << m_name;
    kDebug() << "       crtc: " << event->crtc << "(current " << m_crtc->id() << ")";
    kDebug() << "       mode: " << event->mode << "(current " << mode().id() << ")";
    kDebug() << "       rotation: " << event->rotation;
    kDebug() << "       connection: " << event->connection;

    //FIXME: handling these events incorrectly, causing an X11 I/O error...
    // Disable for now.
    //qWarning() << "FIXME: Output event ignored!";
    //return;

    RRCrtc currentCrtc = m_crtc->id();
    if (event->crtc != currentCrtc)
    {
        changed |= RandR::ChangeCrtc;
        // update crtc settings
        if (currentCrtc != None)
            m_crtc->loadSettings(true);
            //m_screen->crtc(m_currentCrtc)->loadSettings(true);
        setCrtc(m_screen->crtc(event->crtc));
        if (currentCrtc != None)
            m_crtc->loadSettings(true);
    }

    if (event->mode != mode().id())
        changed |= RandR::ChangeMode;

    if (event->rotation != rotation())
        changed |= RandR::ChangeRotation;

    if((event->connection == RR_Connected) != m_connected)
    {
        changed |= RandR::ChangeConnection;
        m_connected = (event->connection == RR_Connected);
        if (!m_connected && currentCrtc != None)
            m_crtc = m_screen->crtc(None);
    }

    if(changed)
        emit outputChanged(m_id, changed);
}

void RandROutput::handlePropertyEvent(XRROutputPropertyNotifyEvent *event)
{
    // TODO: Do something with this!
    // By perusing thru some XOrg drivers, some of the properties that can
    // are configured through XRANDR are:
    // - LVDS Backlights
    // - TV output formats

    char *name = XGetAtomName(QX11Info::display(), event->property);
    kDebug() << "Got XRROutputPropertyNotifyEvent for property Atom " << name;
    XFree(name);
}

QString RandROutput::name() const
{
    return m_name;
}

QString RandROutput::icon() const
{
    // FIXME: check what names we should use and what kind of outputs randr can
    // report. It would also be interesting to be able to get the monitor name
    // using EDID or something like that, just don't know if it is even possible.
    if (m_name.contains("VGA"))
        return "video-display";
    else if (m_name.contains("LVDS"))
        return "video-display";

    // I doubt this is a good choice; can't find anything better in the spec.
    // video-x-generic might work, but that's a mimetype, which is inappropriate
    // for an output connection type.
    else if (m_name.contains("TV"))
        return "multimedia-player";

    return "video-display";
}

CrtcList RandROutput::possibleCrtcs() const
{
    return m_possibleCrtcs;
}

RandRCrtc *RandROutput::crtc() const
{
    return m_crtc;
}

ModeList RandROutput::modes() const
{
    return m_modes;
}

RandRMode RandROutput::mode() const
{
    if (!isConnected())
        return None;

    if (!m_crtc)
        return RandRMode();

    return m_crtc->mode();
}

RandRMode RandROutput::preferredMode(void) const
{
    return m_preferredMode;
}

SizeList RandROutput::sizes() const
{
    SizeList sizeList;

    foreach(const RRMode & m, m_modes)
    {
        RandRMode mode = m_screen->mode(m);
        if (!mode.isValid())
            continue;
        if (sizeList.indexOf(mode.size()) == -1)
            sizeList.append(mode.size());
    }
    return sizeList;
}

QRect RandROutput::rect() const
{
    if (!m_crtc) kDebug() << "No Crtc for output" << m_id;
        Q_ASSERT(m_crtc);
    if (!m_crtc->isValid())
        return QRect(0, 0, 0, 0);

    return m_crtc->rect();
}

RateList RandROutput::refreshRates(const QSize &s) const
{
    RateList list;
    QSize size = s;
    if (!size.isValid())
        size = rect().size();

    foreach(const RRMode & m, m_modes)
    {
        RandRMode mode = m_screen->mode(m);
        if (!mode.isValid())
            continue;
        if (mode.size() == size)
            list.append(mode.refreshRate());
    }
    return list;
}

float RandROutput::refreshRate() const
{
    return m_crtc->mode().refreshRate();
}

int RandROutput::rotations() const
{
    return m_rotations;
}

int RandROutput::rotation() const
{
    if (!isActive())
        return RandR::Rotate0;

    Q_ASSERT(m_crtc);
    return m_crtc->rotation();
}

bool RandROutput::isConnected() const
{
    return m_connected;
}

bool RandROutput::isActive() const
{
    return (m_connected && mode().isValid() && m_crtc->id() != None);
}

void RandROutput::proposeOriginal()
{
    if (m_crtc->id() != None)
        m_crtc->proposeOriginal();
}

void RandROutput::proposeRefreshRate(float rate)
{
    m_originalRate = refreshRate();
    m_proposedRate = rate;
}

void RandROutput::proposeRect(const QRect &r)
{
    m_originalRect = rect();
    m_proposedRect = r;
}

void RandROutput::proposeRotation(int r)
{
    m_originalRotation = rotation();
    m_proposedRotation = r;
}

void RandROutput::slotDisable()
{
    setCrtc(m_screen->crtc(None));
}

void RandROutput::slotEnable()
{
    if(!m_connected)
        return;

    kDebug() << "Attempting to enable " << m_name;
    RandRCrtc *crtc = findEmptyCrtc();

    if(crtc)
        setCrtc(crtc);
}

RandRCrtc *RandROutput::findEmptyCrtc()
{
    RandRCrtc *crtc = 0;

    foreach(const RRCrtc & c, m_possibleCrtcs)
    {
        crtc = m_screen->crtc(c);
        if (crtc->connectedOutputs().count() == 0)
            return crtc;
    }

    return 0;
}

bool RandROutput::tryCrtc(RandRCrtc *crtc, int changes)
{
    RandRCrtc *oldCrtc = m_crtc;

    // if we are not yet using this crtc, switch to use it
    if (crtc->id() != oldCrtc->id())
        setCrtc(crtc);

    crtc->setOriginal();

    if (changes & RandR::ChangeRect)
    {
        crtc->proposeSize(m_proposedRect.size());
        crtc->proposePosition(m_proposedRect.topLeft());
    }
    if (changes & RandR::ChangeRotation)
        crtc->proposeRotation(m_proposedRotation);
    if (changes & RandR::ChangeRate)
        crtc->proposeRefreshRate(m_proposedRate);

    if (crtc->applyProposed())
        return true;

    // revert changes if we didn't succeed
    crtc->proposeOriginal();
    crtc->applyProposed();

    // switch back to the old crtc
    setCrtc(oldCrtc);
    return false;
}

bool RandROutput::setCrtc(RandRCrtc *crtc, bool applyNow)
{
    Q_UNUSED(applyNow);
    if( !crtc || (m_crtc && crtc->id() == m_crtc->id()) )
        return false;

    kDebug() << "Setting CRTC" << crtc->id() << "on output" << m_name << "(previous" << (m_crtc ? m_crtc->id() : 0) << ")";

    if(m_crtc && m_crtc->isValid()) {
        disconnect(m_crtc, SIGNAL(crtcChanged(RRCrtc,int)),
                   this, SLOT(slotCrtcChanged(RRCrtc,int)));

        m_crtc->removeOutput(m_id);
//         m_crtc->applyProposed();
    }
    m_crtc = crtc;
    if (!m_crtc->isValid())
        return false;

    if (!m_crtc->addOutput(m_id)) {
        return false;
    }

    kDebug() << "CRTC outputs:" << m_crtc->connectedOutputs();
    connect(m_crtc, SIGNAL(crtcChanged(RRCrtc,int)),
            this, SLOT(slotCrtcChanged(RRCrtc,int)));

    return true;
}

void RandROutput::slotCrtcChanged(RRCrtc c, int changes)
{
    Q_UNUSED(c);

    //FIXME select which changes we should notify
    emit outputChanged(m_id, changes);
}

bool RandROutput::applyProposed(int changes)
{
    RandRCrtc *crtc;

    QRect r;

    if (changes & RandR::ChangeRect)
        r = m_proposedRect;


    // first try to apply to the already attached crtc if any
    if (m_crtc->isValid())
    {
        crtc = m_crtc;
        if (tryCrtc(crtc, changes))
        {
            return true;
        }
        return false;
    }

    //then try an empty crtc
    crtc = findEmptyCrtc();

    // TODO: check if we can add this output to a CRTC which already has an output
    // connection
    if (!crtc)
        return false;

    // try the crtc, and if no confirmation is needed or the user confirm, save the new settings
    if (tryCrtc(crtc, changes))
    {
        return true;
    }

    return false;
}

