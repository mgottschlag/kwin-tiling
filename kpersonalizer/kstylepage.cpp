/***************************************************************************
                          kstylepage.cpp  -  description
                             -------------------
    begin                : Tue May 22 2001
    copyright            : (C) 2001 by Ralf Nolden
    email                : nolden@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <stdlib.h>

#include <qdir.h>
#include <qlabel.h>
#include <q3listview.h>
#include <qcolor.h>
#include <qstylefactory.h>
#include <qpixmapcache.h>
#include <qstyle.h>
#include <qobject.h>
//Added by qt3to4:
#include <QPixmap>
#include <QX11Info>

#include <kconfig.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kdesktopfile.h>
#include <klocale.h>
#include <k3listview.h>
#include <kipc.h>
#include <ksimpleconfig.h>
#include <dcopclient.h>
#include <kstyle.h>
#include <kicontheme.h>
#include <kiconloader.h>

#include "stylepreview.h"
#include "kstylepage.h"

KStylePage::KStylePage(QWidget *parent, const char *name ) : KStylePageDlg(parent,name) {

	px_stylesSidebar->setPixmap(UserIcon("step4.png"));

	klv_styles->addColumn(i18n("Style"));
	klv_styles->addColumn( i18n( "Description" ) );
	klv_styles->setAllColumnsShowFocus(true);

	kde = new Q3ListViewItem( klv_styles);
	kde->setText( 0, QPixmap::defaultDepth() > 8 ? i18n( "Plastik" ) : i18n( "Light" ) );
	kde->setText( 1, i18n( "KDE default style" ) );

	classic = new Q3ListViewItem( klv_styles);
	classic->setText( 0, i18n( "KDE Classic" ) );
	classic->setText( 1, i18n( "Classic KDE style" ) );

	keramik = new Q3ListViewItem( klv_styles );
	keramik->setText( 0, i18n( "Keramik" ) );
	keramik->setText( 1, i18n( "The previous default style" ) );

	cde = new Q3ListViewItem( klv_styles);
	cde->setText( 0, i18n( "Sunshine" ) );
	cde->setText( 1, i18n( "A very common desktop" ) );

	win = new Q3ListViewItem( klv_styles );
	win->setText( 0, i18n( "Redmond" ) );
	win->setText( 1, i18n( "A style from the northwest of the USA" ) );

	platinum = new Q3ListViewItem( klv_styles );
	platinum->setText( 0, i18n( "Platinum" ) );
	platinum->setText( 1, i18n( "The platinum style" ) );

	connect(klv_styles, SIGNAL(selectionChanged()),
			this, SLOT(slotCurrentChanged()));

	// Note: if the default is changed here it needs to be changed in kdebase/kwin/plugins.cpp
	//       and kdebase/kwin/kcmkwin/kwindecoration/kwindecoration.cpp as well.
	defaultKWinStyle = QPixmap::defaultDepth() > 8 ? "kwin_plastik" : "kwin_quartz";
	appliedStyle = NULL;

	getAvailability();
	getUserDefaults();
	initColors();
}

KStylePage::~KStylePage(){
	delete ckwin;
	delete appliedStyle;
}

void KStylePage::save(bool curSettings){
	kDebug() << "KStylePage::save()" << endl;
	// First, the style, then the colors as styles overwrite color settings
	saveStyle(curSettings);
	saveColors(curSettings);
	saveKWin(curSettings);
	saveIcons(curSettings);
	liveUpdate();
}

/** save the widget-style */
void KStylePage::saveStyle(bool curSettings){
	QString style = curSettings ? currentStyle : origStyle;
	KConfig cfg( "kdeglobals" );
	cfg.setGroup("General");
	cfg.writeEntry( "widgetStyle", style, KConfigBase::Persistent|KConfigBase::Global );
	cfg.sync();
	kDebug() << "KStylePage::saveStyle(): " << style << endl;
}

/** save the KWin-style*/
void KStylePage::saveKWin(bool curSettings){
	QString kwin = origKWinStyle;
	if(curSettings) {
		KDesktopFile* kdf = 0L;
		KStandardDirs* kstd = KGlobal::dirs();
		if (cde->isSelected() && kwin_cde_exist)
			kdf = new KDesktopFile(kstd->findResource("data", "kwin/cde.desktop"));
		else if (win->isSelected() && kwin_win_exist)
			kdf = new KDesktopFile(kstd->findResource("data", "kwin/redmond.desktop"));
		else if (platinum->isSelected() && kwin_system_exist)
			kdf = new KDesktopFile(kstd->findResource("data", "kwin/system.desktop"));
		else if (keramik->isSelected() && kwin_keramik_exist)
			kdf = new KDesktopFile(kstd->findResource("data", "kwin/keramik.desktop"));
		else if (kde->isSelected()) {
			if (kwin_plastik_exist && (QX11Info::appDepth() > 8))
				kdf = new KDesktopFile(kstd->findResource("data", "kwin/plastik.desktop"));
			else if (kwin_quartz_exist)
				kdf = new KDesktopFile(kstd->findResource("data", "kwin/quartz.desktop"));
		}

		if (kdf) {
			kdf->setGroup("Desktop Entry");
			kwin = kdf->readEntry("X-KDE-Library", defaultKWinStyle);
			delete kdf;
		}
		else {
			// if we get here classic is selected (there's no .desktop file for the
			// kde2 kwin style), or none of the other kwin styles were found.
			kwin = "kwin_default";
		}
	}
	ckwin->writeEntry("PluginLib", kwin);
	ckwin->sync();
	kDebug() << "KStylePage::saveKWin(): " << kwin << endl;
}

/** Save the color-scheme */
void KStylePage::saveColors(bool curSettings){
	struct colorSet* toSave;
	if(curSettings)
		toSave=&currentColors;  // set the color struct to save as the style colors
	else
		toSave=&usrColors;

	// the GLOBAL config entries must be taken from the kcsrc file and written to it. Use the default values
	// equals that the file is <default> which is no file. TODO: use the default values in that case (kde selected)
	KConfig *config = KGlobal::config();
	config->setGroup( "General" );
	config->writeEntry("foreground", toSave->foreground, KConfigBase::Persistent|KConfigBase::Global);
	config->writeEntry("background", toSave->background, KConfigBase::Persistent|KConfigBase::Global);
	config->writeEntry("windowForeground", toSave->windowForeground, KConfigBase::Persistent|KConfigBase::Global);
	config->writeEntry("windowBackground", toSave->windowBackground, KConfigBase::Persistent|KConfigBase::Global);
	config->writeEntry("selectForeground", toSave->selectForeground, KConfigBase::Persistent|KConfigBase::Global);
	config->writeEntry("selectBackground", toSave->selectBackground, KConfigBase::Persistent|KConfigBase::Global);
	config->writeEntry("buttonForeground", toSave->buttonForeground, KConfigBase::Persistent|KConfigBase::Global);
	config->writeEntry("buttonBackground", toSave->buttonBackground, KConfigBase::Persistent|KConfigBase::Global);
	config->writeEntry("linkColor", toSave->linkColor, KConfigBase::Persistent|KConfigBase::Global);
	config->writeEntry("visitedLinkColor", toSave->visitedLinkColor, KConfigBase::Persistent|KConfigBase::Global);

	// set to the WM group, *only* the KGlobal one, a kcsrc file only has the group "Color Scheme"  hmpf...
	config->setGroup( "WM" );
	config->writeEntry("activeForeground", toSave->activeForeground, KConfigBase::Persistent|KConfigBase::Global);
	config->writeEntry("inactiveForeground", toSave->inactiveForeground, KConfigBase::Persistent|KConfigBase::Global);
	config->writeEntry("activeBackground", toSave->activeBackground, KConfigBase::Persistent|KConfigBase::Global);
	config->writeEntry("inactiveBackground", toSave->inactiveBackground, KConfigBase::Persistent|KConfigBase::Global);
	config->writeEntry("activeBlend", toSave->activeBlend, KConfigBase::Persistent|KConfigBase::Global);
	config->writeEntry("inactiveBlend", toSave->inactiveBlend, KConfigBase::Persistent|KConfigBase::Global);
	config->writeEntry("activeTitleBtnBg", toSave->activeTitleBtnBg, KConfigBase::Persistent|KConfigBase::Global);
	config->writeEntry("inactiveTitleBtnBg", toSave->inactiveTitleBtnBg, KConfigBase::Persistent|KConfigBase::Global);
        config->writeEntry("alternateBackground", toSave->alternateBackground,KConfigBase::Persistent|KConfigBase::Global);
		
	////////////////////////////////////////////////////
	// KDE-1.x support
	KSimpleConfig *kconfig =
	new KSimpleConfig( QDir::homePath() + "/.kderc" );
	kconfig->setGroup( "General" );
	kconfig->writeEntry("background", toSave->background );
	kconfig->writeEntry("selectBackground", toSave->selectBackground );
	kconfig->writeEntry("foreground", toSave->foreground );
	kconfig->writeEntry("windowForeground", toSave->windowForeground );
	kconfig->writeEntry("windowBackground", toSave->windowBackground );
	kconfig->writeEntry("selectForeground", toSave->selectForeground );
	kconfig->sync();
	delete kconfig;

	config->setGroup("KDE");
	// write the color scheme filename and the contrast, default 7, otherwise from file
	config->writeEntry("colorScheme", toSave->colorFile,KConfigBase::Persistent|KConfigBase::Global);
	config->writeEntry("contrast", toSave->contrast, KConfigBase::Persistent|KConfigBase::Global);
		
	config->sync();

	// background color changes
	KConfig kdesktop("kdesktoprc");
	kdesktop.setGroup("Desktop0"); // we only need to set one desktop

	kdesktop.writeEntry("BackgroundMode", toSave->bgMode);
	kdesktop.writeEntry("Color1", toSave->usrCol1);
	kdesktop.writeEntry("Color2", toSave->usrCol2);
	kdesktop.sync();
	kDebug() << "KStylePage::saveColors(): colorFile: " << toSave->colorFile << endl;
}

/** save the icon-theme*/
void KStylePage::saveIcons(bool curSettings) {
	QString theme = origIcons;
	if (curSettings) {
		if ( (kde->isSelected() || platinum->isSelected() || keramik->isSelected())
			&& icon_crystalsvg_exist)
				theme = "crystalsvg";
		else if ( (classic->isSelected() || cde->isSelected() || win->isSelected())
			&& (QX11Info::appDepth() > 8) && icon_kdeclassic_exist)
				theme = "kdeclassic";
		else if ( (classic->isSelected() || cde->isSelected() || win->isSelected())
			&& (QX11Info::appDepth() <= 8) && icon_Locolor_exist)
				theme = "Locolor";
	}
	// save, what we got
	KGlobal::config()->setGroup("Icons");
	KGlobal::config()->writeEntry("Theme", theme, KConfigBase::Persistent|KConfigBase::Global);
	KIconTheme icontheme(theme);
	const char * const groups[] = { "Desktop", "Toolbar", "MainToolbar", "Small", 0L };
	for (K3Icon::Group i=K3Icon::FirstGroup; i<K3Icon::LastGroup; i++) {
		if (groups[i] == 0L)
			break;
		KGlobal::config()->setGroup(QLatin1String(groups[i]) + "Icons");
		KGlobal::config()->writeEntry("Size", icontheme.defaultSize(i));
	}
	KGlobal::config()->sync();
	kDebug() << "KStylePage::saveIcons(): " << theme << endl;
}

/** called whenever the selection in the listview changes */
void KStylePage::slotCurrentChanged() {
	// fill the currentColors-struct with the selected color-scheme
	getColors(&currentColors, true);
	// fill the currentStyle-string with the selected style-name
	changeCurrentStyle();
	switchPrevStyle();
}

void KStylePage::changeCurrentStyle() {
	// set the style
	if (cde->isSelected() && cde_exist) {
		currentStyle="Motif";
	}
	else if (win->isSelected() && win_exist) {
		currentStyle="Windows";
	}
	else if (platinum->isSelected() && platinum_exist) {
		currentStyle="Platinum";
	}
	else if (keramik->isSelected() && kde_plastik_exist) {
		currentStyle="Keramik";
	}
	else if (classic->isSelected()) {
		// Use the highcolor style if the display supports it
		if ( (QX11Info::appDepth() > 8) && kde_hc_exist ) {
			currentStyle="HighColor";
		}
		else if (kde_def_exist) {
			currentStyle="Default";
		}
	}
	else if (kde->isSelected()) {
		// Use the plastik style if the display supports it
		if ( (QX11Info::appDepth() > 8) && kde_plastik_exist ) {
			currentStyle="Plastik";
		}
		else if (kde_light_exist) {
			currentStyle="Light, 3rd revision";
		}
		// We should never get here
		else if (kde_def_exist) {
			currentStyle="Default";
		}
	}
	// update the preview-widget
	kDebug() << "KStylePage::changeCurrentStyle(): "<< currentStyle << endl;
}

/** to be connected to the OS page. Catches
 *  either KDE, CDE, win or mac and pre-sets the style.
 */
void KStylePage::presetStyle(const QString& style){
	kDebug() << "KStylePage::presetStyle(): "<< style << endl;
	if(style=="KDE") {
		if (kde_plastik_exist)
			klv_styles->setSelected(kde,true);
		else if (kde_hc_exist || kde_def_exist)
			klv_styles->setSelected(classic,true);
	}
	else if(style=="CDE" && cde_exist)
		klv_styles->setSelected(cde,true);
	else if(style=="win" && win_exist)
		klv_styles->setSelected(win,true);
	else if(style=="mac") {
		klv_styles->setSelected(platinum,true);
	}
}

/** set the defaults for this page */
void KStylePage::setDefaults(){
}

/** Fill a colorSet with a colorfile, or the default. */
void KStylePage::getColors(colorSet *set, bool colorfile ){
	KConfig* config;
	bool deleteConfig = false;
	// get the color scheme file and go to the color scheme group
	if(colorfile){
		KGlobal::dirs()->addResourceType("colors", KStandardDirs::kde_default("data")+"kdisplay/color-schemes");
		// set the style
		if (kde->isSelected()) {
			set->bgMode="Flat";
                        set->usrCol1.setNamedColor("#003082");
			set->usrCol2.setNamedColor("#6C8BB9");
			set->colorFile="<default>";
		}
		else if(classic->isSelected()){
			set->bgMode="VerticalGradient";
			set->usrCol1.setNamedColor ("#1E72A0");
			set->usrCol2.setNamedColor ("#C0C0C0");
			set->colorFile=locate("colors", "KDETwo.kcsrc");
		}
		else if(keramik->isSelected()){
			set->bgMode="VerticalGradient";
			set->usrCol1.setNamedColor ("#1E72A0");
			set->usrCol2.setNamedColor ("#C0C0C0");
			set->colorFile=locate("colors","Keramik.kcsrc");
		}
		else if(cde->isSelected()){
			set->bgMode="Flat";
			set->usrCol1.setNamedColor("#718BA5");
			set->usrCol2.setNamedColor ("#C0C0C0");
			set->colorFile=locate("colors","SolarisCDE.kcsrc");
		}
		else if(win->isSelected()){
			set->bgMode="Flat";
			set->usrCol1.setNamedColor("#008183");
			set->usrCol2.setNamedColor ("#C0C0C0");
			set->colorFile=locate("colors","Windows2000.kcsrc");
		}
		else if(platinum->isSelected()){
			set->bgMode="VerticalGradient";
			set->usrCol1.setNamedColor("#2A569D");
			set->usrCol2.setNamedColor("#6C8BB9");
			set->colorFile=locate("colors","EveX.kcsrc");
		}
		set->contrast=7;
		config = new KSimpleConfig(set->colorFile, true);
		config->setGroup("Color Scheme");
		kDebug() << "KStylePage::getColors(): schemefile: " << set->colorFile << endl;
                deleteConfig = true;
	}
	else {
		KConfig kdesktop("kdesktoprc");
		kdesktop.setGroup("Desktop0");
		// set Background (userSettings if available, else default)
		set->bgMode=kdesktop.readEntry("BackgroundMode", "Flat");
                QColor tmp1("#003082");
		QColor tmp2("#C0C0C0");
		set->usrCol1=kdesktop.readEntry("Color1", tmp1);
		set->usrCol2=kdesktop.readEntry("Color2", tmp2);
		// write the color scheme filename and the contrast, default 7, otherwise from file
		config=KGlobal::config();
		config->setGroup("KDE");
		set->colorFile=config->readEntry("colorScheme", "<default>");
		set->contrast=config->readEntry("contrast", 7);
		config->setGroup( "General" );
		kDebug() << "KStylePage::getColors(): schemefile: "<< set->colorFile << endl;
    }
	QColor black( Qt::black ), white( Qt::white );
	set->foreground=config->readEntry( "foreground", black );
	set->background=config->readEntry( "background", widget );
	set->windowForeground=config->readEntry( "windowForeground", black );
	set->windowBackground=config->readEntry( "windowBackground", white );
	set->selectForeground=config->readEntry( "selectForeground", white );
	set->selectBackground=config->readEntry( "selectBackground", kde34Blue );
	set->buttonForeground=config->readEntry( "buttonForeground", black );
	set->buttonBackground=config->readEntry( "buttonBackground", button );
	set->linkColor=config->readEntry( "linkColor", link );
	set->visitedLinkColor=config->readEntry( "visitedLinkColor", visitedLink );
	// it's necessary to set the group, when reading from globalrc
	if(!colorfile)
		config->setGroup( "WM" );
	set->activeForeground=config->readEntry("activeForeground", white);
        set->inactiveForeground=config->readEntry("inactiveForeground", inactiveForeground);
        set->activeBackground=config->readEntry("activeBackground", activeBackground);
        set->inactiveBackground=config->readEntry("inactiveBackground", inactiveBackground);
        set->activeBlend=config->readEntry("activeBlend", activeBlend);
        set->inactiveBlend=config->readEntry("inactiveBlend", inactiveBackground);
        set->activeTitleBtnBg=config->readEntry("activeTitleBtnBg", activeTitleBtnBg);
        set->inactiveTitleBtnBg=config->readEntry("inactiveTitleBtnBg", inactiveTitleBtnBg);
        set->alternateBackground=config->readEntry("alternateBackground", alternateBackground);
        if ( deleteConfig)
            delete config;
}

/** Test widget- and kwin- styles for availability */
void KStylePage::getAvailability() {
	// test, wich styles are available
	kde_keramik_exist = kde_hc_exist = kde_def_exist = cde_exist
		= kde_plastik_exist = win_exist = platinum_exist 
		= kde_light_exist = false;
	QStringList styles = QStyleFactory::keys();
	for (QStringList::iterator it = styles.begin(); it != styles.end(); it++) {
		if (*it == "Keramik") kde_keramik_exist = true;
		else if (*it == "HighColor") kde_hc_exist = true;
		else if (*it == "Default") kde_def_exist = true;
		else if (*it == "Plastik") kde_plastik_exist = true;
		else if (*it == "Motif") cde_exist = true;
		else if (*it == "Windows") win_exist = true;
		else if (*it == "Platinum") platinum_exist = true;
		else if (*it == "Light, 3rd revision") kde_light_exist = true;
	}
	// and disable the ListItems, if they are not.
	if ( !(kde_plastik_exist || kde_light_exist) ) kde->setVisible(false);
	if ( !(kde_hc_exist || kde_def_exist) ) classic->setVisible(false);
	if (!kde_keramik_exist || QPixmap::defaultDepth() <= 8) keramik->setVisible(false);
	if (!cde_exist) cde->setVisible(false);
	if (!win_exist) win->setVisible(false);
	if (!platinum_exist) platinum->setVisible(false);

	// test, wich KWin-styles are available
	kwin_keramik_exist = kwin_system_exist = kwin_plastik_exist
			= kwin_default_exist = kwin_win_exist
			= kwin_cde_exist = kwin_quartz_exist = false;
	KStandardDirs* kstd = KGlobal::dirs();
	if (!kstd->findResource("data", "kwin/keramik.desktop").isNull())
		kwin_keramik_exist = true;
	if (!kstd->findResource("data", "kwin/plastik.desktop").isNull())
		kwin_plastik_exist = true;
	if (!kstd->findResource("data", "kwin/system.desktop").isNull())
		kwin_system_exist = true;
	if (!kstd->findResource("data", "kwin/redmond.desktop").isNull())
		kwin_win_exist = true;
	if (!kstd->findResource("data", "kwin/cde.desktop").isNull())
		kwin_cde_exist = true;
	if (!kstd->findResource("data", "kwin/quartz.desktop").isNull())
		kwin_quartz_exist = true;
	kwin_default_exist = true;	// we can't check for a .desktop-file for the old default because there is none

	// check, wich Icon-themes are available
	icon_crystalsvg_exist = icon_kdeclassic_exist = icon_Locolor_exist = false;
	QStringList icons(KIconTheme::list());
	for (QStringList::iterator it=icons.begin(); it != icons.end(); it++) {
		KIconTheme icontheme(*it);
		if (icontheme.isHidden() || !icontheme.isValid()) continue;
		if (*it == "crystalsvg") icon_crystalsvg_exist = true;
		else if (*it == "kdeclassic") icon_kdeclassic_exist = true;
		else if (*it == "Locolor") icon_Locolor_exist = true;
	}
}

/** get the user's former settings */
void KStylePage::getUserDefaults() {
	// Get the user's current widget-style
	KGlobal::config()->setGroup("General");
	#warning WHERE O WHERE has KStyle::defaultStyle() gone?
	origStyle = KGlobal::config()->readEntry( "widgetStyle", "default" );//KStyle::defaultStyle() );

	// get the user's current KWin-style
	ckwin = new KConfig("kwinrc");
	ckwin->setGroup("Style");
	origKWinStyle = ckwin->readEntry("PluginLib", defaultKWinStyle);

	// get the users current colors
	getColors(&usrColors, false);

	// Get the user's current iconset
	KGlobal::config()->setGroup("Icons");
	origIcons = KGlobal::config()->readEntry("Theme");

	kDebug() << "KStylePage::getUserDefaults(): style: " << origStyle << endl;
	kDebug() << "KStylePage::getUserDefaults(): KWinStyle: " << origKWinStyle << endl;
	kDebug() << "KStylePage::getUserDefaults(): Colors: " << usrColors.colorFile << endl;
	kDebug() << "KStylePage::getUserDefaults(): Icons: " << origIcons << endl;
}

/** initialize KDE default color values */
void KStylePage::initColors() {
	widget.setRgb(239, 239, 239);
	kde34Blue.setRgb(103,141,178);
        inactiveBackground.setRgb(157,170,186);
        activeBackground.setRgb(65,142,220);
        inactiveForeground.setRgb(221,221,221);
        activeBlend.setRgb(107,145,184);
        activeTitleBtnBg.setRgb(127,158,200);
        inactiveTitleBtnBg.setRgb(167,181,199);
        alternateBackground.setRgb(237,244,249);
        
	if (QPixmap::defaultDepth() > 8)
		button.setRgb(221, 223, 228);
	else
		button.setRgb(220, 220, 220);

	link.setRgb(0, 0, 238);
	visitedLink.setRgb(82, 24, 139);
}

/** live-update the system */
void KStylePage::liveUpdate() {
	// tell all apps about the changed icons
	for (int i=0; i<K3Icon::LastGroup; i++) {
		KIPC::sendMessageAll(KIPC::IconChanged, i);
	}
	// tell all apps about the changed style
	KIPC::sendMessageAll(KIPC::StyleChanged);
	// color palette changes
	KIPC::sendMessageAll(KIPC::PaletteChanged);
	// kwin-style
	kapp->dcopClient()->send("kwin*", "", "reconfigure()", QByteArray(""));
	// kdesktop-background
	kapp->dcopClient()->send("kdesktop", "KBackgroundIface", "configure()", QByteArray(""));
}

/** show the previewWidget styled with the selected one */
void KStylePage::switchPrevStyle() {
	QStyle* style = QStyleFactory::create(currentStyle);
	if (!style) return;

	stylePreview->unsetPalette();
	QPalette palette = createPalette();
	style->polish(palette);
	stylePreview->setPalette(palette);

	// Prevent Qt from wrongly caching radio button images
	QPixmapCache::clear();
	// go ahead
	setStyleRecursive( stylePreview, palette, style );
	// this flickers, but reliably draws the widgets corretly.
	stylePreview->resize( stylePreview->sizeHint() );

	delete appliedStyle;
	appliedStyle = style;
}

void KStylePage::setStyleRecursive(QWidget* w, QPalette &palette, QStyle* s) {
	// Apply the new style.
	w->setStyle(s);
	// Recursively update all children.
	QList<QObject*> children = w->children();

	foreach( QObject * child, children ) {
		if (child->isWidgetType())
			setStyleRecursive((QWidget *) child, palette, s);
	}
}

/** create a QPalette of our current colorset */
QPalette KStylePage::createPalette() {
	colorSet *cc = &currentColors;
	QColorGroup disabledgrp(cc->windowForeground, cc->background, cc->background.light(150),
		cc->background.dark(), cc->background.dark(120), cc->background.dark(120),
		cc->windowBackground);
	QColorGroup colgrp(cc->windowForeground, cc->background, cc->background.light(150),
		cc->background.dark(), cc->background.dark(120), cc->foreground,
		cc->windowBackground);
	colgrp.setColor(QPalette::Highlight, cc->selectBackground);
	colgrp.setColor(QPalette::HighlightedText, cc->selectForeground);
	colgrp.setColor(QPalette::Button, cc->buttonBackground);
	colgrp.setColor(QPalette::ButtonText, cc->buttonForeground);
	return QPalette( colgrp, disabledgrp, colgrp);
}

#include "kstylepage.moc"
