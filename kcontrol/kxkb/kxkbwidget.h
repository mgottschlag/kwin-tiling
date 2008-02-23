/*
 *  Copyright (C) 2006 Andriy Rysin (rysin@kde.org)
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


#ifndef KXKBWIDGET_H
#define KXKBWIDGET_H


#include <QList>
#include <QToolButton>

#include <ksystemtrayicon.h>

#include "kxkbconfig.h"


class QMenu;
class XkbRules;
class QAction;
class QPixmap;

/* This class is responsible for displaying flag/label for the layout,
    catching keyboard/mouse events and displaying menu when selected
    This class is abstract and subclasses define how exactly UI will be shown
*/
class KxkbWidget : public QObject
{
    Q_OBJECT
			
public:
    enum { START_MENU_ID = 100, CONFIG_MENU_ID = 130, HELP_MENU_ID = 131 };
    enum { INDICATOR_ONLY=1, NO_MENU = 2, MENU_LAYOUTS_ONLY = 3, MENU_FULL=4 };
    enum { WIDGET_TRAY=0, WIDGET_LABEL=1 };

    void initLayoutList(const QList<LayoutUnit>& layouts, const XkbRules& rule);
    void setCurrentLayout(const LayoutUnit& layout);
    void setError(const QString& layoutInfo="");
    void setShowFlag(bool showFlag) { m_showFlag = showFlag; }
    virtual void setVisible(bool visible) = 0;
	
signals:
    void menuTriggered(QAction*);
    void iconToggled();

protected:
    int m_controlType;

    KxkbWidget(int controlType);
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


/*
    System tray icon to show layouts
*/
class KxkbSysTrayIcon : public KxkbWidget
{
    Q_OBJECT

public:
    KxkbSysTrayIcon(int controlType=MENU_FULL);
    ~KxkbSysTrayIcon() { delete m_indicatorWidget; }

protected:
    QMenu* contextMenu() { return m_indicatorWidget->contextMenu(); }
    void setToolTip(const QString& tip) { m_indicatorWidget->setToolTip(tip); }
    void setPixmap(const QPixmap& pixmap);
    void setText(const QString& /*text*/) { } //m_indicatorWidget->setText(text); }
    void setVisible(bool visible) { m_indicatorWidget->setVisible(visible); }

protected slots:
    void trayActivated(QSystemTrayIcon::ActivationReason);

private:
    KSystemTrayIcon* m_indicatorWidget;
};


/*
    Flexible widget to show layouts
*/
class KxkbLabel : public KxkbWidget
{
    Q_OBJECT

public:
    enum { ICON = 1, TEXT = 2 };

    KxkbLabel(int controlType=MENU_FULL, QWidget* parent=0);
    virtual ~KxkbLabel() { delete m_indicatorWidget; }
    QWidget* widget() { return m_indicatorWidget; }

protected:
    QMenu* contextMenu() { return m_menu; }
    void setToolTip(const QString& tip) { if (m_displayMode==ICON) m_indicatorWidget->setToolTip(tip); }
    void setPixmap(const QPixmap& pixmap);
    void setText(const QString& text) { if (m_displayMode==TEXT) m_indicatorWidget->setText(text); }	
    void setVisible(bool visible) { m_indicatorWidget->setVisible(visible); }
	
protected slots:
    void contextMenuEvent(const QPoint& pos);

private:
    int m_displayMode;
    QToolButton* m_indicatorWidget;
    QMenu* m_menu;
};

#endif
