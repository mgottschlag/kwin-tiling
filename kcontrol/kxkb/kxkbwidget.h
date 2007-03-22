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
#ifndef KXKBWIDGET_H
#define KXKBWIDGET_H

#include <QString>
#include <QList>
#include <QPixmap>
#include <QLabel>
#include <QMenu>

#include <ksystemtrayicon.h>

#include "kxkbconfig.h"


class QMenu;
class XkbRules;
class QAction;

/* This class is responsible for displaying flag/label for the layout,
    catching keyboard/mouse events and displaying menu when selected
*/

class KxkbWidget : public QObject
{
 	Q_OBJECT
			
public:
	enum { START_MENU_ID = 100, CONFIG_MENU_ID = 130, HELP_MENU_ID = 131 };

    void initLayoutList(const QList<LayoutUnit>& layouts, const XkbRules& rule);
    void setCurrentLayout(const LayoutUnit& layout);
// 	void setCurrentLayout(const QString& layout, const QString &variant);
    void setError(const QString& layoutInfo="");
    void setShowFlag(bool showFlag) { m_showFlag = showFlag; }
	
signals:
	void menuTriggered(QAction*);
	void iconToggled();

protected:
	KxkbWidget();
	virtual QMenu* contextMenu() = 0;
	virtual void setToolTip(const QString& tip) = 0;
	virtual void setPixmap(const QPixmap& pixmap) = 0;
	virtual void setText(const QString& text) = 0;

private:
	bool m_showFlag;
    QMap<QString, QString> m_descriptionMap;
	QList<QAction*> m_actions;
	QAction* m_configSeparator;
};


class KxkbSysTrayIcon : public KxkbWidget
{
	Q_OBJECT

public:
	KxkbSysTrayIcon();
	~KxkbSysTrayIcon();

protected:
	QMenu* contextMenu() { return m_tray->contextMenu(); }
	void setToolTip(const QString& tip) { m_tray->setToolTip(tip); }
	void setPixmap(const QPixmap& pixmap);
	void setText(const QString& text) { }//m_tray->setText(text); }

protected slots:
	void trayActivated(QSystemTrayIcon::ActivationReason);

private:
    KSystemTrayIcon* m_tray;
};


class MyLineEdit : public QLabel {
	Q_OBJECT

	public:
		MyLineEdit(QWidget* parent): QLabel(parent) {}
	signals:
		void leftClick();
		void rightClick();
protected:
	void mousePressEvent ( QMouseEvent * event );	
};

class KxkbLabel : public KxkbWidget
{
	Q_OBJECT

public:
	KxkbLabel(QWidget* parent=0);
	~KxkbLabel() { } //delete m_tray; }
	void show() { m_tray->show(); }
    virtual void adjustSize() { m_tray->resize( 24,24/*m_pixmap.size()*/ );}

protected:
	QMenu* contextMenu() { return m_menu; }
	void setToolTip(const QString& tip) { m_tray->setToolTip(tip); }
	void setPixmap(const QPixmap& pixmap);
	void setText(const QString& text) { m_tray->setText(text); }
	
	
protected slots:
//	void trayActivated(QSystemTrayIcon::ActivationReason);
	void rightClick();

private:
    MyLineEdit* m_tray;
	QMenu* m_menu;
};

#endif
