//
// C++ Implementation: kxkbwidget
//
// Description:
//
//
// Author: Andriy Rysin <rysin@kde.org>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//

#include <QSystemTrayIcon>
#include <QMenu>

#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kaction.h>

#include "pixmap.h"
#include "rules.h"
#include "kxkbconfig.h"

#include "kxkbwidget.h"

#include "kxkbwidget.moc"


KxkbWidget::KxkbWidget():
	m_configSeparator(NULL)
{
}

void KxkbWidget::setCurrentLayout(const LayoutUnit& layoutUnit)
{
	setToolTip(m_descriptionMap[layoutUnit.toPair()]);
	setPixmap( LayoutIcon::getInstance().findPixmap(layoutUnit.layout, m_showFlag, layoutUnit.displayName) );
}

void KxkbWidget::setError(const QString& layoutInfo)
{
    QString msg = i18n("Error changing keyboard layout to '%1'", layoutInfo);
	setToolTip(msg);
	setPixmap(LayoutIcon::getInstance().findPixmap("error", m_showFlag));
}


void KxkbWidget::initLayoutList(const QList<LayoutUnit>& layouts, const XkbRules& rules)
{
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

		const QPixmap& layoutPixmap = LayoutIcon::getInstance().findPixmap(layoutName, m_showFlag, (*it).displayName);
//         const QPixmap pix = iconeffect.apply(layoutPixmap, KIcon::Small, KIcon::DefaultState);

		QString layoutString = rules.layouts()[layoutName];
		QString fullName = i18n( layoutString.toLatin1().constData() );
		if( variantName.isEmpty() == false )
			fullName += " (" + variantName + ')';
//		menu->insertItem(pix, fullName, START_MENU_ID + cnt, m_menuStartIndex + cnt);

		QAction* action = new QAction(layoutPixmap, fullName, menu);
		action->setData(START_MENU_ID + cnt);
		m_actions.append(action);
		m_descriptionMap.insert((*it).toPair(), fullName);

		cnt++;
    }
	menu->insertActions(m_configSeparator, m_actions);

	// if show config, if show help
//	if( menu->indexOf(CONFIG_MENU_ID) == -1 ) {
	if( m_configSeparator == NULL ) { // first call
		m_configSeparator = menu->addSeparator();

		QAction* configAction = new QAction(SmallIcon("configure"), i18n("Configure..."), menu);
		configAction->setData(CONFIG_MENU_ID);
		menu->addAction(configAction);

//		if( menu->indexOf(HELP_MENU_ID) == -1 )
		QAction* helpAction = new QAction(SmallIcon("help"), i18n("Help"), menu);
		helpAction->setData(HELP_MENU_ID);
		menu->addAction(helpAction);
	}

/*    if( index != -1 ) { //not first start
		menu->addSeparator();
		KAction* quitAction = KStdAction::quit(this, SIGNAL(quitSelected()), actionCollection());
        if (quitAction)
    	    quitAction->plug(menu);
    }*/
}


KxkbSysTrayIcon::KxkbSysTrayIcon()
{
	m_tray = new KSystemTrayIcon();

 	connect(contextMenu(), SIGNAL(triggered(QAction*)), this, SIGNAL(menuTriggered(QAction*)));
	connect(m_tray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), 
					this, SLOT(trayActivated(QSystemTrayIcon::ActivationReason)));
}

KxkbSysTrayIcon::~KxkbSysTrayIcon()
{
	delete m_tray;
}

void KxkbSysTrayIcon::trayActivated(QSystemTrayIcon::ActivationReason reason)
{
	if( reason == QSystemTrayIcon::Trigger )
	  emit iconToggled();
}

void KxkbSysTrayIcon::setPixmap(const QPixmap& pixmap)
{
	m_tray->setIcon( pixmap );
	if( ! m_tray->isVisible() )
		m_tray->show();
}
