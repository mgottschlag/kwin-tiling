/*
 * main.cpp
 *
 * Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
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

#include <config.h>

#include "main.h"
#include "background.h"
#include "kdm-gen.h"
#include "kdm-dlg.h"
#include "kdm-users.h"
#include "kdm-shut.h"
#include "kdm-conv.h"
#include "kdm-theme.h"

#include <k3urldrag.h>
#include <kaboutdata.h>
#include <kgenericfactory.h>
#include <kimageio.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <kconfig.h>

#include <QDropEvent>
#include <QFile>
#include <QLabel>
#include <QStackedWidget>
#include <QTabWidget>
#include <QVBoxLayout>

#include <sys/types.h>
#include <unistd.h>
#include <locale.h>
#include <pwd.h>
#include <grp.h>

typedef KGenericFactory<KDModule, QWidget> KDMFactory;
K_EXPORT_COMPONENT_FACTORY( kdm, KDMFactory( "kdmconfig" ) )

KUrl *decodeImgDrop( QDropEvent *e, QWidget *wdg )
{
	KUrl::List uris;

	if (K3URLDrag::decode( e, uris ) && (uris.count() > 0)) {
		KUrl *url = new KUrl( uris.first() );

		KMimeType::Ptr mime = KMimeType::findByUrl( *url );
		if (mime && KImageIO::isSupported( mime->name(), KImageIO::Reading ))
			return url;

		QStringList qs = KImageIO::pattern().split( '\n' );
		qs.erase( qs.begin() );

		QString msg = i18n( "%1 "
		                    "does not appear to be an image file.\n"
		                    "Please use files with these extensions:\n"
		                    "%2",
		                    url->fileName(),
		                    qs.join( "\n" ));
		KMessageBox::sorry( wdg, msg );
		delete url;
	}
	return 0;
}

KConfig *config;

KDModule::KDModule( QWidget *parent, const QStringList & )
	: KCModule( KDMFactory::componentData(), parent )
	, minshowuid( 0 )
	, maxshowuid( 0 )
	, updateOK( false )
{
	KAboutData *about =
		new KAboutData( I18N_NOOP( "kcmkdm" ), I18N_NOOP( "KDE Login Manager Config Module" ),
		                0, 0, KAboutData::License_GPL,
		                I18N_NOOP( "(c) 1996 - 2006 The KDM Authors" ) );

	about->addAuthor( "Thomas Tanghus", I18N_NOOP( "Original author" ), "tanghus@earthling.net" );
	about->addAuthor( "Steffen Hansen", 0, "hansen@kde.org" );
	about->addAuthor( "Oswald Buddenhagen", I18N_NOOP( "Current maintainer" ), "ossi@kde.org" );
	about->addAuthor( "Stephen Leaf", 0, "smileaf@smileaf.org" );

	setQuickHelp( i18n( "<h1>Login Manager</h1> In this module you can configure the "
	                    "various aspects of the KDE Login Manager. This includes "
	                    "the look and feel as well as the users that can be "
	                    "selected for login. Note that you can only make changes "
	                    "if you run the module with superuser rights. If you have not started the KDE "
	                    "Control Center with superuser rights (which is absolutely the right thing to "
	                    "do, by the way), click on the <em>Modify</em> button to acquire "
	                    "superuser rights. You will be asked for the superuser password."
	                    "<h2>General</h2> On this tab page, you can configure parts of "
	                    "the Login Manager's look, and which language it should use. "
	                    "The language settings made here have no influence on "
	                    "the user's language settings."
	                    "<h2>Dialog</h2>Here you can configure the look of the \"classical\" "
	                    "dialog based mode if you have chosen to use it. "
	                    "<h2>Background</h2>If you want to set a special background for the dialog based "
	                    "login screen, this is where to do it."
	                    "<h2>Themes</h2> Here you can specify a theme to be used by the Login Manager."
	                    "<h2>Shutdown</h2> Here you can specify who is allowed to shutdown/reboot the machine "
	                    "and whether a boot manager should be used."
	                    "<h2>Users</h2>On this tab page, you can select which users the Login Manager "
	                    "will offer you for logging in."
	                    "<h2>Convenience</h2> Here you can specify a user to be logged in automatically, "
	                    "users not needing to provide a password to log in, and other convenience features.<br>"
	                    "Note, that these settings are security holes by their nature, so use them very carefully.") );

	setAboutData( about );

	setlocale( LC_COLLATE, "C" );

	KGlobal::locale()->insertCatalog( "kcmbackground" );

	QStringList sl;
	QMap<gid_t,QStringList> tgmap;
	QMap<gid_t,QStringList>::Iterator tgmapi;
	QMap<gid_t,QStringList>::ConstIterator tgmapci;
	QMap<QString, QPair<int,QStringList> >::Iterator umapi;

	struct passwd *ps;
	for (setpwent(); (ps = getpwent()); ) {
		QString un( QFile::decodeName( ps->pw_name ) );
		if (usermap.find( un ) == usermap.end()) {
			usermap.insert( un, QPair<int,QStringList>( ps->pw_uid, sl ) );
			if ((tgmapi = tgmap.find( ps->pw_gid )) != tgmap.end())
				(*tgmapi).append( un );
			else
				tgmap[ps->pw_gid] = QStringList(un);
		}
	}
	endpwent();

	struct group *grp;
	for (setgrent(); (grp = getgrent()); ) {
		QString gn( QFile::decodeName( grp->gr_name ) );
		bool delme = false;
		if ((tgmapi = tgmap.find( grp->gr_gid )) != tgmap.end()) {
			if ((*tgmapi).count() == 1 && (*tgmapi).first() == gn)
				delme = true;
			else
				for (QStringList::ConstIterator it = (*tgmapi).begin();
				     it != (*tgmapi).end(); ++it)
			usermap[*it].second.append( gn );
			tgmap.erase( tgmapi );
		}
		if (!*grp->gr_mem ||
		    (delme && !grp->gr_mem[1] && gn == QFile::decodeName( *grp->gr_mem )))
			continue;
		do {
			QString un( QFile::decodeName( *grp->gr_mem ) );
			if ((umapi = usermap.find( un )) != usermap.end()) {
				if (!(*umapi).second.contains( gn ))
					(*umapi).second.append( gn );
			} else
				kWarning() << "group '" << gn << "' contains unknown user '" << un << "'" << endl;
		} while (*++grp->gr_mem);
	}
	endgrent();

	for (tgmapci = tgmap.begin(); tgmapci != tgmap.end(); ++tgmapci)
		kWarning() << "user(s) '" << tgmapci.value().join( "," )
		<< "' have unknown GID " << tgmapci.key() << endl;

	config = new KConfig( QString::fromLatin1(KDE_CONFDIR "/kdm/kdmrc"), KConfig::OnlyLocal);

	QVBoxLayout *top = new QVBoxLayout( this );
	tab = new QTabWidget( this );

	// *****
	// _don't_ add a theme configurator until the theming engine is _really_ done!!
	// *****

	general = new KDMGeneralWidget( this );
	tab->addTab( general, i18n("General (&1)") );
	connect( general, SIGNAL(changed()), SLOT(changed()) );
	connect( general, SIGNAL(useThemeChanged( bool )),
	         SLOT(slotUseThemeChanged( bool )) );

	dialog_stack = new QStackedWidget( this );
	tab->addTab( dialog_stack, i18n("Dialog (&2)") );
	dialog = new KDMDialogWidget( dialog_stack );
	dialog_stack->addWidget( dialog );
	connect( dialog, SIGNAL(changed()), SLOT(changed()) );
	QLabel *lbl = new QLabel(
		i18n("There is no login dialog window in themed mode."),
		dialog_stack );
	lbl->setAlignment( Qt::AlignCenter );
	dialog_stack->addWidget( lbl );

	background_stack = new QStackedWidget( this );
	tab->addTab( background_stack, i18n("Background (&3)") );
	background = new KBackground( background_stack );
	background_stack->addWidget( background );
	connect( background, SIGNAL(changed()), SLOT(changed()) );
	lbl = new QLabel(
		i18n("The background cannot be configured separately in themed mode."),
		background_stack );
	lbl->setAlignment( Qt::AlignCenter );
	background_stack->addWidget( lbl );

	theme_stack = new QStackedWidget( this );
	tab->addTab( theme_stack, i18n("Theme (&4)") );
	lbl = new QLabel(
		i18n("Themed mode is disabled. See \"General\" tab."),
		theme_stack );
	lbl->setAlignment( Qt::AlignCenter );
	theme_stack->addWidget( lbl );
	theme = new KDMThemeWidget( theme_stack );
	theme_stack->addWidget( theme );
	connect( theme, SIGNAL(changed()), SLOT(changed()) );

	sessions = new KDMSessionsWidget( this );
	tab->addTab( sessions, i18n("Shutdown (&5)") );
	connect( sessions, SIGNAL(changed()), SLOT(changed()) );

	users = new KDMUsersWidget( this );
	tab->addTab( users, i18n("Users (&6)") );
	connect( users, SIGNAL(changed()), SLOT(changed()) );
	connect( users, SIGNAL(setMinMaxUID( int,int )), SLOT(slotMinMaxUID( int,int )) );
	connect( this, SIGNAL(addUsers( const QMap<QString,int> & )), users, SLOT(slotAddUsers( const QMap<QString,int> & )) );
	connect( this, SIGNAL(delUsers( const QMap<QString,int> & )), users, SLOT(slotDelUsers( const QMap<QString,int> & )) );
	connect( this, SIGNAL(clearUsers()), users, SLOT(slotClearUsers()) );

	convenience = new KDMConvenienceWidget( this );
	tab->addTab( convenience, i18n("Convenience (&7)") );
	connect( convenience, SIGNAL(changed()), SLOT(changed()) );
	connect( this, SIGNAL(addUsers( const QMap<QString,int> & )), convenience, SLOT(slotAddUsers( const QMap<QString,int> & )) );
	connect( this, SIGNAL(delUsers( const QMap<QString,int> & )), convenience, SLOT(slotDelUsers( const QMap<QString,int> & )) );
	connect( this, SIGNAL(clearUsers()), convenience, SLOT(slotClearUsers()) );

	load();
	if (getuid() != 0 || !config->checkConfigFilesWritable( true )) {
		general->makeReadOnly();
		dialog->makeReadOnly();
		background->makeReadOnly();
		theme->makeReadOnly();
		users->makeReadOnly();
		sessions->makeReadOnly();
		convenience->makeReadOnly();
	}
	top->addWidget( tab );
}

KDModule::~KDModule()
{
	delete config;
}

void KDModule::load()
{
	general->load();
	dialog->load();
	background->load();
	theme->load();
	users->load();
	sessions->load();
	convenience->load();
	propagateUsers();
}


void KDModule::save()
{
	general->save();
	dialog->save();
	background->save();
	theme->save();
	users->save();
	sessions->save();
	convenience->save();
	config->sync();
}


void KDModule::defaults()
{
	if (getuid() == 0) {
		general->defaults();
		dialog->defaults();
		background->defaults();
		theme->defaults();
		users->defaults();
		sessions->defaults();
		convenience->defaults();
		propagateUsers();
	}
}

void KDModule::propagateUsers()
{
	groupmap.clear();
	emit clearUsers();
	QMap<QString,int> lusers;
	QMap<QString, QPair<int,QStringList> >::const_iterator it;
	QStringList::ConstIterator jt;
	QMap<QString,int>::Iterator gmapi;
	for (it = usermap.begin(); it != usermap.end(); ++it) {
		int uid = it.value().first;
		if (!uid || (uid >= minshowuid && uid <= maxshowuid)) {
			lusers[it.key()] = uid;
			for (jt = it.value().second.begin(); jt != it.value().second.end(); ++jt)
				if ((gmapi = groupmap.find( *jt )) == groupmap.end()) {
					groupmap[*jt] = 1;
					lusers['@' + *jt] = -uid;
				} else
					(*gmapi)++;
		}
	}
	emit addUsers( lusers );
	updateOK = true;
}

void KDModule::slotMinMaxUID( int min, int max )
{
	if (updateOK) {
		QMap<QString,int> alusers, dlusers;
		QMap<QString, QPair<int,QStringList> >::const_iterator it;
		QStringList::ConstIterator jt;
		QMap<QString,int>::Iterator gmapi;
		for (it = usermap.begin(); it != usermap.end(); ++it) {
			int uid = it.value().first;
			if (!uid)
				continue;
			if ((uid >= minshowuid && uid <= maxshowuid) &&
			    !(uid >= min && uid <= max))
			{
				dlusers[it.key()] = uid;
				for (jt = it.value().second.begin();
				     jt != it.value().second.end(); ++jt) {
					gmapi = groupmap.find( *jt );
					if (!--(*gmapi)) {
						groupmap.erase( gmapi );
						dlusers['@' + *jt] = -uid;
					}
				}
			} else if ((uid >= min && uid <= max) &&
			           !(uid >= minshowuid && uid <= maxshowuid))
			{
				alusers[it.key()] = uid;
				for (jt = it.value().second.begin();
				     jt != it.value().second.end(); ++jt)
					if ((gmapi = groupmap.find( *jt )) == groupmap.end()) {
						groupmap[*jt] = 1;
						alusers['@' + *jt] = -uid;
					} else
						(*gmapi)++;
			}
		}
		emit delUsers( dlusers );
		emit addUsers( alusers );
	}
	minshowuid = min;
	maxshowuid = max;
}

void KDModule::slotUseThemeChanged( bool use )
{
	dialog_stack->setCurrentIndex( use );
	background_stack->setCurrentIndex( use );
	theme_stack->setCurrentIndex( use );
}

#include "main.moc"
