/*
 *  Copyright (C) 2010 Andriy Rysin (rysin@kde.org)
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

#include "keyboard_config.h"

#include <ksharedconfig.h>
#include <kconfiggroup.h>
//#include <kdebug.h>


static const char* SWITCHING_POLICIES[] = {"Global", "Desktop", "WinClass", "Window", NULL };
//static const char* NUMLOCK_POLICIES[] = {"On", "Off", "NoChange", NULL };
static const char* LIST_SEPARATOR = ",";
static const char* DEFAULT_LAYOUT = "us";
static const char* DEFAULT_MODEL = "pc104";

static const QString CONFIG_FILENAME("keyboardrc");	//TODO: name?
static const QString CONFIG_GROUPNAME("Layout");

static const char* LAYOUT_VARIANT_SEPARATOR_PREFIX = "(";
static const char* LAYOUT_VARIANT_SEPARATOR_SUFFIX = ")";


static int findStringIndex(const char* strings[], const QString& toFind, int defaultIndex)
{
	for(int i=0; strings[i] != NULL; i++) {
		if( toFind == strings[i] ) {
			return i;
		}
	}
	return defaultIndex;
}

static QString& stripVariantName(QString& variant)
{
	if( variant.endsWith(LAYOUT_VARIANT_SEPARATOR_SUFFIX) ) {
		int suffixLen = strlen(LAYOUT_VARIANT_SEPARATOR_SUFFIX);
		return variant.remove(variant.length()-suffixLen, suffixLen);
	}
	return variant;
}

static LayoutConfig createLayoutConfig(const QString& string)
{
	LayoutConfig layoutConfig;
	QStringList lv = string.split(LAYOUT_VARIANT_SEPARATOR_PREFIX);
	layoutConfig.layout = lv[0];
	QString variant = lv.size() > 1 ? stripVariantName(lv[1]) : "";
	layoutConfig.variant = variant;
	return layoutConfig;
}

void KeyboardConfig::setDefaults()
{
//	keyboardRepeat = true;
//	repeatDelay = 660;
//	repeatRate = 25;
//	clickVolume = 0;
//	numlockState = NO_CHANGE;

	keyboardModel = DEFAULT_MODEL;
	resetOldXkbOptions = false;
	xkbOptions.clear();

	// init layouts options
	configureLayouts = false;
	layouts.clear();
	layouts.append(createLayoutConfig(DEFAULT_LAYOUT));

	// switch cotrol options
	switchingPolicy = SWITCH_POLICY_GLOBAL;
//	stickySwitching = false;
//	stickySwitchingDepth = 2;

	// display options
	showFlag = true;
//	displayNames.clear();
}


void KeyboardConfig::load()
{
    KConfigGroup config(KSharedConfig::openConfig( CONFIG_FILENAME, KConfig::NoGlobals ), CONFIG_GROUPNAME);

//    keyboardRepeat = config.readEntry("KeyboardRepeating", true);
//    repeatDelay = config.readEntry( "RepeatDelay", 660 );
//    repeatRate = config.readEntry( "RepeatRate", 25. );
//    clickVolume = config.readEntry("ClickVolume", 0 );
//    QString state = config.readEntry( "NumLock", "NoChange" );
//	numlockState = static_cast<NumLockSate>(findStringIndex(NUMLOCK_POLICIES, state, NO_CHANGE));

    keyboardModel = config.readEntry("Model", "");

    resetOldXkbOptions = config.readEntry("ResetOldOptions", false);
    QString options = config.readEntry("Options", "");
    xkbOptions = options.split(LIST_SEPARATOR, QString::SkipEmptyParts);

    configureLayouts = config.readEntry("Use", false);
    QString layoutsString = config.readEntry("LayoutList", "");
    QStringList layoutStrings = layoutsString.split(LIST_SEPARATOR, QString::SkipEmptyParts);
    if( layoutStrings.isEmpty() ) {
    	layoutStrings.append(DEFAULT_LAYOUT);
    }
    layouts.clear();
    foreach(QString layoutString, layoutStrings) {
    	layouts.append(createLayoutConfig(layoutString));
    }

	QString layoutMode = config.readEntry("SwitchMode", "Global");
	switchingPolicy = static_cast<SwitchingPolicy>(findStringIndex(SWITCHING_POLICIES, layoutMode, SWITCH_POLICY_GLOBAL));

	showFlag = config.readEntry("ShowFlag", true);
}

void KeyboardConfig::save()
{
    KConfigGroup config(KSharedConfig::openConfig( CONFIG_FILENAME, KConfig::NoGlobals ), CONFIG_GROUPNAME);

    config.writeEntry("Model", keyboardModel);

    config.writeEntry("ResetOldOptions", resetOldXkbOptions);
    config.writeEntry("Options", xkbOptions.join(LIST_SEPARATOR));

    config.writeEntry("Use", configureLayouts);

    QStringList layoutStrings;
    foreach(LayoutConfig layoutConfig, layouts) {
    	QString string(layoutConfig.layout);
    	if( ! layoutConfig.variant.isEmpty() ) {
    		string += LAYOUT_VARIANT_SEPARATOR_PREFIX;
    		string += layoutConfig.variant;
    		string += LAYOUT_VARIANT_SEPARATOR_SUFFIX;
    	}
    	layoutStrings.append(string);
    }
    config.writeEntry("LayoutList", layoutStrings.join(LIST_SEPARATOR));

	config.writeEntry("SwitchMode", SWITCHING_POLICIES[switchingPolicy]);

	config.writeEntry("ShowFlag", true);

	config.sync();
}
