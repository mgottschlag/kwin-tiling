/*
 *  Copyright (c) 2000 Matthias Elter <elter@kde.org>
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

#include <qtabwidget.h>
#include <qlayout.h>

#include <kapp.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <klocale.h>
#include <kimageio.h>

#include <dcopclient.h>

#include "main.h"
#include "main.moc"
#include "generaltab_impl.h"
#include "lookandfeeltab_impl.h"
#include "menutab_impl.h"
#include "buttontab.h"
#include "applettab.h"

#include <X11/Xlib.h>


// for multihead
int kickerconfig_screen_number = 0;


KickerConfig::KickerConfig(QWidget *parent, const char *name)
  : KCModule(parent, name)
{
    if (qt_xdisplay())
	kickerconfig_screen_number = DefaultScreen(qt_xdisplay());

    QVBoxLayout *layout = new QVBoxLayout(this);
    tab = new QTabWidget(this);
    layout->addWidget(tab);

    generaltab = new GeneralTab(this);
    tab->addTab(generaltab, i18n("&General"));
    connect(generaltab, SIGNAL(changed()), this, SLOT(configChanged()));

    lnftab = new LookAndFeelTab(this);
    tab->addTab(lnftab, i18n("&Look &  Feel"));
    connect(lnftab, SIGNAL(changed()), this, SLOT(configChanged()));

    menutab = new MenuTab(this);
    tab->addTab(menutab, i18n("&Menus"));
    connect(menutab, SIGNAL(changed()), this, SLOT(configChanged()));

    buttontab = new ButtonTab(this);
    tab->addTab(buttontab, i18n("&Buttons"));
    connect(buttontab, SIGNAL(changed()), this, SLOT(configChanged()));

    applettab = new AppletTab(this);
    tab->addTab(applettab, i18n("&Applets"));
    connect(applettab, SIGNAL(changed()), this, SLOT(configChanged()));


    load();
}

void KickerConfig::configChanged()
{
    emit changed(true);
}

void KickerConfig::load()
{
    generaltab->load();
    lnftab->load();
    menutab->load();
    buttontab->load();
    applettab->load();
    emit changed(false);
}

void KickerConfig::save()
{
    generaltab->save();
    lnftab->save();
    menutab->save();
    buttontab->save();
    applettab->save();

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
    kapp->dcopClient()->send( appname, "Panel", "configure()", data );
}

void KickerConfig::defaults()
{
    generaltab->defaults();
    lnftab->defaults();
    menutab->defaults();
    buttontab->defaults();
    applettab->defaults();
    emit changed(true);
}

QString KickerConfig::quickHelp() const
{
    return i18n("<h1>Panel</h1> Here you can configure the KDE panel (also"
                " referred to as 'kicker'). This includes options like the position and"
                " size of the panel as well as its hiding behaviour and its looks.<p>"
                " Note that you can access some of these options also by directly clicking"
                " on the panel, e.g. dragging it with the left mouse button or using the"
                " context menu on right button click. This context menu also offers you"
                " manipulation of the panel's buttons and applets.");
}

extern "C"
{
    KCModule *create_kicker(QWidget *parent, const char *name)
    {
        KImageIO::registerFormats();
        KGlobal::locale()->insertCatalogue("kcmkicker");
        KGlobal::dirs()->addResourceType("tiles", KStandardDirs::kde_default("data") +
                                         "kicker/tiles");
        KGlobal::dirs()->addResourceType("hb_pics", KStandardDirs::kde_default("data") +
                                         "kcmkicker/pics");
        KGlobal::dirs()->addResourceType("applets", KStandardDirs::kde_default("data") +
                                         "kicker/applets");
        return new KickerConfig(parent, name);
    };
}
