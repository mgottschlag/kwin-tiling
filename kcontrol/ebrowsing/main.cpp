/*
 * main.cpp
 *
 * Copyright (c) 2000 Yves Arrouye <yves@realnames.com>
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

#include <unistd.h>

#include <qlayout.h>

#include <kapp.h>
#include <dcopclient.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <kdialog.h>

#include "ikwsopts.h"
#include "main.h"

KURIFilterModule::KURIFilterModule(QWidget *parent, const char *name)
    : KCModule(parent, name) {

    QVBoxLayout *layout = new QVBoxLayout(this);
    tab = new QTabWidget(this);
    layout->addWidget(tab);

    ikws = new InternetKeywordsOptions(0, this);
    tab->addTab(ikws, i18n("Internet &Keywords"));
    connect(ikws, SIGNAL(changed(bool)), this, SLOT(moduleChanged(bool)));

    load();
}

void KURIFilterModule::load() {
    ikws->load();
}

void KURIFilterModule::save() {
    ikws->save();
}

void KURIFilterModule::defaults() {
    ikws->defaults();
}

void KURIFilterModule::moduleChanged(bool state) {
    emit changed(state);
}

#if 0

void KURIFilterModule::resizeEvent(QResizeEvent *) {
    tab->setGeometry(0,0,width(),height());
}

#endif

extern "C" {

KCModule *create_kurifilt(QWidget *parent, const char *name) {
    KGlobal::locale()->insertCatalogue("kcmkurifilt");
    return new KURIFilterModule(parent, name);
}

}

#include "main.moc"
