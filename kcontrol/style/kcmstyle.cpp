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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qframe.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlistbox.h>
#include <qslider.h>
#include <qstylefactory.h>
#include <qtabwidget.h>
#include <qvbox.h>
#include <qsettings.h>
#include <qstyle.h>
#include <qapplication.h>
#include <qobjectlist.h>
#include <qpixmapcache.h>

#include <dcopclient.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <kipc.h>
#include <kaboutdata.h>
#include <kdialog.h>
#include <kgenericfactory.h>
#include <kmessagebox.h>
#include <kstyle.h>

#include "kcmstyle.h"
#include "stylepreview.h"


// Plugin Interface
// Danimo: Why do we use the old interface?!
extern "C" 
{
	KCModule *create_style(QWidget *parent, const char*) 
	{
		KGlobal::locale()->insertCatalogue("kcmstyle");
		return new KCMStyle(parent, "kcmstyle");
	}
}

/*
typedef KGenericFactory<KWidgetSettingsModule, QWidget> GeneralFactory;
K_EXPORT_COMPONENT_FACTORY( libkcm_kcmstyle, GeneralFactory );
*/


// All this just to make the default QListBox height smaller.
class QStyleListBox: public QListBox
{
	public:
		QStyleListBox( QWidget* parent ) : QListBox( parent) {};

		QSize minimumSizeHint() const 
		{
			QSize size = QListBox::minimumSizeHint();
			size.setHeight( size.height() / 2 );	// Ensure centerCurrentItem() is still visible.
			return size;
		}
		QSize sizeHint() const 
		{
			QSize size = QListBox::sizeHint();
			size.setHeight( size.height() / 2 );
			return size;
		}
};

KCMStyle::KCMStyle( QWidget* parent, const char* name )
	: KCModule( parent, name ), appliedStyle(NULL)
{
	// Setup pages and mainLayout
    mainLayout = new QVBoxLayout( this ); 
    tabWidget  = new QTabWidget( this );
	mainLayout->addWidget( tabWidget );

    page1 = new QWidget( tabWidget );
	page1Layout = new QVBoxLayout( page1, KDialog::marginHint(), KDialog::spacingHint() );
	page2 = new QWidget( tabWidget );
	page2Layout = new QVBoxLayout( page2, KDialog::marginHint(), KDialog::spacingHint() );
	page3 = new QWidget( tabWidget );
	page3Layout = new QVBoxLayout( page3, KDialog::marginHint(), KDialog::spacingHint() );

	// Add Page1 (Style)
	// -----------------
	gbWidgetStyle = new QGroupBox( 1, Qt::Horizontal, i18n("Widget Style"), page1 );
	lbStyle = new QStyleListBox( gbWidgetStyle );
	stylePreview = new StylePreview( page1 );
	page1Layout->addWidget( gbWidgetStyle );
	page1Layout->addWidget( stylePreview );

	// Connect all required stuff
	connect(lbStyle, SIGNAL(highlighted(const QString&)), this, SLOT(updateStyleTimer(const QString&)));
	connect(&switchStyleTimer, SIGNAL(timeout()), this, SLOT(styleChanged()));
	
	// Add Page2 (Effects)
	// -------------------
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

	comboTooltipEffect = new QComboBox( FALSE, containerFrame );
	comboTooltipEffect->insertItem( i18n("Disable") );
	comboTooltipEffect->insertItem( i18n("Animate") );
	comboTooltipEffect->insertItem( i18n("Fade") );
	lblTooltipEffect = new QLabel( i18n("&ToolTip Effect:"), containerFrame );
	lblTooltipEffect->setBuddy( comboTooltipEffect );
	containerLayout->addWidget( lblTooltipEffect, 1, 0 );
	containerLayout->addWidget( comboTooltipEffect, 1, 1 );

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

	// Separator.
	QFrame* hline = new QFrame ( gbEffects );
	hline->setFrameStyle( QFrame::HLine | QFrame::Sunken );

	// Now implement the Menu Transparency container.
	menuContainer = new QFrame( gbEffects );
	menuContainer->setFrameStyle( QFrame::NoFrame | QFrame::Plain );
	menuContainer->setMargin(0);
	menuContainerLayout = new QGridLayout( menuContainer, 1, 1,    // rows, columns
        KDialog::marginHint(), KDialog::spacingHint() );

	menuPreview = new MenuPreview( menuContainer, /* opacity */ 90, MenuPreview::Blend );

	comboMenuEffectType = new QComboBox( FALSE, menuContainer );
	comboMenuEffectType->insertItem( i18n("Software Tint") );
	comboMenuEffectType->insertItem( i18n("Software Blend") );
// Hmm, ###### ADD XRENDER TEST configure.in.in
#ifdef HAVE_XRENDER
	comboMenuEffectType->insertItem( i18n("XRender Blend") );
#endif

	// So much stuffing around for a simple slider..
	sliderBox = new QVBox( menuContainer );
	sliderBox->setSpacing( KDialog::spacingHint() );
	sliderBox->setMargin( 0 );
	slOpacity = new QSlider( 0, 100, 5, /*opacity*/ 90, Qt::Horizontal, sliderBox );
	slOpacity->setTickmarks( QSlider::Below );
	slOpacity->setTickInterval( 10 );
	QHBox* box1 = new QHBox( sliderBox );
	box1->setSpacing( KDialog::spacingHint() );
	box1->setMargin( 0 );
	QLabel* lbl = new QLabel( i18n("0%"), box1 );
	lbl->setAlignment( AlignLeft );
	lbl = new QLabel( i18n("50%"), box1 );
	lbl->setAlignment( AlignHCenter );
	lbl = new QLabel( i18n("100%"), box1 );
	lbl->setAlignment( AlignRight );

	lblMenuEffectType = new QLabel( comboMenuEffectType, i18n("Menu Trans&lucency Type:"), menuContainer );
	lblMenuEffectType->setAlignment( AlignBottom | AlignLeft );
	lblMenuOpacity    = new QLabel( slOpacity, i18n("Menu &Opacity:"), menuContainer );
	lblMenuOpacity->setAlignment( AlignBottom | AlignLeft );
	menuContainerLayout->addWidget( lblMenuEffectType, 0, 0 );
	menuContainerLayout->addWidget( comboMenuEffectType, 1, 0 );
	menuContainerLayout->addWidget( lblMenuOpacity, 2, 0 );
	menuContainerLayout->addWidget( sliderBox, 3, 0 );
	menuContainerLayout->addMultiCellWidget( menuPreview, 0, 3, 1, 1 );

	// Layout page2.
	page2Layout->addWidget( gbEffects );
	QSpacerItem* sp1 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
	page2Layout->addItem( sp1 );

	// Data flow stuff.
	connect( cbEnableEffects,     SIGNAL(toggled(bool)), containerFrame, SLOT(setEnabled(bool)) );
	connect( cbEnableEffects,     SIGNAL(toggled(bool)), this, SLOT(menuEffectChanged(bool)) );
	connect( slOpacity,           SIGNAL(valueChanged(int)),menuPreview, SLOT(setOpacity(int)) );
	connect( comboMenuEffect,     SIGNAL(activated(int)), this, SLOT(menuEffectChanged()) );
	connect( comboMenuEffect,     SIGNAL(highlighted(int)), this, SLOT(menuEffectChanged()) );
	connect( comboMenuEffectType, SIGNAL(activated(int)), this, SLOT(menuEffectTypeChanged()) );
	connect( comboMenuEffectType, SIGNAL(highlighted(int)), this, SLOT(menuEffectTypeChanged()) );

	// Add Page3 (Miscellaneous)
	// -------------------------
	gbToolbarSettings = new QGroupBox( 1, Qt::Horizontal, i18n("Misc Toolbar Settings"), page3 );
	cbHoverButtons = new QCheckBox( i18n("High&light buttons under mouse"), gbToolbarSettings );
	cbTransparentToolbars = new QCheckBox( i18n("Transparent Tool&bars when moving"), gbToolbarSettings );
	cbEnableTooltips = new QCheckBox( i18n("E&nable Tooltips"), gbToolbarSettings );

	QWidget * dummy = new QWidget( gbToolbarSettings );
	QHBoxLayout* box2 = new QHBoxLayout( dummy, 0, KDialog::spacingHint() );
	lbl = new QLabel( i18n("Toolbar &Icons:"), dummy );
	comboToolbarIcons = new QComboBox( FALSE, dummy );
	comboToolbarIcons->insertItem( i18n("Icons only") );
	comboToolbarIcons->insertItem( i18n("Text only") );
	comboToolbarIcons->insertItem( i18n("Text alongside icons") );
	comboToolbarIcons->insertItem( i18n("Text under icons") );
	lbl->setBuddy( comboToolbarIcons );

	box2->addWidget( lbl );
	box2->addWidget( comboToolbarIcons );
	QSpacerItem* sp2 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
	box2->addItem( sp2 );

	gbVisualAppearance = new QGroupBox( 1, Qt::Horizontal, i18n("Visual Appearance"), page3 );
	cbIconsOnButtons = new QCheckBox( i18n("Sho&w Icons on buttons"), gbVisualAppearance );
	cbTearOffHandles = new QCheckBox( i18n("Show tear-off handles in &popup menus"), gbVisualAppearance );

	// Layout page3.
	page3Layout->addWidget( gbToolbarSettings );
	page3Layout->addWidget( gbVisualAppearance );
	QSpacerItem* sp3 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
	page3Layout->addItem( sp3 );

	// Do all the setDirty connections.
	connect(lbStyle, SIGNAL(selectionChanged()), this, SLOT(setDirty()));
	// Page2
	connect( cbEnableEffects,     SIGNAL(toggled(bool)),    this, SLOT(setDirty()));
	connect( comboTooltipEffect,  SIGNAL(highlighted(int)), this, SLOT(setDirty()));
	connect( comboComboEffect,    SIGNAL(highlighted(int)), this, SLOT(setDirty()));
	connect( comboMenuEffect,     SIGNAL(highlighted(int)), this, SLOT(setDirty()));
	connect( comboMenuEffectType, SIGNAL(highlighted(int)), this, SLOT(setDirty()));
	connect( slOpacity,           SIGNAL(valueChanged(int)),this, SLOT(setDirty()));
	// Page3
	connect( cbHoverButtons,        SIGNAL(toggled(bool)),    this, SLOT(setDirty()));
	connect( cbTransparentToolbars, SIGNAL(toggled(bool)),    this, SLOT(setDirty()));
	connect( cbEnableTooltips,      SIGNAL(toggled(bool)),    this, SLOT(setDirty()));
	connect( cbIconsOnButtons,      SIGNAL(toggled(bool)),    this, SLOT(setDirty()));
	connect( cbTearOffHandles,      SIGNAL(toggled(bool)),    this, SLOT(setDirty()));
	connect( comboToolbarIcons,     SIGNAL(highlighted(int)), this, SLOT(setDirty()));

	addWhatsThis();

	// Load settings
	load();

	// Insert the pages into the tabWidget
	tabWidget->insertTab( page1, i18n("&Style"));
	tabWidget->insertTab( page2, i18n("Effe&cts"));
	tabWidget->insertTab( page3, i18n("&Miscellaneous"));
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
	loadEffects(settings);

	// Page3 - Misc.
	loadMisc();
}


void KCMStyle::save()
{
	QSettings* settings = new QSettings();
	settings->writeEntry("/qt/style", currentStyle);

	bool allowMenuTransparency = false;

	// Read the KStyle flags to see if the style writer
	// has enabled menu translucency in the style.
	if (appliedStyle && appliedStyle->inherits("KStyle"))
	{
		KStyle* style = dynamic_cast<KStyle*>(appliedStyle);
		if (style) {
			KStyle::KStyleFlags flags = style->styleFlags();
			if (flags & KStyle::AllowMenuTransparency)
				allowMenuTransparency = true;
		}
	}
	
	// Warn the user if they're applying a style that doesn't support
	// menu translucency and they enabled it.
    if ( (!allowMenuTransparency) &&
		(cbEnableEffects->isChecked()) &&
		(comboMenuEffect->currentItem() == 3) )	// Make Translucent
    {
		KMessageBox::information( this, 
			i18n("This module has detected that the currently selected style (") +
			lbStyle->currentText() + i18n(") does not support Menu Translucency, therefore "
			"this Menu Effect has been disabled.") );
			comboMenuEffect->setCurrentItem(0);    // Disable menu effect.
	}

	// Save effects.
	QStringList effects;
	if (cbEnableEffects->isChecked()) 
	{
		QString engine("Disabled");
		effects << "general";
		switch (comboComboEffect->currentItem()) {
			case 1: effects << "animatecombo"; break;
		}
		switch (comboTooltipEffect->currentItem()) {
			case 1: effects << "animatetooltip"; break;
			case 2: effects << "fadetooltip"; break;
		}
		switch (comboMenuEffect->currentItem()) {
			case 1: effects << "animatemenu"; break;
			case 2: effects << "fademenu"; break;
			case 3: {		// Make Translucent
				switch (comboMenuEffectType->currentItem()) {
					case 0: engine = "SoftwareTint"; break;
					case 1: engine = "SoftwareBlend"; break;
					case 2: engine = "XRender"; break;
				}
				break;
			}
		}
		settings->writeEntry("/KStyle/Settings/MenuTransparencyEngine", engine);
		settings->writeEntry("/KStyle/Settings/MenuOpacity", slOpacity->value()/100.0);
	} else {
		effects << "none";
		// Turn off all KStyle's added effects.
		settings->writeEntry("/KStyle/Settings/MenuTransparencyEngine", "Disabled");
	}
	settings->writeEntry("/qt/GUIEffects", effects);

	// Delete the QSettings before applying the changes. 
	// This works like KConfig::sync(), and not doing this *will* cause
	// style changes to fail. I'm not joking.
	delete settings;

	// KDE specifics (Misc page)
	// -------------------------
	KConfig* config = kapp->config();

	config->setGroup("Toolbar style");
	config->writeEntry( "Highlighting", cbHoverButtons->isChecked(), true, true );
	config->writeEntry( "TransparentMoving", cbTransparentToolbars->isChecked(), true, true );
	QString tbIcon;
	switch( comboToolbarIcons->currentItem() )
	{
		case 1: tbIcon = "TextOnly"; break;
		case 2: tbIcon = "IconTextRight"; break;
		case 3: tbIcon = "IconTextBottom"; break;
		case 0:
		default: tbIcon = "IconOnly"; break;
	}
	config->writeEntry( "IconText", tbIcon, true, true );

	config->setGroup("KDE");
	config->writeEntry( "ShowIconsOnPushButtons", cbIconsOnButtons->isChecked(), true, true );
	config->writeEntry( "EffectNoTooltip", !cbEnableTooltips->isChecked(), true, true );
	config->writeEntry( "InsertTearOffHandle", cbTearOffHandles->isChecked(), true, true );
	config->sync();

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

	// Effects..
	cbEnableEffects->setChecked(false);
	comboTooltipEffect->setCurrentItem(0);
	comboComboEffect->setCurrentItem(0);
	comboMenuEffect->setCurrentItem(0);

	comboMenuEffectType->setCurrentItem(0);
	slOpacity->setValue(90);

	// Miscellanous
	cbHoverButtons->setChecked(true);
	cbTransparentToolbars->setChecked(false);
	cbEnableTooltips->setChecked(true);
	comboToolbarIcons->setCurrentItem(0);
	cbIconsOnButtons->setChecked(true);
	cbTearOffHandles->setChecked(true);
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
			"This module allows you to modify the visual appearance "
			"of user interface elements, such as the widget style "
			"and effects.");
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

	// Sort after the match is found.
	lbStyle->sort();
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

	// Prevent Qt from wrongly caching radio button images
	QPixmapCache::clear(); 

	setStyleRecursive( stylePreview, style );
	delete appliedStyle;
	appliedStyle = style;
}


void KCMStyle::setStyleRecursive(QWidget* w, QStyle* s)
{
	// Don't let broken styles kill the palette
	// for other styles being previewed. (e.g SGI style)
	w->unsetPalette();

	QPalette newPalette(w->palette());
	s->polish( newPalette );
	w->setPalette(newPalette);

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

void KCMStyle::loadEffects(QSettings& settings)
{
	QStringList effects = settings.readListEntry("/qt/GUIEffects");

	cbEnableEffects->setChecked( false );
	for (QStringList::Iterator it = effects.begin(); it != effects.end(); ++it )
	{
		if (*it == "general")
			cbEnableEffects->setChecked(true);
		if (*it == "animatemenu") 
			comboMenuEffect->setCurrentItem(1);
		if (*it == "fademenu") 
			comboMenuEffect->setCurrentItem(2);
		if (*it == "animatecombo") 
			comboComboEffect->setCurrentItem(1);
		if (*it == "animatetooltip") 
			comboTooltipEffect->setCurrentItem(1);
		if (*it == "fadetooltip") 
			comboTooltipEffect->setCurrentItem(2);
	}

	// KStyle Menu transparency options...
	QString effectEngine = settings.readEntry("/KStyle/Settings/MenuTransparencyEngine", "Disabled");
	if (effectEngine != "Disabled")
		comboMenuEffect->setCurrentItem(3);

// #### Add XRENDER check!
#ifdef HAVE_XRENDER
	if (effectEngine == "XRender") 
	{
		comboMenuEffectType->setCurrentItem(2);
		comboMenuEffect->setCurrentItem(3);
	} else if (effectEngine == "SoftwareBlend") 
	{
		comboMenuEffectType->setCurrentItem(1);
		comboMenuEffect->setCurrentItem(3);
#else
	if (effectEngine == "XRender" || effectEngine == "SoftwareBlend") 
	{
		comboMenuEffectType->setCurrentItem(1);	// Software Blend
		comboMenuEffect->setCurrentItem(3);
#endif
	} else if (effectEngine == "SoftwareTint") 
	{
		comboMenuEffectType->setCurrentItem(0);
		comboMenuEffect->setCurrentItem(3);
	} else 
	{
		comboMenuEffectType->setCurrentItem(0);
		comboMenuEffect->setCurrentItem(0);		// Menu Effects disabled
	}

	if (comboMenuEffect->currentItem() != 3)	// If not translucency...
		menuPreview->setPreviewMode( MenuPreview::Tint );
	else if (comboMenuEffectType->currentItem() == 0)
		menuPreview->setPreviewMode( MenuPreview::Tint );
	else
		menuPreview->setPreviewMode( MenuPreview::Blend );

	slOpacity->setValue( (int)(100 * settings.readDoubleEntry("/KStyle/Settings/MenuOpacity", 0.90)) );

	if (cbEnableEffects->isChecked()) {
		containerFrame->setEnabled( true );
		menuContainer->setEnabled( comboMenuEffect->currentItem() == 3 );
	} else {
		menuContainer->setEnabled( false );
		containerFrame->setEnabled( false );
	}
}


void KCMStyle::menuEffectTypeChanged()
{
	MenuPreview::PreviewMode mode;

	if (comboMenuEffect->currentItem() != 3)
		mode = MenuPreview::Tint;
	else if (comboMenuEffectType->currentItem() == 0)
		mode = MenuPreview::Tint;
	else
		mode = MenuPreview::Blend;

	menuPreview->setPreviewMode(mode);
}


void KCMStyle::menuEffectChanged()
{
	menuEffectChanged( cbEnableEffects->isChecked() );
}


void KCMStyle::menuEffectChanged( bool enabled )
{
	if (enabled &&
		comboMenuEffect->currentItem() == 3) {
		menuContainer->setEnabled(true);
	} else
		menuContainer->setEnabled(false);
}


// ----------------------------------------------------------------
// All the Miscellaneous stuff
// ----------------------------------------------------------------

void KCMStyle::loadMisc()
{
	// KDE's Part via KConfig
	KConfig* config = kapp->config();

	config->setGroup("Toolbar style");
	cbHoverButtons->setChecked(config->readBoolEntry("Highlighting", true));
	cbTransparentToolbars->setChecked(config->readBoolEntry("TransparentMoving", false));

	QString tbIcon = config->readEntry("IconText", "IconOnly");
	if (tbIcon == "TextOnly")
		comboToolbarIcons->setCurrentItem(1);
	else if (tbIcon == "IconTextRight")
		comboToolbarIcons->setCurrentItem(2);
	else if (tbIcon == "IconTextBottom")
		comboToolbarIcons->setCurrentItem(3);
	else
		comboToolbarIcons->setCurrentItem(0);

	config->setGroup("KDE");
	cbIconsOnButtons->setChecked(config->readBoolEntry("ShowIconsOnPushButtons", true));
	cbEnableTooltips->setChecked(!config->readBoolEntry("EffectNoTooltip", false));
	cbTearOffHandles->setChecked(config->readBoolEntry("InsertTearOffHandle",true));
}

void KCMStyle::addWhatsThis()
{
	// Page1
/*	QWhatsThis::add( , i18n("") );
	QWhatsThis::add( , i18n("") );

	QWhatsThis::add( , i18n("") );
	QWhatsThis::add( , i18n("") );
	QWhatsThis::add( , i18n("") );
	QWhatsThis::add( , i18n("") );
	QWhatsThis::add( , i18n("") );
	QWhatsThis::add( , i18n("") ); */
}

#include "kcmstyle.moc"

// vim: set noet ts=4:
