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
 	m_menuStartIndex(-1),
	m_prevLayoutCount(0)
{
// 	kDebug() << "Creating KxkbWidget with " << label_ << ", " << contextMenu_ << endl;
// 	kDebug() << "Creating KxkbWidget with startMenuIndex " << m_menuStartIndex << endl;
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

	if( m_menuStartIndex == -1 )
		m_menuStartIndex = menu->count();

//	int index = menu->indexOf(0);

    m_descriptionMap.clear();
//    menu->clear();
//    menu->insertTitle( kapp->miniIcon(), kapp->caption() );

	for(int ii=0; ii<m_prevLayoutCount; ++ii) {
		menu->removeItem(START_MENU_ID + ii);
		kDebug() << "remove item: " << START_MENU_ID + ii << endl;
	}
/*	menu->removeItem(CONFIG_MENU_ID);
	menu->removeItem(HELP_MENU_ID);*/

//    KIconEffect iconeffect;

	int cnt = 0;
    QList<LayoutUnit>::ConstIterator it;
    for (it=layouts.begin(); it != layouts.end(); ++it)
    {
		const QString layoutName = (*it).layout;
		const QString variantName = (*it).variant;

		const QPixmap& layoutPixmap = LayoutIcon::getInstance().findPixmap(layoutName, m_showFlag, (*it).displayName);
//         const QPixmap pix = iconeffect.apply(layoutPixmap, KIcon::Small, KIcon::DefaultState);
		const QPixmap pix = layoutPixmap;

		QString layoutString = rules.layouts()[layoutName];
		QString fullName = i18n( layoutString.toLatin1().constData() );
		if( variantName.isEmpty() == false )
			fullName += " (" + variantName + ')';
		menu->insertItem(pix, fullName, START_MENU_ID + cnt, m_menuStartIndex + cnt);
		m_descriptionMap.insert((*it).toPair(), fullName);

		cnt++;
    }

	m_prevLayoutCount = cnt;

	// if show config, if show help
	if( menu->indexOf(CONFIG_MENU_ID) == -1 ) {
		menu->addSeparator();
		menu->insertItem(SmallIcon("configure"), i18n("Configure..."), CONFIG_MENU_ID);
		if( menu->indexOf(HELP_MENU_ID) == -1 )
			menu->insertItem(SmallIcon("help"), i18n("Help"), HELP_MENU_ID);
	}

/*    if( index != -1 ) { //not first start
		menu->addSeparator();
		KAction* quitAction = KStdAction::quit(this, SIGNAL(quitSelected()), actionCollection());
        if (quitAction)
    	    quitAction->plug(menu);
    }*/
}

// void KxkbWidget::mouseReleaseEvent(QMouseEvent *ev)
// {
//     if (ev->button() == QMouseEvent::LeftButton)
//         emit toggled();
//     KSystemTray::mouseReleaseEvent(ev);
// }

KxkbSysTrayIcon::KxkbSysTrayIcon()
{
	m_tray = new KSystemTrayIcon();

 	connect(contextMenu(), SIGNAL(menuActivated(int)), this, SIGNAL(menuActivated(int)));
	connect(m_tray, SIGNAL(trayActivated(QSystemTrayIcon::ActivationReason)), 
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
//	  emit KxkbWidget::iconToggled();
}

void KxkbSysTrayIcon::setPixmap(const QPixmap& pixmap)
{
	m_tray->setIcon( pixmap );
	if( ! m_tray->isVisible() )
		m_tray->show();
}
