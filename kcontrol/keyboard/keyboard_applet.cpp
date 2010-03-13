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

#include "x11_helper.h"
#include "xkb_rules.h"


K_EXPORT_PLASMA_APPLET(keyboard, KeyboardApplet)

KeyboardApplet::KeyboardApplet(QObject *parent, const QVariantList &args):
	Plasma::Applet(parent, args),
	xEventNotifier(XEventNotifier::XKB),
	actionGroup(NULL),
	rules(NULL)
{
	if( ! X11Helper::xkbSupported(NULL) ) {
		setFailedToLaunch(true, "XKB extension failed to initialize");
		return;
	}

	setHasConfigurationInterface(true);

	resize(32, 32);
	setAspectRatioMode(Plasma::IgnoreAspectRatio);
	setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding));
	setBackgroundHints(DefaultBackground);

	rules = Rules::readRules();
}

KeyboardApplet::~KeyboardApplet()
{
	delete actionGroup;
	delete rules;
}

void KeyboardApplet::readConfig()
{
	KConfigGroup config = Plasma::Applet::config("KeyboardLayout");
	drawFlag = config.readEntry("ShowFlag", true);
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

QString KeyboardApplet::getDisplayText(const QString& fullLayout)
{
	if( fullLayout.isEmpty() )
		return QString("--");

	return fullLayout.split(X11Helper::LEFT_VARIANT_STR)[0];
}

QString KeyboardApplet::getLongText(const QString& fullLayout)
{
	if( fullLayout.isEmpty() )
		return "";

	if( rules == NULL ) {
		return fullLayout;
	}

	QString layout = fullLayout.split(X11Helper::LEFT_VARIANT_STR)[0];

	const LayoutInfo* layoutInfo = rules->getLayoutInfo(layout);
	return layoutInfo != NULL && ! layoutInfo->description.isEmpty()
			? layoutInfo->description : fullLayout;
}

const QIcon KeyboardApplet::getFlag(const QString& layout)
{
	return drawFlag ? flags.getIcon(layout) : QIcon();
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
	//TODO: cache font?
	QFont font = p->font();
	int fontSize = contentsRect.height() > 9 ? contentsRect.height()-3 : 6;
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

void KeyboardApplet::createConfigurationInterface(KConfigDialog *parent)
{
//	Applet::createConfigurationInterface(parent);
	QWidget* widget = new QWidget();
//	QCheckBox* showFlagCheckbox =
			new QCheckBox(i18n("Show Flag"), widget);
	parent->addPage(widget, i18n("Indicator Options"));
	//TODO: apply config
}

//TODO: exclude XInput somehow nicer
int XEventNotifier::registerForNewDeviceEvent(Display* /*display*/)
{
	return -1;
}

bool XEventNotifier::isNewDeviceEvent(XEvent* /*event*/)
{
	return false;
}
