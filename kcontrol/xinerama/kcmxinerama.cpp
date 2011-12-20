/**
 * kcmxinerama.cpp
 *
 * Copyright (c) 2002-2004 George Staikos <staikos@kde.org>
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


#include "kcmxinerama.h"
#include <kaboutdata.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <QtDBus/QtDBus>

#include <QCheckBox>
#include <QtGui/QDesktopWidget>
#include <QLayout>
#include <QLabel>
#include <QComboBox>
#include <QColor>
#include <QPushButton>
//Added by qt3to4:
#include <QFrame>
#include <QGridLayout>
#include "kwin_interface.h"

K_PLUGIN_FACTORY(KCMXineramaFactory, registerPlugin<KCMXinerama>();)
K_EXPORT_PLUGIN(KCMXineramaFactory("kcmxinerama"))


KCMXinerama::KCMXinerama(QWidget *parent, const QVariantList &)
  : KCModule(KCMXineramaFactory::componentData(), parent), xw(0) {

	KAboutData *about =
	new KAboutData(I18N_NOOP("kcmxinerama"), 0,
			ki18n("KDE Multiple Monitor Configurator"),
			0, KLocalizedString(), KAboutData::License_GPL,
			ki18n("(c) 2002-2003 George Staikos"));

	about->addAuthor(ki18n("George Staikos"), KLocalizedString(), "staikos@kde.org");
	setAboutData( about );
        setButtons(Apply);
	setQuickHelp( i18n("<h1>Multiple Monitors</h1> This module allows you to configure KDE support"
     " for multiple monitors."));

	config = new KConfig("kdeglobals", KConfig::NoGlobals);
	ksplashrc = new KConfig("ksplashrc", KConfig::NoGlobals);

	_timer.setSingleShot(true);
	connect(&_timer, SIGNAL(timeout()), this, SLOT(clearIndicator()));

	QGridLayout *grid = new QGridLayout(this );
        grid->setMargin( KDialog::marginHint() );
        grid->setSpacing( KDialog::spacingHint() );

	// Setup the panel
	_displays = QApplication::desktop()->numScreens();

	if (QApplication::desktop()->isVirtualDesktop()) {
		QStringList dpyList;
		xw = new XineramaWidget(this);
		grid->addWidget(xw, 0, 0);

		xw->headTable->setRowCount(_displays);

		for (int i = 0; i < _displays; i++) {
			QString l = i18n("Display %1", i+1);
			QRect geom = QApplication::desktop()->screenGeometry(i);
			xw->_unmanagedDisplay->addItem(l);
			dpyList.append(l);
			QTableWidgetItem *item = new QTableWidgetItem(QString::number(geom.x()));
			item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			xw->headTable->setItem(i, 0, item);
			item = new QTableWidgetItem(QString::number(geom.y()));
			item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			xw->headTable->setItem(i, 1, item);
			item = new QTableWidgetItem(QString::number(geom.width()));
			item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			xw->headTable->setItem(i, 2, item);
			item = new QTableWidgetItem(QString::number(geom.height()));
			item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			xw->headTable->setItem(i, 3, item);
		}

		xw->_unmanagedDisplay->addItem(i18n("Display Containing the Pointer"));

		xw->headTable->setVerticalHeaderLabels(dpyList);

		connect(xw->_unmanagedDisplay, SIGNAL(activated(int)),
			this, SLOT(windowIndicator(int)));
		connect(xw->_identify, SIGNAL(clicked()),
			this, SLOT(indicateWindows()));

		connect(xw, SIGNAL(configChanged()), this, SLOT(changed()));
	} else { // no Xinerama
    QGridLayout *layout = new QGridLayout(this);
    m_noXineramaMessage = new QWidget();
    QLabel *labelicon = new QLabel(m_noXineramaMessage);
    labelicon->setPixmap(KIcon("preferences-desktop-display-multiple", KIconLoader::global(), QStringList() << "" << "dialog-error").pixmap(128, 128));
    layout->addWidget(labelicon, 0, 1, Qt::AlignHCenter);
    QLabel *labeltext = new QLabel(i18n("You do not appear to have a single desktop spread across multiple monitors."), m_noXineramaMessage);
    layout->addWidget(labeltext,1, 1, Qt::AlignHCenter);
    layout->setRowStretch(2, 1);
    layout->setColumnStretch(0, 1);
    layout->setColumnStretch(2, 1);
    m_noXineramaMessage->setLayout(layout);
 
    grid->addWidget(m_noXineramaMessage);
	}

	grid->activate();
}

KCMXinerama::~KCMXinerama() {
	_timer.stop();
	delete ksplashrc;
	ksplashrc = 0;
	delete config;
	config = 0;
	clearIndicator();
}

#define KWIN_XINERAMA              "XineramaEnabled"
#define KWIN_XINERAMA_MOVEMENT     "XineramaMovementEnabled"
#define KWIN_XINERAMA_PLACEMENT    "XineramaPlacementEnabled"
#define KWIN_XINERAMA_MAXIMIZE     "XineramaMaximizeEnabled"
#define KWIN_XINERAMA_FULLSCREEN   "XineramaFullscreenEnabled"

void KCMXinerama::load() {
	if (QApplication::desktop()->isVirtualDesktop() && xw) {
		int item = 0;
		KConfigGroup group = config->group("Windows");
		xw->_enableXinerama->setChecked(group.readEntry(KWIN_XINERAMA, true));
		xw->_enableResistance->setChecked(group.readEntry(KWIN_XINERAMA_MOVEMENT, true));
		xw->_enablePlacement->setChecked(group.readEntry(KWIN_XINERAMA_PLACEMENT, true));
		xw->_enableMaximize->setChecked(group.readEntry(KWIN_XINERAMA_MAXIMIZE, true));
		xw->_enableFullscreen->setChecked(group.readEntry(KWIN_XINERAMA_FULLSCREEN, true));
		item = group.readEntry("Unmanaged", QApplication::desktop()->primaryScreen());
		if ((item < 0 || item >= _displays) && (item != -3))
			xw->_unmanagedDisplay->setCurrentIndex(QApplication::desktop()->primaryScreen());
		else if (item == -3) // pointer warp
			xw->_unmanagedDisplay->setCurrentIndex(_displays);
		else	xw->_unmanagedDisplay->setCurrentIndex(item);
	}
	emit changed(false);
}


void KCMXinerama::save() {
	if (QApplication::desktop()->isVirtualDesktop() && xw) {
		KConfigGroup group = config->group("Windows");
		group.writeEntry(KWIN_XINERAMA,
					xw->_enableXinerama->isChecked());
		group.writeEntry(KWIN_XINERAMA_MOVEMENT,
					xw->_enableResistance->isChecked());
		group.writeEntry(KWIN_XINERAMA_PLACEMENT,
					xw->_enablePlacement->isChecked());
		group.writeEntry(KWIN_XINERAMA_MAXIMIZE,
					xw->_enableMaximize->isChecked());
		group.writeEntry(KWIN_XINERAMA_FULLSCREEN,
					xw->_enableFullscreen->isChecked());
		int item = xw->_unmanagedDisplay->currentIndex();
		group.writeEntry("Unmanaged", item == _displays ? -3 : item);
		group.sync();

                org::kde::KWin kwin("org.kde.kwin", "/KWin", QDBusConnection::sessionBus());
                kwin.reconfigure();
	}

	KMessageBox::information(this, i18n("Some settings may affect only newly started applications."), i18n("KDE Multiple Monitors"), "nomorexineramaplease");

	emit changed(false);
}

void KCMXinerama::defaults() {
	if (QApplication::desktop()->isVirtualDesktop() && xw) {
		xw->_enableXinerama->setChecked(true);
		xw->_enableResistance->setChecked(true);
		xw->_enablePlacement->setChecked(true);
		xw->_enableMaximize->setChecked(true);
		xw->_enableFullscreen->setChecked(true);
		xw->_unmanagedDisplay->setCurrentIndex(
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

	_timer.start(1500);
}

void KCMXinerama::windowIndicator(int dpy) {
	if (dpy >= _displays)
		return;

	_timer.stop();

	clearIndicator();
	_indicators.append(indicator(dpy));

	_timer.start(1500);
}

QWidget *KCMXinerama::indicator(int dpy) {
	QLabel *si = new QLabel(QString::number(dpy+1), 0, Qt::X11BypassWindowManagerHint);

	QFont fnt = KGlobalSettings::generalFont();
	fnt.setPixelSize(100);
	si->setFont(fnt);
	si->setFrameStyle(QFrame::Panel);
	si->setFrameShadow(QFrame::Plain);
	si->setAlignment(Qt::AlignCenter);

	QPoint screenCenter(QApplication::desktop()->screenGeometry(dpy).center());
	QRect targetGeometry(QPoint(0,0), si->sizeHint());
        targetGeometry.moveCenter(screenCenter);
	si->setGeometry(targetGeometry);
	si->show();

	return si;
}

void KCMXinerama::clearIndicator() {
	qDeleteAll(_indicators);
	_indicators.clear();
}

#include "kcmxinerama.moc"

