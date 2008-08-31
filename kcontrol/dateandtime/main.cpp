/*
 *  main.cpp
 *
 *  Copyright (C) 1998 Luca Montecchiani <m.luca@usa.net>
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
 *
 */
#include "main.h"

#include <unistd.h>

#include <QLabel>
#include <QLayout>
//Added by qt3to4:
#include <QVBoxLayout>

#include <QtDBus/QtDBus>
#include <kaboutdata.h>
#include <kapplication.h>
#include <kdialog.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kprocess.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>

#include "main.moc"

#include "tzone.h"
#include "dtime.h"
#include "helper.h"

K_PLUGIN_FACTORY(KlockModuleFactory, registerPlugin<KclockModule>();)
K_EXPORT_PLUGIN(KlockModuleFactory("kcmkclock"))


KclockModule::KclockModule(QWidget *parent, const QVariantList &)
  : KCModule(KlockModuleFactory::componentData(), parent/*, name*/)
{
  KAboutData *about =
  new KAboutData(I18N_NOOP("kcmclock"), 0, ki18n("KDE Clock Control Module"),
                  0, KLocalizedString(), KAboutData::License_GPL,
                  ki18n("(c) 1996 - 2001 Luca Montecchiani"));

  about->addAuthor(ki18n("Luca Montecchiani"), ki18n("Original author"), "m.luca@usa.net");
  about->addAuthor(ki18n("Paul Campbell"), ki18n("Current Maintainer"), "paul@taniwha.com");
  about->addAuthor(ki18n("Benjamin Meyer"), ki18n("Added NTP support"), "ben+kcmclock@meyerhome.net");
  setAboutData( about );
  setQuickHelp( i18n("<h1>Date & Time</h1> This control module can be used to set the system date and"
    " time. As these settings do not only affect you as a user, but rather the whole system, you"
    " can only change these settings when you start the Control Center as root. If you do not have"
    " the root password, but feel the system time should be corrected, please contact your system"
    " administrator."));

  KGlobal::locale()->insertCatalog("timezones4"); // For time zone translations

  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setMargin(0);
  layout->setSpacing(KDialog::spacingHint());

  dtime = new Dtime(this);
  layout->addWidget(dtime);
  connect(dtime, SIGNAL(timeChanged(bool)), this, SIGNAL(changed(bool)));

  tzone = new Tzone(this);
  layout->addWidget(tzone);
  connect(tzone, SIGNAL(zoneChanged(bool)), this, SIGNAL(changed(bool)));

  setButtons(Help|Apply);

  process = NULL;
}

void KclockModule::save()
{
  QStringList helperargs;
  dtime->save( helperargs );
  tzone->save( helperargs );
  QString helper = KStandardDirs::findExe( "kcmdatetimehelper" );
  QString kdesu = KStandardDirs::findExe( "kdesu" );
  bool ok = true;
  if( helper.isEmpty() || kdesu.isEmpty())
    ok = false;
  else {
    process = new KProcess(this);
    *process << kdesu;
    *process << "--attach" << QString::number(window()->winId()) << "--" << helper;
    *process << helperargs;
    process->start();
    connect( process, SIGNAL( finished(int, QProcess::ExitStatus) ),
		     SLOT( slotDateTimeHelperFinished(int) ) );
    ok = process->waitForStarted();
    if(!ok) {
      delete process;
      process = NULL;
    }
  }
  if( !ok )
    KMessageBox::error( this, i18n( "Failed to set system date/time/timezone."), i18n( "Date/Time Error" ));
}
void KclockModule::slotDateTimeHelperFinished(int exitCode)
{
    dtime->processHelperErrors( exitCode );
    tzone->processHelperErrors( exitCode);

    process->deleteLater();
    process = NULL;
#if 0
  // Tell the clock applet about the change so that it can update its timezone
  QDBusInterface clock("org.kde.kicker", "/Applets/Clock", "org.kde.kicker.ClockApplet");
  clock.call("reconfigure");
#endif

}
void KclockModule::load()
{
  dtime->load();
  tzone->load();
}
