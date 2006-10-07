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

#include <QLayout>
//Added by qt3to4:
#include <QVBoxLayout>
#include <QDropEvent>

#include <kaboutdata.h>
#include <kgenericfactory.h>
#include <kimageio.h>
#include <kmessagebox.h>
#include <k3urldrag.h>

#include "kdm-appear.h"
#include "kdm-font.h"
#include "kdm-users.h"
#include "kdm-shut.h"
#include "kdm-conv.h"

#include "main.h"
#include "background.h"

#include <sys/types.h>
#include <sys/stat.h>
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

KSimpleConfig *config;

KDModule::KDModule( QWidget *parent, const QStringList & )
	: KCModule( KDMFactory::instance(), parent )
	, minshowuid( 0 )
	, maxshowuid( 0 )
	, updateOK( false )
{
	KAboutData *about =
		new KAboutData( I18N_NOOP( "kcmkdm" ), I18N_NOOP( "KDE Login Manager Config Module" ),
		                0, 0, KAboutData::License_GPL,
		                I18N_NOOP( "(c) 1996 - 2005 The KDM Authors" ) );

	about->addAuthor( "Thomas Tanghus", I18N_NOOP( "Original author" ), "tanghus@earthling.net" );
	about->addAuthor( "Steffen Hansen", 0, "hansen@kde.org" );
	about->addAuthor( "Oswald Buddenhagen", I18N_NOOP( "Current maintainer" ), "ossi@kde.org" );

	setQuickHelp( i18n( "<h1>Login Manager</h1> In this module you can configure the "
	                    "various aspects of the KDE Login Manager. This includes "
	                    "the look and feel as well as the users that can be "
	                    "selected for login. Note that you can only make changes "
	                    "if you run the module with superuser rights. If you have not started the KDE "
	                    "Control Center with superuser rights (which is absolutely the right thing to "
	                    "do, by the way), click on the <em>Modify</em> button to acquire "
	                    "superuser rights. You will be asked for the superuser password."
	                    "<h2>Appearance</h2> On this tab page, you can configure how "
	                    "the Login Manager should look, which language it should use, and which "
	                    "GUI style it should use. The language settings made here have no influence on "
	                    "the user's language settings."
	                    "<h2>Font</h2>Here you can choose the fonts that the Login Manager should use "
	                    "for various purposes like greetings and user names. "
	                    "<h2>Background</h2>If you want to set a special background for the login "
	                    "screen, this is where to do it."
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

	config = new KSimpleConfig( QString::fromLatin1(KDE_CONFDIR "/kdm/kdmrc") );

	QVBoxLayout *top = new QVBoxLayout( this );
	tab = new QTabWidget( this );

	// *****
	// _don't_ add a theme configurator until the theming engine is _really_ done!!
	// *****

	appearance = new KDMAppearanceWidget( this );
	tab->addTab( appearance, i18n("A&ppearance") );
	connect( appearance, SIGNAL(changed( bool )), SIGNAL(changed( bool )) );

	font = new KDMFontWidget( this );
	tab->addTab( font, i18n("&Font") );
	connect( font, SIGNAL(changed( bool )), SIGNAL(changed( bool )) );

	background = new KBackground( this );
	tab->addTab( background, i18n("&Background") );
	connect( background, SIGNAL(changed( bool )), SIGNAL(changed( bool )) );

	sessions = new KDMSessionsWidget( this );
	tab->addTab( sessions, i18n("&Shutdown") );
	connect( sessions, SIGNAL(changed( bool )), SIGNAL(changed( bool )) );

	users = new KDMUsersWidget( this );
	tab->addTab( users, i18n("&Users") );
	connect( users, SIGNAL(changed( bool )), SIGNAL(changed( bool )) );
	connect( users, SIGNAL(setMinMaxUID( int,int )), SLOT(slotMinMaxUID( int,int )) );
	connect( this, SIGNAL(addUsers( const QMap<QString,int> & )), users, SLOT(slotAddUsers( const QMap<QString,int> & )) );
	connect( this, SIGNAL(delUsers( const QMap<QString,int> & )), users, SLOT(slotDelUsers( const QMap<QString,int> & )) );
	connect( this, SIGNAL(clearUsers()), users, SLOT(slotClearUsers()) );

	convenience = new KDMConvenienceWidget( this );
	tab->addTab( convenience, i18n("Con&venience") );
	connect( convenience, SIGNAL(changed( bool )), SIGNAL(changed( bool )) );
	connect( this, SIGNAL(addUsers( const QMap<QString,int> & )), convenience, SLOT(slotAddUsers( const QMap<QString,int> & )) );
	connect( this, SIGNAL(delUsers( const QMap<QString,int> & )), convenience, SLOT(slotDelUsers( const QMap<QString,int> & )) );
	connect( this, SIGNAL(clearUsers()), convenience, SLOT(slotClearUsers()) );

	load();
	if (getuid() != 0 || !config->checkConfigFilesWritable( true )) {
		appearance->makeReadOnly();
		font->makeReadOnly();
		background->makeReadOnly();
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
	appearance->load();
	font->load();
	background->load();
	users->load();
	sessions->load();
	convenience->load();
	propagateUsers();
}


void KDModule::save()
{
	appearance->save();
	font->save();
	background->save();
	users->save();
	sessions->save();
	convenience->save();
	config->sync();
}


void KDModule::defaults()
{
	if (getuid() == 0) {
		appearance->defaults();
		font->defaults();
		background->defaults();
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

#include "main.moc"
