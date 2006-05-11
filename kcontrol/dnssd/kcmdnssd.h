/***************************************************************************
 *   Copyright (C) 2004,2005 by Jakub Stachowski                           *
 *   qbast@go2.pl                                                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/

#ifndef _KCMDNSSD_H_
#define _KCMDNSSD_H_

#include <QMap>

#include <configdialog.h>
#include <kaboutdata.h>

class KSimpleConfig;
class KCMDnssd: public ConfigDialog
{
	Q_OBJECT

public:
	KCMDnssd( QWidget *parent=0, const char *name=0, const QStringList& = QStringList() );
	~KCMDnssd();
	virtual void save();
	virtual void load();
private Q_SLOTS:
	void wdchanged();
private: 
	void loadMdnsd();
	bool saveMdnsd();
	QMap<QString,QString> mdnsdLines;
	bool m_wdchanged;
	KSimpleConfig* domain;
};

#endif
