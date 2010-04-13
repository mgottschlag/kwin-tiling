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

#include <kconfigdialog.h>

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
	xEventNotifier(XEventNotifier::XKB),
	actionGroup(NULL),
	rules(NULL),
	keyboardConfig(new KeyboardConfig())
{
	if( ! X11Helper::xkbSupported(NULL) ) {
		setFailedToLaunch(true, "XKB extension failed to initialize");
		return;
	}

	setHasConfigurationInterface(false);

//	resize(32, 32);
	setMinimumSize(16, 16);

	setAspectRatioMode(Plasma::KeepAspectRatio);
	setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
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
	update();
}

QString KeyboardApplet::getDisplayText(const QString& fullLayoutName)
{
	if( fullLayoutName.isEmpty() )
		return QString("--");

	LayoutConfig layoutConfig = LayoutConfig::createLayoutConfig(fullLayoutName);
	QString layoutText = layoutConfig.layout;

	foreach(const LayoutConfig& lc, keyboardConfig->layouts) {
		if( layoutConfig.layout == lc.layout && layoutConfig.variant == lc.variant ) {
			layoutText = lc.getDisplayName();
			break;
		}
	}

	return layoutText;
}

QString KeyboardApplet::getLongText(const QString& fullLayoutName)
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

const QIcon KeyboardApplet::getFlag(const QString& layout)
{
	return keyboardConfig->showFlag ? flags.getIcon(layout) : QIcon();
}

void KeyboardApplet::paintInterface(QPainter *p, const QStyleOptionGraphicsItem */*option*/, const QRect &contentsRect)
{
	p->setRenderHint(QPainter::SmoothPixmapTransform);
	p->setRenderHint(QPainter::Antialiasing);

	QString layout = X11Helper::getCurrentLayout();
	QString layoutText = getDisplayText(layout);

	const QIcon icon(getFlag(layout));
	if( ! icon.isNull() ) {
		QPixmap pixmap = icon.pixmap(contentsRect.size());
		p->drawPixmap(contentsRect, pixmap);
	}
	p->save();
	p->setPen(Qt::black);
	QFont font = p->font();
	int fontSize = layoutText.length() == 2
			? contentsRect.height() * 7 / 10
			: contentsRect.height() * 5 / 10;
	if( fontSize < 6 ) {
		fontSize = 6;
	}
	font.setPixelSize(fontSize);
	p->setFont(font);
	p->drawText(contentsRect, Qt::AlignCenter | Qt::AlignHCenter, layoutText);
	p->restore();
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
	kDebug() << "actionTriggerd" << action->data().toString();
	X11Helper::setLayout(action->data().toString());
}

QList<QAction*> KeyboardApplet::contextualActions()
{
	delete actionGroup;
	actionGroup = new QActionGroup(this);
	QStringList layouts = X11Helper::getLayoutsList();
	foreach(QString layout, layouts) {
		QAction* action;
		QString menuText = getLongText(layout);
//		if( pixmap != NULL ) {
			action = new QAction(getFlag(layout), menuText, actionGroup);
//		}
//		else {
//			action = new QAction(menuText, actionGroup);
//		}
		action->setData(layout);
		actionGroup->addAction(action);
	}
	connect(actionGroup, SIGNAL(triggered(QAction*)), this, SLOT(actionTriggered(QAction*)));
	return actionGroup->actions();
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

//TODO: exclude XInput somehow nicer
int XEventNotifier::registerForNewDeviceEvent(Display* /*display*/)
{
	return -1;
}

bool XEventNotifier::isNewDeviceEvent(XEvent* /*event*/)
{
	return false;
}
