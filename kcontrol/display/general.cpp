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

#include <kapp.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstddirs.h>
#include <kconfig.h>
#include <ksimpleconfig.h>
#include <kthemebase.h>
#include <kmessagebox.h>
#include <kwm.h>
#include <kdialog.h>
#include <kipc.h>
#include <kprocess.h>
#include <kcmodule.h>

#include "general.h"


/**** DLL Interface for kcontrol ****/

extern "C" {
    KCModule *create_style(QWidget *parent, const char *name) {
	return new KGeneral(parent, name);
    }
}


/**** KIconStyle ****/

KIconStyle::KIconStyle(QWidget *parent, const char *name)
    : QGroupBox(parent, name)
{
    m_PanelStyle = m_KonqStyle = m_KDEStyle = 0;

    setTitle(i18n("Icon style"));

    QGridLayout *grid = new QGridLayout(this, 7, 3, KDialog::marginHint(),
					KDialog::spacingHint());
    grid->addRowSpacing(0, fontMetrics().lineSpacing());
    QLabel *lbl = new QLabel(i18n("Normal"), this);

    grid->addWidget(lbl, 1, 1);
    lbl = new QLabel(i18n("Large"), this);
    grid->addWidget(lbl, 1, 2);

    // Panel
    lbl = new QLabel(i18n("Panel"), this);
    grid->addWidget(lbl, 2, 0);
    panelGroup = new QButtonGroup();
    connect(panelGroup, SIGNAL(clicked(int)), SLOT(slotPanel(int)));
    QRadioButton *rb = new QRadioButton(this);
    panelGroup->insert(rb);
    grid->addWidget(rb, 2, 1);
    rb = new QRadioButton(this);
    panelGroup->insert(rb);
    grid->addWidget(rb, 2, 2);

    // Konq
    lbl = new QLabel(i18n("Konqueror"), this);
    grid->addWidget(lbl, 3, 0);
    konqGroup = new QButtonGroup();
    connect(konqGroup, SIGNAL(clicked(int)), SLOT(slotKonq(int)));
    rb = new QRadioButton(this);
    konqGroup->insert(rb);
    grid->addWidget(rb, 3, 1);
    rb = new QRadioButton(this);
    konqGroup->insert(rb);
    grid->addWidget(rb, 3, 2);

    // KDE
    lbl = new QLabel(i18n("Other"), this);
    grid->addWidget(lbl, 4, 0);
    kdeGroup = new QButtonGroup();
    connect(kdeGroup, SIGNAL(clicked(int)), SLOT(slotKDE(int)));
    rb = new QRadioButton(this);
    kdeGroup->insert(rb);
    grid->addWidget(rb, 4, 1);
    rb = new QRadioButton(this);
    kdeGroup->insert(rb);
    grid->addWidget(rb, 4, 2);

    QFrame *frame = new QFrame(this);
    frame->setFrameStyle(QFrame::HLine|QFrame::Sunken);
    grid->addMultiCellWidget(frame, 5, 5, 0, 2);
      
    singleClick = new QCheckBox(i18n("Single &click to activate"), this);
    connect(singleClick, SIGNAL(clicked()), SLOT(slotSingleClick()));

    grid->addMultiCellWidget(singleClick, 6, 6, 0, 2);

    config = new KConfig("kcmdisplayrc");
    load();
}


KIconStyle::~KIconStyle()
{
    delete config;
}


void KIconStyle::load()
{
    config->setGroup("KDE");
    
    QString s = config->readEntry("kpanelIconStyle", "Normal");
    m_PanelStyle = 0;
    if (s == "Large") m_PanelStyle = 1;
    panelGroup->setButton(m_PanelStyle);

    s = config->readEntry("kfmIconStyle", "Normal");
    m_KonqStyle = 0;
    if (s == "Large") m_KonqStyle = 1;
    konqGroup->setButton(m_KonqStyle);

    s = config->readEntry("kfmIconStyle", "Normal");
    m_KDEStyle = 0;
    if (s == "Large") m_KDEStyle = 1;
    kdeGroup->setButton(m_KDEStyle);

    bool b = config->readBoolEntry("SingleClick", true);
    singleClick->setChecked(b);

    bChanged = false;
}


void KIconStyle::save()
{
    if (!bChanged)
	return;

    config->setGroup("KDE");

    config->writeEntry("kpanelIconStyle", m_PanelStyle ? "Large" : "Normal");
    // TODO: notify kicker
    config->writeEntry("konqIconStyle", m_KonqStyle ? "Large" : "Normal");
    // TODO: notify konqy
    config->writeEntry("KDEIconStyle", m_KDEStyle ? "Large" : "Normal");

    config->writeEntry("SingleClick", singleClick->isChecked(), true, true);

    KMessageBox::information(0L, i18n("The icon style change will "
	    "not all be applied until you restart KDE."));
    bChanged = false;
}


void KIconStyle::defaults()
{
    m_PanelStyle = 0; panelGroup->setButton(0);
    m_KonqStyle = 0; konqGroup->setButton(0);
    m_KDEStyle = 0; kdeGroup->setButton(0);
    singleClick->setChecked(true);
    bChanged = true; emit changed(bChanged);
}


void KIconStyle::slotPanel(int style)
{
    m_PanelStyle = style;
    bChanged = true; emit changed(bChanged);
}

void KIconStyle::slotKonq(int style)
{
    m_KonqStyle = style;
    bChanged = true; emit changed(bChanged);
}

void KIconStyle::slotKDE(int style)
{
    m_KDEStyle = style;
    bChanged = true; emit changed(bChanged);
}

void KIconStyle::slotSingleClick()
{
  bChanged = true; emit changed(bChanged);
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

}


KThemeListBox::~KThemeListBox()
{
    delete kconfig;
}


void KThemeListBox::readThemeDir(const QString &directory)
{
    QString name, desc;

    kconfig->setGroup("KDE");
    QString defName = "Qt Platinum";
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


    // Icon style
    iconStyle = new KIconStyle(this);
    connect(iconStyle, SIGNAL(changed(bool)), SIGNAL(changed(bool)));
    lay->addWidget(iconStyle);


    // Drawing settings
    styles = new QGroupBox ( i18n( "Other settings for drawing:" ), this );
    topLayout->addWidget(styles, 10);
    QBoxLayout *vlay = new QVBoxLayout (styles, KDialog::marginHint(),
					KDialog::spacingHint());
    vlay->addSpacing(styles->fontMetrics().lineSpacing());

    cbMac = new QCheckBox( i18n( "&Menubar on top of the screen in "
                                 "the style of MacOS" ), styles);
    connect( cbMac, SIGNAL( clicked() ), SLOT( slotMacStyle()  )  );
    vlay->addWidget( cbMac, 10 );

    cbRes = new QCheckBox( i18n( "&Apply fonts and colors to non-KDE apps" ), styles);
    connect( cbRes, SIGNAL( clicked() ), SLOT( slotUseResourceManager()  )  );
    vlay->addWidget( cbRes, 10 );

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

    tbHilite = new QCheckBox( i18n( "&Highlight buttons under mouse" ), tbStyle);
    tbTransp = new QCheckBox( i18n( "Tool&bars are transparent when"
				" moving" ), tbStyle);

    connect( tbIcon , SIGNAL( clicked() ), SLOT( slotChangeTbStyle()  )  );
    connect( tbText , SIGNAL( clicked() ), SLOT( slotChangeTbStyle()  )  );
    connect( tbAside, SIGNAL( clicked() ), SLOT( slotChangeTbStyle()  )  );
    connect( tbUnder, SIGNAL( clicked() ), SLOT( slotChangeTbStyle()  )  );

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
    
    tbHilite->setChecked(tbHilite);
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

    iconStyle->defaults();
    themeList->defaults();
    showSettings();

    m_bChanged = true;
    emit changed(true);
}


void KGeneral::load()
{
    readSettings();
    showSettings();

    iconStyle->load();
    themeList->load();

    m_bChanged = false;
    emit changed(false);
}


void KGeneral::save()
{
    iconStyle->save();
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
    config->writeEntry("useResourceManager", useRM);
    config->sync();
    
    if (useRM) {
	QApplication::setOverrideCursor( waitCursor );
	KProcess proc;
	proc.setExecutable("krdb");
	proc.start( KProcess::Block );
	QApplication::restoreOverrideCursor();
    }
    
    KIPC::sendMessageAll("KDEChangeGeneral");
    KWM::sendKWMCommand((macStyle ? "macStyleOn" : "macStyleOff"));
    QApplication::syncX();

    m_bChanged = false;
    emit changed(false);
}


#include "general.moc"
