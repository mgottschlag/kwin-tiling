//
// C++ Implementation: kxkbconfig
//
// Description: 
//
//
// Author: Andriy Rysin <rysin@kde.org>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <assert.h>

#include <QRegExp>
#include <QStringList>
#include <QHash>

#include <kconfig.h>
#include <kdebug.h>

#include "kxkbconfig.h"
#include "x11helper.h"



static const char* switchModes[SWITCH_POLICY_COUNT] = {
  "Global", "WinClass", "Window"
};

const LayoutUnit DEFAULT_LAYOUT_UNIT = LayoutUnit("us", "");
const char* DEFAULT_MODEL = "pc104";

LayoutUnit KxkbConfig::getDefaultLayout()
{
	if( m_layouts.size() == 0 )
		return DEFAULT_LAYOUT_UNIT;
	
	return m_layouts[0];
}

bool KxkbConfig::load(int loadMode) 
{
	KConfig *config = new KConfig("kxkbrc", true, false);
	config->setGroup("Layout");

// Even if the layouts have been disabled we still want to set Xkb options
// user can always switch them off now in the "Options" tab
	m_enableXkbOptions = config->readEntry("EnableXkbOptions", false);
	
	if( m_enableXkbOptions == true || loadMode == LOAD_ALL ) {
		m_resetOldOptions = config->readEntry("ResetOldOptions", false);
		m_options = config->readEntry("Options", "");
		kDebug() << "Xkb options (enabled=" << m_enableXkbOptions << "): " << m_options << endl;
	}
	
	m_useKxkb = config->readEntry("Use", false);
	kDebug() << "Use kxkb " << m_useKxkb << endl;

	if( (m_useKxkb == false && loadMode == LOAD_ACTIVE_OPTIONS )
	  		|| loadMode == LOAD_INIT_OPTIONS )
		return true;

	m_model = config->readEntry("Model", DEFAULT_MODEL);
	kDebug() << "Model: " << m_model << endl;
	
	QStringList layoutList;
	if( config->hasKey("LayoutList") ) {
		config->readEntry("LayoutList", layoutList);
	}
	else { // old config
		QString mainLayout = config->readEntry("Layout", DEFAULT_LAYOUT_UNIT.toPair());
		config->readEntry("Additional", layoutList);
		layoutList.prepend(mainLayout);
	}
	if( layoutList.count() == 0 )
		layoutList.append("us");
	
	m_layouts.clear();
	for(QStringList::ConstIterator it = layoutList.begin(); it != layoutList.end() ; ++it) {
		m_layouts.append( LayoutUnit(*it) );
		kDebug() << " layout " << LayoutUnit(*it).toPair() << " in list: " << m_layouts.contains( LayoutUnit(*it) ) << endl;
	}

	kDebug() << "Found " << m_layouts.count() << " layouts, default is " << getDefaultLayout().toPair() << endl;
	
	QStringList displayNamesList;
	config->readEntry("DisplayNames", displayNamesList, ',');
	for(QStringList::ConstIterator it = displayNamesList.begin(); it != displayNamesList.end() ; ++it) {
		QStringList displayNamePair = (*it).split(':');
		if( displayNamePair.count() == 2 ) {
			LayoutUnit layoutUnit( displayNamePair[0] );
			if( m_layouts.contains( layoutUnit ) ) {
				m_layouts[m_layouts.findIndex(layoutUnit)].displayName = displayNamePair[1].left(3);
			}
		}
	}

// 	m_includes.clear();
	if( X11Helper::areSingleGroupsSupported() ) {
		if( config->hasKey("IncludeGroups") ) {
			QStringList includeList;
			config->readEntry("IncludeGroups", includeList, ',');
			for(QStringList::ConstIterator it = includeList.begin(); it != includeList.end() ; ++it) {
				QStringList includePair = (*it).split(':');
				if( includePair.count() == 2 ) {
					LayoutUnit layoutUnit( includePair[0] );
					if( m_layouts.contains( layoutUnit ) ) {
						m_layouts[m_layouts.findIndex(layoutUnit)].includeGroup = includePair[1];
						kDebug() << "Got inc group: " << includePair[0] << ": " << includePair[1] << endl;
					}
				}
			}
		}
		else { //old includes format
			kDebug() << "Old includes..." << endl;
			QStringList includeList;
			config->readEntry("Includes", includeList);
			for(QStringList::ConstIterator it = includeList.begin(); it != includeList.end() ; ++it) {
				QString layoutName = LayoutUnit::parseLayout( *it );
				LayoutUnit layoutUnit( layoutName, "" );
				kDebug() << "old layout for inc: " << layoutUnit.toPair() << " included " << m_layouts.contains( layoutUnit ) << endl;
				if( m_layouts.contains( layoutUnit ) ) {
					QString variantName = LayoutUnit::parseVariant(*it);
					m_layouts[m_layouts.findIndex(layoutUnit)].includeGroup = variantName;
					kDebug() << "Got inc group: " << layoutUnit.toPair() << ": " <<  variantName << endl;
				}
			}
		}
	}

	m_showSingle = config->readEntry("ShowSingle", false);
	m_showFlag = config->readEntry("ShowFlag", true);
	
	QString layoutOwner = config->readEntry("SwitchMode", "Global");

	if( layoutOwner == "WinClass" ) {
		m_switchingPolicy = SWITCH_POLICY_WIN_CLASS;
	}
	else if( layoutOwner == "Window" ) {
		m_switchingPolicy = SWITCH_POLICY_WINDOW;
	}
	else /*if( layoutOwner == "Global" )*/ {
		m_switchingPolicy = SWITCH_POLICY_GLOBAL;
	}
	
	if( m_layouts.count() < 2 && m_switchingPolicy != SWITCH_POLICY_GLOBAL ) {
		kWarning() << "Layout count is less than 2, using Global switching policy" << endl;
		m_switchingPolicy = SWITCH_POLICY_GLOBAL;
	}
	
	kDebug() << "Layout owner mode " << layoutOwner << endl;
	
	m_stickySwitching = config->readEntry("StickySwitching", false);
	m_stickySwitchingDepth = config->readEntry("StickySwitchingDepth", "2").toInt();
	if( m_stickySwitchingDepth < 2 )
		m_stickySwitchingDepth = 2;

	if( m_stickySwitching == true ) {
		if( m_layouts.count() < 3 ) {
			kWarning() << "Layout count is less than 3, sticky switching will be off" << endl;
			m_stickySwitching = false;
		}
		else	
		if( (int)m_layouts.count() - 1 < m_stickySwitchingDepth ) {
			kWarning() << "Sticky switching depth is more than layout count -1, adjusting..." << endl;
			m_stickySwitchingDepth = m_layouts.count() - 1;
		}
	}

	delete config;

	return true;
}

void KxkbConfig::save() 
{
	KConfig *config = new KConfig("kxkbrc", false, false);
	config->setGroup("Layout");

	config->writeEntry("Model", m_model);

	config->writeEntry("EnableXkbOptions", m_enableXkbOptions );
	config->writeEntry("ResetOldOptions", m_resetOldOptions);
	config->writeEntry("Options", m_options );

	QStringList layoutList;
	QStringList includeList;
	QStringList displayNamesList;
	
	QList<LayoutUnit>::ConstIterator it;
	for(it = m_layouts.begin(); it != m_layouts.end(); ++it) {
		const LayoutUnit& layoutUnit = *it;
		
		layoutList.append( layoutUnit.toPair() );
		
		if( layoutUnit.includeGroup.isEmpty() == false ) {
			QString incGroupUnit = QString("%1:%2").arg(layoutUnit.toPair(), layoutUnit.includeGroup);
			includeList.append( incGroupUnit );
		}
	
		QString displayName( layoutUnit.displayName );
		kDebug() << " displayName " << layoutUnit.toPair() << " : " << displayName << endl;
		if( displayName.isEmpty() == false && displayName != layoutUnit.layout ) {
			displayName = QString("%1:%2").arg(layoutUnit.toPair(), displayName);
			displayNamesList.append( displayName );
		}
	}
	
	config->writeEntry("LayoutList", layoutList);
	kDebug() << "Saving Layouts: " << layoutList << endl;
 	
	config->writeEntry("IncludeGroups", includeList);
 	kDebug() << "Saving includeGroups: " << includeList << endl;
	
//	if( displayNamesList.empty() == false )
		config->writeEntry("DisplayNames", displayNamesList);
// 	else
// 		config->deleteEntry("DisplayNames");

	config->writeEntry("Use", m_useKxkb);
	config->writeEntry("ShowSingle", m_showSingle);
	config->writeEntry("ShowFlag", m_showFlag);

	config->writeEntry("SwitchMode", switchModes[m_switchingPolicy]);
	
	config->writeEntry("StickySwitching", m_stickySwitching);
	config->writeEntry("StickySwitchingDepth", m_stickySwitchingDepth);

	// remove old options 
 	config->deleteEntry("Variants");
	config->deleteEntry("Includes");
	config->deleteEntry("Encoding");
	config->deleteEntry("AdditionalEncodings");
	config->deleteEntry("Additional");
	config->deleteEntry("Layout");
	
	config->sync();

	delete config;
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
	if( layoutUnit.variant == "" )
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
//	kDebug() << "getLayout: " << varLine.mid(pos, len) << endl;
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
