/**
 * kcmxinerama.cpp
 *
 * Copyright (c) 2002-2003 George Staikos <staikos@kde.org>
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


#include "kcmxinerama.h"
#include <klocale.h>
#include <kglobal.h>
#include <dcopclient.h>
#include <kaboutdata.h>
#include <qcheckbox.h>
#include <kconfig.h>
#include <kdialog.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <kapplication.h>
#include <qtable.h>
#include <qcolor.h>
#include <kwin.h>
#include <qpushbutton.h>


KCMXinerama::KCMXinerama(QWidget *parent, const char *name)
  : KCModule(parent, name) {
	_indicators.setAutoDelete(true);

	config = new KConfig("kdeglobals", false, false);
	ksplashrc = new KConfig("ksplashrc", false, false);

	connect(&_timer, SIGNAL(timeout()), this, SLOT(clearIndicator()));

	QGridLayout *grid = new QGridLayout(this, 1, 1, KDialog::marginHint(),
							KDialog::spacingHint());

	// Setup the panel
	_displays = QApplication::desktop()->numScreens();

	if (QApplication::desktop()->isVirtualDesktop()) {
		QStringList dpyList;
		xw = new XineramaWidget(this);
		grid->addWidget(xw, 0, 0);
		QString label = i18n("Display %1");

		xw->headTable->setNumRows(_displays);

		for (int i = 0; i < _displays; i++) {
			QString l = label.arg(i+1);
			QRect geom = QApplication::desktop()->screenGeometry(i);
			xw->_kdmDisplay->insertItem(l);
			xw->_unmanagedDisplay->insertItem(l);
			xw->_ksplashDisplay->insertItem(l);
			dpyList.append(l);
			xw->headTable->setText(i, 0, QString::number(geom.x()));
			xw->headTable->setText(i, 1, QString::number(geom.y()));
			xw->headTable->setText(i, 2, QString::number(geom.width()));
			xw->headTable->setText(i, 3, QString::number(geom.height()));
		}

		xw->_unmanagedDisplay->insertItem(i18n("Display Containing The Pointer"));

		xw->headTable->setRowLabels(dpyList);

		connect(xw->_ksplashDisplay, SIGNAL(activated(int)),
			this, SLOT(windowIndicator(int)));
		connect(xw->_kdmDisplay, SIGNAL(activated(int)),
			this, SLOT(windowIndicator(int)));
		connect(xw->_unmanagedDisplay, SIGNAL(activated(int)),
			this, SLOT(windowIndicator(int)));
		connect(xw->_identify, SIGNAL(clicked()),
			this, SLOT(indicateWindows()));

		connect(xw, SIGNAL(configChanged()), this, SLOT(configChanged()));
	} else { // no Xinerama
		QLabel *ql = new QLabel(i18n("<qt><p>This module is only for configuring systems with a single desktop spread across multiple monitors. You do not appear to have this configuration.</p></qt>"), this);
		grid->addWidget(ql, 0, 0);
	}

	grid->activate();

	load();
}

KCMXinerama::~KCMXinerama() {
	_timer.stop();
	delete ksplashrc;
	ksplashrc = 0;
	delete config;
	config = 0;
	clearIndicator();
}

void KCMXinerama::configChanged() {
	emit changed(true);
}


#define KWIN_XINERAMA              "XineramaEnabled"
#define KWIN_XINERAMA_MOVEMENT     "XineramaMovementEnabled"
#define KWIN_XINERAMA_PLACEMENT    "XineramaPlacementEnabled"
#define KWIN_XINERAMA_MAXIMIZE     "XineramaMaximizeEnabled"

void KCMXinerama::load() {
	if (QApplication::desktop()->isVirtualDesktop()) {
		int item = 0;
		config->setGroup("Windows");
		xw->_enableXinerama->setChecked(config->readBoolEntry(KWIN_XINERAMA, true));
		xw->_enableResistance->setChecked(config->readBoolEntry(KWIN_XINERAMA_MOVEMENT, true));
		xw->_enablePlacement->setChecked(config->readBoolEntry(KWIN_XINERAMA_PLACEMENT, true));
		xw->_enableMaximize->setChecked(config->readBoolEntry(KWIN_XINERAMA_MAXIMIZE, true));
		item = config->readNumEntry("Unmanaged", QApplication::desktop()->primaryScreen());
		if ((item < 0 || item >= _displays) && (item != -3))
			xw->_unmanagedDisplay->setCurrentItem(QApplication::desktop()->primaryScreen());
		else if (item == -3) // pointer warp
			xw->_unmanagedDisplay->setCurrentItem(_displays);
		else	xw->_unmanagedDisplay->setCurrentItem(item);

		ksplashrc->setGroup("Xinerama");
		item = ksplashrc->readNumEntry("KSplashScreen", QApplication::desktop()->primaryScreen());
		if (item < 0 || item >= _displays)
			xw->_ksplashDisplay->setCurrentItem(QApplication::desktop()->primaryScreen());
		else xw->_ksplashDisplay->setCurrentItem(item);

	}
	emit changed(false);
}


void KCMXinerama::save() {
	if (QApplication::desktop()->isVirtualDesktop()) {
		config->setGroup("Windows");
		config->writeEntry(KWIN_XINERAMA,
					xw->_enableXinerama->isChecked());
		config->writeEntry(KWIN_XINERAMA_MOVEMENT,
					xw->_enableResistance->isChecked());
		config->writeEntry(KWIN_XINERAMA_PLACEMENT,
					xw->_enablePlacement->isChecked());
		config->writeEntry(KWIN_XINERAMA_MAXIMIZE,
					xw->_enableMaximize->isChecked());
		int item = xw->_unmanagedDisplay->currentItem();
		config->writeEntry("Unmanaged", item == _displays ? -3 : item);
		config->sync();

		if (!kapp->dcopClient()->isAttached())
			kapp->dcopClient()->attach();
		kapp->dcopClient()->send("kwin", "", "reconfigure()", "");

		ksplashrc->setGroup("Xinerama");
		ksplashrc->writeEntry("KSplashScreen", xw->_enableXinerama->isChecked() ? xw->_ksplashDisplay->currentItem() : -2 /* ignore Xinerama */);
		ksplashrc->sync();
	}
	emit changed(false);
}

void KCMXinerama::defaults() {
	if (QApplication::desktop()->isVirtualDesktop()) {
		xw->_enableXinerama->setChecked(true);
		xw->_enableResistance->setChecked(true);
		xw->_enablePlacement->setChecked(true);
		xw->_enableMaximize->setChecked(true);
		xw->_kdmDisplay->setCurrentItem(
				QApplication::desktop()->primaryScreen());
		xw->_unmanagedDisplay->setCurrentItem(
				QApplication::desktop()->primaryScreen());
		xw->_ksplashDisplay->setCurrentItem(
				QApplication::desktop()->primaryScreen());
		emit changed(true);
	} else {
		emit changed(false);
	}
}

void KCMXinerama::indicateWindows() {
	_timer.stop();

	clearIndicator();
	for (int i = 0; i < _displays; i++)
		_indicators.append(indicator(i));

	_timer.start(1500, true);
}

void KCMXinerama::windowIndicator(int dpy) {
	if (dpy >= _displays)
		return;

	_timer.stop();

	clearIndicator();
	_indicators.append(indicator(dpy));

	_timer.start(1500, true);
}

QWidget *KCMXinerama::indicator(int dpy) {
	QRect r = QApplication::desktop()->screenGeometry(dpy);

	r.setLeft(r.left() + 5*r.width()/12);
	r.setRight(r.left() + 1*r.width()/6);
	r.setTop(r.top() + 5*r.height()/12);
	r.setBottom(r.top() + 1*r.height()/6);

	QWidget *_screenIndicator = new QWidget(0, "Screen Indicator", WStyle_StaysOnTop | WStyle_Customize | WStyle_NoBorder);
	_screenIndicator->setGeometry(r);

	QLabel *l = new QLabel(QString::number(dpy+1), _screenIndicator);
	QFont fnt;
	fnt.setPixelSize(48);
	fnt.setBold(true);
	l->setFont(fnt);
	l->setPaletteForegroundColor(QColor(0,0xff,0));
	l->setBackgroundColor(QColor(0,0,0));
	l->resize(l->minimumSizeHint());
	l->move(QPoint((_screenIndicator->width() - l->width())/2, (_screenIndicator->height() - l->height())/2));

	_screenIndicator->setGeometry(r);
	_screenIndicator->setBackgroundColor(QColor(0,0,0));
	KWin::setOnAllDesktops(_screenIndicator->winId(), true);
	KWin::setState(_screenIndicator->winId(), NET::StaysOnTop | NET::Sticky | NET::SkipTaskbar | NET::SkipPager);
        KWin::setType(_screenIndicator->winId(), NET::Override);

	_screenIndicator->show();

	return _screenIndicator;
}

void KCMXinerama::clearIndicator() {
	_indicators.clear();
}

QString KCMXinerama::quickHelp() const {
	return i18n("<h1>Multiple Monitors</h1> This module allows you to configure KDE support"
     " for multiple monitors.");
}

const KAboutData* KCMXinerama::aboutData() const {
 
	KAboutData *about =
	new KAboutData(I18N_NOOP("kcmxinerama"), I18N_NOOP("KDE Multiple Monitor Configurator"),
		0, 0, KAboutData::License_GPL,
		I18N_NOOP("(c) 2002-2003 George Staikos"));
 
	about->addAuthor("George Staikos", 0, "staikos@kde.org");
 
	return about;
}

extern "C" {
KCModule *create_xinerama(QWidget *parent, const char *name) {
	KGlobal::locale()->insertCatalogue("kcmxinerama");
	return new KCMXinerama(parent, name);
}

void init_xinerama() {
}
}


#include "kcmxinerama.moc"

