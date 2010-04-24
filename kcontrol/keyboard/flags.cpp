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

#include <QtCore/QStringList>
#include <QtGui/QPixmap>
#include <QtGui/QIcon>

#include "x11_helper.h"

//for text handling
#include "keyboard_config.h"
#include "xkb_rules.h"


static const QString flagTemplate("l10n/%1/flag.png");

Flags::Flags()
{
//	QPixmap emptyPixmap(16, 16);
//	emptyPixmap.fill(Qt::transparent);
//	emptyIcon = new QIcon(emptyPixmap);
}

Flags::~Flags()
{
//	foreach(QString layout, pixmaps.keys()) {
//		delete pixmaps[layout];
//	}
//	delete emptyIcon;
}


//const QPixmap* Flags::getPixmap(const QString& layout)
//{
//	if( pixmaps.contains(layout) ) {
//		return pixmaps[ layout ];
//	}
//
//	QPixmap* pixmap = NULL;
//	if( ! layout.isEmpty() ) {
//		QString countryCode = getCountryFromLayoutName( layout );
//		if( ! countryCode.isEmpty() ) {
//			QString file = KStandardDirs::locate("locale", flagTemplate.arg(countryCode));
//			kDebug() << "Creating pixmap for" << layout << "with" << file;
//			pixmap = new QPixmap(file);
//		}
//	}
//	pixmaps[ layout ] = pixmap;
//	return pixmap;
//}

const QIcon Flags::getIcon(const QString& layout)
{
	if( icons.contains(layout) ) {
		return icons[ layout ];
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
	icons[ layout ] = icon;
	return icon;
}

//static
//const QStringList NON_COUNTRY_LAYOUTS = QString("ara,brai,epo,latam,mao").split(",");

QString Flags::getCountryFromLayoutName(const QString& layout)
{
	QString countryCode = layout.split(X11Helper::LEFT_VARIANT_STR)[0];

	if( countryCode == "nec_vndr/jp" )
		return "jp";

//	if( NON_COUNTRY_LAYOUTS.contain(layout) )
	if( layout.length() > 2 )
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


