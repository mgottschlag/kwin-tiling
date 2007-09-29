/*
 *  Copyright (C) 2003-2006 Andriy Rysin (rysin@kde.org)
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

#ifndef __PIXMAP_H__
#define __PIXMAP_H__


#include <QtGui/QPixmap>
#include <QtCore/QHash>
#include <QtCore/QCharRef>


class LayoutIcon {

private:
	static LayoutIcon* instance;
	static const QString flagTemplate;
	
	QHash<QString, QPixmap*> m_pixmapCache;
	QFont m_labelFont;

	LayoutIcon();
	QPixmap* createErrorPixmap();
	void dimPixmap(QPixmap& pixmap);
	QString getCountryFromLayoutName(const QString& layoutName);
	
  public:
	static const QString& ERROR_CODE;
	
	static LayoutIcon& getInstance();
	const QPixmap& findPixmap(const QString& code, bool showFlag, const QString& displayName="");
};

#endif
