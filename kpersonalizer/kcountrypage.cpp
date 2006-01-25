/***************************************************************************
                          kcountrypage.cpp  -  description
                             -------------------
    begin                : Tue May 22 2001
    copyright            : (C) 2001 by Ralf Nolden
    email                : nolden@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qstringlist.h>
#include <qlabel.h>
#include <qmap.h>
//Added by qt3to4:
#include <QPixmap>

#include <kapplication.h>
#include <ksimpleconfig.h>
#include <klocale.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <dcopclient.h>
#include <kprocess.h>
#include <klanguagebutton.h>

#include "kfindlanguage.h"

#include "kcountrypage.h"

KCountryPage::KCountryPage(QWidget *parent, const char *name ) : KCountryPageDlg(parent,name) {

	px_introSidebar->setPixmap(UserIcon("step1.png"));

	connect(cb_country, SIGNAL(activated(const QString &)), SLOT(setLangForCountry(const QString &)));
	connect(cb_language, SIGNAL(activated(const QString &)), SLOT(setLanguageChanged()));

	// naturally, the language is not changed on startup
	b_savedLanguageChanged = false;
	b_startedLanguageChanged = false;

	// set appropriate KDE version (kapplication.h)
	txt_welcome->setText(i18n("<h3>Welcome to KDE %1</h3>").arg(KDE_VERSION_STRING));

	flang = new KFindLanguage();

	// need this ones for decision over restarts of kp, kicker, etc
	s_oldlocale = KGlobal::locale()->language();

	// load the Menus and guess the selection
	loadCountryList(cb_country);
	fillLanguageMenu(cb_language);
	cb_language->setCurrentItem(flang->getBestLang());
	cb_country->setCurrentItem("C");

	// Highlight the users's country
	for(int i = 0; i < cb_country->count(); i++) {
		if(cb_country->id(i) == flang->getCountry()) {
			cb_country->setCurrentItem(cb_country->id(i));
			break;
		}
	}

	setLanguageChanged();
}

KCountryPage::~KCountryPage(){
	delete flang;
}


void KCountryPage::loadCountryList(KLanguageButton *combo) {

	QString sub = QLatin1String("l10n/");

	// clear the list
	combo->clear();

	QStringList regionfiles = KGlobal::dirs()->findAllResources("locale", sub + "*.desktop");
	QMap<QString,QString> regionnames;

	for ( QStringList::ConstIterator it = regionfiles.begin(); it != regionfiles.end(); ++it ) {
		KSimpleConfig entry(*it);
		entry.setGroup(QLatin1String("KCM Locale"));
		QString name = entry.readEntry(QLatin1String("Name"), i18n("without name"));

		QString tag = *it;
		int index;

		index = tag.findRev('/');
		if (index != -1)
			tag = tag.mid(index + 1);

		index = tag.findRev('.');
		if (index != -1)
			tag.truncate(index);

		regionnames.insert(name, tag);
	}

	for ( QMap<QString,QString>::ConstIterator mit = regionnames.begin(); mit != regionnames.end(); ++mit ) {
		combo->insertSubmenu( mit.key(), '-' + mit.data(), sub );
	}

	// add all languages to the list
	QStringList countrylist = KGlobal::dirs()->findAllResources("locale", sub + "*/entry.desktop");
	countrylist.sort();

	for ( QStringList::ConstIterator it = countrylist.begin(); it != countrylist.end(); ++it ) {
		KSimpleConfig entry(*it);
		entry.setGroup(QLatin1String("KCM Locale"));
		QString name = entry.readEntry(QLatin1String("Name"), i18n("without name"));
		QString submenu = '-' + entry.readEntry("Region");

		QString tag = *it;
		int index = tag.findRev('/');
		tag.truncate(index);
		index = tag.findRev('/');
		tag = tag.mid(index+1);

		QPixmap flag( locate( "locale", QString::fromLatin1("l10n/%1/flag.png").arg(tag) ) );
		QIcon icon( flag );
		combo->insertItem( icon, name, tag, submenu );
	}
}

void KCountryPage::fillLanguageMenu(KLanguageButton *combo) {
	combo->clear();
	QString submenu; // we are working on this menu
	QStringList langlist = flang->getLangList();
	QMap<QString,QString> langmap = flang->getLangMap();
	QStringList::ConstIterator it;
	for ( it = langlist.begin(); it != langlist.end(); ++it ) {
		if ((*it).isNull()) {
			combo->insertSeparator();
			submenu = QLatin1String("all");
			combo->insertSubmenu(i18n("All"), submenu, QString());
			continue;
		}
		combo->insertItem(langmap[(*it)], (*it), submenu );
	}
}

/** No descriptions */
bool KCountryPage::save(KLanguageButton *comboCountry, KLanguageButton *comboLang) {
	kdDebug() << "KCountryPage::save()" << endl;
	KConfigBase *config = KGlobal::config();

	config->setGroup(QLatin1String("Locale"));
	config->writeEntry(QLatin1String("Country"), comboCountry->current(), KConfigBase::Normal|KConfigBase::Global);
	config->writeEntry(QLatin1String("Language"), comboLang->current(), KConfigBase::Normal|KConfigBase::Global);
	config->sync();

	// only make the system reload the language, if the selected one deferes from the old saved one.
	if (b_savedLanguageChanged) {
		// Tell kdesktop about the new language
		DCOPCString replyType; QByteArray replyData;
		QByteArray data, da;
		QDataStream stream( &data, QIODevice::WriteOnly );

		stream.setVersion(QDataStream::Qt_3_1);
		stream << comboLang->current();
		if ( !kapp->dcopClient()->isAttached() )
			kapp->dcopClient()->attach();
		// ksycoca needs to be rebuilt
		KProcess proc;
		proc << QLatin1String("kbuildsycoca");
		proc.start(KProcess::DontCare);
		kdDebug() << "KLocaleConfig::save : sending signal to kdesktop" << endl;
		// inform kicker and kdeskop about the new language
		kapp->dcopClient()->send( "kicker", "Panel", "restart()", QString());
		// call, not send, so that we know it's done before coming back
		// (we both access kdeglobals...)
		kapp->dcopClient()->call( "kdesktop", "KDesktopIface", "languageChanged(QString)", data, replyType, replyData );
	}
	// KPersonalizer::next() probably waits for a return-value
	return true;
}

void KCountryPage::setLangForCountry(const QString &country) {
	KSimpleConfig ent(locate("locale", "l10n/" + country + "/entry.desktop"), true);
	ent.setGroup(QLatin1String("KCM Locale"));
	langs = ent.readEntry(QLatin1String("Languages"),QStringList());

	QString lang = QLatin1String("en_US");
	// use the first INSTALLED langauge in the list, or default to C
	for ( QStringList::Iterator it = langs.begin(); it != langs.end(); ++it ) {
		if (cb_language->contains(*it)) {
			lang = *it;
			break;
		}
    }

	cb_language->setCurrentItem(lang);
	setLanguageChanged();
}

void KCountryPage::setLanguageChanged() {
	// is the selcted language the same like the one in kdeglobals from before the start?
	b_savedLanguageChanged = (flang->getOldLang() != cb_language->current().lower());
	// is the selected language the same like the one we started kp with from main.cpp?
	b_startedLanguageChanged = (s_oldlocale != cb_language->current());
}


#include "kcountrypage.moc"
