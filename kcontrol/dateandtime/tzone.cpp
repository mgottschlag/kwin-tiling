/*
 *  tzone.cpp
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
#include "tzone.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <config-workspace.h>

#include <QLabel>
#include <QComboBox>
#include <QLayout>
#include <QFile>
//Added by qt3to4:
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextStream>
#include <QByteArray>
#include <QBoxLayout>

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdialog.h>
#include <kio/netaccess.h>
#include <KProcess>
#include <kstandarddirs.h>
#include <ksystemtimezone.h>
#include <kdefakes.h>

//#include "xpm/world.xpm"
#include "tzone.moc"

#if defined(USE_SOLARIS)
#include <ktemporaryfile.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

#include "helper.h"

Tzone::Tzone(QWidget * parent)
  : QGroupBox(parent)
{
    setTitle(i18n("To change the timezone, select your area from the list below"));

    QVBoxLayout *lay = new QVBoxLayout(this);

    tzonelist = new KTimeZoneWidget(this);
    connect( tzonelist, SIGNAL(itemSelectionChanged()), SLOT(handleZoneChange()) );

    m_local = new QLabel(this);

    lay->addWidget(m_local);
    lay->addWidget(tzonelist);

    load();
}

void Tzone::load()
{
    currentZone();

    // read the currently set time zone
    tzonelist->setSelected(KSystemTimeZones::local().name(), true);
}

void Tzone::currentZone()
{
    QByteArray result(100, '\0');

    time_t now = time(0);
    tzset();
    strftime(result.data(), result.size(), "%Z", localtime(&now));
    m_local->setText(i18n("Current local timezone: %1 (%2)",
                          KTimeZoneWidget::displayName(KSystemTimeZones::local()),
                          QLatin1String(result)));
}

// FIXME: Does the logic in this routine actually work correctly? For example,
// on non-Solaris systems which do not use /etc/timezone?
void Tzone::save( QStringList& helperargs )
{
    QStringList selectedZones(tzonelist->selection());

    if (selectedZones.count() > 0)
    {
      QString selectedzone(selectedZones[0]);
      helperargs << "tz" << selectedzone; // make the helper set the timezone
    } else {
      helperargs << "tzreset"; // // make the helper reset the timezone
    }

    currentZone();
}

void Tzone::processHelperErrors( int code )
{
  if( code & ERROR_TZONE )
    KMessageBox::error( this, i18n("Error setting new timezone."),
        i18n("Timezone Error"));
}
