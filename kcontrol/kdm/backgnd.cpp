/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kcmdisplay.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 *
 * Based on old backgnd.cpp:
 *
 * Copyright (c)  Martin R. Jones 1996
 * Converted to a kcc module by Matthias Hoelzer 1997
 * Gradient backgrounds by Mark Donohoe 1997
 * Pattern backgrounds by Stephan Kulow 1998
 * Randomizing & dnd & new display modes by Matej Koss 1998
 *
 * You can Freely distribute this program under the GNU General Public
 * License. See the file "COPYING" for the exact licensing terms.
 */

#include <qobject.h>
#include <qlayout.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qlabel.h>
#include <qlistbox.h>
#include <qgroupbox.h>
#include <qcombobox.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qdragobject.h>
#include <qhbox.h>
#include <qevent.h>

#include <kapp.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <kiconloader.h>
#include <kcolorbtn.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kpixmap.h>
#include <dcopclient.h>
#include <ksimpleconfig.h>

#include <bgdefaults.h>
#include <bgsettings.h>
#include <bgrender.h>
#include <bgdialogs.h>
#include <backgnd.h>


/**** DLL Interface ****/

extern "C" {
    KCModule *create_background(QWidget *parent, const char *name) {
	return new KBackground(parent, name);
    }
}


/**** KBGMonitor ****/

void KBGMonitor::dropEvent(QDropEvent *e)
{
    if (!QUriDrag::canDecode(e))
	return;

    QStringList uris;
    if (QUriDrag::decodeLocalFiles(e, uris) && (uris.count() > 0)) {
	QString uri = *uris.begin();
	uri.prepend('/');
	emit imageDropped(uri);
    }
}
				
					
void KBGMonitor::dragEnterEvent(QDragEnterEvent *e)
{
    e->accept(QImageDrag::canDecode(e)|| QUriDrag::canDecode(e));
}


/**** KBackground ****/

KBackground::KBackground(QWidget *parent, const char *name)
    : KCModule(parent, name)
{
    m_pDirs = KGlobal::dirs();

    // Top layout
    QGridLayout *top = new QGridLayout(this, 2, 2);
    top->setSpacing(10); top->setMargin(10);
    top->setColStretch(0, 1);
    top->setColStretch(1, 2);

    // A nice button size. Translators can adapt this
    QPushButton *pbut = new QPushButton(i18n("abcdefgh"), this);
    QSize bsize = pbut->sizeHint();
    delete pbut;

    /*
    // Desktop chooser at (0, 0)
    QGroupBox *group = new QGroupBox(i18n("Desktop"), this);
    top->addWidget(group, 0, 0);
    QVBoxLayout *vbox = new QVBoxLayout(group);
    vbox->setMargin(10);
    vbox->setSpacing(10);
    vbox->addSpacing(10);
    m_pDeskList = new QListBox(group);
    connect(m_pDeskList, SIGNAL(highlighted(int)), SLOT(slotSelectDesk(int)));
    vbox->addWidget(m_pDeskList);
    m_pCBCommon = new QCheckBox(i18n("&Common Background"), group);
    vbox->addWidget(m_pCBCommon);
    connect(m_pCBCommon, SIGNAL(toggled(bool)), SLOT(slotCommonDesk(bool)));
    */

    // Background settings
    QGroupBox *group = new QGroupBox(i18n("Background"), this);
    top->addWidget(group, 1, 0);
    QGridLayout *grid = new QGridLayout(group, 5, 2);
    grid->setMargin(10);
    grid->setSpacing(10);
    grid->addRowSpacing(0, 10);
    grid->setColStretch(0, 0);

    QLabel *lbl = new QLabel(i18n("&Mode:"), group);
    lbl->setFixedSize(lbl->sizeHint());
    grid->addWidget(lbl, 1, 0, Qt::AlignLeft);
    m_pBackgroundBox = new QComboBox(group);
    connect(m_pBackgroundBox, SIGNAL(activated(int)), SLOT(slotBGMode(int)));
    lbl->setBuddy(m_pBackgroundBox);
    grid->addWidget(m_pBackgroundBox, 1, 1);

    lbl = new QLabel(i18n("Color &1:"), group);
    lbl->setFixedSize(lbl->sizeHint());
    grid->addWidget(lbl, 2, 0, Qt::AlignLeft);
    m_pColor1But = new KColorButton(group);
    connect(m_pColor1But, SIGNAL(changed(const QColor &)),
	    SLOT(slotColor1(const QColor &)));
    grid->addWidget(m_pColor1But, 2, 1);
    lbl->setBuddy(m_pColor1But);

    lbl = new QLabel(i18n("Color &2:"), group);
    lbl->setFixedSize(lbl->sizeHint());
    grid->addWidget(lbl, 3, 0, Qt::AlignLeft);
    m_pColor2But = new KColorButton(group);
    connect(m_pColor2But, SIGNAL(changed(const QColor &)),
	    SLOT(slotColor2(const QColor &)));
    grid->addWidget(m_pColor2But, 3, 1);
    lbl->setBuddy(m_pColor2But);

    QHBoxLayout *hbox = new QHBoxLayout();
    grid->addLayout(hbox, 4, 1);
    m_pBGSetupBut = new QPushButton(i18n("&Setup"), group);
    m_pBGSetupBut->setFixedSize(bsize);
    connect(m_pBGSetupBut, SIGNAL(clicked()), SLOT(slotBGSetup()));
    hbox->addWidget(m_pBGSetupBut);
    hbox->addStretch();


    // Preview monitor at (0,1)
    lbl = new QLabel(this);
    lbl->setPixmap(locate("data", "kcontrol/pics/monitor.png"));
    lbl->setFixedSize(lbl->sizeHint());
    top->addMultiCellWidget(lbl, 0,0,0,1, AlignCenter);
    m_pMonitor = new KBGMonitor(lbl);
    m_pMonitor->setGeometry(20, 10, 157, 111);
    connect(m_pMonitor, SIGNAL(imageDropped(QString)), SLOT(slotImageDropped(QString)));

    // Wallpaper at (1,1)
    group = new QGroupBox(i18n("Wallpaper"), this);
    top->addWidget(group, 1, 1);
    grid = new QGridLayout(group, 6, 2);
    grid->setMargin(10); grid->setSpacing(10);
    grid->addRowSpacing(0, 10);
    grid->addRowSpacing(4, 10);

    lbl = new QLabel(i18n("M&ode:"), group);
    lbl->setFixedSize(lbl->sizeHint());
    grid->addWidget(lbl, 1, 0, Qt::AlignLeft);
    m_pArrangementBox = new QComboBox(group);
    connect(m_pArrangementBox, SIGNAL(activated(int)), SLOT(slotWPMode(int)));
    lbl->setBuddy(m_pArrangementBox);
    grid->addWidget(m_pArrangementBox, 1, 1);

    lbl = new QLabel(i18n("&Wallpaper"), group);
    lbl->setFixedSize(lbl->sizeHint());
    grid->addWidget(lbl, 2, 0, Qt::AlignLeft);
    m_pWallpaperBox = new QComboBox(group);
    lbl->setBuddy(m_pWallpaperBox);
    connect(m_pWallpaperBox, SIGNAL(activated(const QString &)),
	    SLOT(slotWallpaper(const QString &)));
    grid->addWidget(m_pWallpaperBox, 2, 1);

    hbox = new QHBoxLayout();
    grid->addLayout(hbox, 3, 1);
    m_pBrowseBut = new QPushButton(i18n("&Browse"), group);
    m_pBrowseBut->setFixedSize(bsize);
    connect(m_pBrowseBut, SIGNAL(clicked()), SLOT(slotBrowseWallpaper()));
    hbox->addWidget(m_pBrowseBut);
    hbox->addStretch();


    m_pCBMulti = new QCheckBox(i18n("M&ultiple:"), group);
    m_pCBMulti->setFixedSize(m_pCBMulti->sizeHint());
    connect(m_pCBMulti, SIGNAL(toggled(bool)), SLOT(slotMultiMode(bool)));
    grid->addWidget(m_pCBMulti, 4, 0);
    hbox = new QHBoxLayout();
    grid->addLayout(hbox, 4, 1);
    m_pMSetupBut = new QPushButton(i18n("S&etup"), group);
    m_pMSetupBut->setFixedSize(bsize);
    connect(m_pMSetupBut, SIGNAL(clicked()), SLOT(slotSetupMulti()));
    hbox->addWidget(m_pMSetupBut);
    hbox->addStretch();

    m_pCBMulti->hide();
    m_pMSetupBut->hide();

    m_Desk = 0;
    m_Max = 1;
    for (int i=0; i<m_Max; i++) {
        KSimpleConfig *c = new KSimpleConfig(locate("config", "kdmdesktoprc"));
	m_Renderer[i] = new KBackgroundRenderer(i, c);
	connect(m_Renderer[i], SIGNAL(imageDone(int)), SLOT(slotPreviewDone(int)));
    }

    init();
    apply();
}


/*
 * Fill all check/listboxen
 */
void KBackground::init()
{
    int i;

    /*
    // Desktop names
    for (i=0; i<KWin::numberOfDesktops(); i++)
	m_pDeskList->insertItem(m_pGlobals->deskName(i));
    */

    // Background modes: make sure these match with kdesktop/bgrender.cc !!
    m_pBackgroundBox->insertItem(i18n("Flat"));
    m_pBackgroundBox->insertItem(i18n("Pattern"));
    m_pBackgroundBox->insertItem(i18n("Background Program"));
    m_pBackgroundBox->insertItem(i18n("Horizontal Gradient"));
    m_pBackgroundBox->insertItem(i18n("Vertical Gradient"));
    m_pBackgroundBox->insertItem(i18n("Pyramid Gradient"));
    m_pBackgroundBox->insertItem(i18n("Pipecross Gradient"));
    m_pBackgroundBox->insertItem(i18n("Elliptic Gradient"));

    // Wallpapers
    QStringList lst = m_pDirs->findAllResources("wallpaper", "*", false, true);
    for (i=0; i<(int)lst.count(); i++) {
	int n = lst[i].findRev('/');
	QString s = lst[i].mid(n+1);
	m_pWallpaperBox->insertItem(s);
	m_Wallpaper[s] = i;
    }

    // Wallpaper tilings: again they must match the ones from bgrender.cc
    m_pArrangementBox->insertItem(i18n("No Wallpaper"));
    m_pArrangementBox->insertItem(i18n("Centred"));
    m_pArrangementBox->insertItem(i18n("Tiled"));
    m_pArrangementBox->insertItem(i18n("Center Tiled"));
    m_pArrangementBox->insertItem(i18n("Centred Maxpect"));
    m_pArrangementBox->insertItem(i18n("Scaled"));
}


void KBackground::apply()
{
    int desk = m_Desk;
    desk = 0;
    KBackgroundRenderer *r = m_Renderer[desk];

    /*
    // Desktop names
    if (m_pGlobals->commonBackground()) {
	m_pCBCommon->setChecked(true);
	m_pDeskList->setEnabled(false);
    } else  {
	m_pCBCommon->setChecked(false);
	m_pDeskList->setEnabled(true);
	m_pDeskList->setCurrentItem(m_Desk);
    }
    */

    // Background mode
    m_pBackgroundBox->setCurrentItem(r->backgroundMode());
    m_pColor1But->setColor(r->colorA());
    m_pColor2But->setColor(r->colorB());
    switch (r->backgroundMode()) {
    case KBackgroundSettings::Program:
	m_pColor1But->setEnabled(false);
	m_pColor2But->setEnabled(false);
	m_pBGSetupBut->setEnabled(true);
	break;
    case KBackgroundSettings::Flat:
	m_pColor1But->setEnabled(true);
	m_pColor2But->setEnabled(false);
	m_pBGSetupBut->setEnabled(false);
	break;
    case KBackgroundSettings::Pattern:
	m_pColor1But->setEnabled(true);
	m_pColor2But->setEnabled(true);
	m_pBGSetupBut->setEnabled(true);
	break;
    default:
	m_pColor1But->setEnabled(true);
	m_pColor2But->setEnabled(true);
	m_pBGSetupBut->setEnabled(false);
	break;
    }

    // Wallpaper mode
    QString wp = r->wallpaper();
    if (wp.isEmpty())
	wp = QString(" ");
    if (!m_Wallpaper.contains(wp)) {
	int count = m_Wallpaper.count();
	m_Wallpaper[wp] = count;
	m_pWallpaperBox->insertItem(wp);
	m_pWallpaperBox->setCurrentItem(count);
    }
    m_pWallpaperBox->setCurrentItem(m_Wallpaper[wp]);
    m_pArrangementBox->setCurrentItem(r->wallpaperMode());

    // Multi mode
    if (r->multiWallpaperMode() == KBackgroundSettings::NoMulti) {
	m_pCBMulti->setChecked(false);
	m_pWallpaperBox->setEnabled(true);
	m_pBrowseBut->setEnabled(true);
	m_pMSetupBut->setEnabled(false);
    } else {
	m_pCBMulti->setChecked(true);
	m_pWallpaperBox->setEnabled(false);
	m_pBrowseBut->setEnabled(false);
	m_pMSetupBut->setEnabled(true);
    }

    // Start preview render
    r->setPreview(m_pMonitor->size());
    r->start();
}


void KBackground::load()
{
    int desk = m_Desk;
    desk = 0;
    m_Renderer[desk]->load(desk);

    apply();
    emit changed(false);
}


void KBackground::save()
{
    qDebug("Saving stuff...");
    for (int i=0; i<m_Max; i++)
	m_Renderer[i]->writeSettings();

    emit changed(false);
}


void KBackground::defaults()
{
    int desk = m_Desk;
    desk = 0;
    KBackgroundRenderer *r = m_Renderer[desk];

    if (r->isActive())
	r->stop();
    r->setBackgroundMode(KBackgroundSettings::Flat);
    r->setColorA(_defColorA);
    r->setColorB(_defColorB);
    r->setWallpaperMode(KBackgroundSettings::NoWallpaper);
    r->setMultiWallpaperMode(KBackgroundSettings::NoMulti);
    apply();
    emit changed(true);
}


void KBackground::slotSelectDesk(int desk)
{
    if (desk == m_Desk)
	return;

    if (m_Renderer[m_Desk]->isActive())
	m_Renderer[m_Desk]->stop();
    m_Desk = desk;
    apply();
}


void KBackground::slotCommonDesk(bool /*common*/)
{
    apply();
    emit changed(true);
}


/*
 * Called from the "Background Mode" combobox.
 */
void KBackground::slotBGMode(int mode)
{
    int desk = m_Desk;
	desk = 0;
    KBackgroundRenderer *r = m_Renderer[desk];

    if (mode == r->backgroundMode())
	return;

    r->stop();
    r->setBackgroundMode(mode);
    apply();
    emit changed(true);
}


/*
 * Called from the "Background Setup" pushbutton.
 */
void KBackground::slotBGSetup()
{
    int desk = m_Desk;
	desk = 0;
    KBackgroundRenderer *r = m_Renderer[desk];

    switch (r->backgroundMode()) {
    case KBackgroundSettings::Pattern:
    {
	KPatternSelectDialog dlg;
	QString cur = r->KBackgroundPattern::name();
	dlg.setCurrent(cur);
	if ((dlg.exec() == QDialog::Accepted) && !dlg.pattern().isEmpty()) {
	    r->stop();
	    r->setPattern(dlg.pattern());
	    r->start();
	    emit changed(true);
	}
	break;
    }
    case KBackgroundSettings::Program:
    {
	KProgramSelectDialog dlg;
	QString cur = r->KBackgroundProgram::name();
	dlg.setCurrent(cur);
	if ((dlg.exec() == QDialog::Accepted) && !dlg.program().isEmpty()) {
	    r->stop();
	    r->setProgram(dlg.program());
	    r->start();
	    emit changed(true);
	}
	break;
    }
    default:
	break;
    }
}


void KBackground::slotColor1(const QColor &color)
{
    int desk = m_Desk;
	desk = 0;
    KBackgroundRenderer *r = m_Renderer[desk];

    if (color == r->colorA())
	return;

    r->stop();
    r->setColorA(color);
    r->start();
    emit changed(true);
}


void KBackground::slotColor2(const QColor &color)
{
    int desk = m_Desk;
	desk = 0;
    KBackgroundRenderer *r = m_Renderer[desk];

    if (color == r->colorB())
	return;

    r->stop();
    r->setColorB(color);
    r->start();
    emit changed(true);
}


void KBackground::slotImageDropped(QString uri)
{
    int desk = m_Desk;
	desk = 0;
    KBackgroundRenderer *r = m_Renderer[desk];
    if (uri == r->wallpaper())
	return;

    if (!m_Wallpaper.contains(uri)) {
	int count = m_Wallpaper.count();
	m_Wallpaper[uri] = count;
	m_pWallpaperBox->insertItem(uri);
	m_pWallpaperBox->setCurrentItem(count);
    }

    r->stop();
    r->setWallpaper(uri);
    r->start();
    emit changed(true);
}


void KBackground::slotMultiMode(bool multi)
{
    int desk = m_Desk;
	desk = 0;
    KBackgroundRenderer *r = m_Renderer[desk];
    if (multi == (r->multiWallpaperMode() != KBackgroundSettings::NoMulti))
	return;

    r->stop();
    r->setMultiWallpaperMode(multi ? 1 : 0);
    r->start();

    if (multi) {
	m_pWallpaperBox->setEnabled(false);
	m_pBrowseBut->setEnabled(false);
	m_pMSetupBut->setEnabled(true);
    } else {
	m_pWallpaperBox->setEnabled(true);
	m_pBrowseBut->setEnabled(true);
	m_pMSetupBut->setEnabled(false);
    }
    emit changed(true);
}


void KBackground::slotWallpaper(const QString &wallpaper)
{
    int desk = m_Desk;
	desk = 0;
    KBackgroundRenderer *r = m_Renderer[desk];

    if (wallpaper == r->wallpaper())
	return;

    r->stop();
    r->setWallpaper(wallpaper);
    r->start();
    emit changed(true);
}


void KBackground::slotBrowseWallpaper()
{
    int desk = m_Desk;
	desk = 0;
    KBackgroundRenderer *r = m_Renderer[desk];

    QString file = KFileDialog::getOpenFileName();
    if (file.isEmpty())
	return;
    if (file == r->wallpaper())
	return;

    if (!m_Wallpaper.contains(file)) {
	int count = m_Wallpaper.count();
	m_Wallpaper[file] = count;
	m_pWallpaperBox->insertItem(file);
	m_pWallpaperBox->setCurrentItem(count);
    }

    r->stop();
    r->setWallpaper(file);
    r->start();
    emit changed(true);
}


/*
 * Called from the "Wallpaper Arrangement" combobox.
 */
void KBackground::slotWPMode(int mode)
{
    int desk = m_Desk;
	desk = 0;
    KBackgroundRenderer *r = m_Renderer[desk];

    if (mode == r->wallpaperMode())
	return;

    r->stop();
    r->setWallpaperMode(mode);
    r->start();
    emit changed(true);
}


void KBackground::slotSetupMulti()
{
    int desk = m_Desk;
	desk = 0;
    KBackgroundRenderer *r = m_Renderer[desk];

    KMultiWallpaperDialog dlg(r);
    if (dlg.exec() == QDialog::Accepted) {
	r->stop();
	r->start();
	emit changed(true);
    }
}


void KBackground::slotPreviewDone(int desk_done)
{
    qDebug("Preview for desktop %d done", desk_done);

    int desk = m_Desk;
	desk = 0;
    if (desk != desk_done)
	return;
    KBackgroundRenderer *r = m_Renderer[desk];

    KPixmap pm;
    if (QPixmap::defaultDepth() < 15)
	pm.convertFromImage(*r->image(), KPixmap::LowColor);
    else
	pm.convertFromImage(*r->image());

    m_pMonitor->setBackgroundPixmap(pm);
}


#include "backgnd.moc"
