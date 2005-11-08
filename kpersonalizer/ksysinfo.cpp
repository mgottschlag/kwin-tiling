/***************************************************************************
                          ksysinfo.cpp  -  description
                             -------------------
    begin                : Don Jul 11 2002
    copyright            : (C) 2002 by Carsten Wolff, Christoph Held
    email                :             wolff@kde.org, c-held@web.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/*
 * Code partly taken from kcontrol/info and kcontrol/fonts
 */

#include <qfontdatabase.h>
#include <qfont.h>
#include <qstring.h>
#include <qstringlist.h>

#include <kdebug.h>

#include <X11/Xlib.h>

#include "ksysinfo.h"

KSysInfo::KSysInfo() {
	m_fdb = new QFontDatabase();
	initXInfo();
	initFontFamilies();
	initHWInfo();
	kdDebug() << "KSysInfo: XServer Vendor: " << m_xvendor << endl
	          << "KSysInfo: XServer from XFree Inc: " << m_xfree_inc << endl
	          << "KSysInfo: XServer Release Number: " << m_xrelease << endl
	          << "KSysInfo: XRENDER support is: " << m_xrender << endl
	          << "KSysInfo: Chosen normal font: "<< m_normal_font << endl
	          << "KSysInfo: Chosen fixed width font: "<< m_fixed_font << endl
	          << "KSysInfo: CPU speed: " << m_cpu_speed << " MHz" << endl;
}

KSysInfo::~KSysInfo() {
	delete m_fdb;
}

/*
 * XServer - Info
 */

void KSysInfo::initXInfo() {
	Display *dpy = XOpenDisplay(0);
	// vendor
	m_xvendor = !dpy ? QString::null : (QString)ServerVendor(dpy);
	// XFree-Inc?
	m_xfree_inc = m_xvendor.contains("XFree86");
	// X.org ?
	m_xorg = m_xvendor.contains("X.Org");
	// release-number
	m_xrelease = !dpy ? 0 : VendorRelease(dpy);
	// RENDER-support
	m_xrender = false;
	int extCount;
	QString extension;
	char **extensions = XListExtensions( dpy, &extCount );
	for (int i = 0; i < extCount; i++ ) {
		extension=QString( extensions[i] );
		extension=extension.trimmed();
		if (!extension.compare("RENDER"))
			m_xrender=true;
	}
	XCloseDisplay (dpy);
}

bool KSysInfo::isXfromXFreeInc() {
	return m_xfree_inc;
}

bool KSysInfo::isXfromXOrg() {
	return m_xorg;
}

int KSysInfo::getXRelease() {
	return m_xrelease;
}

bool KSysInfo::getRenderSupport(){
	return m_xrender;
}

/*
 * Font - Info
 */

void KSysInfo::initFontFamilies() {
	QFontDatabase fdb;
	QStringList families = fdb.families();
	m_normal_font = QString::null;
	m_fixed_font = QString::null;
	int normal_priority = 0, fixed_priority = 0;
	for (int i=0; i < families.count(); i++) {
		const QString font( families.at(i) );
		//add further NORMAL fonts here
		if ( (font.contains("Arial [") || font=="Arial") && normal_priority < 15 ) {
			m_normal_font = font;
			normal_priority = 15;
		} else if ( font.contains("Vera Sans") && normal_priority < 12 ) {
			m_normal_font = font;
			normal_priority = 12;
		} else if ( (font.contains("Luxi Sans") || font.contains("Lucidux Sans")) && normal_priority < 10 ) {
			m_normal_font = font;
			normal_priority = 10;
		} else if ( font.contains("Helmet") && normal_priority < 7 ) {
			m_normal_font = font;
			normal_priority = 7;
		} else if ( font.contains("Nimbus Sans") && normal_priority < 5 ) {
			m_normal_font = font;
			normal_priority = 5;
		} else if ( font.contains("Sans") && m_fdb->isSmoothlyScalable(font) && normal_priority < 3 ) {
			m_normal_font = font;
			normal_priority = 3;
		} else if ( m_fdb->isSmoothlyScalable(font) && !(m_fdb->isFixedPitch(font,"Normal") && m_fdb->isFixedPitch(font,"Bold")) && normal_priority < 2) {
			m_normal_font = font;
			normal_priority = 2;
		} else if ( m_fdb->isSmoothlyScalable(font) && normal_priority < 1 ) {
			m_normal_font = font;
			normal_priority = 1;
		}
		//add further FIXED fonts here
		if (font.contains("Courier New") && fixed_priority < 15){
			m_fixed_font = font;
			fixed_priority = 15;
		} else if ( (font.contains("Luxi Mono") || font.contains("Lucidux Mono")) && fixed_priority < 10 ) {
			m_fixed_font = font;
			fixed_priority = 10;
		} else if (font.contains("Andale Mono") && fixed_priority < 5) {
			m_fixed_font = font;
			fixed_priority = 5;
		} else if ( font.contains("Mono") && m_fdb->isSmoothlyScalable(font) && fixed_priority < 3 ) {
			m_fixed_font = font;
			fixed_priority = 3;
		} else if ( m_fdb->isSmoothlyScalable(font) && m_fdb->isFixedPitch(font,"Normal") && fixed_priority < 2 ) {
			m_fixed_font = font;
			fixed_priority = 2;                        
		} else if ( m_fdb->isSmoothlyScalable(font) && fixed_priority < 1 ) {
			m_fixed_font = font;
			fixed_priority = 1;
		}
	}
}

QFont KSysInfo::getNormalFont() {
	return m_fdb->font(m_normal_font,"Normal",12); // this will return the current font, if !m_normal_font
}

QFont KSysInfo::getSmallFont(){
	return m_fdb->font(m_normal_font,"Normal",11);
}

QFont KSysInfo::getBoldFont(){
	return m_fdb->font(m_normal_font,"Bold",12);
}

QFont KSysInfo::getFixedWidthFont(){
	return m_fdb->font(m_fixed_font,"Normal",10);
}

/**
 * Hardware - Info
 * (Architecture - dependent code)
 */

///////////////////
#ifdef __linux__
///////////////////

	#include <qfile.h>
	#include <math.h>

	void KSysInfo::initHWInfo() {
		char buf[512];
		QFile *file = new QFile("/proc/cpuinfo");

		m_cpu_speed = 0;

		if (!file->exists()) {  //CPU info file does not exist, use default value
			delete file;   //only the object :o)
			return;
		}

		if (!file->open(QIODevice::ReadOnly)) {   //No read access, use default value
			delete file;
			return;
		}

		// File Parser
		while (file->readLine(buf, sizeof(buf) - 1) > 0) {
			QString s1 = QString::fromLocal8Bit(buf);
			QString s2 = s1.mid(s1.find(":") + 1);
			s1.truncate(s1.find(":"));
			s1=s1.trimmed();
			s2=s2.trimmed();
			if(s1.contains("MHz")){
				float fspeed = s2.toFloat(0);
				fspeed = floor(fspeed);
				m_cpu_speed = (int)fspeed;
			}
		}
		delete file;
	}

///////////////////
//#elif sgi
///////////////////

///////////////////
//#elif __FreeBSD__
///////////////////

///////////////////
//#elif hpux
///////////////////

///////////////////
//#elif __NetBSD__
///////////////////

///////////////////
//#elif __OpenBSD__
///////////////////

///////////////////
//#elif defined(__svr4__) && defined(sun)
///////////////////

///////////////////
//#elif __svr4__
///////////////////

///////////////////
//#elif _AIX
///////////////////

///////////////////
#else
///////////////////

	void KSysInfo::initHWInfo() {
		m_cpu_speed = 0;
	}

#endif

int KSysInfo::getCpuSpeed() {
	return m_cpu_speed;
}
