/*
 * $Id$
 *
 * KCMStyle
 * Copyright (C) 2002 Karol Szwed <gallium@kde.org>
 * Copyright (C) 2002 Daniel Molkentin <molkentin@kde.org>
 *
 * Portions Copyright (C) 2000 TrollTech AS.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qframe.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlistbox.h>
#include <qstylefactory.h>
#include <qtabwidget.h>
#include <qvbox.h>
#include <qsettings.h>
#include <qstyle.h>
#include <qapplication.h>
#include <qobjectlist.h>

#include <dcopclient.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <kipc.h>
#include <kaboutdata.h>
#include <kdialog.h>
#include <kgenericfactory.h>

#include "kcmstyle.h"
#include "stylepreview.h"


// Plugin Interface
extern "C" 
{
	KCModule *create_style(QWidget *parent, const char *name) 
	{
		KGlobal::locale()->insertCatalogue(QString::fromLatin1("kcmstyle"));
		return new KCMStyle(parent, name);
	}
}

/*
typedef KGenericFactory<KWidgetSettingsModule, QWidget> GeneralFactory;
K_EXPORT_COMPONENT_FACTORY( libkcm_kcmstyle, GeneralFactory );
*/


KCMStyle::KCMStyle( QWidget* parent, const char* name )
	: KCModule( parent, name ), appliedStyle(NULL)
{
	// Setup pages and mainLayout
    mainLayout = new QVBoxLayout( this ); 
    tabWidget  = new QTabWidget( this );
	mainLayout->addWidget( tabWidget );

    page1 = new QVBox( tabWidget );
	page1->setSpacing( KDialog::spacingHint() );
	page1->setMargin( KDialog::marginHint() );

	page2 = new QWidget( tabWidget );
	page2Layout = new QVBoxLayout( page2, KDialog::marginHint(), KDialog::spacingHint() );

	page3 = new QWidget( tabWidget );

	// Add Page1 (Style)
	gbWidgetStyle = new QGroupBox( 1, Qt::Horizontal, i18n("Widget Style"), page1 );
	lbStyle = new QListBox( gbWidgetStyle );
	stylePreview = new StylePreview( page1 );
	
	// Add Page2 (Effects)
	gbEffects = new QGroupBox( 1, Qt::Horizontal, i18n("GUI Effects"), page2 );
	cbEnableEffects = new QCheckBox( i18n("&Enable GUI Effects"), gbEffects );

	containerFrame = new QFrame( gbEffects );
	containerFrame->setFrameStyle( QFrame::NoFrame | QFrame::Plain );
	containerFrame->setMargin(0);
	containerLayout = new QGridLayout( containerFrame, 1, 1,	// rows, columns
		KDialog::marginHint(), KDialog::spacingHint() );

	comboComboEffect = new QComboBox( FALSE, containerFrame );
	comboComboEffect->insertItem( i18n("Disable") );
	comboComboEffect->insertItem( i18n("Animate") );
	lblComboEffect = new QLabel( i18n("ComboBo&x Effect:"), containerFrame );
	lblComboEffect->setBuddy( comboComboEffect );
	containerLayout->addWidget( lblComboEffect, 0, 0 );
	containerLayout->addWidget( comboComboEffect, 0, 1 );

	comboToolTipEffect = new QComboBox( FALSE, containerFrame );
	comboToolTipEffect->insertItem( i18n("Disable") );
	comboToolTipEffect->insertItem( i18n("Animate") );
	comboToolTipEffect->insertItem( i18n("Fade") );
	lblToolTipEffect = new QLabel( i18n("&ToolTip Effect:"), containerFrame );
	lblToolTipEffect->setBuddy( comboToolTipEffect );
	containerLayout->addWidget( lblToolTipEffect, 1, 0 );
	containerLayout->addWidget( comboToolTipEffect, 1, 1 );

	comboMenuEffect = new QComboBox( FALSE, containerFrame );
	comboMenuEffect->insertItem( i18n("Disable") );
	comboMenuEffect->insertItem( i18n("Animate") );
	comboMenuEffect->insertItem( i18n("Fade") );
	comboMenuEffect->insertItem( i18n("Make Translucent") );
	lblMenuEffect = new QLabel( i18n("&Menu Effect:"), containerFrame );
	lblMenuEffect->setBuddy( comboMenuEffect );
	containerLayout->addWidget( lblMenuEffect, 2, 0 );
	containerLayout->addWidget( comboMenuEffect, 2, 1 );
	
	// Push the [label combo] to the left.
	comboSpacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
	containerLayout->addItem( comboSpacer, 1, 2 );

	// Finalise page2 layout.
	page2Layout->addWidget( gbEffects );
	QSpacerItem* sp1 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
	page2Layout->addItem( sp1 );

	// Add Page3 (Miscellaneous)

	palette = QApplication::palette();

	// Load settings
	load();

	// Insert the pages into the tabWidget
	tabWidget->insertTab( page1, i18n("&Style"));
	tabWidget->insertTab( page2, i18n("Effe&cts"));
	tabWidget->insertTab( page3, i18n("&Miscellaneous"));

	// Connect all required stuff
	connect(lbStyle, SIGNAL(selectionChanged()), this, SLOT(setDirty()));
	connect(lbStyle, SIGNAL(highlighted(const QString&)), this, SLOT(updateStyleTimer(const QString&)));
	connect(&switchStyleTimer, SIGNAL(timeout()), this, SLOT(styleChanged()));
}

KCMStyle::~KCMStyle()
{
	delete appliedStyle;
}

void KCMStyle::load()
{
	QSettings settings;

	// Page1 - Build up the Style ListBox
	loadStyle(settings);

	// Page2 - Effects
	// Page3 - Misc.
}

void KCMStyle::save()
{
	QSettings* settings = new QSettings();
	qWarning((QString("Changing Style to: ") + currentStyle).latin1());
	settings->writeEntry("/qt/style", currentStyle);

	// Is this equivalent to KConfig's sync()?!!
	// How darn stupid.
	delete settings;

	// Propagate changes to all Qt applications.
	QApplication::x11_apply_settings();

	// Now allow KDE apps to reconfigure themselves.
	// KIPC::sendMessageAll(KIPC::StyleChanged);	// REDUNDANT - use Qt's method.
	KIPC::sendMessageAll(KIPC::ToolbarStyleChanged, 0);
	KIPC::sendMessageAll(KIPC::SettingsChanged);

	// Hmm, I'll soon fix KWin from flickering so000 much.. - Karol.
	// kapp->dcopClient()->send("kwin*", "", "reconfigure()", "");
}

void KCMStyle::defaults()
{
	// Select default style
	QListBoxItem* item;
	if ( (item = lbStyle->findItem("Highcolor")) )
		lbStyle->setCurrentItem(item);
	else if ( (item = lbStyle->findItem("Default")) )
		lbStyle->setCurrentItem(item);
	else if ( (item = lbStyle->findItem("Windows")) )
		lbStyle->setCurrentItem(item);
	else if ( (item = lbStyle->findItem("Platinum")) )
		lbStyle->setCurrentItem(item);
	else if ( (item = lbStyle->findItem("Motif")) )
		lbStyle->setCurrentItem(item);
	else
		lbStyle->setCurrentItem((int)0);	// Use any available style 

	// Other stuff...
}

const KAboutData* KCMStyle::aboutData() const
{
	KAboutData *about = 
		new KAboutData( I18N_NOOP("kcmstyle"), 
						I18N_NOOP("KDE Style Module"),
						0, 0, KAboutData::License_GPL,
						I18N_NOOP("(C) 2002 Karol Szwed, Daniel Molkentin"));

	about->addAuthor("Karol Szwed", 0, "gallium@kde.org");
	about->addAuthor("Daniel Molkentin", 0, "molkentin@kde.org");
	return about;
}

QString KCMStyle::quickHelp() const
{
	return i18n("<h1>Style Settings</h1>"
			"This module allows you to define the behaviour "
			"for some widgets inside of KDE applications. "
			"This includes the toolbar general animation effects.<br>"
			"Note that some settings like the GUI Effects will also "
			"apply to Qt only apps."); // ### REVISE!
}

void KCMStyle::setDirty()
{
	emit changed(true);
}


// ----------------------------------------------------------------
// All the Style Switching / Preview stuff
// ----------------------------------------------------------------

void KCMStyle::loadStyle(QSettings& settings)
{
	// Insert all the styles into the listbox.
	lbStyle->clear();
	QStringList styles = QStyleFactory::keys();
	lbStyle->insertStringList( styles );
	lbStyle->setSelectionMode( QListBox::Single );
	lbStyle->sort();

	// Find out which style is currently being used
	// This uses Qtconfig's method of style matching for compatibility
	QString currentStyle = settings.readEntry("/qt/style");
	if (currentStyle.isNull())
		currentStyle = QApplication::style().className();

	QStringList::iterator it = styles.begin();
	int styleNo = 0;
	while (it != styles.end())
	{
		if (*it == currentStyle)
			break;
		styleNo++;
		it++;
	}

	// Check if we found a match...
	if ( styleNo < lbStyle->count() )
		lbStyle->setCurrentItem(styleNo);
	else {
		// No match was found, so try to find the
		// closest match to the Style's className
		styleNo = 0;
		it = styles.begin();
		while (it != styles.end())
		{
			if (currentStyle.contains(*it))
				break;
			styleNo++;
			it++;
		}
		if (styleNo < lbStyle->count() )
			lbStyle->setCurrentItem(styleNo);
		else {
			lbStyle->insertItem(i18n("Unknown Style"));
			lbStyle->setCurrentItem( lbStyle->count() - 1 );
		}
	}
	currentStyle = lbStyle->currentText();
}

void KCMStyle::updateStyleTimer(const QString& style)
{
	currentStyle = style;
	switchStyleTimer.start(500, TRUE);
}

void KCMStyle::styleChanged()
{
	switchStyle( currentStyle );
}

void KCMStyle::switchStyle(const QString& styleName)
{
	// Create an instance of the new style...
	QStyle* style = QStyleFactory::create(styleName);
	if (!style)
		return;

	setStyleRecursive( stylePreview, style );
	delete appliedStyle;
	appliedStyle = style;
}

void KCMStyle::setStyleRecursive(QWidget* w, QStyle* s)
{
	// Don't let broken styles kill the palette
	// for other styles being previewed. (e.g SGI style)
	w->setPalette(palette);

	// Apply the new style.
	w->setStyle(s);

	// Recursively update all children.
	const QObjectList *children = w->children();
	if (!children)
		return;

	// Apply the style to each child widget.
	QPtrListIterator<QObject> childit(*children);
	QObject *child;
	while ((child = childit.current()) != 0) 
	{
		++childit;
		if (child->isWidgetType())
			setStyleRecursive((QWidget *) child, s);
	}
}


// ----------------------------------------------------------------
// All the Effects stuff
// ----------------------------------------------------------------




#include "kcmstyle.moc"

// vim: set noet ts=4:
