/*
 *  Copyright (C) 2010 Andriy Rysin (rysin@kde.org)
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


#ifndef FLAGS_H_
#define FLAGS_H_

#include <QtCore/QString>
#include <QtCore/QMap>

class QPixmap;
class QIcon;
class LayoutUnit;
class KeyboardConfig;
class Rules;

class Flags {

public:
	Flags();
	virtual ~Flags();

	const QIcon getIcon(const QString& layout);
	const QIcon getIconWithText(const LayoutUnit& layoutUnit, const KeyboardConfig& keyboardConfig);
	const QPixmap& getTransparentPixmap() const { return *transparentPixmap; }
	void clearCache();

	static QString getLongText(const LayoutUnit& layoutUnit, const Rules* rules);
	static QString getShortText(const LayoutUnit& layoutUnit, const KeyboardConfig& keyboardConfig);

private:
	QString getCountryFromLayoutName(const QString& fullLayoutName) const;

	QMap<QString, QIcon> iconMap;
	QMap<QString, QIcon> iconOrTextMap;
	QPixmap* transparentPixmap;
};

#endif /* FLAGS_H_ */
