// vi: ts=8 sts=4 sw=4
//
// KDE Display fonts setup tab
//
// Copyright (c)  Mark Donohoe 1997
//                lars Knoll 1999
// Ported to kcontrol2 by Geert Jansen.

/* $Id$ */

#include <qlayout.h>
#include <qlistbox.h>
#include <qlabel.h>
#include <qapplication.h>

#include <kconfig.h>
#include <ksimpleconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <kipc.h>
#include <kstddirs.h>
#include <kprocess.h>

#include "fonts.h"
#include "fonts.moc"


/**** DLL Interface ****/

extern "C" {
    KCModule *create_fonts(QWidget *parent, const char *name) {
	KGlobal::locale()->insertCatalogue("kcmdisplay");
	return new KFonts(parent, name);
    }
}


/**** FontUseItem ****/

FontUseItem::FontUseItem(QString n, QFont default_fnt, bool f)
	: selected(0)
{
    _text = n;
    _default = _font = default_fnt;
    fixed = f;
}

QString FontUseItem::fontString(QFont rFont)
{
    QString aValue;
    aValue = rFont.rawName();
    return aValue;
}

void FontUseItem::setRC(QString group, QString key, QString rcfile)
{
    _rcgroup = group;
    _rckey = key;
    _rcfile = rcfile;
}

void FontUseItem::setDefault()
{
    _font = _default;
}

void FontUseItem::readFont()
{
    KConfigBase *config;
    
    bool deleteme = false;
    if (_rcfile.isEmpty())
	config = KGlobal::config();
    else
    {
	 config = new KSimpleConfig(locate("config", _rcfile), true);
	 deleteme = true;
    }

    config->setGroup(_rcgroup);
    QFont tmpFnt(_font);
    _font = config->readFontEntry(_rckey, &tmpFnt);
    if (deleteme) delete config;
}

void FontUseItem::writeFont()
{
    KConfigBase *config;
    
    if (_rcfile.isEmpty()) {
	config = KGlobal::config();
	config->setGroup(_rcgroup);
	config->writeEntry(_rckey, _font, true, true);
	config->sync();
    } else {
	config = new KSimpleConfig(locate("config", _rcfile));
	config->setGroup(_rcgroup);
	config->writeEntry(_rckey, _font);
	config->sync();
	delete config;
    }
}


/**** KFonts ****/

KFonts::KFonts(QWidget *parent, const char *name)
	: KCModule(parent, name)
{
    int i;
    _changed = false;

    KConfig *cfg = KGlobal::config();
    cfg->setGroup("X11");
    useRM = cfg->readBoolEntry("useResourceManager", true);
    
    QBoxLayout *topLayout = new QVBoxLayout(this, 10, 10);
    QBoxLayout *pushLayout = new QHBoxLayout( 5 );
    topLayout->addLayout( pushLayout );
    
    FontUseItem *item = new FontUseItem( i18n("General font"),
			    QFont( "helvetica", 12 ) );
    item->setRC( "General", "font" );
    fontUseList.append( item );
    
    item = new FontUseItem( i18n("Fixed font"),
			    QFont( "fixed", 12 ), true );
    item->setRC( "General", "fixedFont" );
    fontUseList.append( item );

    item = new FontUseItem( i18n("Toolbar font"),
			    QFont( "helvetica", 10 ) );
    item->setRC( "General", "toolBarFont" );
    fontUseList.append( item );

    item = new FontUseItem( i18n("Menu font"),
			    QFont( "helvetica", 12 ) );
    item->setRC( "General", "menuFont" );
    fontUseList.append( item );
  
    item = new FontUseItem( i18n("Window title font"),
			    QFont( "helvetica", 12, QFont::Bold ) );
    item->setRC( "WM", "titleFont" );
    fontUseList.append( item );
			    
    item = new FontUseItem( i18n("Panel button font"),
			    QFont( "helvetica", 12 )  );
    item->setRC( "kpanel", "DesktopButtonFont", "kpanelrc" );
    fontUseList.append( item );
    
    item = new FontUseItem( i18n("Panel clock font"),
			    QFont( "helvetica", 12, QFont::Normal) );
    item->setRC( "kpanel", "DateFont", "kpanelrc" );
    fontUseList.append( item );
    
    for ( i = 0; i < (int) fontUseList.count(); i++ )
	fontUseList.at( i )->readFont();
    
    lbFonts = new QListBox( this );
    for ( i=0; i < (int) fontUseList.count(); i++ )
	 lbFonts->insertItem( fontUseList.at( i )->text() );
    lbFonts->adjustSize();
    lbFonts->setMinimumSize(lbFonts->size());
    
    connect( lbFonts, SIGNAL( highlighted( int ) ),
	     SLOT( slotPreviewFont( int ) ) );
    pushLayout->addWidget(lbFonts, 2);
    
    fntChooser = new KFontChooser( this );
    connect( fntChooser, SIGNAL( fontSelected(const QFont &) ), this,
	     SLOT( slotSetFont(const QFont &) ) );
    pushLayout->addWidget(fntChooser, 5);
    
    lbFonts->setCurrentItem( 0 );
}

KFonts::~KFonts()
{
}

void KFonts::defaults()
{
    int ci = lbFonts->currentItem();
    for ( int i = 0; i < (int) fontUseList.count(); i++ )
	fontUseList.at( i )->setDefault();

    fontUseList.at( ci );
    slotPreviewFont( ci );

    _changed = true;
    emit changed(true);
}

void KFonts::load()
{	
    int ci = lbFonts->currentItem();
    for ( int i = 0; i < (int) fontUseList.count(); i++ )
	fontUseList.at( i )->readFont();

    fontUseList.at( ci );
    slotPreviewFont( ci );

    _changed = true;
    emit changed(false);
}

void KFonts::save()
{
    if ( !_changed )
	return;
	    
    for ( int i = 0; i < (int) fontUseList.count(); i++ )
	fontUseList.at( i )->writeFont();	

    fontUseList.at( lbFonts->currentItem() );

    KIPC::sendMessageAll("KDEChangeGeneral");
    if (useRM) {
	QApplication::setOverrideCursor( waitCursor );
	KProcess proc;
	proc.setExecutable("krdb");
	proc.start( KProcess::Block );
	QApplication::restoreOverrideCursor();
    }

    _changed = false;
    emit changed(false);
}

int KFonts::buttons()
{
    return KCModule::Help | KCModule::Default | KCModule::Reset |
	   KCModule::Cancel | KCModule::Apply | KCModule::Ok;
}

void KFonts::slotSetFont(const QFont &fnt)
{
    fontUseList.current()->setFont( fnt );
    _changed = true;
    emit changed(true);
}

void KFonts::slotPreviewFont( int index )
{
    fntChooser->setFont( fontUseList.at( index )->font(),
			 fontUseList.at( index )->spacing() );
}
