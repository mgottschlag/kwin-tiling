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


#include <QSystemTrayIcon>
#include <QMenu>
#include <QMouseEvent>

#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kaction.h>

#include "pixmap.h"
#include "rules.h"
#include "kxkbconfig.h"

#include "kxkbwidget.h"

#include "kxkbwidget.moc"


KxkbWidget::KxkbWidget(int controlType):
	m_controlType(controlType),
	m_configSeparator(NULL)
{
}

void KxkbWidget::setCurrentLayout(const LayoutUnit& layoutUnit)
{
        QString tip = m_descriptionMap[layoutUnit.toPair()];
        if( tip == NULL || tip.length() == 0 )
            tip = layoutUnit.toPair();
	setToolTip(tip);
	const QPixmap& icon = 
            LayoutIcon::getInstance().findPixmap(layoutUnit.layout, m_showFlag, layoutUnit.getDisplayName());
//	kDebug() << "setting pixmap: " << icon.width();
	setPixmap( icon );
	kDebug() << "setting text: " << layoutUnit.layout;
	setText(layoutUnit.layout);
}

void KxkbWidget::setError(const QString& layoutInfo)
{
    QString msg = i18n("Error changing keyboard layout to '%1'", layoutInfo);
    setToolTip(msg);
    setPixmap(LayoutIcon::getInstance().findPixmap("error", false));
}


void KxkbWidget::initLayoutList(const QList<LayoutUnit>& layouts, const XkbRules& rules)
{
    if( m_controlType <= NO_MENU ) {
	kDebug() << "indicator with no menu requested";
	return;
    }

    QMenu* menu = contextMenu();

    m_descriptionMap.clear();

//    menu->clear();
//    menu->addTitle( qApp->windowIcon(), KGlobal::caption() );
//    menu->setTitle( KGlobal::mainComponent().aboutData()->programName() );

    for(QList<QAction*>::Iterator it=m_actions.begin(); it != m_actions.end(); it++ )
	menu->removeAction(*it);
    m_actions.clear();
	
    int cnt = 0;
    QList<LayoutUnit>::ConstIterator it;
    for (it=layouts.begin(); it != layouts.end(); ++it)
    {
		const QString layoutName = (*it).layout;
		const QString variantName = (*it).variant;

		const QPixmap& layoutPixmap = 
                    LayoutIcon::getInstance().findPixmap(layoutName, m_showFlag, (*it).getDisplayName());
//         const QPixmap pix = iconeffect.apply(layoutPixmap, KIcon::Small, KIcon::DefaultState);

		QString layoutString = rules.layouts()[layoutName];
		QString fullName = layoutString;
		if( variantName.isEmpty() == false )
			fullName += " (" + variantName + ')';
//		menu->insertItem(pix, fullName, START_MENU_ID + cnt, m_menuStartIndex + cnt);

		QAction* action = new QAction(layoutPixmap, fullName, menu);
		action->setData(START_MENU_ID + cnt);
		m_actions.append(action);
		m_descriptionMap.insert((*it).toPair(), fullName);

//	    kDebug() << "added" << (*it).toPair() << "to context menu";

		cnt++;
    }
    menu->insertActions(m_configSeparator, m_actions);

	// if show config, if show help
//	if( menu->indexOf(CONFIG_MENU_ID) == -1 ) {
	if( m_configSeparator == NULL && m_controlType >= MENU_FULL ) { // first call
		m_configSeparator = menu->addSeparator();

		QAction* configAction = new QAction(SmallIcon("configure"), i18n("Configure..."), menu);
		configAction->setData(CONFIG_MENU_ID);
		menu->addAction(configAction);

//		if( menu->indexOf(HELP_MENU_ID) == -1 )
		QAction* helpAction = new QAction(SmallIcon("help-contents"), i18n("Help"), menu);
		helpAction->setData(HELP_MENU_ID);
		menu->addAction(helpAction);
	}
//	else {
//	    kDebug() << "indicator with menu 'layouts only' requested";
//	}

//	menu->update();

/*    if( index != -1 ) { //not first start
		menu->addSeparator();
		KAction* quitAction = KStdAction::quit(this, SIGNAL(quitSelected()), actionCollection());
        if (quitAction)
    	    quitAction->plug(menu);
    }*/
}


// ----------------------------
// QSysTrayIcon implementation

KxkbSysTrayIcon::KxkbSysTrayIcon(int controlType):
    KxkbWidget(controlType)
{
	m_indicatorWidget = new KSystemTrayIcon();

	connect(contextMenu(), SIGNAL(triggered(QAction*)), this, SIGNAL(menuTriggered(QAction*)));
	connect(m_indicatorWidget, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), 
					this, SLOT(trayActivated(QSystemTrayIcon::ActivationReason)));
}

void KxkbSysTrayIcon::trayActivated(QSystemTrayIcon::ActivationReason reason)
{
	if( reason == QSystemTrayIcon::Trigger )
	  emit iconToggled();
}

void KxkbSysTrayIcon::setPixmap(const QPixmap& pixmap)
{
    m_indicatorWidget->setIcon( pixmap );
}

// ----------------------------

KxkbLabel::KxkbLabel(int controlType, QWidget* parent):
		KxkbWidget(controlType),
		m_displayMode(ICON)
{
    m_indicatorWidget = new QToolButton(parent);

    m_indicatorWidget->setAutoRaise(true);
    m_indicatorWidget->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));

    connect( m_indicatorWidget, SIGNAL(clicked(bool)), this, SIGNAL(iconToggled()) );
//    connect( m_indicatorWidget, SIGNAL(toggled(bool)), this, SIGNAL(iconToggled()) );

    m_menu = new QMenu(m_indicatorWidget);
    if( m_controlType >= KxkbWidget::MENU_LAYOUTS_ONLY ) {
        m_indicatorWidget->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(m_indicatorWidget, SIGNAL(customContextMenuRequested(const QPoint &)), 
                    this, SLOT(contextMenuEvent(const QPoint&)));
    }
}

void KxkbLabel::contextMenuEvent(const QPoint& pos)
{
    QMenu* menu = contextMenu();
    menu->exec(pos);
}

void KxkbLabel::setPixmap(const QPixmap& pixmap)
{
    m_indicatorWidget->setIconSize(QSize(24,24));
    m_indicatorWidget->setIcon( pixmap );
}
