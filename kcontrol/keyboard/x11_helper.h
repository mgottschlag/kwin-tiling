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


#ifndef X11_HELPER_H_
#define X11_HELPER_H_

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtGui/QWidget>

//#include <X11/X.h>
//#include <X11/Xlib.h>

//TODO: can we catch events without QWidget?
class XEventNotifier : public QWidget {
	Q_OBJECT

Q_SIGNALS:
	void newDevice();
	void layoutChanged();
	void layoutMapChanged();

public:
	enum EventType { ALL, XKB, XINPUT };

	XEventNotifier(EventType eventType=ALL, QWidget* parent=NULL);
	void start();
	void stop();

protected:
    bool x11Event(XEvent * e);

private:
	bool isXkbEvent(XEvent* event);
	bool isNewDeviceEvent(XEvent* event);
	int registerForNewDeviceEvent(Display* dpy);
	int registerForXkbEvents(Display* display);
	bool isGroupSwitchEvent(XEvent* event);
	bool isLayoutSwitchEvent(XEvent* event);

	int xkbOpcode;
	int xinputEventType;
    const EventType eventType;
};

struct XkbConfig {
	QString keyboardModel;
	QStringList layouts;
	QStringList variants;
	QStringList options;

	bool isValid() { return ! layouts.empty(); }
};

class X11Helper
{
public:
	static int MAX_GROUP_COUNT;
	static const char* LEFT_VARIANT_STR;
	static const char* RIGHT_VARIANT_STR;

	static bool xkbSupported(int* xkbOpcode);

	static void switchToNextLayout();
	static bool isDefaultLayout();
	static bool setDefaultLayout();
	static bool setLayout(const QString& layout);
	static QString getCurrentLayout();
	static QStringList getLayoutsList();

private:
	enum FetchType { ALL, LAYOUTS_ONLY };
	static bool getGroupNames(Display* dpy, XkbConfig* xkbConfig, FetchType fetchType);
	static unsigned int getGroup();
	static bool setGroup(unsigned int group);
};

#endif /* X11_HELPER_H_ */
