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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <config.h>

#include <qlayout.h>
#include <qdragobject.h>

#include <klocale.h>
#include <kstandarddirs.h>
#include <kimageio.h>
#include <kmessagebox.h>
#include <kaboutdata.h>
#include <kgenericfactory.h>

#include "kdm-appear.h"
#include "kdm-font.h"
#include "backgnd.h"
#include "kdm-users.h"
#include "kdm-sess.h"
#include "kdm-conv.h"

#include "main.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

typedef KGenericFactory<KDModule, QWidget> KDMFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kdm, KDMFactory("kdmconfig") );

KURL *decodeImgDrop(QDropEvent *e, QWidget *wdg)
{
    QStringList uris;

    if (QUriDrag::decodeToUnicodeUris(e, uris) && (uris.count() > 0)) {
	KURL *url = new KURL(*uris.begin());

	KImageIO::registerFormats();
	if( KImageIO::canRead(KImageIO::type(url->fileName())) )
	    return url;

	QStringList qs = QStringList::split('\n', KImageIO::pattern());
	qs.remove(qs.begin());

	QString msg = i18n( "%1 "
			    "does not appear to be an image file.\n"
			    "Please use files with these extensions:\n"
			    "%2")
			    .arg(url->fileName())
			    .arg(qs.join("\n"));
	KMessageBox::sorry( wdg, msg);
	delete url;
    }
    return 0;
}

KSimpleConfig *config;

KDModule::KDModule(QWidget *parent, const char *name, const QStringList &)
  : KCModule(KDMFactory::instance(), parent, name)
  , minshowuid(0)
  , maxshowuid(0)
  , updateOK(false)
{
  struct passwd *ps;
  for (setpwent(); (ps = getpwent()); )
    usermap.insert( QString::fromLocal8Bit( ps->pw_name ), ps->pw_uid );
  endpwent();

  config = new KSimpleConfig( QString::fromLatin1( KDE_CONFDIR "/kdm/kdmrc" ));

  QVBoxLayout *top = new QVBoxLayout(this);
  tab = new QTabWidget(this);

  appearance = new KDMAppearanceWidget(this);
  tab->addTab(appearance, i18n("A&ppearance"));
  connect(appearance, SIGNAL(changed(bool)), SLOT(moduleChanged(bool)));

  font = new KDMFontWidget(this);
  tab->addTab(font, i18n("&Font"));
  connect(font, SIGNAL(changed(bool)), SLOT(moduleChanged(bool)));

  background = new KBackground(this);
  tab->addTab(background, i18n("&Background"));
  connect(background, SIGNAL(changed(bool)), SLOT(moduleChanged(bool)));

  sessions = new KDMSessionsWidget(this);
  tab->addTab(sessions, i18n("&Sessions"));
  connect(sessions, SIGNAL(changed(bool)), SLOT(moduleChanged(bool)));

  users = new KDMUsersWidget(this, 0);
  tab->addTab(users, i18n("&Users"));
  connect(users, SIGNAL(changed(bool)), SLOT(moduleChanged(bool)));
  connect(users, SIGNAL(setMinMaxUID(int,int)), SLOT(slotMinMaxUID(int,int)));
  connect(this, SIGNAL(addUsers(const QMap<QString,int> &)), users, SLOT(slotAddUsers(const QMap<QString,int> &)));
  connect(this, SIGNAL(delUsers(const QMap<QString,int> &)), users, SLOT(slotDelUsers(const QMap<QString,int> &)));
  connect(this, SIGNAL(clearUsers()), users, SLOT(slotClearUsers()));

  convenience = new KDMConvenienceWidget(this, 0);
  tab->addTab(convenience, i18n("Con&venience"));
  connect(convenience, SIGNAL(changed(bool)), SLOT(moduleChanged(bool)));
  connect(this, SIGNAL(addUsers(const QMap<QString,int> &)), convenience, SLOT(slotAddUsers(const QMap<QString,int> &)));
  connect(this, SIGNAL(delUsers(const QMap<QString,int> &)), convenience, SLOT(slotDelUsers(const QMap<QString,int> &)));
  connect(this, SIGNAL(clearUsers()), convenience, SLOT(slotClearUsers()));

  load();

  if (getuid() != 0) {
    appearance->makeReadOnly();
    font->makeReadOnly();
    background->makeReadOnly();
    users->makeReadOnly();
    sessions->makeReadOnly();
    convenience->makeReadOnly();
  }

  top->addWidget(tab);
}

KDModule::~KDModule()
{
  delete config;
}

QString KDModule::quickHelp() const
{
    return i18n(    "<h1>Login Manager</h1> In this module you can configure the "
                    "various aspects of the KDE Login Manager. This includes "
                    "the look and feel as well as the users that can be "
                    "selected for login. Note that you can only make changes "
                    "if you run the module with superuser rights. If you haven't started the KDE "
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
                    "<h2>Sessions</h2> Here you can specify which types of sessions the Login "
                    "Manager should offer you for logging in and who is allowed to "
                    "shutdown/reboot the machine."
                    "<h2>Users</h2>On this tab page, you can select which users the Login Manager "
                    "will offer you for logging in."
                    "<h2>Convenience</h2> Here you can specify a user to be logged in automatically, "
		    "users not needing to provide a password to log in, and other features ideal for "
		    "lazy people. ;-)<br>"
		    "Note, that these settings are security holes by their nature, so use them very carefully.");
}

const KAboutData* KDModule::aboutData() const
{
   KAboutData *about =
   new KAboutData(I18N_NOOP("kcmkdm"), I18N_NOOP("KDE Login Manager Config Module"),
                  0, 0, KAboutData::License_GPL,
                  I18N_NOOP("(c) 1996 - 2002 The KDM Authors"));

   about->addAuthor("Thomas Tanghus", I18N_NOOP("Original author"), "tanghus@earthling.net");
	about->addAuthor("Steffen Hansen", 0, "hansen@kde.org");
	about->addAuthor("Oswald Buddenhagen", I18N_NOOP("Current maintainer"), "ossi@kde.org");

   return about;
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
  appearance->defaults();
  font->defaults();
  background->defaults();
  users->defaults();
  sessions->defaults();
  convenience->defaults();
  propagateUsers();
}


void KDModule::moduleChanged(bool state)
{
  emit changed(state);
}


void KDModule::resizeEvent(QResizeEvent *)
{
  tab->setGeometry(0,0,width(),height());
}

void KDModule::propagateUsers()
{
  emit clearUsers();
  QMap<QString,int> lusers;
  QMapConstIterator<QString,int> it;
  for (it = usermap.begin(); it != usermap.end(); ++it) {
    int uid = it.data();
    if (!uid || (uid >= minshowuid && uid <= maxshowuid))
      lusers[it.key()] = uid;
  }
  emit addUsers(lusers);
  updateOK = true;
}

void KDModule::slotMinMaxUID(int min, int max)
{
  if (updateOK) {
    QMap<QString,int> alusers, dlusers;
    QMapConstIterator<QString,int> it;
    for (it = usermap.begin(); it != usermap.end(); ++it) {
      int uid = it.data();
      if (!uid) continue;
      if ((uid >= minshowuid && uid <= maxshowuid) &&
	  !(uid >= min && uid <= max))
          dlusers[it.key()] = uid;
      else
      if ((uid >= min && uid <= max) &&
	  !(uid >= minshowuid && uid <= maxshowuid))
          alusers[it.key()] = uid;
    }
    emit delUsers(dlusers);
    emit addUsers(alusers);
  }
  minshowuid = min;
  maxshowuid = max;
}

#include "main.moc"
