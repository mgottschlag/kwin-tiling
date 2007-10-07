/*
 *  Copyright (C) 2006 Andriy Rysin (rysin@kde.org)
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


#include <assert.h>

#include <QRegExp>
#include <QHash>

#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>

#include "kxkbconfig.h"
#include "x11helper.h"



static const char* switchModes[SWITCH_POLICY_COUNT] = {
  "Global", "Desktop", "WinClass", "Window"
};

const LayoutUnit DEFAULT_LAYOUT_UNIT = LayoutUnit("us", "");
const char* DEFAULT_MODEL = "pc104";

KxkbConfig::KxkbConfig() {
	m_layouts.append( DEFAULT_LAYOUT_UNIT );
}

int KxkbConfig::getDefaultLayout()
{
	if( m_layouts.size() == 0 )
		return 0; //DEFAULT_LAYOUT_UNIT;

	return 0; //m_layouts[0];
}

bool KxkbConfig::load(int loadMode)
{
    kDebug() << "Reading configuration";
    KConfigGroup config(KSharedConfig::openConfig( "kxkbrc", KConfig::CascadeConfig ), "Layout");

// Even if the layouts have been disabled we still want to set Xkb options
// user can always switch them off now in the "Options" tab
	m_enableXkbOptions = config.readEntry("EnableXkbOptions", false);

	if( m_enableXkbOptions == true || loadMode == LOAD_ALL ) {
		m_resetOldOptions = config.readEntry("ResetOldOptions", false);
		m_options = config.readEntry("Options", "");
		kDebug() << "Xkb options (enabled=" << m_enableXkbOptions << "): " << m_options;
	}

	m_useKxkb = config.readEntry("Use", false);
	kDebug() << "Use kxkb " << m_useKxkb;

	m_indicatorOnly = config.readEntry("IndicatorOnly", false);
	kDebug() << "Indicator only " << m_indicatorOnly << endl;

	if( (m_useKxkb == false && loadMode == LOAD_ACTIVE_OPTIONS )
	  		|| loadMode == LOAD_INIT_OPTIONS )
		return true;

	m_showSingle = config.readEntry("ShowSingle", false);
	m_showFlag = config.readEntry("ShowFlag", true);

	if( (m_indicatorOnly == true && loadMode == LOAD_ACTIVE_OPTIONS ) )
		return true;

	m_model = config.readEntry("Model", DEFAULT_MODEL);
	kDebug() << "Model: " << m_model;

	QStringList layoutList;
	if( config.hasKey("LayoutList") ) {
		layoutList = config.readEntry("LayoutList", layoutList);
	}
	else { // old config
		QString mainLayout = config.readEntry("Layout", DEFAULT_LAYOUT_UNIT.toPair());
		layoutList = config.readEntry("Additional", layoutList);
		layoutList.prepend(mainLayout);
	}
	if( layoutList.count() == 0 )
		layoutList.append("us");

	m_layouts.clear();
	for(QStringList::ConstIterator it = layoutList.begin(); it != layoutList.end() ; ++it) {
		LayoutUnit layoutUnit(*it);
		m_layouts.append( layoutUnit );
		kDebug() << " added layout " << layoutUnit.toPair();
	}

	kDebug() << "Found " << m_layouts.count() << " layouts, default is " << m_layouts[getDefaultLayout()].toPair();

	QStringList displayNamesList;
	displayNamesList = config.readEntry("DisplayNames", displayNamesList, ',');
	for(QStringList::ConstIterator it = displayNamesList.begin(); it != displayNamesList.end() ; ++it) {
		QStringList displayNamePair = (*it).split(':');
		if( displayNamePair.count() == 2 ) {
			LayoutUnit layoutUnit( displayNamePair[0] );
			if( m_layouts.contains( layoutUnit ) ) {
				m_layouts[m_layouts.indexOf(layoutUnit)].displayName = displayNamePair[1].left(3);
			}
		}
	}

//	m_showSingle = config.readEntry("ShowSingle", false);
//	m_showFlag = config.readEntry("ShowFlag", true);

	QString layoutOwner = config.readEntry("SwitchMode", "Global");

	if( layoutOwner == "WinClass" ) {
		m_switchingPolicy = SWITCH_POLICY_WIN_CLASS;
	}
	else if( layoutOwner == "Window" ) {
		m_switchingPolicy = SWITCH_POLICY_WINDOW;
	}
	else if( layoutOwner == "Desktop" ) {
		m_switchingPolicy = SWITCH_POLICY_DESKTOP;
	}
	else /*if( layoutOwner == "Global" )*/ {
		m_switchingPolicy = SWITCH_POLICY_GLOBAL;
	}

	if( m_layouts.count() < 2 && m_switchingPolicy != SWITCH_POLICY_GLOBAL ) {
		kWarning() << "Layout count is less than 2, using Global switching policy" ;
		m_switchingPolicy = SWITCH_POLICY_GLOBAL;
	}

	kDebug() << "Layout owner mode " << layoutOwner;

	m_stickySwitching = config.readEntry("StickySwitching", false);
	m_stickySwitchingDepth = config.readEntry("StickySwitchingDepth", "2").toInt();
	if( m_stickySwitchingDepth < 2 )
		m_stickySwitchingDepth = 2;

	if( m_stickySwitching == true ) {
		if( m_layouts.count() < 3 ) {
			kWarning() << "Layout count is less than 3, sticky switching will be off" ;
			m_stickySwitching = false;
		}
		else
		if( (int)m_layouts.count() - 1 < m_stickySwitchingDepth ) {
			kWarning() << "Sticky switching depth is more than layout count -1, adjusting..." ;
			m_stickySwitchingDepth = m_layouts.count() - 1;
		}
	}

	return true;
}

static QString addNum(QString& str, int n)
{
  if( str.length() >= 3 ) return str.left(2) + n;
  return str + n;
}

void KxkbConfig::updateDisplayNames()
{
  for(int i=0; i<m_layouts.count(); i++) {
	LayoutUnit& lu = m_layouts[i];
	int cnt = 1;
	for(int j=i; j<m_layouts.count(); j++) {
	  LayoutUnit& lu2 = m_layouts[i];
	  if( lu.layout == lu2.layout ) {
		++cnt;
		lu.displayName = addNum(lu.layout, 1);
		lu2.displayName = addNum(lu2.layout, cnt);
	  }
	}
  }
}

bool KxkbConfig::setConfiguredLayouts(QList<LayoutUnit> layoutUnits)
{
  kDebug() << "resetting layouts to " << layoutUnits.count() << " active in X server" << endl;
  m_layouts = layoutUnits;
  updateDisplayNames();
  return true; //TODO ?
}

void KxkbConfig::save()
{
	KConfigGroup config(KSharedConfig::openConfig( "kxkbrc", KConfig::CascadeConfig ), "Layout");

	config.writeEntry("Model", m_model);

	config.writeEntry("EnableXkbOptions", m_enableXkbOptions );
	config.writeEntry("IndicatorOnly", m_indicatorOnly );
	config.writeEntry("ResetOldOptions", m_resetOldOptions);
	config.writeEntry("Options", m_options );

	QStringList layoutList;
	QStringList includeList;
	QStringList displayNamesList;

	QList<LayoutUnit>::ConstIterator it;
	for(it = m_layouts.begin(); it != m_layouts.end(); ++it) {
		const LayoutUnit& layoutUnit = *it;

		layoutList.append( layoutUnit.toPair() );

// 		if( layoutUnit.includeGroup.isEmpty() == false ) {
// 			QString incGroupUnit = QString("%1:%2").arg(layoutUnit.toPair(), layoutUnit.includeGroup);
// 			includeList.append( incGroupUnit );
// 		}

		QString displayName( layoutUnit.displayName );
		kDebug() << " displayName " << layoutUnit.toPair() << " : " << displayName;
		if( displayName.isEmpty() == false && displayName != layoutUnit.layout ) {
			displayName = QString("%1:%2").arg(layoutUnit.toPair(), displayName);
			displayNamesList.append( displayName );
		}
	}

	config.writeEntry("LayoutList", layoutList);
	kDebug() << "Saving Layouts: " << layoutList;

	config.writeEntry("IncludeGroups", includeList);
 	kDebug() << "Saving includeGroups: " << includeList;

//	if( displayNamesList.empty() == false )
		config.writeEntry("DisplayNames", displayNamesList);
// 	else
// 		config.deleteEntry("DisplayNames");

	config.writeEntry("Use", m_useKxkb);
	config.writeEntry("ShowSingle", m_showSingle);
	config.writeEntry("ShowFlag", m_showFlag);

	config.writeEntry("SwitchMode", switchModes[m_switchingPolicy]);

	config.writeEntry("StickySwitching", m_stickySwitching);
	config.writeEntry("StickySwitchingDepth", m_stickySwitchingDepth);

	// remove old options
 	config.deleteEntry("Variants");
	config.deleteEntry("Includes");
	config.deleteEntry("Encoding");
	config.deleteEntry("AdditionalEncodings");
	config.deleteEntry("Additional");
	config.deleteEntry("Layout");

	config.sync();
}

void KxkbConfig::setDefaults()
{
	m_model = DEFAULT_MODEL;

	m_enableXkbOptions = false;
	m_resetOldOptions = false;
	m_options = "";

	m_layouts.clear();
	m_layouts.append( DEFAULT_LAYOUT_UNIT );

	m_useKxkb = false;
	m_indicatorOnly = false;
	m_showSingle = false;
	m_showFlag = true;

	m_switchingPolicy = SWITCH_POLICY_GLOBAL;

	m_stickySwitching = false;
	m_stickySwitchingDepth = 2;
}

QStringList KxkbConfig::getLayoutStringList(/*bool compact*/)
{
	QStringList layoutList;
	for(QList<LayoutUnit>::ConstIterator it = m_layouts.begin(); it != m_layouts.end(); ++it) {
		const LayoutUnit& layoutUnit = *it;
		layoutList.append( layoutUnit.toPair() );
	}
	return layoutList;
}


QString KxkbConfig::getDefaultDisplayName(const QString& code_)
{
	QString displayName;

	if( code_.length() <= 2 ) {
		displayName = code_;
	}
	else {
		int sepPos = code_.indexOf(QRegExp("[-_]"));
		QString leftCode = code_.mid(0, sepPos);
		QString rightCode;
		if( sepPos != -1 )
			rightCode = code_.mid(sepPos+1);

		if( rightCode.length() > 0 )
			displayName = leftCode.left(2) + rightCode.left(1).toLower();
		else
			displayName = leftCode.left(3);
	}

	return displayName;
}

QString KxkbConfig::getDefaultDisplayName(const LayoutUnit& layoutUnit, bool single)
{
	if( layoutUnit.variant.isEmpty() )
		return getDefaultDisplayName( layoutUnit.layout );

	QString displayName = layoutUnit.layout.left(2);
	if( single == false )
		displayName += layoutUnit.variant.left(1);
	return displayName;
}

/**
 * @brief Gets the single layout part of a layout(variant) string
 * @param[in] layvar String in form layout(variant) to parse
 * @return The layout found in the string
 */
const QString LayoutUnit::parseLayout(const QString &layvar)
{
	static const char* LAYOUT_PATTERN = "[a-zA-Z0-9_/-]*";
	QString varLine = layvar.trimmed();
	QRegExp rx(LAYOUT_PATTERN);
	int pos = rx.indexIn(varLine, 0);
	int len = rx.matchedLength();
  // check for errors
	if( pos < 0 || len < 2 )
		return "";
//	kDebug() << "getLayout: " << varLine.mid(pos, len);
	return varLine.mid(pos, len);
}

/**
 * @brief Gets the single variant part of a layout(variant) string
 * @param[in] layvar String in form layout(variant) to parse
 * @return The variant found in the string, no check is performed
 */
const QString LayoutUnit::parseVariant(const QString &layvar)
{
	static const char* VARIANT_PATTERN = "\\([a-zA-Z0-9_-]*\\)";
	QString varLine = layvar.trimmed();
	QRegExp rx(VARIANT_PATTERN);
	int pos = rx.indexIn(varLine, 0);
	int len = rx.matchedLength();
  // check for errors
	if( pos < 2 || len < 2 )
		return "";
	return varLine.mid(pos+1, len-2);
}
