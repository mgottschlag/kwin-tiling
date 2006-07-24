//
// C++ Interface: kxkbtraywindow
//
// Description: 
//
//
// Author: Andriy Rysin <rysin@kde.org>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef KXKBSYSTEMTRAY_H
#define KXKBSYSTEMTRAY_H

#include <ksystemtrayicon.h>

#include <QMouseEvent>
#include <qstring.h>
#include <qlist.h>
#include <qlabel.h>

#include "kxkbconfig.h"

class QSystemTrayIcon;
class QMenu;
class XkbRules;

/* This class is responsible for displaying flag/label for the layout,
    catching keyboard/mouse events and displaying menu when selected
*/

class KxkbLabelController: public QObject
{
// 	Q_OBJECT
			
public:
	enum { START_MENU_ID = 100, CONFIG_MENU_ID = 130, HELP_MENU_ID = 131 };

	KxkbLabelController(QSystemTrayIcon *tray, QMenu* contextMenu);

    void initLayoutList(const QList<LayoutUnit>& layouts, const XkbRules& rule);
    void setCurrentLayout(const LayoutUnit& layout);
// 	void setCurrentLayout(const QString& layout, const QString &variant);
    void setError(const QString& layoutInfo="");
    void setShowFlag(bool showFlag) { m_showFlag = showFlag; }
	
// signals:
// 
// 	void menuActivated(int);
//     void toggled();

// protected:
// 
//     void mouseReleaseEvent(QMouseEvent *);

private:
    QSystemTrayIcon* tray;
	QMenu* contextMenu;
	
	const int m_menuStartIndex;
	bool m_showFlag;
	int m_prevLayoutCount;
    QMap<QString, QString> m_descriptionMap;
	
	void setToolTip(const QString& tip);
	void setPixmap(const QPixmap& pixmap);
};

#endif
