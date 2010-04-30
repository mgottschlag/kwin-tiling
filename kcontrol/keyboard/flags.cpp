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

#include "flags.h"

#include <kdebug.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kglobalsettings.h>

#include <QtCore/QStringList>
#include <QtGui/QPixmap>
#include <QtGui/QPainter>
#include <QtGui/QIcon>

#include "x11_helper.h"

//for text handling
#include "keyboard_config.h"
#include "xkb_rules.h"

static const int FLAG_MAX_WIDTH = 21;
static const int FLAG_MAX_HEIGHT = 14;
static const QString flagTemplate("l10n/%1/flag.png");

Flags::Flags():
	transparentIcon(new QPixmap(FLAG_MAX_WIDTH, FLAG_MAX_HEIGHT))
{
	transparentIcon->fill(Qt::transparent);
}

Flags::~Flags()
{
	delete transparentIcon;
}

const QIcon Flags::getIcon(const QString& layout)
{
	if( iconMap.contains(layout) ) {
		return iconMap[ layout ];
	}

	QIcon icon;
	if( ! layout.isEmpty() ) {
		QString countryCode = getCountryFromLayoutName( layout );
		if( ! countryCode.isEmpty() ) {
			QString file = KStandardDirs::locate("locale", flagTemplate.arg(countryCode));
//			kDebug() << "Creating icon for" << layout << "with" << file;
			icon.addFile(file);
		}
	}
	iconMap[ layout ] = icon;
	return icon;
}

//static
//const QStringList NON_COUNTRY_LAYOUTS = QString("ara,brai,epo,latam,mao").split(",");

QString Flags::getCountryFromLayoutName(const QString& fullLayoutName)
{
	LayoutConfig layoutConfig = LayoutConfig::createLayoutConfig(fullLayoutName);
	QString countryCode = layoutConfig.layout;

	if( countryCode == "nec_vndr/jp" )
		return "jp";

//	if( NON_COUNTRY_LAYOUTS.contain(layout) )
	if( countryCode.length() > 2 )
		return "";

	return countryCode;
}

//TODO: move this to some other class?

QString Flags::getDisplayText(const QString& fullLayoutName, const KeyboardConfig& keyboardConfig)
{
	if( fullLayoutName.isEmpty() )
		return QString("--");

	LayoutConfig layoutConfig = LayoutConfig::createLayoutConfig(fullLayoutName);
	QString layoutText = layoutConfig.layout;

	foreach(const LayoutConfig& lc, keyboardConfig.layouts) {
		if( layoutConfig.layout == lc.layout && layoutConfig.variant == lc.variant ) {
			layoutText = lc.getDisplayName();
			break;
		}
	}

	return layoutText;
}

QString Flags::getLongText(const QString& fullLayoutName, const Rules* rules)
{
	if( fullLayoutName.isEmpty() )
		return "";

	if( rules == NULL ) {
		return fullLayoutName;
	}

	LayoutConfig layoutConfig = LayoutConfig::createLayoutConfig(fullLayoutName);
	QString layoutText = fullLayoutName;

	const LayoutInfo* layoutInfo = rules->getLayoutInfo(layoutConfig.layout);
	if( layoutInfo != NULL ) {
		layoutText = layoutInfo->description;

		if( ! layoutConfig.variant.isEmpty() ) {
			const VariantInfo* variantInfo = layoutInfo->getVariantInfo(layoutConfig.variant);
			QString variantText = variantInfo != NULL ? variantInfo->description : layoutConfig.variant;

			return QString("%1 - %2").arg(layoutText, variantText);
		}
	}

	return layoutText;
}

const QIcon Flags::getIconWithText(const QString& fullLayoutName, const KeyboardConfig& keyboardConfig)
{
	QString keySuffix(keyboardConfig.showFlag ? "_wf" : "_nf");
	QString key(fullLayoutName + keySuffix);
	if( iconOrTextMap.contains(key) ) {
		return iconOrTextMap[ key ];
	}

	if( keyboardConfig.showFlag ) {
		QIcon icon = getIcon(fullLayoutName);
		if( ! icon.isNull() ) {
			iconOrTextMap[ key ] = icon;
			return icon;
		}
	}

	QPixmap pm = QPixmap(KIconLoader::SizeLarge, KIconLoader::SizeLarge);
	pm.fill(Qt::transparent);

	QPainter p(&pm);
	p.setRenderHint(QPainter::SmoothPixmapTransform);
	p.setRenderHint(QPainter::Antialiasing);

	QFont font = p.font();
	QString layoutText = Flags::getDisplayText(fullLayoutName, keyboardConfig);

	int height = pm.height();

	int fontSize = layoutText.length() == 2
			? height * 7 / 10
			: height * 5 / 10;

	int smallestReadableSize = KGlobalSettings::smallestReadableFont().pixelSize();
	if( fontSize < smallestReadableSize ) {
		fontSize = smallestReadableSize;
	}

	font.setPixelSize(fontSize);
	p.setFont(font);
	p.drawText(pm.rect(), Qt::AlignCenter | Qt::AlignHCenter, layoutText);

	QIcon icon(pm);
	iconOrTextMap[ key ] = icon;

	return icon;
}
