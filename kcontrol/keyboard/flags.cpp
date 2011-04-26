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
#include <klocalizedstring.h>

#include <plasma/svg.h>
#include <plasma/paintutils.h>

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
static const char flagTemplate[] = "l10n/%1/flag.png";

Flags::Flags():
	svg(NULL)
{
	transparentPixmap = new QPixmap(FLAG_MAX_WIDTH, FLAG_MAX_HEIGHT);
	transparentPixmap->fill(Qt::transparent);
}

Flags::~Flags()
{
	if( svg != NULL ) {
		disconnect(svg, SIGNAL(repaintNeeded()), this, SLOT(themeChanged()));
		delete svg;
	}
	delete transparentPixmap;
}

const QIcon Flags::getIcon(const QString& layout)
{
	if( iconMap.contains(layout) ) {
		return iconMap[ layout ];
	}

	QIcon icon;
	if( ! layout.isEmpty() ) {
		if( layout == "epo" ) {
			QString file = KStandardDirs::locate("data", "kcmkeyboard/pics/epo.png");
			icon.addFile(file);
		}
		else {
			QString countryCode = getCountryFromLayoutName( layout );
			if( ! countryCode.isEmpty() ) {
				QString file = KStandardDirs::locate("locale", QString(flagTemplate).arg(countryCode));
				//			kDebug() << "Creating icon for" << layout << "with" << file;
				icon.addFile(file);
			}
		}
	}
	iconMap[ layout ] = icon;
	return icon;
}

//static
//const QStringList NON_COUNTRY_LAYOUTS = QString("ara,brai,epo,latam,mao").split(",");

QString Flags::getCountryFromLayoutName(const QString& layout)  const
{
	QString countryCode = layout;

	if( countryCode == "nec_vndr/jp" )
		return "jp";

//	if( NON_COUNTRY_LAYOUTS.contain(layout) )
	if( countryCode.length() > 2 )
		return "";

	return countryCode;
}

//TODO: move this to some other class?

QString Flags::getShortText(const LayoutUnit& layoutUnit, const KeyboardConfig& keyboardConfig)
{
	if( layoutUnit.isEmpty() )
		return QString("--");

	QString layoutText = layoutUnit.layout;

	foreach(const LayoutUnit& lu, keyboardConfig.layouts) {
		if( layoutUnit.layout == lu.layout && layoutUnit.variant == lu.variant ) {
			layoutText = lu.getDisplayName();
			break;
		}
	}

//TODO: good autolabel
//	if( layoutText == layoutUnit.layout && layoutUnit.getDisplayName() != layoutUnit.layout ) {
//		layoutText = layoutUnit.getDisplayName();
//	}

	return layoutText;
}

QString Flags::getFullText(const LayoutUnit& layoutUnit, const KeyboardConfig& keyboardConfig, const Rules* rules)
{
	QString shortText = Flags::getShortText(layoutUnit, keyboardConfig);
	QString longText = Flags::getLongText(layoutUnit, rules);
	return i18nc("short layout label - full layout name", "%1 - %2", shortText, longText);
}

static QString getDisplayText(const QString& layout, const QString& variant, const Rules* rules)
{
	if( variant.isEmpty() )
		return layout;
	if( rules == NULL || rules->version == "1.0" )
		return i18nc("layout - variant", "%1 - %2", layout, variant);
	return variant;
}

QString Flags::getLongText(const LayoutUnit& layoutUnit, const Rules* rules)
{
	if( rules == NULL ) {
		return getDisplayText(layoutUnit.layout, layoutUnit.variant, rules);
	}

	QString layoutText = layoutUnit.layout;
	const LayoutInfo* layoutInfo = rules->getLayoutInfo(layoutUnit.layout);
	if( layoutInfo != NULL ) {
		layoutText = layoutInfo->description;

		if( ! layoutUnit.variant.isEmpty() ) {
			const VariantInfo* variantInfo = layoutInfo->getVariantInfo(layoutUnit.variant);
			QString variantText = variantInfo != NULL ? variantInfo->description : layoutUnit.variant;

			layoutText = getDisplayText(layoutText, variantText, rules);
		}
	}

	return layoutText;
}

const QIcon Flags::getIconWithText(const LayoutUnit& layoutUnit, const KeyboardConfig& keyboardConfig)
{
	QString keySuffix(keyboardConfig.showFlag ? "_wf" : "_nf");
	QString key(layoutUnit.toString() + keySuffix);
	if( iconOrTextMap.contains(key) ) {
		return iconOrTextMap[ key ];
	}

	if( keyboardConfig.showFlag ) {
		QIcon icon = getIcon(layoutUnit.layout);
		if( ! icon.isNull() ) {
			iconOrTextMap[ key ] = icon;
			return icon;
		}
	}

	QString layoutText = Flags::getShortText(layoutUnit, keyboardConfig);

	QPixmap pm = QPixmap(KIconLoader::SizeLarge, KIconLoader::SizeLarge);
//	pm.fill(Qt::transparent);

	QPainter p(&pm);
//	p.setRenderHint(QPainter::SmoothPixmapTransform);
//	p.setRenderHint(QPainter::Antialiasing);

	QFont font = p.font();

	int height = pm.height();
	int fontSize = layoutText.length() == 2
			? height * 7 / 10
			: height * 5 / 10;

	int smallestReadableSize = KGlobalSettings::smallestReadableFont().pixelSize();
	if( fontSize < smallestReadableSize ) {
		fontSize = smallestReadableSize;
	}
	font.setPixelSize(fontSize);
	
//	p.setFont(font);
//	p.setPen(Qt::black);
//	p.drawText(pm.rect(), Qt::AlignCenter | Qt::AlignHCenter, layoutText);
//	QIcon icon(pm);

	// we init svg so that we get notification about theme change
	//Plasma::Svg* svg =
	getSvg();
//    QPixmap pixmap = Plasma::PaintUtils::texturedText(layoutText, font, svg);
    QPixmap pixmap = Plasma::PaintUtils::shadowText(layoutText, font);

    QIcon icon(pixmap);
	iconOrTextMap[ key ] = icon;

	return icon;
}

Plasma::Svg* Flags::getSvg()
{
	if( svg == NULL ) {
		svg = new Plasma::Svg;
	    svg->setImagePath("widgets/labeltexture");
	    svg->setContainsMultipleImages(true);
	    connect(svg, SIGNAL(repaintNeeded()), this, SLOT(themeChanged()));
	}
	return svg;
}

void Flags::themeChanged()
{
//	kDebug() << "Theme changed, new text color" << Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
	clearCache();
	emit pixmapChanged();
}

void Flags::clearCache()
{
//	kDebug() << "Clearing flag pixmap cache";
	iconOrTextMap.clear();
}
