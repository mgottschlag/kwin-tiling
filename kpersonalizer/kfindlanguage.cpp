/***************************************************************************
                          kfindlanguage.cpp  -  description
                             -------------------
    begin                : Tue May 22 2001
    copyright            : (C) 2002 by Carsten Wolff
    email                : wolff@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdlib.h>

#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>

#include <ksimpleconfig.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <klocale.h>

#include "kfindlanguage.h"

KFindLanguage::KFindLanguage() {
	KConfig *config = KGlobal::config();
	config->setGroup("Locale");

	m_oldlang = config->readEntry("Language");
	m_oldlang = m_oldlang.lower();
	m_oldlang = m_oldlang.left(m_oldlang.find(':')); // only use the first lang

	m_country = config->readEntry("Country", "C");
	if (m_country == "C") {
		m_country = QLatin1String(getenv("LANG"));
		if(m_country.left(5) == "nn_NO") // glibc's nn_NO is KDE's no_NY
			m_country = "no";
		if(m_country.contains("_"))
			m_country = m_country.mid(m_country.find("_")+1);
		if(m_country.contains("."))
			m_country = m_country.left(m_country.find("."));
		if(m_country.contains("@"))
			m_country = m_country.left(m_country.find("@"));
		if(m_country != "C")
			m_country = m_country.lower();
		if(m_country == "en") // special-case "en" - should be "en_GB" or "en_US", but plain "en" is in use quite often
			m_country = "C";
	}

	// get the users primary Languages
	KSimpleConfig ent(locate("locale", QString::fromLatin1("l10n/%1/entry.desktop").arg(m_country)), true);
	ent.setGroup("KCM Locale");
	QStringList langs = ent.readListEntry("Languages");
	if (langs.isEmpty())
		langs.append("en_US");

	// add the primary languages for the country to the list
	QStringList prilang;
	for ( QStringList::ConstIterator it = langs.begin(); it != langs.end(); ++it ) {
		QString str = locate("locale", *it + "/entry.desktop");
		if (!str.isNull())
			prilang << str;
	}

	// add all languages to the list
	QStringList alllang = KGlobal::dirs()->findAllResources("locale", "*/entry.desktop", false, true);
	alllang.sort();
	QStringList langlist = prilang;
	if (langlist.count() > 0)
		langlist << QString(); // separator
	langlist += alllang;

	for ( QStringList::ConstIterator it = langlist.begin();	it != langlist.end(); ++it ) {
		KSimpleConfig entry(*it);
		entry.setGroup("KCM Locale");
		QString name = entry.readEntry("Name", i18n("without name"));

		QString tag = *it;
		int index = tag.findRev('/');
		tag = tag.left(index);
		index = tag.findRev('/');
		tag = tag.mid(index+1);

		m_langlist << tag;
		m_langmap.insert(tag, name);
	}

	// now find the best language for the user
	QString compare = m_oldlang;
	if (m_oldlang.isEmpty()) {
		compare = langs.first();
		for(QStringList::Iterator it = langs.begin(); it != langs.end(); ++it) {
			if (*it == QString::fromLatin1(getenv("LANG")).mid(3, 2).lower())
				compare = *it;
		}
	}
	if(compare == "c")
		compare = "C";

	// Find the users's language
	int bestmatch = -1;

	QStringList::ConstIterator it;
	for( it = m_langlist.begin(); it != m_langlist.end(); ++it) {
		int match=0;
		QString l = (*it).left((*it).find(";"));
		if (l == "C")
			match++;
		if(l.contains(compare))
			match+=2;
		if(l.left(compare.length()) == compare)
			match+=10;
		if(compare == "en_US" && l == "C")
			match+=50;
		if (l == compare)
			match+=100;
		if(match > bestmatch) {
			bestmatch=match;
			m_bestlang=l;
		}
	}
}

KFindLanguage::~KFindLanguage() {
}

QStringList KFindLanguage::getLangList() const {
	return m_langlist;
}

QMap<QString,QString> KFindLanguage::getLangMap() const {
	return m_langmap;
}

QString KFindLanguage::getBestLang() const {
	return m_bestlang;
}

QString KFindLanguage::getOldLang() const {
	return m_oldlang;
}

QString KFindLanguage::getCountry() const {
	return m_country;
}
