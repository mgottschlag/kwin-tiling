/* This file is proposed to be part of the KDE base.
 * Copyright (C) 2003 Laur Ivan <laurivan@eircom.net>
 *
 * Many thanks to:
 *  - Bernardo Hung <deciare@gta.igs.net> for the enhanced shadow
 *    algorithm (currently used)
 *  - Tim Jansen <tim@tjansen.de> for the API updates and fixes.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
 
#include <QColor>
#include <kdebug.h>
#include "kdesktopshadowsettings.h"


//#define DEBUG

KDesktopShadowSettings::KDesktopShadowSettings(KConfig *cfg) : 
    KShadowSettings(),
    m_textColor(QColor(255,255,255)),
    _UID(0L)
{
    setConfig(cfg);
}

KDesktopShadowSettings::~KDesktopShadowSettings()
{
}

/**
 *
 */
void KDesktopShadowSettings::setUID(unsigned long val)
{
    if (val == 0L || val == _UID)
	_UID++;
    else
	_UID = val;
}

unsigned long KDesktopShadowSettings::UID()
{
    return _UID;
}

/**
 * Loads a new configuration
 */
void KDesktopShadowSettings::setConfig(KConfig *val)
{
    config = val;

    if (val == NULL)
	return;

    // increment the UID so the items will rebuild their
    // pixmaps
    setUID();

    config->setGroup("FMSettings");
    m_textColor = config->readEntry("NormalTextColor", Qt::white);
    m_bgColor = config->readEntry("ItemTextBackground");
    m_isEnabled = config->readEntry("ShadowEnabled", true);

#ifdef DEBUG
    // debug
    kDebug(1204) << "setConfig()" << endl;
#endif

    if (config->hasKey(SHADOW_CONFIG_ENTRY))
	fromString(config->readEntry(SHADOW_CONFIG_ENTRY, QString()));
    
#ifdef DEBUG
    // debug
    kDebug(1204) << "           \t" << SHADOW_TEXT_COLOR << "=" << m_textColor << endl;
    kDebug(1204) << "           \t" << SHADOW_TEXT_BACKGROUND << "=" << m_bgColor << endl;
    kDebug(1204) << "           \t" << toString() << endl;
#endif
}
