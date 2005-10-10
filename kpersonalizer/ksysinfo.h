/***************************************************************************
                          ksysinfo.h  -  description
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

#ifndef KSYSINFO_H
#define KSYSINFO_H

class QString;
class QFont;
class QFontDatabase;

class KSysInfo {
public:
	KSysInfo();
	~KSysInfo();
	/* XServer - info */
	bool isXfromXFreeInc();
	bool isXfromXOrg();
	int getXRelease();
	bool getRenderSupport();
	/* font - info */
	QFont getNormalFont();
	QFont getSmallFont();
	QFont getBoldFont();
	QFont getFixedWidthFont();
	/* Hardware - info */
	int getCpuSpeed();
private:
	void initXInfo();
	void initFontFamilies();
	void initHWInfo();
private:
	/* XServer - info */
	QString m_xvendor;
	bool m_xfree_inc;
	bool m_xorg;
	int m_xrelease;
	bool m_xrender;
	/* font - info */
	QFontDatabase* m_fdb;
	QString m_normal_font;
	QString m_fixed_font;
	/* Hardware - info */
	int m_cpu_speed;
};

#endif
