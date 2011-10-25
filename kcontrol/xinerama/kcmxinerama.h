/**
 * kcmxinerama.h
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

#ifndef _KCM_XINERAMA_H
#define _KCM_XINERAMA_H

#include <kcmodule.h>
#include <QTimer>

#include "xineramawidget.h"

class KConfig;
class QWidget;

class KCMXinerama : public KCModule {
	Q_OBJECT
public:
	KCMXinerama(QWidget *parent, const QVariantList &args);
	virtual ~KCMXinerama();

	void load();
	void save();
	void defaults();

public Q_SLOTS:
	void windowIndicator(int dpy);
	void clearIndicator();

	void indicateWindows();

private:
	QWidget* indicator(int dpy);
	KConfig *config, *ksplashrc;
	XineramaWidget *xw;
	QTimer _timer;
	QList<QWidget *> _indicators;
  QWidget* m_noXineramaMessage;

	int _displays;

};

#endif

