// KDE Display fonts setup tab
//
// Copyright (c)  Mark Donohoe 1997
//                lars Knoll 1999
//                Rik Hemsley 2000
//
// Ported to kcontrol2 by Geert Jansen.

/* $Id$ */

#include <qlayout.h>
#include <qlistbox.h>
#include <qlabel.h>
#include <qapplication.h>
#include <qwhatsthis.h>
#include <qtooltip.h>
#include <qframe.h>

// X11 headers
#undef Bool
#undef Unsorted

#include <kfontdialog.h>
#include <kconfig.h>
#include <ksimpleconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <kipc.h>
#include <kstddirs.h>
#include <kprocess.h>
#include <stdlib.h>

#include "fonts.h"
#include "fonts.moc"

/**** DLL Interface ****/

extern "C" {
  KCModule *create_fonts(QWidget *parent, const char *name) {
    KGlobal::locale()->insertCatalogue("kcmfonts");
    return new KFonts(parent, name);
  }
}


/**** FontUseItem ****/

FontUseItem::FontUseItem(
  QWidget * parent,
  QLabel * prvw,
  QString n,
  QString grp,
  QString key,
  QString rc,
  QFont default_fnt,
  bool f
)
  : QObject(),
    prnt(parent),
    preview(prvw),
    _text(n),
    _rcfile(rc),
    _rcgroup(grp),
    _rckey(key),
    _font(default_fnt),
    _default(default_fnt),
    fixed(f)
{
  readFont();
  updateLabel();
}

QString FontUseItem::fontString(QFont rFont)
{
  QString aValue;
  aValue = rFont.rawName();
  return aValue;
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

void FontUseItem::choose()
{
  int result = KFontDialog::getFont(_font, fixed, prnt);

  if (KDialog::Accepted == result) {
    updateLabel();
    emit changed();
  }
}

void FontUseItem::updateLabel()
{
  QString fontDesc = _font.family() + ' ' + QString::number(_font.pointSize()) + ' ' + QFont::encodingName(_font.charSet());

  preview->setText(fontDesc);
  preview->setFont(_font);
}

/**** KFonts ****/

KFonts::KFonts(QWidget *parent, const char *name)
    :   KCModule(parent, name),
        _changed(false)
{
  QStringList nameGroupKeyRc;

  nameGroupKeyRc
    << i18n("General")        << "General"    << "font"         << ""
    << i18n("Fixed width")    << "General"    << "fixed"        << ""
    << i18n("Desktop icon")   << "FMSettings" << "StandardFont" << "kdesktoprc"
    << i18n("File manager")   << "FMSettings" << "StandardFont" << "konquerorrc"
    << i18n("Toolbar")        << "General"    << "toolBarFont"  << ""
    << i18n("Menu")           << "General"    << "menuFont"     << ""
    << i18n("Window title")   << "WM"         << "activeFont"   << "";
// Disabled these two for beta release, as they don't do anything.
//    << i18n("Taskbar button") << "General"    << "taskbarFont"  << "kickerrc"
//    << i18n("Panel clock")    << "General"    << "dateFont"     << "kickerrc";

  QValueList<QFont> defaultFontList;

  QFont defaultFont("helvetica", 12);

  defaultFontList
    <<  defaultFont
    <<  defaultFont
    <<  defaultFont
    <<  defaultFont
    <<  QFont("helvetica", 10)
    <<  defaultFont
    <<  QFont("helvetica", 12, QFont::Bold)
    <<  defaultFont
    <<  QFont("helvetica", 10);

  QValueList<bool> fixedList;

  fixedList
    <<  false
    <<  true
    <<  false
    <<  false
    <<  false
    <<  false
    <<  false
    <<  false
    <<  false;

  QStringList quickHelpList;

  quickHelpList
    << i18n("Used for normal text (e.g. button labels, list items).")
    << i18n("A non-proportional font (i.e. typewriter font).")
    << i18n("Used to display the names of icons on the desktop.")
    << i18n("Used to display the names of icons in the file manager.")
    << i18n("Used to display text beside toolbar icons.")
    << i18n("Used by menu bars and popup menus.")
    << i18n("Used by the window titlebar.")
    << i18n("Used by panel taskbar buttons.")
    << i18n("Used by the panel clock.");

  QGridLayout * fontUseLayout =
    new QGridLayout(
      this,
      nameGroupKeyRc.count() / 4,
      3,
      KDialog::marginHint(),
      KDialog::spacingHint()
    );

  fontUseLayout->setColStretch(0, 0);
  fontUseLayout->setColStretch(1, 1);
  fontUseLayout->setColStretch(2, 0);

  QValueList<QFont>::ConstIterator defaultFontIt(defaultFontList.begin());
  QValueList<bool>::ConstIterator fixedListIt(fixedList.begin());
  QStringList::ConstIterator quickHelpIt(quickHelpList.begin());
  QStringList::ConstIterator it(nameGroupKeyRc.begin());

  unsigned int count = 0;

  while (it != nameGroupKeyRc.end()) {

    QLabel * preview = new QLabel(this);
    preview->setFrameStyle(QFrame::Box | QFrame::Plain);
    // preview->setMaximumWidth(200);

    FontUseItem * i =
      new FontUseItem(
        this,
        preview,
        *it++,
        *it++,
        *it++,
        *it++,
        *defaultFontIt++,
        *fixedListIt++
      );

    fontUseList.append(i);
    connect(i, SIGNAL(changed()), this, SLOT(fontChanged()));

    QWhatsThis::add(preview, i18n("This is a preview of the \"%1\" font. You can change this font by using the \"Choose...\" button at right.").arg(i->text()));
    QToolTip::add(preview, i18n("Preview of the \"%1\" font").arg(i->text()));

    QLabel * fontUse = new QLabel(i->text(), this);

    QWhatsThis::add(fontUse, *quickHelpIt++);

    QPushButton * chooseButton = new QPushButton(i18n("Choose..."), this);

    QWhatsThis::add(chooseButton, i18n("Click to select a font"));

    connect(chooseButton, SIGNAL(clicked()), i, SLOT(choose()));

    fontUseLayout->addWidget(fontUse, count, 0);
    fontUseLayout->addWidget(preview, count, 1);
    fontUseLayout->addWidget(chooseButton, count, 2);

    ++count;
  }

  fontUseLayout->setRowStretch( count, 1 );
}

KFonts::~KFonts()
{
}

void KFonts::fontChanged()
{
  _changed = true;
  emit changed(true);
}

void KFonts::defaults()
{
  for ( int i = 0; i < (int) fontUseList.count(); i++ )
    fontUseList.at( i )->setDefault();

  _changed = true;
  emit changed(true);
}

QString KFonts::quickHelp() const
{
    return i18n( "<h1>Fonts</h1> This module allows you to choose which"
      " fonts will be used to display text in KDE. You can select not only"
      " the font family (for example, <em>helvetica</em> or <em>times</em>),"
      " but also the attributes that make up a specific font (for example,"
      " <em>bold</em> style and <em>12 points</em> in height.)<p>"
      " Just click the \"Choose\" button that next to the font you want to"
      " change."
      " You can ask KDE to try and apply font and color settings to non-KDE"
      " applications as well. See the \"Style\" control module under"
      " \"Themes\" for more information.");
}

void KFonts::load()
{
  for ( int i = 0; i < (int) fontUseList.count(); i++ )
    fontUseList.at( i )->readFont();

  _changed = true;
  emit changed(false);
}

void KFonts::save()
{
  if ( !_changed )
    return;

  for ( FontUseItem* i = fontUseList.first(); i; i = fontUseList.next() )
      i->writeFont();

  // KDE-1.x support
  KSimpleConfig* config = new KSimpleConfig( QCString(::getenv("HOME")) + "/.kderc" );
  config->setGroup( "General" );
  for ( FontUseItem* i = fontUseList.first(); i; i = fontUseList.next() ) {
      qDebug("write entry %s", i->rcKey().latin1() );
      config->writeEntry( i->rcKey(), i->font() );
  }
  config->sync();
  delete config;

  KIPC::sendMessageAll(KIPC::FontChanged);

  KConfig * cfg = KGlobal::config();
  cfg->setGroup("X11");

  if (cfg->readBoolEntry("useResourceManager", true)) {
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

// vim:ts=2:sw=2:tw=78
