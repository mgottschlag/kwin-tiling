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

#include "general.h"

#include <X11/Xlib.h>

/**** DLL Interface for kcontrol ****/

extern "C" {
    KCModule *create_style(QWidget *parent, const char *name) {
    KGlobal::locale()->insertCatalogue(QString::fromLatin1("kcmstyle"));
    return new KGeneral(parent, name);
    }

    void init_style() {
        KConfig config("kcmdisplayrc");
        config.setGroup("X11");
        if (config.readBoolEntry( "useResourceManager", true ))
        {
            KProcess proc;
            proc.setExecutable("krdb");
            proc.start( KProcess::Block );
        }
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
    kconfig = new KConfig("kcmdisplayrc");

    addColumn(i18n("Name:"));
    addColumn(i18n("Description:"));
    setAllColumnsShowFocus(true);
    KGlobal::dirs()->addResourceType("themes", KStandardDirs::kde_default("data") + "kstyle/themes");
    QStringList list = KGlobal::dirs()->resourceDirs("themes");
    for (QStringList::ConstIterator it = list.begin(); it != list.end(); it++)
        readThemeDir(*it);

    setFixedHeight(120);

    if (!currentItem())
        setSelected(firstChild(), true);

    ensureItemVisible(currentItem());
}


KThemeListBox::~KThemeListBox()
{
    delete kconfig;
}


void KThemeListBox::readThemeDir(const QString &directory)
{
    qDebug("Reading theme dir: %s", directory.latin1());
    QString name, desc;

    kconfig->setGroup("KDE");
    QString defName = "Default";
    QString curName = kconfig->readEntry("widgetStyleName", defName);

    QDir dir(directory, "*.themerc");
    if (!dir.exists())
    return;

    const QFileInfoList *list = dir.entryInfoList();
    QFileInfoListIterator it(*list);
    QFileInfo *fi;
    while ((fi = it.current())){
    KSimpleConfig config(fi->absFilePath(), true);
    config.setGroup("Misc");
    name = config.readEntry("Name", fi->baseName());
    desc = config.readEntry("Comment", i18n("No description available."));
    QListViewItem *item = new QListViewItem(this, name, desc, fi->absFilePath());
    if (name == curName) {
        curItem = item;
        setSelected(item, true);
        ensureItemVisible(item);
    }
    if (name == defName)
        defItem = item;
    ++it;
    }
}


void KThemeListBox::defaults()
{
    setSelected(defItem, true);
}


void KThemeListBox::load()
{
    setSelected(curItem, true);
}


void KThemeListBox::save()
{
    if (currentItem()->text(0) == curName)
        return;
    // check for legacy theme
    KSimpleConfig configTest(currentItem()->text(2));
    configTest.setGroup("Misc");
    if(configTest.hasKey("RCPath")){
        qDebug("Legacy theme");
        QFile input(configTest.readEntry("RCPath"));
        if(!input.open(IO_ReadOnly)){
            qDebug("Could not open input file!");
            return;
        }
        QFile output(QDir::home().absPath() + "/.gtkrc");
        if(!output.open(IO_WriteOnly)){
            qDebug("Could not open output file!");
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

    KThemeBase::applyConfigFile(currentItem()->text(2));
    curItem = currentItem();
    curName = curItem->text(0);
}


/**** KGeneral ****/

KGeneral::KGeneral(QWidget *parent, const char *name)
    : KCModule(parent, name)
{
    m_bChanged = false;
    useRM = true;
    macStyle = false;

    config = new KConfig("kcmdisplayrc");
    QBoxLayout *topLayout = new QVBoxLayout( this, KDialog::marginHint(),
                         KDialog::spacingHint() );

    // my little style list (mosfet 04/26/99)
    QBoxLayout *lay = new QHBoxLayout;
    topLayout->addLayout(lay);

    QGroupBox *themeBox = new QGroupBox(1, Vertical,
                    i18n("Widget style and theme:"),
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
    styles = new QGroupBox ( i18n( "Other settings for drawing:" ), this );
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

    tbStyle = new QButtonGroup( i18n( "Style options for toolbars:" ), this);
    topLayout->addWidget(tbStyle, 10);

    vlay = new QVBoxLayout( tbStyle, 10 );
    vlay->addSpacing( tbStyle->fontMetrics().lineSpacing() );

    QGridLayout *grid = new QGridLayout(2, 2);
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
    grid->addWidget(tbText, 0, 1);
    grid->addWidget(tbAside, 1, 0);
    grid->addWidget(tbUnder, 1, 1);

    vlay->addWidget(tbHilite, 10);
    vlay->addWidget(tbTransp, 10);

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
    tbUseText = 0;
    else if (tbText->isChecked() )
    tbUseText = 2;
    else if (tbAside->isChecked() )
    tbUseText = 1;
    else if (tbUnder->isChecked() )
    tbUseText = 3;
    else
    tbUseText = 0 ;

    tbUseHilite = tbHilite->isChecked();
    tbMoveTransparent = tbTransp->isChecked();

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

    config->setGroup( "Toolbar style" );
    tbUseText = config->readNumEntry( "IconText", 0);
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

    tbHilite->setChecked(tbUseHilite);
    tbTransp->setChecked(tbMoveTransparent);

    switch (tbUseText) {
    case 0:
        tbIcon->setChecked(true);
        break;
    case 1:
        tbAside->setChecked(true);
        break;
    case 2:
        tbText->setChecked(true);
        break;
    case 3:
        tbUnder->setChecked(true);
        break;
    default:
        tbIcon->setChecked(true);
    }
}


void KGeneral::defaults()
{
    useRM = true;
    macStyle = false;
    tbUseText = 0;
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

    config->setGroup("KDE");
    config->writeEntry("macStyle", macStyle, true, true);
    config->setGroup("Toolbar style");
    config->writeEntry("IconText", tbUseText, true, true);
    config->writeEntry("Highlighting", (int) tbUseHilite, true, true);
    config->writeEntry("TransparentMoving", (int) tbMoveTransparent,
        true, true);

    config->setGroup("X11");
    config->writeEntry("useResourceManager", useRM, true, true);
    config->sync();

    if (useRM) {
    QApplication::setOverrideCursor( waitCursor );
    KProcess proc;
    proc.setExecutable("krdb");
    proc.start( KProcess::Block );
    QApplication::restoreOverrideCursor();
    }

    KIPC::sendMessageAll(KIPC::StyleChanged);
    QApplication::syncX();

    m_bChanged = false;
    emit changed(false);
}


#include "general.moc"
