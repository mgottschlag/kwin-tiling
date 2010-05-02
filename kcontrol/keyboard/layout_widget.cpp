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
	xEventNotifier(XEventNotifier::XKB),
	keyboardConfig(new KeyboardConfig()),
	flags(new Flags())
{
	if( ! X11Helper::xkbSupported(NULL) ) {
//		setFailedToLaunch(true, "XKB extension failed to initialize");
		hide();
		return;
	}

	widget = new QPushButton(this);
	widget->setFlat(true);

	keyboardConfig->load();

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
	QString fullLayoutText = X11Helper::getCurrentLayout();
	LayoutConfig layoutConfig = LayoutConfig::createLayoutConfig(fullLayoutText);
	QIcon icon;
	if( keyboardConfig->showFlag ) {
		icon = flags->getIcon(layoutConfig.layout);
	}

	QString longText = Flags::getLongText(layoutConfig, NULL);
	if( ! icon.isNull() ) {
		widget->setIcon(icon);
		widget->setText("");
		widget->setToolTip(longText);
	}
	else {
		QString shortText = Flags::getShortText(fullLayoutText, *keyboardConfig);
		widget->setIcon(icon);
		widget->setText(shortText);
		widget->setToolTip(longText);
//		widget->setShortcut(QKeySequence());
	}
}


LayoutTrayIcon::LayoutTrayIcon():
	xEventNotifier(XEventNotifier::XKB),
	keyboardConfig(new KeyboardConfig()),
	rules(NULL),
	flags(new Flags()),
	actionGroup(NULL)
{
    m_notifierItem = new KStatusNotifierItem(this);
    m_notifierItem->setCategory(KStatusNotifierItem::SystemServices);
    m_notifierItem->setStatus(KStatusNotifierItem::Active);
    m_notifierItem->setToolTipTitle(i18nc("tooltip title", "Keyboard Layout"));

    KMenu* menu = new KMenu("");
    m_notifierItem->setContextMenu(menu);
//    m_notifierItem->contextMenu()->addTitle(i18nc("tooltip title", "Keyboard Layout"));
	m_notifierItem->setStandardActionsEnabled(false);

//    connect(m_notifierItem->contextMenu(), SIGNAL(triggered(QAction*)), SIGNAL(menuTriggered(QAction*)));
    connect(m_notifierItem, SIGNAL(activateRequested(bool, QPoint)), SLOT(toggleLayout()));

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
//	connect(widget, SIGNAL(clicked(bool)), this, SLOT(toggleLayout()));
	connect(&xEventNotifier, SIGNAL(layoutChanged()), this, SLOT(layoutChanged()));
	connect(&xEventNotifier, SIGNAL(layoutMapChanged()), this, SLOT(layoutMapChanged()));
	xEventNotifier.start();
}

void LayoutTrayIcon::destroy()
{
	xEventNotifier.stop();
	disconnect(&xEventNotifier, SIGNAL(layoutMapChanged()), this, SLOT(layoutMapChanged()));
	disconnect(&xEventNotifier, SIGNAL(layoutChanged()), this, SLOT(layoutChanged()));
}

void LayoutTrayIcon::layoutMapChanged()
{
	keyboardConfig->load();

	QList<QAction*> actions = contextualActions();
	foreach(QAction* action, actions) {
		m_notifierItem->contextMenu()->addAction(action);
	}

	layoutChanged();
}

void LayoutTrayIcon::layoutChanged()
{
	QString fullLayoutText = X11Helper::getCurrentLayout();
	QString layoutText = Flags::getShortText(fullLayoutText, *keyboardConfig);
	QString longText = Flags::getLongText(fullLayoutText, rules);

	m_notifierItem->setTitle(layoutText);
	m_notifierItem->setToolTipSubTitle(longText);

	const QIcon icon(flags->getIcon(fullLayoutText));
	m_notifierItem->setToolTipIconByPixmap(icon);

	QIcon textOrIcon = flags->getIconWithText(fullLayoutText, *keyboardConfig);
	m_notifierItem->setIconByPixmap( textOrIcon );
}

void LayoutTrayIcon::toggleLayout()
{
	X11Helper::switchToNextLayout();
}

void LayoutTrayIcon::actionTriggered(QAction* action)
{
	X11Helper::setLayout(action->data().toString());
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

	QStringList layouts = X11Helper::getLayoutsList();
	foreach(const QString& layout, layouts) {
		QString menuText = Flags::getLongText(layout, rules);
		QAction* action = new QAction(getFlag(layout), menuText, actionGroup);
		action->setData(layout);
		actionGroup->addAction(action);
	}
	connect(actionGroup, SIGNAL(triggered(QAction*)), this, SLOT(actionTriggered(QAction*)));
	return actionGroup->actions();
}
