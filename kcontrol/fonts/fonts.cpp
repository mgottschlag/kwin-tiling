// KDE Display fonts setup tab
//
// Copyright (c)  Mark Donohoe 1997
//                lars Knoll 1999
//                Rik Hemsley 2000
//
// Ported to kcontrol2 by Geert Jansen.

/* $Id$ */

#include <qlayout.h>
#include <qlabel.h>
#include <qwhatsthis.h>
#include <qtooltip.h>
#include <qcheckbox.h>
#include <qsettings.h>

// X11 headers
#undef Bool
#undef Unsorted

#include <dcopclient.h>
#include <ksimpleconfig.h>
#include <klocale.h>
#include <kipc.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <kgenericfactory.h>
#include <stdlib.h>

#include "fonts.h"
#include "fonts.moc"

#include <kdebug.h>
#include <qpushbutton.h>

/**** DLL Interface ****/
typedef KGenericFactory<KFonts, QWidget> FontFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_fonts, FontFactory("kcmfonts") );

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
  updateLabel();
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
  updateLabel();
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
    config = new KSimpleConfig(locateLocal("config", _rcfile));
    config->setGroup(_rcgroup);
    config->writeEntry(_rckey, _font);
    config->sync();
    delete config;
  }
}

void FontUseItem::choose()
{
  KFontDialog dlg( prnt, "Font Selector", fixed, true, QStringList(), true );
  dlg.setFont( _font, fixed );
  int result = dlg.exec();
  if (KDialog::Accepted == result)
  {
    _font = dlg.font();
    updateLabel();
    emit changed();
  }
}

void FontUseItem::applyFontDiff( const QFont &fnt, int fontDiffFlags )
{
  if (fontDiffFlags && KFontChooser::FontDiffSize) {
    _font.setPointSize( fnt.pointSize() );
  }
  if (fontDiffFlags && KFontChooser::FontDiffFamily) {
    if (!fixed) _font.setFamily( fnt.family() );
  }
  if (fontDiffFlags && KFontChooser::FontDiffStyle) {
    _font.setBold( fnt.bold() );
    _font.setItalic( fnt.italic() );
    _font.setUnderline( fnt.underline() );
  }
  updateLabel();
}

void FontUseItem::updateLabel()
{
  QString fontDesc = _font.family() + ' ' + QString::number(_font.pointSize());

  preview->setText(fontDesc);
  preview->setFont(_font);
}

/**** KFonts ****/

KFonts::KFonts(QWidget *parent, const char *name, const QStringList &)
    :   KCModule(FontFactory::instance(), parent, name),
        _changed(false)
{
  QStringList nameGroupKeyRc;

  nameGroupKeyRc
    << i18n("General")        << "General"    << "font"         << ""
    << i18n("Fixed width")    << "General"    << "fixed"        << ""
    << i18n("Toolbar")        << "General"    << "toolBarFont"  << ""
    << i18n("Menu")           << "General"    << "menuFont"     << ""
    << i18n("Window title")   << "WM"         << "activeFont"   << ""
    << i18n("Taskbar")        << "General"    << "taskbarFont"  << "";

  QValueList<QFont> defaultFontList;

  // Keep in sync with kglobalsettings.

  QFont f0("helvetica", 12);
  QFont f1("courier", 12);
  QFont f2("helvetica", 10);
  QFont f3("helvetica", 12, QFont::Bold);
  QFont f4("helvetica", 11);

  f0.setPointSize(12);
  f1.setPointSize(10);
  f2.setPointSize(12);
  f3.setPointSize(12);
  f4.setPointSize(11);

  defaultFontList << f0 << f1 << f2 << f0 << f3 << f4;

  QValueList<bool> fixedList;

  fixedList
    <<  false
    <<  true
    <<  false
    <<  false
    <<  false
    <<  false;

  QStringList quickHelpList;

  quickHelpList
    << i18n("Used for normal text (e.g. button labels, list items).")
    << i18n("A non-proportional font (i.e. typewriter font).")
    << i18n("Used to display text beside toolbar icons.")
    << i18n("Used by menu bars and popup menus.")
    << i18n("Used by the window titlebar.")
    << i18n("Used by the taskbar.");

  QVBoxLayout * layout =
    new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());

  QGridLayout * fontUseLayout =
    new QGridLayout(layout, nameGroupKeyRc.count() / 4, 3);

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
    preview->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
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

    QWhatsThis::add(preview, i18n("This is a preview of the \"%1\" font. You can change this font by clicking the \"Choose...\" button to the right.").arg(i->text()));
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

   QHBoxLayout *lay = new QHBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());
   QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
   lay->addItem( spacer );
   QPushButton * fontAdjustButton = new QPushButton(i18n("Adjust All Fonts..."), this);
   QWhatsThis::add(fontAdjustButton, i18n("Click to change all fonts"));
   lay->addWidget( fontAdjustButton );
   connect(fontAdjustButton, SIGNAL(clicked()), this, SLOT(slotApplyFontDiff()));
   layout->addItem(lay);

   cbAA = new QCheckBox( i18n( "Use A&nti-Aliasing for fonts" ),this);

   QWhatsThis::add(cbAA,
     i18n(
       "If this option is selected, KDE will smooth the edges of curves in "
			 "fonts and some images."));

   layout->addWidget(cbAA);

   layout->addStretch(1);

   connect(cbAA, SIGNAL(clicked()), SLOT(slotUseAntiAliasing()));

   useAA = QSettings().readBoolEntry("/qt/useXft");
   useAA_original = useAA;

   cbAA->setChecked(useAA);
}

KFonts::~KFonts()
{
  fontUseList.setAutoDelete(true);
  fontUseList.clear();
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

  useAA = false;
  cbAA->setChecked(useAA);

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
      " Just click the \"Choose\" button that is next to the font you want to"
      " change."
      " You can ask KDE to try and apply font and color settings to non-KDE"
      " applications as well. See the \"Style\" control module for more"
			" information.");
}

void KFonts::load()
{
  for ( int i = 0; i < (int) fontUseList.count(); i++ )
    fontUseList.at( i )->readFont();

  useAA = QSettings().readBoolEntry("/qt/useXft");
  useAA_original = useAA;
  kdDebug() << "AA:" << useAA << endl;
  cbAA->setChecked(useAA);

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
      kdDebug () << "write entry " <<  i->rcKey() << endl;
      config->writeEntry( i->rcKey(), i->font() );
  }
  config->sync();
  delete config;

  QSettings().writeEntry("/qt/useXft", useAA);

  if (useAA)
    QSettings().writeEntry("/qt/enableXft", useAA);

  KIPC::sendMessageAll(KIPC::FontChanged);

  if(useAA != useAA_original) {
    KMessageBox::information(this, i18n("You have changed anti-aliasing related settings.\nThis change will only affect newly started applications."), i18n("Anti-Aliasing Settings Changed"), "AAsettingsChanged", false);
    useAA_original = useAA;
  }
  _changed = false;
  emit changed(false);
}

int KFonts::buttons()
{
  return KCModule::Help | KCModule::Default | KCModule::Apply;
}

void KFonts::slotApplyFontDiff()
{
  QFont font = QFont(fontUseList.first()->font());
  int fontDiffFlags = 0;
  int ret = KFontDialog::getFontDiff(font,fontDiffFlags);

  if (ret && fontDiffFlags) 
  {
    for ( int i = 0; i < (int) fontUseList.count(); i++ )
      fontUseList.at( i )->applyFontDiff( font,fontDiffFlags );
    _changed = true;
    emit changed(true);
  }
}

void KFonts::slotUseAntiAliasing()
{
    useAA = cbAA->isChecked();
    _changed = true;
    emit changed(true);
}

// vim:ts=2:sw=2:tw=78
