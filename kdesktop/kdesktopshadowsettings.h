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
#ifndef __FX_DATA_DESKTOP
#define __FX_DATA_DESKTOP

#include <QColor>
#include <kstandarddirs.h>
#include <ksharedconfig.h>

#include "kshadowsettings.h"

#define FX_GROUP	"FX"

#define ALGO_KEY	"Shadow.Algorithm"
#define MULT_KEY	"Shadow.MultiplicationFactor"
#define OPAC_KEY	"Shadow.MaxOpacity"
#define OFSX_KEY	"Shadow.OffsetX"
#define OFSY_KEY	"Shadow.OffsetY"
#define THIK_KEY	"Shadow.Thickness"
#define SELT_KEY	"Shadow.SelectionType"

/**
 * This implementation of FxData will read a the default configuration
 * file. The values used for shadow are frouped under "FX".
 *
 * The values are:
 *  Shadow.Algorithm:      the algorithm used for making the sahdow
 *  Shadow.Scale	   the normailsation factor for veraging the sum
 *  Shadow.MaxOpacity	   the maximum allowable opacity (255 = 100%opaque)
 *  Shadow.OffsetX	   the X-coordinate offset (0 centered)
 *  Shadow.OffsetY         the Y-coordinate offset (0 centered)
 *  Shadow.Thickness	   the shadow thickness (usually 3-5 px)
 *  Shadow.SelectionType   the selection type - inverse video or use
 *			   the selection colours.
 *
 * 06-Feb-03: Added simple UID algorithm 
 *
 */
class KDesktopShadowSettings : public KShadowSettings
{
 public:
    /**
     * Constructor
     * @param cfg the configuration file
     */
    KDesktopShadowSettings(const KSharedConfigPtr &cfg = KSharedConfigPtr());
    
    virtual ~KDesktopShadowSettings();
    
    /**
     * Sets a specific configuration file after the object's creation
     * @param config new configuration object
     */
    void setConfig(const KSharedConfigPtr &);
    
    /**
     * Returns the text color as definied in the configuraiton
     * @return the text color as definied in the configuraiton
     */
    QColor &textColor(){ return m_textColor; }
    
    /**
     * Returns the shadow color as definied in the configuraiton
     * @return the shadow color as definied in the configuraiton
     */
    QColor &bgColor() {  return m_bgColor; }
    
    /**
     * Returns true if the shadow engine is enabled.
     * @return true if the shadow engine is enabled.
     */
    bool isEnabled() { return m_isEnabled; }
    
    /**
     * Returns an UID for shadow rebuilding purposes
     * @return an UID for shadow rebuilding purposes
     */
    unsigned long UID();
    
    /**
     * (Re)sets an UID for shadow rebuilding purposes
     * @param the new UID (if 0/default, increments the stored UID)
     */
    void setUID(unsigned long val = 0L);
    
 private:
    KSharedConfigPtr config;
    QColor m_textColor;
    QColor m_bgColor;
    bool m_isEnabled;
    
    // uid of the object. Use this to determine the oportunity of a new
    // rebuild.
    unsigned long _UID;
};

#endif
