/* vi: ts=8 sts=4 sw=4
 *
 * This file is part of the KDE project, module kcmdisplay.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 * Copyright (C) 1997 Mark Donohoe
 *
 * You can Freely distribute this program under the GNU General Public
 * License. See the file "COPYING" for the exact licensing terms.
 */

#include <qgroupbox.h>
#include <qbuttongroup.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qlistview.h>
#include <qframe.h>
#include <qsizepolicy.h>
#include <qwhatsthis.h>
#include <qtextstream.h>

#include <kapp.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstddirs.h>
#include <kconfig.h>
#include <ksimpleconfig.h>
#include <kthemebase.h>
#include <kwin.h>
#include <kdialog.h>
#include <kipc.h>
#include <kprocess.h>
#include <kcmodule.h>
#include <kiconloader.h>
#include <kglobalsettings.h>
#include <kdirwatch.h>
#ifdef HAVE_AA
#include <kmessagebox.h>
#endif

#include "general.h"

#include <kdebug.h>

#include <X11/Xlib.h>

/**** DLL Interface for kcontrol ****/

extern void runRdb();

extern "C" {
    KCModule *create_style(QWidget *parent, const char *name) {
      KGlobal::locale()->insertCatalogue(QString::fromLatin1("kcmstyle"));
      return new KGeneral(parent, name);
    }

    void init_style() {
        KConfig config("kcmdisplayrc", true, false);
        config.setGroup("X11");
        if (config.readBoolEntry( "useResourceManager", true ))
	  runRdb();
        // Write some Qt root property.
       QByteArray properties;
       QDataStream d(properties, IO_WriteOnly);
       d << kapp->palette() << KGlobalSettings::generalFont();
       Atom a = XInternAtom(qt_xdisplay(), "_QT_DESKTOP_PROPERTIES", false);
       XChangeProperty(qt_xdisplay(),  qt_xrootwin(), a, a, 8, PropModeReplace,
               (unsigned char*) properties.data(), properties.size());

    }
}


/**** KThemeListBox ****/


// mosfet's style stuff 04/26/99 (mosfet)
KThemeListBox::KThemeListBox(QWidget *parent, const char *name)
    : QListView(parent, name)
{
    KConfig kconfig("kstylerc");
    kconfig.setGroup("KDE");
    defName = QString::fromLatin1("KDE default");
    curTheme = kconfig.readEntry("currentTheme");

    addColumn(i18n("Name"));
    addColumn(i18n("Description"));
    setAllColumnsShowFocus(true);
    KGlobal::dirs()->addResourceType("themes", KStandardDirs::kde_default("data") + "kstyle/themes");

    if (curTheme.isEmpty())
       curTheme = locate("themes", "default.themerc");

    connect(KDirWatch::self(), SIGNAL(dirty(const QString&)),
            this, SLOT(rescan()));
    localThemeDir = locateLocal("themes","");
    KDirWatch::self()->addDir(localThemeDir);
    KDirWatch::self()->startScan();

    setFixedHeight(120);

    rescan();
}


KThemeListBox::~KThemeListBox()
{
    KDirWatch::self()->removeDir(localThemeDir);
}


void KThemeListBox::readTheme(const QString &file)
{
    KSimpleConfig config(file, true);
    if (!config.hasGroup("KDE")) return;

    config.setGroup("Misc");
    QString name = config.readEntry("Name");
    if (name.isEmpty())
    {
       name = file;
       int i = name.findRev('/');
       if (i != -1)
          name = name.mid(i+1);
       i = name.find('.');
       if (i > 0)
          name.truncate(i);
    }
    QString desc = config.readEntry("Comment", i18n("No description available."));
    config.setGroup( "KDE" );

    QString style = config.readEntry( "widgetStyle" );
    QListViewItem *item = new QListViewItem(this, name, desc, file);
    if (file == curTheme) {
        curItem = item;
        setSelected(item, true);
        ensureItemVisible(item);
    }
    if (name == defName)
        defItem = item;
}


void KThemeListBox::defaults()
{
    setSelected(defItem, true);
    ensureItemVisible( defItem );
}


void KThemeListBox::load()
{
    if (0 != curItem) {
        setSelected(curItem, true);
        ensureItemVisible( curItem );
    }
}

void KThemeListBox::rescan()
{
    clear();
    curItem = 0;
    defItem = 0;

    QStringList list = KGlobal::dirs()->findAllResources("themes", "*.themerc", true, true);
    for (QStringList::ConstIterator it = list.begin(); it != list.end(); it++)
        readTheme(*it);

    if (!currentItem() )
        setSelected(firstChild(), true);

    ensureItemVisible(currentItem());
}


void KThemeListBox::save()
{
    if (currentItem()->text(2) == curTheme)
        return;
    // check for legacy theme
    KSimpleConfig configTest(currentItem()->text(2));
    configTest.setGroup("Misc");
    if(configTest.hasKey("RCPath")){
        kdDebug () << "Legacy theme" << endl;
        QFile input(configTest.readEntry("RCPath"));
        if(!input.open(IO_ReadOnly)){
            kdDebug() << "Could not open input file!" << endl;
            return;
        }
        QFile output(QDir::home().absPath() + "/.gtkrc");
        if(!output.open(IO_WriteOnly)){
            kdDebug () << "Could not open output file!" << endl;
            return;
        }
        QTextStream outStream(&output);
        QTextStream inStream(&input);
        outStream << "pixmap_path \"" << configTest.readEntry("PixmapPath") << "\""<< endl;
        while(!inStream.atEnd())
            outStream << inStream.readLine() << endl;
        input.close();
        output.close();
    }

    curItem = currentItem();
    curTheme = curItem->text(2);
    KThemeBase::applyConfigFile(currentItem()->text(2));

// This section can be removed when kdelibs 2.1 is released.
    KConfig kconfig("kstylerc");
    kconfig.setGroup("KDE");
    kconfig.writeEntry("currentTheme", curTheme);
    kconfig.sync();
// End section
}


/**** KGeneral ****/

KGeneral::KGeneral(QWidget *parent, const char *name)
    : KCModule(parent, name)
{
    m_bChanged = false;
    m_bToolbarsDirty = false;
    useRM = true;
    macStyle = false;

    config = new KConfig("kcmdisplayrc");
    QBoxLayout *topLayout = new QVBoxLayout( this, KDialog::marginHint(),
                         KDialog::spacingHint() );

    // my little style list (mosfet 04/26/99)
    QBoxLayout *lay = new QHBoxLayout;
    topLayout->addLayout(lay);

    QGroupBox *themeBox = new QGroupBox(1, Vertical,
                    i18n("Widget style and theme"),
                    this);
    lay->addWidget(themeBox);

    themeList = new KThemeListBox(themeBox);
    connect(themeList, SIGNAL(currentChanged(QListViewItem*)),
            SLOT(slotChangeStylePlugin(QListViewItem*)));
    QWhatsThis::add( themeBox, i18n("Here you can choose from a list of"
      " predefined widget styles (the way e.g. buttons are drawn) which"
      " may or may not be combined with a theme (additional information"
      " like a marble texture or a gradient).") );

    // Drawing settings
    styles = new QGroupBox ( i18n( "Other settings for drawing" ), this );
    topLayout->addWidget(styles, 10);
    QBoxLayout *vlay = new QVBoxLayout (styles, KDialog::marginHint(),
                    KDialog::spacingHint());
    vlay->addSpacing(styles->fontMetrics().lineSpacing());

    cbMac = new QCheckBox( i18n( "&Menubar on top of the screen in "
                                 "the style of MacOS" ), styles);
    QWhatsThis::add( cbMac, i18n("If this option is selected, applications"
      " won't have their menubar attached to their own window anymore."
      " Instead, there is one menu bar at the top of the screen which shows"
      " the menu of the currently active application. Maybe you know this"
      " behavior from MacOS.") );

    connect( cbMac, SIGNAL( clicked() ), SLOT( slotMacStyle()  )  );
    vlay->addWidget( cbMac, 10 );

    cbRes = new QCheckBox( i18n( "&Apply fonts and colors to non-KDE apps" ), styles);
    connect( cbRes, SIGNAL( clicked() ), SLOT( slotUseResourceManager()  )  );
    vlay->addWidget( cbRes, 10 );

    QWhatsThis::add( cbRes, i18n("If this option is selected, KDE will try"
      " to force your preferences regarding colors and fonts even on non-KDE"
      " applications. While this works fine with most applications, it <em>may</em>"
      " give strange results sometimes.") );

#ifdef HAVE_AA
    cbAA = new QCheckBox( i18n( "Use A&nti-Aliasing for fonts and icons" ), styles);
    connect( cbAA, SIGNAL( clicked() ), SLOT ( slotUseAntiAliasing() ) );
    vlay->addWidget( cbAA, 10 );
    QWhatsThis::add( cbAA, i18n( "If this option is selected, KDE will use"
      " anti-aliased fonts and pixmaps, meaning fonts can use more than"
      " just one color to simulate curves.") );
#endif

    tbStyle = new QButtonGroup( i18n( "Style options for toolbars" ), this);
    topLayout->addWidget(tbStyle, 10);

    vlay = new QVBoxLayout( tbStyle, 10 );
    vlay->addSpacing( tbStyle->fontMetrics().lineSpacing() );

    QGridLayout *grid = new QGridLayout(4, 2);
    vlay->addLayout( grid );

    tbIcon   = new QRadioButton( i18n( "&Icons only" ), tbStyle);
    tbText   = new QRadioButton( i18n( "&Text only" ), tbStyle);
    tbAside  = new QRadioButton( i18n( "Text a&side icons" ), tbStyle);
    tbUnder  = new QRadioButton( i18n( "Text &under icons" ), tbStyle);

    QWhatsThis::add( tbIcon, i18n("Shows only icons on toolbar buttons. Best option for low resolutions.") );
    QWhatsThis::add( tbText, i18n("Shows only text on toolbar buttons.") );
    QWhatsThis::add( tbAside, i18n("Shows icons and text on toolbar buttons. Text is aligned aside the icon.") );
    QWhatsThis::add( tbUnder, i18n("Shows icons and text on toolbar buttons. Text is aligned below the icon.") );

    tbHilite = new QCheckBox( i18n( "&Highlight buttons under mouse" ), tbStyle);
    tbTransp = new QCheckBox( i18n( "Tool&bars are transparent when"
                " moving" ), tbStyle);

    QWhatsThis::add( tbHilite, i18n("If this option is selected, toolbar buttons"
      " will change their color when the mouse cursor is moved over them.") );
    QWhatsThis::add( tbTransp, i18n("If this option is selected, only the skeleton of"
      " a toolbar will be shown when moving that toolbar.") );

    connect( tbIcon , SIGNAL( clicked() ), SLOT( slotChangeTbStyle()  )  );
    connect( tbText , SIGNAL( clicked() ), SLOT( slotChangeTbStyle()  )  );
    connect( tbAside, SIGNAL( clicked() ), SLOT( slotChangeTbStyle()  )  );
    connect( tbUnder, SIGNAL( clicked() ), SLOT( slotChangeTbStyle()  )  );
    connect( tbHilite, SIGNAL( clicked() ), SLOT( slotChangeTbStyle()  )  );
    connect( tbTransp, SIGNAL( clicked() ), SLOT( slotChangeTbStyle()  )  );

    grid->addWidget(tbIcon, 0, 0);
    grid->addWidget(tbText, 1, 0);
    grid->addWidget(tbAside, 2, 0);
    grid->addWidget(tbUnder, 3, 0);

    grid->addWidget(tbHilite, 0, 1);
    grid->addWidget(tbTransp, 1, 1);

    topLayout->addStretch( 100 );
    load();
}


KGeneral::~KGeneral()
{
    delete config;
}


void KGeneral::slotChangeStylePlugin(QListViewItem *)
{
    m_bChanged = true;
    emit changed(true);
}


void KGeneral::slotChangeTbStyle()
{
    if (tbIcon->isChecked() )
        tbUseText = "IconOnly";
    else if (tbText->isChecked() )
        tbUseText = "TextOnly";
    else if (tbAside->isChecked() )
        tbUseText = "IconTextRight";
    else if (tbUnder->isChecked() )
        tbUseText = "IconTextBottom";
    else
        tbUseText = "IconOnly";

    tbUseHilite = tbHilite->isChecked();
    tbMoveTransparent = tbTransp->isChecked();

    m_bToolbarsDirty = true;
    m_bChanged = true;
    emit changed(true);
}

void KGeneral::slotUseResourceManager()
{
    useRM = cbRes->isChecked();

    m_bChanged = true;
    emit changed(true);
}

void KGeneral::slotMacStyle()
{
    macStyle = cbMac->isChecked();

    m_bChanged = true;
    emit changed(true);
}

void KGeneral::slotUseAntiAliasing()
{
#ifdef HAVE_AA
    useAA = cbAA->isChecked();
    m_bChanged = true;
    emit changed(true);
#endif
}

void KGeneral::readSettings()
{
    config->setGroup("KDE");
    QString str = config->readEntry( "widgetStyle", "Platinum" );
    if ( str == "Platinum" )
    applicationStyle = WindowsStyle;
    else if ( str == "Windows 95" )
    applicationStyle = WindowsStyle;
    else
    applicationStyle = MotifStyle;
    macStyle = config->readBoolEntry( "macStyle", false);
#ifdef HAVE_AA
    useAA = config->readBoolEntry( "AntiAliasing", true);
    useAA_original = useAA;
#endif

    config->setGroup( "Toolbar style" );
    tbUseText = config->readEntry( "IconText", "IconOnly");
    int val = config->readNumEntry( "Highlighting", 1);
    tbUseHilite = val? true : false;
    tbMoveTransparent = config->readBoolEntry( "TransparentMoving", true);

    config->setGroup("X11");
    useRM = config->readBoolEntry( "useResourceManager", true );
}


void KGeneral::showSettings()
{
    cbRes->setChecked(useRM);
    cbMac->setChecked(macStyle);
#ifdef HAVE_AA
    cbAA->setChecked(useAA);
#endif

    tbHilite->setChecked(tbUseHilite);
    tbTransp->setChecked(tbMoveTransparent);


    tbIcon->setChecked( true ); // we need a default
    tbAside->setChecked( tbUseText == "IconTextRight" );
    tbText->setChecked( tbUseText == "TextOnly" );
    tbUnder->setChecked( tbUseText == "IconTextBottom" );
}


void KGeneral::defaults()
{
    useRM = true;
    macStyle = false;
#ifdef HAVE_AA
    useAA = true;
    useAA_original = true;
#endif
    tbUseText = "IconOnly";
    tbUseHilite = true;
    tbMoveTransparent = true;

    themeList->defaults();
    showSettings();

    m_bChanged = true;
    emit changed(true);
}

QString KGeneral::quickHelp() const
{
    return i18n("<h1>Style</h1> In this module you can configure how"
      " your KDE applications will look.<p>"
      " Styles and themes affect the way buttons, menus etc. are drawn"
      " in KDE.  Think of styles as plugins that provide a general look"
      " for KDE and of themes as a way to fine-tune those styles. E.g."
      " you might use the \"System\" style but use a theme that provides"
      " a color gradient or a marble texture.<p>"
      " Apart from styles and themes you can here configure the behavior"
      " of menubars and toolbars.");
}


void KGeneral::load()
{
    readSettings();
    showSettings();

    themeList->load();

    m_bChanged = false;
    emit changed(false);
}


void KGeneral::save()
{
    themeList->save();

    if (!m_bChanged)
    return;

#ifdef HAVE_AA
    if(useAA != useAA_original) {
	KMessageBox::information(this, i18n("You have changed anti-aliasing related settings.\nThis change won't take effect before you restart KDE."), i18n("Anti-aliasing settings changed"), "AAsettingsChanged", false);
	useAA_original = useAA;
    }
#endif

    config->setGroup("KDE");
    config->writeEntry("macStyle", macStyle, true, true);
#ifdef HAVE_AA
    config->writeEntry("AntiAliasing", useAA, true, true);
#endif
    config->setGroup("Toolbar style");
    config->writeEntry("IconText", tbUseText, true, true);
    config->writeEntry("Highlighting", (int) tbUseHilite, true, true);
    config->writeEntry("TransparentMoving", (int) tbMoveTransparent,
        true, true);

    config->setGroup("X11");
    config->writeEntry("useResourceManager", useRM);
    config->sync();

    if (useRM) {
      QApplication::setOverrideCursor( waitCursor );
      runRdb();
      QApplication::restoreOverrideCursor();
    }

    KIPC::sendMessageAll(KIPC::StyleChanged);
    if ( m_bToolbarsDirty )
        KIPC::sendMessageAll(KIPC::ToolbarStyleChanged, 0 /* we could use later for finer granularity */);
    QApplication::syncX();

    m_bChanged = false;
    emit changed(false);
}


#include "general.moc"
