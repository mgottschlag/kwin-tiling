/*
 * $Id$
 *
 * KCMStyle
 * Copyright (C) 2002 Karol Szwed <gallium@kde.org>
 * Copyright (C) 2002 Daniel Molkentin <molkentin@kde.org>
 *
 * Portions Copyright (C) 2000 TrollTech AS.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qdict.h>
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
#include <qwhatsthis.h>

#include <dcopclient.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kdebug.h>
#include <klocale.h>
#include <kipc.h>
#include <kaboutdata.h>
#include <kdialog.h>
#include <kgenericfactory.h>
#include <klistview.h>
#include <kmessagebox.h>
#include <ksimpleconfig.h>
#include <kstyle.h>
#include <kstandarddirs.h>

#include "../krdb/krdb.h"

#include "kcmstyle.h"
#include "stylepreview.h"

#include <X11/Xlib.h>
// X11 namespace cleanup
#undef Below
#undef KeyPress
#undef KeyRelease


/**** DLL Interface for kcontrol ****/

void applyMultiHead(bool active)
{
	// Pass env. var to kdeinit.
	QCString name = "KDE_MULTIHEAD";
	QCString value = active ? "true" : "false";
	QByteArray params;
	QDataStream stream(params, IO_WriteOnly);
	stream << name << value;
	kapp->dcopClient()->send("klauncher", "klauncher", "setLaunchEnv(QCString,QCString)", params);
}

// Plugin Interface
// Danimo: Why do we use the old interface?!
extern "C"
{
	KCModule *create_style(QWidget *parent, const char*)
	{
		KGlobal::locale()->insertCatalogue("kcmstyle");
		return new KCMStyle(parent, "kcmstyle");
	}

    void init_style()
	{
		uint flags = KRdbExportQtSettings | KRdbExportQtColors;
		KConfig config("kcmdisplayrc", true, true);
		config.setGroup("X11");
		bool exportKDEColors = config.readBoolEntry("exportKDEColors", true);
		if (exportKDEColors)
			flags |= KRdbExportColors;
		runRdb( flags );

		bool isActive = !config.readBoolEntry( "disableMultihead", false) && 
			                                      (ScreenCount(qt_xdisplay()) > 1);
		applyMultiHead( isActive );

        config.setGroup("KDE");

        // Write some Qt root property.
#ifndef __osf__      // this crashes under Tru64 randomly -- will fix later
        QByteArray properties;
        QDataStream d(properties, IO_WriteOnly);
        d.setVersion( 3 );      // Qt2 apps need this.
        d << kapp->palette() << KGlobalSettings::generalFont();
        Atom a = XInternAtom(qt_xdisplay(), "_QT_DESKTOP_PROPERTIES", false);

        // do it for all root windows - multihead support
        int screen_count = ScreenCount(qt_xdisplay());
        for (int i = 0; i < screen_count; i++)
            XChangeProperty(qt_xdisplay(),  RootWindow(qt_xdisplay(), i),
                            a, a, 8, PropModeReplace,
                            (unsigned char*) properties.data(), properties.size());
#endif
    }
}

/*
typedef KGenericFactory<KWidgetSettingsModule, QWidget> GeneralFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kcmstyle, GeneralFactory );
*/


// All this just to make the default QListBox height smaller.
class KStyleListView: public KListView
{
	public:
		KStyleListView( QWidget* parent ) : KListView(parent)
		{
		    addColumn(i18n("Name"));
		    addColumn(i18n("Description"));
		    setAllColumnsShowFocus(true);
			setSorting(0, true);	// Sort by Name
			setFullWidth(true);
			setResizeMode(LastColumn);
		};

		QSize minimumSizeHint() const
		{
			QSize size = KListView::minimumSizeHint();
			size.setHeight( size.height() / 2 );
			return size;
		}

		QSize sizeHint() const
		{
			QSize size = KListView::sizeHint();
			size.setHeight( size.height() / 2 );
			return size;
		}
};

KCMStyle::KCMStyle( QWidget* parent, const char* name )
	: KCModule( parent, name ), appliedStyle(NULL)
{
	m_bMacDirty = false;
	m_bEffectsDirty = false;
	m_bStyleDirty= false;
	m_bToolbarsDirty = false;

	KGlobal::dirs()->addResourceType("themes",
		KStandardDirs::kde_default("data") + "kstyle/themes");

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
	lvStyle = new KStyleListView( gbWidgetStyle );
	stylePreview = new StylePreview( page1 );
	page1Layout->addWidget( gbWidgetStyle );
	page1Layout->addWidget( stylePreview );

	// Connect all required stuff
	connect(lvStyle, SIGNAL(currentChanged(QListViewItem*)), this, SLOT(updateStyleTimer(QListViewItem*)));
	connect(&switchStyleTimer, SIGNAL(timeout()), this, SLOT(styleChanged()));

	// Add Page2 (Effects)
	// -------------------
	gbEffects = new QGroupBox( 1, Qt::Horizontal, i18n("GUI Effects"), page2 );
	cbEnableEffects = new QCheckBox( i18n("&Enable GUI effects"), gbEffects );

	containerFrame = new QFrame( gbEffects );
	containerFrame->setFrameStyle( QFrame::NoFrame | QFrame::Plain );
	containerFrame->setMargin(0);
	containerLayout = new QGridLayout( containerFrame, 1, 1,	// rows, columns
		KDialog::marginHint(), KDialog::spacingHint() );

	comboComboEffect = new QComboBox( FALSE, containerFrame );
	comboComboEffect->insertItem( i18n("Disable") );
	comboComboEffect->insertItem( i18n("Animate") );
	lblComboEffect = new QLabel( i18n("ComboBo&x effect:"), containerFrame );
	lblComboEffect->setBuddy( comboComboEffect );
	containerLayout->addWidget( lblComboEffect, 0, 0 );
	containerLayout->addWidget( comboComboEffect, 0, 1 );

	comboTooltipEffect = new QComboBox( FALSE, containerFrame );
	comboTooltipEffect->insertItem( i18n("Disable") );
	comboTooltipEffect->insertItem( i18n("Animate") );
	comboTooltipEffect->insertItem( i18n("Fade") );
	lblTooltipEffect = new QLabel( i18n("&ToolTip effect:"), containerFrame );
	lblTooltipEffect->setBuddy( comboTooltipEffect );
	containerLayout->addWidget( lblTooltipEffect, 1, 0 );
	containerLayout->addWidget( comboTooltipEffect, 1, 1 );

	comboMenuEffect = new QComboBox( FALSE, containerFrame );
	comboMenuEffect->insertItem( i18n("Disable") );
	comboMenuEffect->insertItem( i18n("Animate") );
	comboMenuEffect->insertItem( i18n("Fade") );
	comboMenuEffect->insertItem( i18n("Make Translucent") );
	lblMenuEffect = new QLabel( i18n("&Menu effect:"), containerFrame );
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

	lblMenuEffectType = new QLabel( comboMenuEffectType, i18n("Menu trans&lucency type:"), menuContainer );
	lblMenuEffectType->setAlignment( AlignBottom | AlignLeft );
	lblMenuOpacity    = new QLabel( slOpacity, i18n("Menu &opacity:"), menuContainer );
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
	cbTransparentToolbars = new QCheckBox( i18n("Transparent tool&bars when moving"), gbToolbarSettings );
	cbEnableTooltips = new QCheckBox( i18n("E&nable tooltips"), gbToolbarSettings );

	QWidget * dummy = new QWidget( gbToolbarSettings );
	QHBoxLayout* box2 = new QHBoxLayout( dummy, 0, KDialog::spacingHint() );
	lbl = new QLabel( i18n("Toolbar &icons:"), dummy );
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
	cbIconsOnButtons = new QCheckBox( i18n("Sho&w icons on buttons"), gbVisualAppearance );
	cbTearOffHandles = new QCheckBox( i18n("Show tear-off handles in &popup menus"), gbVisualAppearance );
	cbTearOffHandles->hide(); // reenable when the corresponding Qt method is virtual an properly reimplemented
	cbMacMenubar = new QCheckBox( i18n("&Menubar on top of the screen in the style of MacOS"), gbVisualAppearance );

	// Layout page3.
	page3Layout->addWidget( gbToolbarSettings );
	page3Layout->addWidget( gbVisualAppearance );
	QSpacerItem* sp3 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
	page3Layout->addItem( sp3 );

	// Do all the setDirty connections.
	connect(lvStyle, SIGNAL(selectionChanged()), this, SLOT(setStyleDirty()));
	// Page2
	connect( cbEnableEffects,     SIGNAL(toggled(bool)),    this, SLOT(setEffectsDirty()));
	connect( comboTooltipEffect,  SIGNAL(highlighted(int)), this, SLOT(setEffectsDirty()));
	connect( comboComboEffect,    SIGNAL(highlighted(int)), this, SLOT(setEffectsDirty()));
	connect( comboMenuEffect,     SIGNAL(highlighted(int)), this, SLOT(setStyleDirty()));
	connect( comboMenuEffectType, SIGNAL(highlighted(int)), this, SLOT(setStyleDirty()));
	connect( slOpacity,           SIGNAL(valueChanged(int)),this, SLOT(setEffectsDirty()));
	// Page3
	connect( cbHoverButtons,        SIGNAL(toggled(bool)),    this, SLOT(setToolbarsDirty()));
	connect( cbTransparentToolbars, SIGNAL(toggled(bool)),    this, SLOT(setToolbarsDirty()));
	connect( cbEnableTooltips,      SIGNAL(toggled(bool)),    this, SLOT(setEffectsDirty()));
	connect( cbIconsOnButtons,      SIGNAL(toggled(bool)),    this, SLOT(setEffectsDirty()));
	connect( cbTearOffHandles,      SIGNAL(toggled(bool)),    this, SLOT(setEffectsDirty()));
	connect( comboToolbarIcons,     SIGNAL(highlighted(int)), this, SLOT(setToolbarsDirty()));
	connect( cbMacMenubar,          SIGNAL(toggled(bool)),    this, SLOT(setMacDirty()));

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
	KSimpleConfig config( "kdeglobals", true );
	// Page1 - Build up the Style ListBox
	loadStyle( config );

	// Page2 - Effects
	loadEffects( config );

	// Page3 - Misc.
	loadMisc( config );

	m_bMacDirty = false;
	m_bEffectsDirty = false;
	m_bStyleDirty= false;
	m_bToolbarsDirty = false;
}


void KCMStyle::save()
{
	// Don't do anything if we don't need to.
	if ( !(m_bToolbarsDirty | m_bMacDirty |
		   m_bEffectsDirty  | m_bStyleDirty ) )
		return;

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
			lvStyle->currentItem()->text(2) +
			i18n(") does not support Menu Translucency, therefore "
			"this Menu Effect has been disabled.") );
			comboMenuEffect->setCurrentItem(0);    // Disable menu effect.
	}

	// Save effects.
	KConfig config( "kdeglobals" );
	config.setGroup("KDE");

	config.writeEntry( "EffectsEnabled", cbEnableEffects->isChecked());
	int item = comboComboEffect->currentItem();
	config.writeEntry( "EffectAnimateCombo", item == 1 );
	item = comboTooltipEffect->currentItem();
	config.writeEntry( "EffectAnimateTooltip", item == 1);
	config.writeEntry( "EffectFadeTooltip", item == 2 );
	item = comboMenuEffect->currentItem();
	config.writeEntry( "EffectAnimateMenu", item == 1 );
	config.writeEntry( "EffectFadeMenu", item == 2 );

	// Handle KStyle's menu effects
	QString engine("Disabled");
	if (item == 3 && cbEnableEffects->isChecked())	// Make Translucent
		switch( comboMenuEffectType->currentItem())
		{
			case 1: engine = "SoftwareBlend"; break;
			case 2: engine = "XRender"; break;
			default:
			case 0: engine = "SoftwareTint"; break;
		}

	{	// Braces force a QSettings::sync()
		QSettings settings;	// Only for KStyle stuff
		settings.writeEntry("/KStyle/Settings/MenuTransparencyEngine", engine);
		settings.writeEntry("/KStyle/Settings/MenuOpacity", slOpacity->value()/100.0);
	}

	// Misc page
	config.writeEntry( "ShowIconsOnPushButtons", cbIconsOnButtons->isChecked(), true, true );
	config.writeEntry( "EffectNoTooltip", !cbEnableTooltips->isChecked(), true, true );
	config.writeEntry( "InsertTearOffHandle", cbTearOffHandles->isChecked(), true, true );
	config.writeEntry( "macStyle", cbMacMenubar->isChecked(), true, true );

	config.setGroup("General");
	config.writeEntry( "widgetStyle", currentStyle );

	config.setGroup("Toolbar style");
	config.writeEntry( "Highlighting", cbHoverButtons->isChecked(), true, true );
	config.writeEntry( "TransparentMoving", cbTransparentToolbars->isChecked(), true, true );
	QString tbIcon;
	switch( comboToolbarIcons->currentItem() )
	{
		case 1: tbIcon = "TextOnly"; break;
		case 2: tbIcon = "IconTextRight"; break;
		case 3: tbIcon = "IconTextBottom"; break;
		case 0:
		default: tbIcon = "IconOnly"; break;
	}
	config.writeEntry( "IconText", tbIcon, true, true );
	config.sync();

	// Export the changes we made to qtrc, and update all qt-only 
	// applications on the fly, ensuring that we still follow the user's
	// export fonts/colors settings.
	if (m_bStyleDirty | m_bEffectsDirty)	// Export only if necessary
	{
		uint flags = KRdbExportQtSettings;
		KConfig kconfig("kcmdisplayrc", true, true);
		kconfig.setGroup("X11");
		bool exportKDEColors = kconfig.readBoolEntry("exportKDEColors", true);
		if (exportKDEColors)
			flags |= KRdbExportColors;
		runRdb( flags );
	}

	// Now allow KDE apps to reconfigure themselves.
	if ( m_bStyleDirty )
		KIPC::sendMessageAll(KIPC::StyleChanged);

	if (m_bMacDirty)
		kapp->dcopClient()->send("kdesktop", "KDesktopIface", "configure()", QByteArray());

	if ( m_bToolbarsDirty || m_bMacDirty )
		// ##### FIXME - Doesn't apply all settings correctly due to bugs in
		// KApplication/KToolbar
		KIPC::sendMessageAll(KIPC::ToolbarStyleChanged);

	if (m_bEffectsDirty) {
		KIPC::sendMessageAll(KIPC::SettingsChanged);
		kapp->dcopClient()->send("kwin*", "", "reconfigure()", "");
	}

	// Clean up
	m_bMacDirty      = false;
	m_bEffectsDirty  = false;
	m_bToolbarsDirty = false;
	m_bStyleDirty    = false;
}


void KCMStyle::defaults()
{
	// Select default style
	QListViewItem* item;
	if ( (item = lvStyle->findItem("HighColor", 2, ExactMatch)) )
		lvStyle->setCurrentItem(item);
	else if ( (item = lvStyle->findItem("Default", 2, ExactMatch)) )
		lvStyle->setCurrentItem(item);
	else if ( (item = lvStyle->findItem("Windows", 2, ExactMatch)) )
		lvStyle->setCurrentItem(item);
	else if ( (item = lvStyle->findItem("Platinum", 2, ExactMatch)) )
		lvStyle->setCurrentItem(item);
	else if ( (item = lvStyle->findItem("Motif", 2, ExactMatch)) )
		lvStyle->setCurrentItem(item);
	else
		lvStyle->setCurrentItem(lvStyle->firstChild());	// Use any available style

	// Effects..
	cbEnableEffects->setChecked(false);
	comboTooltipEffect->setCurrentItem(0);
	comboComboEffect->setCurrentItem(0);
	comboMenuEffect->setCurrentItem(0);

	comboMenuEffectType->setCurrentItem(0);
	slOpacity->setValue(90);

	// Miscellanous
	cbHoverButtons->setChecked(true);
	cbTransparentToolbars->setChecked(true);
	cbEnableTooltips->setChecked(true);
	comboToolbarIcons->setCurrentItem(0);
	cbIconsOnButtons->setChecked(false);
	cbTearOffHandles->setChecked(false);
	cbMacMenubar->setChecked(false);
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


void KCMStyle::setMacDirty()
{
	m_bMacDirty = true;
	emit changed(true);
}

void KCMStyle::setEffectsDirty()
{
	m_bEffectsDirty = true;
	emit changed(true);
}

void KCMStyle::setToolbarsDirty()
{
	m_bToolbarsDirty = true;
	emit changed(true);
}

void KCMStyle::setStyleDirty()
{
	m_bStyleDirty = true;
	emit changed(true);
}

// ----------------------------------------------------------------
// All the Style Switching / Preview stuff
// ----------------------------------------------------------------

struct StyleEntry {
	QString name;
	QString desc;
};

void KCMStyle::loadStyle( KSimpleConfig& config )
{
	lvStyle->viewport()->setUpdatesEnabled(false);
	lvStyle->clear();

	// Create a dictionary of WidgetStyle to Name and Desc. mappings.
	QDict<StyleEntry> styleEntries;
	styleEntries.setAutoDelete(true);

	QString strWidgetStyle;
	QStringList list = KGlobal::dirs()->findAllResources("themes", "*.themerc", true, true);
	for (QStringList::iterator it = list.begin(); it != list.end(); it++)
	{
		KSimpleConfig config( *it, true );
		if ( !(config.hasGroup("KDE") && config.hasGroup("Misc")) )
			continue;

		config.setGroup("KDE");

		strWidgetStyle = config.readEntry("WidgetStyle");
		if (strWidgetStyle.isNull())
			continue;

		// We have a widgetstyle, so lets read the i18n entries for it...
		StyleEntry* entry = new StyleEntry;
		config.setGroup("Misc");
		entry->name = config.readEntry("Name");
		entry->desc = config.readEntry("Comment", i18n("No description available."));

		// Insert the entry into our dictionary.
		styleEntries.insert(strWidgetStyle, entry);
	}

	QStringList styles = QStyleFactory::keys();
	StyleEntry* entry;
	for (QStringList::iterator it = styles.begin(); it != styles.end(); it++)
	{
		// Find a match for the key in the dictionary
		if ((entry = styleEntries.find(*it)) != 0) {
			if (entry->name.isNull())
				lvStyle->insertItem( new QListViewItem(lvStyle, *it,
												   entry->desc, *it) );
			else
				lvStyle->insertItem( new QListViewItem(lvStyle, entry->name,
												   entry->desc, *it) );
		} else
			// We didn't find a match, so use the key as the name
			// and a default description.
			lvStyle->insertItem( new QListViewItem(lvStyle, *it,
									i18n("No description available."), *it) );
	}

	// Find out which style is currently being used
	config.setGroup("General");
	QString defaultStyle = (QPixmap::defaultDepth() > 8) ? "HighColor" : "Default";
	QString currentStyle = config.readEntry("widgetStyle", defaultStyle);

	// Find the current style in the list
	QListViewItem* current = lvStyle->findItem( currentStyle, 2, Qt::ExactMatch);
	if (!current)
		current = lvStyle->findItem( currentStyle, 2, Qt::Contains);
	if (!current)
		current = lvStyle->findItem( QApplication::style().className(), 2, Qt::Contains);
	if (!current)
		current = lvStyle->firstChild();	// Last fallback

	// Select the current style
	lvStyle->sort();
	lvStyle->setCurrentItem( current );
	lvStyle->ensureItemVisible( current );
	lvStyle->viewport()->setUpdatesEnabled(true);
	lvStyle->viewport()->repaint(false);

	currentStyle = lvStyle->currentItem()->text(2);
	m_bStyleDirty = false;
}


void KCMStyle::updateStyleTimer( QListViewItem* item )
{
	currentStyle = item->text(2);
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

void KCMStyle::loadEffects( KSimpleConfig& config )
{
	// Load effects.
	config.setGroup("KDE");

	cbEnableEffects->setChecked( config.readBoolEntry( "EffectsEnabled", false) );

	if ( config.readBoolEntry( "EffectAnimateCombo", false) )
		comboComboEffect->setCurrentItem( 1 );
	else
		comboComboEffect->setCurrentItem( 0 );

	if ( config.readBoolEntry( "EffectAnimateTooltip", false) )
		comboTooltipEffect->setCurrentItem( 1 );
	else if ( config.readBoolEntry( "EffectFadeTooltip", false) )
		comboTooltipEffect->setCurrentItem( 2 );
	else
		comboTooltipEffect->setCurrentItem( 0 );

	if ( config.readBoolEntry( "EffectAnimateMenu", false) )
		comboMenuEffect->setCurrentItem( 1 );
	else if ( config.readBoolEntry( "EffectFadeMenu", false) )
		comboMenuEffect->setCurrentItem( 2 );
	else
		comboMenuEffect->setCurrentItem( 0 );

	// KStyle Menu transparency options...
	QSettings settings;
	QString effectEngine = settings.readEntry("/KStyle/Settings/MenuTransparencyEngine", "Disabled");

#ifdef HAVE_XRENDER
	if (effectEngine == "XRender") {
		comboMenuEffectType->setCurrentItem(2);
		comboMenuEffect->setCurrentItem(3);
	} else if (effectEngine == "SoftwareBlend") {
		comboMenuEffectType->setCurrentItem(1);
		comboMenuEffect->setCurrentItem(3);
#else
	if (effectEngine == "XRender" || effectEngine == "SoftwareBlend") {
		comboMenuEffectType->setCurrentItem(1);	// Software Blend
		comboMenuEffect->setCurrentItem(3);
#endif
	} else if (effectEngine == "SoftwareTint") {
		comboMenuEffectType->setCurrentItem(0);
		comboMenuEffect->setCurrentItem(3);
	} else
		comboMenuEffectType->setCurrentItem(0);

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

	m_bEffectsDirty = false;
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

	m_bEffectsDirty = true;
}


void KCMStyle::menuEffectChanged()
{
	menuEffectChanged( cbEnableEffects->isChecked() );
	m_bEffectsDirty = true;
}


void KCMStyle::menuEffectChanged( bool enabled )
{
	if (enabled &&
		comboMenuEffect->currentItem() == 3) {
		menuContainer->setEnabled(true);
	} else
		menuContainer->setEnabled(false);
	m_bEffectsDirty = true;
}


// ----------------------------------------------------------------
// All the Miscellaneous stuff
// ----------------------------------------------------------------

void KCMStyle::loadMisc( KSimpleConfig& config )
{
	// KDE's Part via KConfig
	config.setGroup("Toolbar style");
	cbHoverButtons->setChecked(config.readBoolEntry("Highlighting", true));
	cbTransparentToolbars->setChecked(config.readBoolEntry("TransparentMoving", true));

	QString tbIcon = config.readEntry("IconText", "IconOnly");
	if (tbIcon == "TextOnly")
		comboToolbarIcons->setCurrentItem(1);
	else if (tbIcon == "IconTextRight")
		comboToolbarIcons->setCurrentItem(2);
	else if (tbIcon == "IconTextBottom")
		comboToolbarIcons->setCurrentItem(3);
	else
		comboToolbarIcons->setCurrentItem(0);

	config.setGroup("KDE");
	cbIconsOnButtons->setChecked(config.readBoolEntry("ShowIconsOnPushButtons", false));
	cbEnableTooltips->setChecked(!config.readBoolEntry("EffectNoTooltip", false));
	cbTearOffHandles->setChecked(config.readBoolEntry("InsertTearOffHandle", false));
	cbMacMenubar->setChecked(config.readBoolEntry("macStyle", false));

	m_bMacDirty = false;
	m_bToolbarsDirty = false;
}

void KCMStyle::addWhatsThis()
{
	// Page1
	QWhatsThis::add( lvStyle, i18n("Here you can choose from a list of"
							" predefined widget styles (e.g. the way buttons are drawn) which"
							" may or may not be combined with a theme (additional information"
							" like a marble texture or a gradient).") );
	QWhatsThis::add( stylePreview, i18n("This area shows a preview of the currently selected style "
							"without having to apply it to the whole desktop.") );

	// Page2
	QWhatsThis::add( gbEffects, i18n("This page allows you to enable various widget style effects. "
							"For best performance, it is advisable to disable all effects.") );
	QWhatsThis::add( cbEnableEffects, i18n( "If you check this box, you can select several effects "
							"for different widgets like combo boxes, menus or tooltips.") );
	QWhatsThis::add( comboComboEffect, i18n( "<p><b>Disable: </b>Don't use any combo box effects.</p>\n"
							"<b>Animate: </b>Do some animation.") );
	QWhatsThis::add( comboTooltipEffect, i18n( "<p><b>Disable: </b>Don't use any tooltip effects.</p>\n"
							"<p><b>Animate: </b>Do some animation.</p>\n"
							"<b>Fade: </b>Fade in tooltips using alpha-blending.") );
	QWhatsThis::add( comboMenuEffect, i18n( "<p><b>Disable: </b>Don't use any menu effects.</p>\n"
							"<p><b>Animate: </b>Do some animation.</p>\n"
							"<p><b>Fade: </b>Fade in menus using alpha-blending.</p>\n"
							"<b>Make Translucent: </b>Alpha-blend menus for a see-through effect. (KDE styles only)") );
	QWhatsThis::add( comboMenuEffectType, i18n( "<p><b>Software Tint: </b>Alpha-blend using a flat color.</p>\n"
							"<p><b>Software Blend: </b>Alpha-blend using an image.</p>\n"
							"<b>XRender Blend: </b>Use the XFree RENDER extension for image blending (if available). "
							"This method may be slower than the Software routines on non-accelerated displays, "
							"but may however improve performance on remote displays.</p>\n") );
	QWhatsThis::add( slOpacity, i18n("By adjusting this slider you can control the menu effect opacity.") );

	// Page3
	QWhatsThis::add( gbToolbarSettings, i18n("<b>Note:</b> that all widgets in this combobox "
							"do not apply to Qt-only applications!") );
	QWhatsThis::add( cbHoverButtons, i18n("If this option is selected, toolbar buttons will change "
							"their color when the mouse cursor is moved over them." ) );
	QWhatsThis::add( cbTransparentToolbars, i18n("If you check this box, the toolbars will be "
							"transparent when moving them around.") );
	QWhatsThis::add( cbEnableTooltips, i18n( "If you check this option, the KDE application "
							"will offer tooltips when the cursor remains over items in the toolbar." ) );
	QWhatsThis::add( comboToolbarIcons, i18n( "<p><b>Icons only:</b> Shows only icons on toolbar buttons. "
							"Best option for low resolutions.</p>"
							"<p><b>Text only: </b>Shows only text on toolbar buttons.</p>"
							"<p><b>Text alongside icons: </b> Shows icons and text on toolbar buttons. "
							"Text is aligned alongside the icon.</p>"
							"<b>Text under icons: </b> Shows icons and text on toolbar buttons. "
							"Text is aligned below the icon.") );
	QWhatsThis::add( cbIconsOnButtons, i18n( "If you enable this option, KDE Applications will "
							"show small icons alongside some important buttons.") );
	QWhatsThis::add( cbTearOffHandles, i18n( "If you enable this option some pop-up menus will "
							"show so called tear-off handles. If you click them, you get the menu "
							"inside a widget. This can be very helpful when performing "
							"the same action multiple times.") );
	QWhatsThis::add( cbMacMenubar, i18n("If this option is selected, applications"
							" won't have their menubar attached to their own window anymore."
							" Instead, there is one menu bar at the top of the screen which shows"
							" the menu of the currently active application. You might recognize"
							" this behavior from MacOS.") );
}

#include "kcmstyle.moc"

// vim: set noet ts=4:
