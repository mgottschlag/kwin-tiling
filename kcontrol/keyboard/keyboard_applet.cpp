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

#include "keyboard_applet.h"

#include <kglobalsettings.h>
#include <KIconLoader>
#include <plasma/theme.h>
#include <plasma/tooltipmanager.h>
#include <ktoolinvocation.h>

#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QAction>
#include <QtGui/QActionGroup>
#include <QtGui/QCheckBox>
#include <QtDBus/QtDBus>

#include "x11_helper.h"
#include "xkb_rules.h"
#include "keyboard_config.h"
#include "keyboard_dbus.h"


K_EXPORT_PLASMA_APPLET(keyboard, KeyboardApplet)

KeyboardApplet::KeyboardApplet(QObject *parent, const QVariantList &args):
	Plasma::Applet(parent, args),
	xEventNotifier(),
	actionGroup(NULL),
	rules(NULL),
	keyboardConfig(new KeyboardConfig())
{
	if( ! X11Helper::xkbSupported(NULL) ) {
		setFailedToLaunch(true, i18n("XKB extension failed to initialize"));
		return;
	}

	resize(48,48);

	setHasConfigurationInterface(false);

	setAspectRatioMode(Plasma::KeepAspectRatio);
	//setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
	setBackgroundHints(DefaultBackground);

	rules = Rules::readRules();

    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.connect(QString(), KEYBOARD_DBUS_OBJECT_PATH, KEYBOARD_DBUS_SERVICE_NAME, KEYBOARD_DBUS_CONFIG_RELOAD_MESSAGE, this, SLOT( configChanged() ));
}

KeyboardApplet::~KeyboardApplet()
{
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.disconnect(QString(), KEYBOARD_DBUS_OBJECT_PATH, KEYBOARD_DBUS_SERVICE_NAME, KEYBOARD_DBUS_CONFIG_RELOAD_MESSAGE, this, SLOT( configChanged() ));

    delete actionGroup;
	delete rules;
}

void KeyboardApplet::keyboardConfigChanged()
{
	readConfig();
	update();
}

void KeyboardApplet::readConfig()
{
//	KConfigGroup config = Plasma::Applet::config("KeyboardLayout");
	keyboardConfig->load();
//	drawFlag = keyboardConfig->readEntry("ShowFlag", true);
}

void KeyboardApplet::configChanged()
{
	Applet::configChanged();
	readConfig();
}

void KeyboardApplet::init()
{
	Applet::init();

	readConfig();
	connect(&xEventNotifier, SIGNAL(layoutChanged()), this, SLOT(layoutChanged()));
	connect(&xEventNotifier, SIGNAL(layoutMapChanged()), this, SLOT(layoutChanged()));
	xEventNotifier.start();

	layoutChanged();
}

void KeyboardApplet::destroy()
{
	xEventNotifier.stop();
	disconnect(&xEventNotifier, SIGNAL(layoutMapChanged()), this, SLOT(layoutChanged()));
	disconnect(&xEventNotifier, SIGNAL(layoutChanged()), this, SLOT(layoutChanged()));
	Applet::destroy();
}

void KeyboardApplet::layoutChanged()
{
	updateTooltip();
	update();
}

void KeyboardApplet::updateTooltip()
{
	LayoutUnit layoutUnit = X11Helper::getCurrentLayout();
	if( layoutUnit.isEmpty() )
		return;

	const QIcon icon(getFlag(layoutUnit.layout));
	Plasma::ToolTipContent data(name(), flags.getLongText(layoutUnit, rules), icon);
	Plasma::ToolTipManager::self()->setContent(this, data);
}

const QIcon KeyboardApplet::getFlag(const QString& layout)
{
	return keyboardConfig->showFlag ? flags.getIcon(layout) : QIcon();
}

void KeyboardApplet::paintInterface(QPainter *p, const QStyleOptionGraphicsItem */*option*/, const QRect &contentsRect)
{
	p->setRenderHint(QPainter::SmoothPixmapTransform);
	p->setRenderHint(QPainter::Antialiasing);
	//p->setBrush(Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));

	LayoutUnit layoutUnit = X11Helper::getCurrentLayout();
	if( layoutUnit.isEmpty() )
		return;

	const QIcon icon(getFlag(layoutUnit.layout));
	if( ! icon.isNull() ) {
		QPixmap pixmap = icon.pixmap(contentsRect.size());
		p->drawPixmap(contentsRect, pixmap);
	}
	else {
		QString shortText = Flags::getShortText(layoutUnit, *keyboardConfig);
		kDebug() << "applet: LayoutChanged" << layoutUnit.toString() << shortText;

		p->save();
		p->setPen(Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
		QFont font = Plasma::Theme::defaultTheme()->font(Plasma::Theme::DesktopFont);
		int height = qMin(contentsRect.height(), contentsRect.width());
		int fontSize = shortText.length() == 2
				? height * 7 / 15
				: height * 5 / 15;

		int smallestReadableSize = KGlobalSettings::smallestReadableFont().pixelSize();
		if( fontSize < smallestReadableSize ) {
			fontSize = smallestReadableSize;
		}
		font.setPixelSize(fontSize);
		p->setFont(font);
		p->drawText(contentsRect, Qt::AlignCenter, shortText);
		p->restore();
	}
}

void KeyboardApplet::mousePressEvent ( QGraphicsSceneMouseEvent * event )
{
	if( event->button() == Qt::LeftButton ) {
		X11Helper::switchToNextLayout();
	}
	event->ignore();
}

void KeyboardApplet::actionTriggered(QAction* action)
{
	QString data = action->data().toString();
	if( data == "config" ) {
		QStringList args;
		args << "kcm_keyboard";
		KToolInvocation::kdeinitExec("kcmshell4", args);
	}
	else {
		X11Helper::setLayout(LayoutUnit(action->data().toString()));
	}
}

QList<QAction*> KeyboardApplet::contextualActions()
{
	if( actionGroup ) {
		disconnect(actionGroup, SIGNAL(triggered(QAction*)), this, SLOT(actionTriggered(QAction*)));
		delete actionGroup;
	}
	actionGroup = new QActionGroup(this);

	X11Helper::getLayoutsList(); //UGLY: seems to be more reliable with extra call
	QList<LayoutUnit> layouts = X11Helper::getLayoutsList();
	foreach(const LayoutUnit& layoutUnit, layouts) {
		QString shortText = Flags::getShortText(layoutUnit, *keyboardConfig);
		QString longText = Flags::getLongText(layoutUnit, rules);
		QString menuText = i18nc("short layout label - full layout name", "%1 - %2", shortText, longText);
//		QString menuText = longText;
		QAction* action = new QAction(getFlag(layoutUnit.layout), menuText, actionGroup);
		action->setData(layoutUnit.toString());
		actionGroup->addAction(action);
	}
	QAction* separator = new QAction(actionGroup);
	separator->setSeparator(true);
	actionGroup->addAction(separator);
	QAction* configAction = new QAction(i18n("Configure..."), actionGroup);
	actionGroup->addAction(configAction);
	configAction->setData("config");
	connect(actionGroup, SIGNAL(triggered(QAction*)), this, SLOT(actionTriggered(QAction*)));
	return actionGroup->actions();
}

void KeyboardApplet::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & Plasma::FormFactorConstraint) {
	int iconSize;
        if (formFactor() == Plasma::Planar ||
            formFactor() == Plasma::MediaCenter) {
            iconSize = IconSize(KIconLoader::Desktop);
        } else {
            iconSize = IconSize(KIconLoader::Small);
        }
	setMinimumSize(iconSize, iconSize);
    }
}

//void KeyboardApplet::createConfigurationInterface(KConfigDialog *parent)
//{
////	Applet::createConfigurationInterface(parent);
//	QWidget* widget = new QWidget();
////	QCheckBox* showFlagCheckbox =
//			new QCheckBox(i18n("Show Flag"), widget);
//	parent->addPage(widget, i18n("Indicator Options"));
//	//TODO: apply config
//}

