/*
 *  Copyright (c) 2002 Stephan Binner <binner@kde.org>
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
 */

#include <qlayout.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kimageio.h>

#include <dcopclient.h>

#include "menutab_kcm.moc"
#include "menutab_impl.h"

#include <X11/Xlib.h>
#include <kaboutdata.h>


// for multihead
extern int kickerconfig_screen_number;


MenuConfig::MenuConfig(QWidget *parent, const char *name)
  : KCModule(parent, name)
{
    kickerconfig_screen_number=0;
    if (qt_xdisplay())
      kickerconfig_screen_number = DefaultScreen(qt_xdisplay());

    QVBoxLayout *layout = new QVBoxLayout(this);

    menutab = new MenuTab(this);
    layout->addWidget(menutab);

    connect(menutab, SIGNAL(changed()), this, SLOT(configChanged()));

    load();
}

void MenuConfig::configChanged()
{
    emit changed(true);
}

void MenuConfig::load()
{
    menutab->load();
    emit changed(false);
}

void MenuConfig::save()
{
    menutab->save();

    emit changed(false);

    // Tell kicker about the new config file.
    if (!kapp->dcopClient()->isAttached())
        kapp->dcopClient()->attach();
    QByteArray data;

    QCString appname;
    if (kickerconfig_screen_number == 0)
	appname = "kicker";
    else
	appname.sprintf("kicker-screen-%d", kickerconfig_screen_number);
    kapp->dcopClient()->send( appname, "kicker", "configure()", data );
}

void MenuConfig::defaults()
{
    menutab->defaults();

    emit changed(true);
}

QString MenuConfig::quickHelp() const
{
    return i18n("<h1>Panel</h1> Here you can configure the KDE panel (also"
                " referred to as 'kicker'). This includes options like the position and"
                " size of the panel, as well as its hiding behavior and its looks.<p>"
                " Note that you can also access some of these options directly by clicking"
                " on the panel, e.g. dragging it with the left mouse button or using the"
                " context menu on right mouse button click. This context menu also offers you"
                " manipulation of the panel's buttons and applets.");
}

const KAboutData* MenuConfig::aboutData() const
{

    KAboutData *about =
    new KAboutData(I18N_NOOP("kcmkicker"), I18N_NOOP("KDE Panel Control Module"),
                  0, 0, KAboutData::License_GPL,
                  I18N_NOOP("(c) 1999 - 2001 Matthias Elter\n(c) 2002 Aaron J. Seigo"));

    about->addAuthor("Matthias Elter", 0, "elter@kde.org");
    about->addAuthor("Aaron J. Seigo", 0, "aseigo@olympusproject.org");

    return about;
}
