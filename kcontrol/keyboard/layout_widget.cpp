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

#include <kpluginfactory.h>

#include <QtGui/QLabel>

#include "x11_helper.h"
#include "keyboard_config.h"

K_PLUGIN_FACTORY(LayoutWidgetFactory, registerPlugin<LayoutWidget>();)
K_EXPORT_PLUGIN(LayoutWidgetFactory("keyboard_layout_widget"))

LayoutWidget::LayoutWidget(QWidget* parent, const QList<QVariant>& /*args*/):
	QWidget(parent),
	xEventNotifier(XEventNotifier::XKB),
	keyboardConfig(new KeyboardConfig())
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
	QString layout = X11Helper::getCurrentLayout();
	QString layoutText = Flags::getDisplayText(layout, *keyboardConfig);
//	QString longText = Flags::getLongText(layout, rules);
	widget->setText(layoutText);
//	widget->setToolTip(longText);

//	const QPixmap* pixmap = getFlag(layout);
//	if( pixmap != NULL ) {
//		p->drawPixmap(contentsRect, *pixmap);
//	}
}

//QString LayoutWidget::getDisplayText(const QString& layout)
//{
//	if( layout.isEmpty() )
//		return QString("--");
//
//	return layout.split(X11Helper::LEFT_VARIANT_STR)[0];
//}

//const QPixmap* LayoutWidget::getFlag(const QString& layout)
//{
//	const QPixmap* pixmap = NULL;
//	if( drawFlag ) {
//		pixmap = flags.getPixmap(layout);
//	}
//	return pixmap;
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
