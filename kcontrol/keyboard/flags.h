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

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QMap>

class QPixmap;
class QIcon;
class LayoutUnit;
class KeyboardConfig;
class Rules;
namespace Plasma {
	class Svg;
}

class Flags : public QObject
{
	Q_OBJECT

public:
	Flags();
	virtual ~Flags();

	const QIcon getIcon(const QString& layout);
	const QIcon getIconWithText(const LayoutUnit& layoutUnit, const KeyboardConfig& keyboardConfig);
	const QPixmap& getTransparentPixmap() const { return *transparentPixmap; }

	static QString getLongText(const LayoutUnit& layoutUnit, const Rules* rules);
	static QString getShortText(const LayoutUnit& layoutUnit, const KeyboardConfig& keyboardConfig);
	void clearCache();

public Q_SLOTS:
	void themeChanged();

Q_SIGNALS:
	void pixmapChanged();

private:
	QString getCountryFromLayoutName(const QString& fullLayoutName) const;
	Plasma::Svg* getSvg();

	QMap<QString, QIcon> iconMap;
	QMap<QString, QIcon> iconOrTextMap;
	QPixmap* transparentPixmap;
	Plasma::Svg* svg;
};

#endif /* FLAGS_H_ */
