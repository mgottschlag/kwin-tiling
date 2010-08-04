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


#include "layout_widget.h"

//#include <kdebug.h>

// for sys tray icon
#include <kstatusnotifieritem.h>
#include <klocalizedstring.h>
#include <kmenu.h>
#include <QtGui/QAction>
#include <QtGui/QMenu>
#include "xkb_rules.h"

#include <QtGui/QPushButton>

#include "x11_helper.h"
#include "keyboard_config.h"
#include "flags.h"



LayoutWidget::LayoutWidget(QWidget* parent, const QList<QVariant>& /*args*/):
	QWidget(parent),
	xEventNotifier(),
	keyboardConfig(new KeyboardConfig()),
	flags(new Flags())
{
	if( ! X11Helper::xkbSupported(NULL) ) {
//		setFailedToLaunch(true, "XKB extension failed to initialize");
		hide();
		return;
	}

	keyboardConfig->load();
	bool show = keyboardConfig->showIndicator
			&& ( keyboardConfig->showSingle || X11Helper::getLayoutsList().size() > 1 );
	if( ! show ) {
		hide();
		return;
	}

	widget = new QPushButton(this);
	widget->setFlat(true);

	layoutChanged();
	init();
}

LayoutWidget::~LayoutWidget()
{
	destroy();
}

void LayoutWidget::init()
{
	connect(widget, SIGNAL(clicked(bool)), this, SLOT(toggleLayout()));
	connect(&xEventNotifier, SIGNAL(layoutChanged()), this, SLOT(layoutChanged()));
	connect(&xEventNotifier, SIGNAL(layoutMapChanged()), this, SLOT(layoutChanged()));
	xEventNotifier.start();
}

void LayoutWidget::destroy()
{
	xEventNotifier.stop();
	disconnect(&xEventNotifier, SIGNAL(layoutMapChanged()), this, SLOT(layoutChanged()));
	disconnect(&xEventNotifier, SIGNAL(layoutChanged()), this, SLOT(layoutChanged()));
}

void LayoutWidget::toggleLayout()
{
	X11Helper::switchToNextLayout();
}

void LayoutWidget::layoutChanged()
{
	LayoutUnit layoutUnit = X11Helper::getCurrentLayout();
	if( layoutUnit.isEmpty() )
		return;

	QIcon icon;
	if( keyboardConfig->showFlag ) {
		icon = flags->getIcon(layoutUnit.layout);
	}

	QString longText = Flags::getLongText(layoutUnit, NULL);
	if( ! icon.isNull() ) {
		widget->setIcon(icon);
		widget->setText("");
		widget->setToolTip(longText);
	}
	else {
		QString shortText = Flags::getShortText(layoutUnit, *keyboardConfig);
		widget->setIcon(icon);
		widget->setText(shortText);
		widget->setToolTip(longText);
//		widget->setShortcut(QKeySequence());
	}
}

//
// Layout Tray Icon
//
LayoutTrayIcon::LayoutTrayIcon():
	xEventNotifier(),
	keyboardConfig(new KeyboardConfig()),
	rules(NULL),
	flags(new Flags()),
	actionGroup(NULL)
{
    m_notifierItem = new KStatusNotifierItem(this);
    m_notifierItem->setCategory(KStatusNotifierItem::SystemServices);
    m_notifierItem->setStatus(KStatusNotifierItem::Active);
    m_notifierItem->setToolTipTitle(i18nc("tooltip title", "Keyboard Layout"));
    m_notifierItem->setTitle(i18nc("tooltip title", "Keyboard Layout"));

	KMenu* menu = new KMenu("");
    m_notifierItem->setContextMenu(menu);
	m_notifierItem->setStandardActionsEnabled(false);

	rules = Rules::readRules();

    layoutMapChanged();

    m_notifierItem->setStatus(KStatusNotifierItem::Active);

    init();
}

LayoutTrayIcon::~LayoutTrayIcon()
{
	destroy();
}

void LayoutTrayIcon::init()
{
    connect(m_notifierItem, SIGNAL(activateRequested(bool, QPoint)), this, SLOT(toggleLayout()));
	connect(&xEventNotifier, SIGNAL(layoutChanged()), this, SLOT(layoutChanged()));
	connect(&xEventNotifier, SIGNAL(layoutMapChanged()), this, SLOT(layoutMapChanged()));
	xEventNotifier.start();
}

void LayoutTrayIcon::destroy()
{
	xEventNotifier.stop();
	disconnect(&xEventNotifier, SIGNAL(layoutMapChanged()), this, SLOT(layoutMapChanged()));
	disconnect(&xEventNotifier, SIGNAL(layoutChanged()), this, SLOT(layoutChanged()));
    disconnect(m_notifierItem, SIGNAL(activateRequested(bool, QPoint)), this, SLOT(toggleLayout()));
}

void LayoutTrayIcon::layoutMapChanged()
{
	keyboardConfig->load();
	flags->clearCache();

	KMenu* menu = m_notifierItem->contextMenu();
	menu->clear();
	QList<QAction*> actions = contextualActions();
	menu->addActions(actions);

	layoutChanged();
}

void LayoutTrayIcon::layoutChanged()
{
	LayoutUnit layoutUnit = X11Helper::getCurrentLayout();
	if( layoutUnit.isEmpty() )
		return;

	QString shortText = Flags::getShortText(layoutUnit, *keyboardConfig);
//	kDebug() << "systray: LayoutChanged" << layoutUnit.toString() << shortText;
	QString longText = Flags::getLongText(layoutUnit, rules);

	m_notifierItem->setToolTipSubTitle(longText);

	const QIcon icon(getFlag(layoutUnit.layout));
	m_notifierItem->setToolTipIconByPixmap(icon);

	QIcon textOrIcon = flags->getIconWithText(layoutUnit, *keyboardConfig);
	m_notifierItem->setIconByPixmap( textOrIcon );
}

void LayoutTrayIcon::toggleLayout()
{
	X11Helper::switchToNextLayout();
}

void LayoutTrayIcon::actionTriggered(QAction* action)
{
	X11Helper::setLayout(LayoutUnit(action->data().toString()));
}

const QIcon LayoutTrayIcon::getFlag(const QString& layout) const
{
	return keyboardConfig->showFlag ? flags->getIcon(layout) : QIcon();
}

QList<QAction*> LayoutTrayIcon::contextualActions()
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
//		QString menuText = i18nc("map name - full layout text", "%1 - %2", shortText, longText);
		QString menuText = longText;
		QAction* action = new QAction(getFlag(layoutUnit.layout), menuText, actionGroup);
		action->setData(layoutUnit.toString());
		actionGroup->addAction(action);
	}
	connect(actionGroup, SIGNAL(triggered(QAction*)), this, SLOT(actionTriggered(QAction*)));
	return actionGroup->actions();
}
