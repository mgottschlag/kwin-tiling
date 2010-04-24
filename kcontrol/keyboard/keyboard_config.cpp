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
#include <kdebug.h>


static const char* SWITCHING_POLICIES[] = {"Global", "Desktop", "WinClass", "Window", NULL };
static const char* LIST_SEPARATOR = ",";
static const char* DEFAULT_LAYOUT = "us";
static const char* DEFAULT_MODEL = "pc104";

static const QString CONFIG_FILENAME("kxkbrc");
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

LayoutConfig LayoutConfig::createLayoutConfig(const QString& fullLayoutName)
{
	LayoutConfig layoutConfig;
	QStringList lv = fullLayoutName.split(LAYOUT_VARIANT_SEPARATOR_PREFIX);
	layoutConfig.layout = lv[0];
	QString variant = lv.size() > 1 ? stripVariantName(lv[1]) : "";
	layoutConfig.variant = variant;
	return layoutConfig;
}

void KeyboardConfig::setDefaults()
{
	keyboardModel = DEFAULT_MODEL;
	resetOldXkbOptions = false;
	xkbOptions.clear();

	// init layouts options
	configureLayouts = false;
	layouts.clear();
	layouts.append(LayoutConfig::createLayoutConfig(DEFAULT_LAYOUT));

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
    	layouts.append(LayoutConfig::createLayoutConfig(layoutString));
    }

	QString layoutMode = config.readEntry("SwitchMode", "Global");
	switchingPolicy = static_cast<SwitchingPolicy>(findStringIndex(SWITCHING_POLICIES, layoutMode, SWITCH_POLICY_GLOBAL));

	showFlag = config.readEntry("ShowFlag", true);

    QString labelsStr = config.readEntry("DisplayNames", "");
    QStringList labels = labelsStr.split(LIST_SEPARATOR, QString::KeepEmptyParts);
    for(int i=0; i<labels.count() && i<layouts.count(); i++) {
    	if( !labels[i].isEmpty() && labels[i] != layouts[i].layout ) {
    		layouts[i].setDisplayName(labels[i]);
    	}
    }

	kDebug() << "configuring layouts" << configureLayouts << "configuring options" << resetOldXkbOptions;
}

void KeyboardConfig::save()
{
    KConfigGroup config(KSharedConfig::openConfig( CONFIG_FILENAME, KConfig::NoGlobals ), CONFIG_GROUPNAME);

    config.writeEntry("Model", keyboardModel);

    config.writeEntry("ResetOldOptions", resetOldXkbOptions);
    if( resetOldXkbOptions ) {
    	config.writeEntry("Options", xkbOptions.join(LIST_SEPARATOR));
    }
    else {
        config.deleteEntry("Options");
    }

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

    QStringList displayNames;
    foreach(LayoutConfig layoutConfig, layouts) {
    	displayNames << layoutConfig.getRawDisplayName();
    }
    config.writeEntry("DisplayNames", displayNames.join(LIST_SEPARATOR));

	config.writeEntry("SwitchMode", SWITCHING_POLICIES[switchingPolicy]);

	config.writeEntry("ShowFlag", showFlag);

	config.sync();
}
