// KDE Display fonts setup tab
//
// Copyright (c)  Mark Donohoe 1997
//                lars Knoll 1999
//                Rik Hemsley 2000
//
// Ported to kcontrol2 by Geert Jansen.

/* $Id$ */

#include <qlayout.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qwhatsthis.h>
#include <qtooltip.h>
#include <qcheckbox.h>
#include <qsettings.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <knuminput.h>
#include <qpushbutton.h>

#include <dcopclient.h>
#include <kapplication.h>
#include <ksimpleconfig.h>
#include <kipc.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <kgenericfactory.h>
#include <stdlib.h>

#include "fonts.h"
#include "fonts.moc"

#include <kdebug.h>

#include <X11/Xlib.h>

// X11 headers
#undef Bool
#undef Unsorted
#undef None

static const char *aa_rgb_xpm[]={
"12 12 3 1",
"a c #0000ff",
"# c #00ff00",
". c #ff0000",
"....####aaaa",
"....####aaaa",
"....####aaaa",
"....####aaaa",
"....####aaaa",
"....####aaaa",
"....####aaaa",
"....####aaaa",
"....####aaaa",
"....####aaaa",
"....####aaaa",
"....####aaaa"};
static const char *aa_bgr_xpm[]={
"12 12 3 1",
". c #0000ff",
"# c #00ff00",
"a c #ff0000",
"....####aaaa",
"....####aaaa",
"....####aaaa",
"....####aaaa",
"....####aaaa",
"....####aaaa",
"....####aaaa",
"....####aaaa",
"....####aaaa",
"....####aaaa",
"....####aaaa",
"....####aaaa"};
static const char *aa_vrgb_xpm[]={
"12 12 3 1",
"a c #0000ff",
"# c #00ff00",
". c #ff0000",
"............",
"............",
"............",
"............",
"############",
"############",
"############",
"############",
"aaaaaaaaaaaa",
"aaaaaaaaaaaa",
"aaaaaaaaaaaa",
"aaaaaaaaaaaa"};
static const char *aa_vbgr_xpm[]={
"12 12 3 1",
". c #0000ff",
"# c #00ff00",
"a c #ff0000",
"............",
"............",
"............",
"............",
"############",
"############",
"############",
"############",
"aaaaaaaaaaaa",
"aaaaaaaaaaaa",
"aaaaaaaaaaaa",
"aaaaaaaaaaaa"};

static QPixmap aaPixmaps[]={ QPixmap(aa_rgb_xpm), QPixmap(aa_bgr_xpm),
                             QPixmap(aa_vrgb_xpm), QPixmap(aa_vbgr_xpm) };

/**** DLL Interface ****/
typedef KGenericFactory<KFonts, QWidget> FontFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_fonts, FontFactory("kcmfonts") );

/**** FontUseItem ****/

FontUseItem::FontUseItem(
  QWidget * parent,
  const QString &name,
  const QString &grp,
  const QString &key,
  const QString &rc,
  const QFont &default_fnt,
  bool f
)
  : KFontRequester(parent, 0L, f),
    _rcfile(rc),
    _rcgroup(grp),
    _rckey(key),
    _default(default_fnt)
{
  setTitle( name );
  readFont();
}

void FontUseItem::setDefault()
{
  setFont( _default, isFixedOnly() );
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
  QFont tmpFnt(_default);
  setFont( config->readFontEntry(_rckey, &tmpFnt), isFixedOnly() );
  if (deleteme) delete config;
}

void FontUseItem::writeFont()
{
  KConfigBase *config;

  if (_rcfile.isEmpty()) {
    config = KGlobal::config();
    config->setGroup(_rcgroup);
    config->writeEntry(_rckey, font(), true, true);
  } else {
    config = new KSimpleConfig(locateLocal("config", _rcfile));
    config->setGroup(_rcgroup);
    config->writeEntry(_rckey, font());
    config->sync();
    delete config;
  }
}

void FontUseItem::applyFontDiff( const QFont &fnt, int fontDiffFlags )
{
  QFont _font( font() );

  if (fontDiffFlags & KFontChooser::FontDiffSize) {
    _font.setPointSize( fnt.pointSize() );
  }
  if (fontDiffFlags & KFontChooser::FontDiffFamily) {
    if (!isFixedOnly()) _font.setFamily( fnt.family() );
  }
  if (fontDiffFlags & KFontChooser::FontDiffStyle) {
    _font.setBold( fnt.bold() );
    _font.setItalic( fnt.italic() );
    _font.setUnderline( fnt.underline() );
  }

  setFont( _font, isFixedOnly() );
}

/**** KFonts ****/

static QCString desktopConfigName()
{
  int desktop=0;
  if (qt_xdisplay())
    desktop = DefaultScreen(qt_xdisplay());
  QCString name;
  if (desktop == 0)
    name = "kdesktoprc";
  else
    name.sprintf("kdesktop-screen-%drc", desktop);

  return name;
}

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
    << i18n("Taskbar")        << "General"    << "taskbarFont"  << ""
    << i18n("Desktop")        << "FMSettings" << "StandardFont" << desktopConfigName();

  QValueList<QFont> defaultFontList;

  // Keep in sync with kdelibs/kdecore/kglobalsettings.cpp

  QFont f0("helvetica", 12);
  QFont f1("courier", 12);
  QFont f2("helvetica", 10);
  QFont f3("helvetica", 12, QFont::Bold);
  QFont f4("helvetica", 11);

  f0.setPointSize(12);
  f1.setPointSize(12);
  f2.setPointSize(10);
  f3.setPointSize(12);
  f4.setPointSize(11);

  defaultFontList << f0 << f1 << f2 << f0 << f3 << f4 << f0;

  QValueList<bool> fixedList;

  fixedList
    <<  false
    <<  true
    <<  false
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
    << i18n("Used by the taskbar.")
    << i18n("Used for desktop icons.");

  QVBoxLayout * layout =
    new QVBoxLayout(this, 0, KDialog::spacingHint());

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

    QString name = *it; it++;
    QString group = *it; it++;
    QString key = *it; it++;
    QString file = *it; it++;

    FontUseItem * i =
      new FontUseItem(
        this,
        name,
        group,
        key,
        file,
        *defaultFontIt++,
        *fixedListIt++
      );

    fontUseList.append(i);
    connect(i, SIGNAL(fontSelected(const QFont &)), SLOT(fontSelected()));

    QLabel * fontUse = new QLabel(name+":", this);
    QWhatsThis::add(fontUse, *quickHelpIt++);

    fontUseLayout->addWidget(fontUse, count, 0);
    fontUseLayout->addWidget(i, count, 1);

    ++count;
  }

   QHBoxLayout *lay = new QHBoxLayout(layout, KDialog::spacingHint());
   lay->addStretch();
   QPushButton * fontAdjustButton = new QPushButton(i18n("Ad&just All Fonts..."), this);
   QWhatsThis::add(fontAdjustButton, i18n("Click to change all fonts"));
   lay->addWidget( fontAdjustButton );
   connect(fontAdjustButton, SIGNAL(clicked()), SLOT(slotApplyFontDiff()));

   QGroupBox *aaBox=new QGroupBox(i18n("An&ti-Aliasing"), this);

   aaBox->setColumnLayout(1, Qt::Horizontal);
   aaBox->layout()->setSpacing(KDialog::spacingHint());

   cbAA = new QCheckBox( i18n( "Use a&nti-aliasing for fonts" ), aaBox);
   QWhatsThis::add(cbAA, i18n("If this option is selected, KDE will smooth the edges of curves in "
                              "fonts."));

   QHBox *hbox =  new QHBox(aaBox);
   QSpacerItem *spacer = new QSpacerItem( 20, 0, QSizePolicy::Fixed, QSizePolicy::Minimum );
   hbox->layout()->addItem(spacer);
   aaExcludeRange=new QCheckBox(i18n("E&xclude range:"), hbox),
   aaExcludeFrom=new KDoubleNumInput(0, 72, 8.0, 1, 1, hbox),
   aaExcludeFrom->setSuffix(i18n(" pt"));
   aaExcludeToLabel = new QLabel(i18n(" to "), hbox);
   aaExcludeTo=new KDoubleNumInput(0, 72, 15.0, 1, 1, hbox);
   aaExcludeTo->setSuffix(i18n(" pt"));
   QWidget *dummy = new QWidget(hbox);
   hbox->setStretchFactor(dummy, 1);

   hbox =  new QHBox(aaBox);
   spacer = new QSpacerItem( 20, 0, QSizePolicy::Fixed, QSizePolicy::Minimum );
   hbox->layout()->addItem(spacer);
   aaUseSubPixel=new QCheckBox(i18n("&Use sub-pixel hinting:"), hbox);
   aaSubPixelType=new QComboBox(false, hbox);
   dummy = new QWidget(hbox);
   hbox->setStretchFactor(dummy, 1);

   for(int t=KXftConfig::SubPixel::None+1; t<=KXftConfig::SubPixel::Vbgr; ++t)
       aaSubPixelType->insertItem(aaPixmaps[t-1], KXftConfig::description((KXftConfig::SubPixel::Type)t));

   setAaWidgets();

   connect(aaExcludeRange, SIGNAL(toggled(bool)),
           this, SLOT(slotAaChange()));
   connect(aaUseSubPixel, SIGNAL(toggled(bool)),
           this, SLOT(slotAaChange()));
   connect(aaExcludeFrom, SIGNAL(valueChanged(double)),
           this, SLOT(slotAaChange()));
   connect(aaExcludeTo, SIGNAL(valueChanged(double)),
           this, SLOT(slotAaChange()));
   connect(aaSubPixelType, SIGNAL(activated(const QString &)),
           this, SLOT(slotAaChange()));

   layout->addWidget(aaBox);

   layout->addStretch(1);

   connect(cbAA, SIGNAL(clicked()), SLOT(slotUseAntiAliasing()));

   useAA = QSettings().readBoolEntry("/qt/useXft");
   useAA_original = useAA;

   enableAaWidgets();

   cbAA->setChecked(useAA);
}

KFonts::~KFonts()
{
  fontUseList.setAutoDelete(true);
  fontUseList.clear();
}

void KFonts::fontSelected()
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

  aaExcludeRange->setChecked(true);
  aaExcludeFrom->setValue(8.0);
  aaExcludeTo->setValue(15.0);
  aaUseSubPixel->setChecked(false);
  enableAaWidgets();

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
  for ( uint i = 0; i < fontUseList.count(); i++ )
    fontUseList.at( i )->readFont();

  useAA = QSettings().readBoolEntry("/qt/useXft");
  useAA_original = useAA;
  kdDebug(1208) << "AA:" << useAA << endl;
  cbAA->setChecked(useAA);

  setAaWidgets();

  _changed = true;
  emit changed(false);
}

void KFonts::save()
{
  if ( !_changed )
    return;

  _changed = false;

  for ( FontUseItem* i = fontUseList.first(); i; i = fontUseList.next() )
      i->writeFont();

  KGlobal::config()->sync();

  // KDE-1.x support
  KSimpleConfig* config = new KSimpleConfig( QCString(::getenv("HOME")) + "/.kderc" );
  config->setGroup( "General" );
  for ( FontUseItem* i = fontUseList.first(); i; i = fontUseList.next() ) {
      if("font"==i->rcKey())
          QSettings().writeEntry("/qt/font", i->font().toString());
      kdDebug(1208) << "write entry " <<  i->rcKey() << endl;
      config->writeEntry( i->rcKey(), i->font() );
  }
  config->sync();
  delete config;

  QSettings().writeEntry("/qt/useXft", useAA);

  if (useAA)
    QSettings().writeEntry("/qt/enableXft", useAA);

  KIPC::sendMessageAll(KIPC::FontChanged);

  kapp->processEvents(); // Process font change ourselves

  KXftConfig xft(KXftConfig::ExcludeRange|KXftConfig::SubPixelType);

  if(aaExcludeRange->isChecked())
      xft.setExcludeRange(aaExcludeFrom->value()-1,
                          aaExcludeTo->value()+1);
  else
      xft.setExcludeRange(0, 0);

  if(aaUseSubPixel->isChecked())
      xft.setSubPixelType(getAaSubPixelType());
  else
      xft.setSubPixelType(KXftConfig::SubPixel::None);

  if((useAA != useAA_original) || xft.changed()) {
    KMessageBox::information(this,
      i18n(
        "<p>You have changed anti-aliasing related settings. This change will only affect newly started applications.</p>"
      ), i18n("Anti-Aliasing Settings Changed"), "AAsettingsChanged", false);
    useAA_original = useAA;
  }

  xft.apply();
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

  if (ret == KDialog::Accepted && fontDiffFlags)
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
    enableAaWidgets();
    _changed = true;
    emit changed(true);
}

void KFonts::slotAaChange()
{
    enableAaWidgets();

    _changed = true;
    emit changed(true);
}

void KFonts::setAaWidgets()
{
   double aaFrom, aaTo;
   KXftConfig     xft(KXftConfig::ExcludeRange|KXftConfig::SubPixelType);

   if(xft.getExcludeRange(aaFrom, aaTo))
       aaExcludeRange->setChecked(true);
   else
   {
       aaExcludeRange->setChecked(false);
       aaFrom=8.0;
       aaTo=15.0;
   }

   aaExcludeFrom->setValue(aaFrom+1);
   aaExcludeTo->setValue(aaTo-1);

   KXftConfig::SubPixel::Type aaSpType;

   if(!xft.getSubPixelType(aaSpType) || KXftConfig::SubPixel::None==aaSpType)
       aaUseSubPixel->setChecked(false);
   else
   {
       int idx=getIndex(aaSpType);

       if(idx>-1)
       {
           aaUseSubPixel->setChecked(true);
           aaSubPixelType->setCurrentItem(idx);
       }
       else
           aaUseSubPixel->setChecked(false);
   }

   enableAaWidgets();
}

int KFonts::getIndex(KXftConfig::SubPixel::Type aaSpType)
{
    int pos=-1;
    int index;

    for(index=0; index<aaSubPixelType->count(); ++index)
        if(aaSubPixelType->text(index)==KXftConfig::description(aaSpType))
        {
            pos=index;
            break;
        }

    return pos;
}

KXftConfig::SubPixel::Type KFonts::getAaSubPixelType()
{
    int t;

    for(t=KXftConfig::SubPixel::None; t<=KXftConfig::SubPixel::Vbgr; ++t)
        if(aaSubPixelType->currentText()==KXftConfig::description((KXftConfig::SubPixel::Type)t))
            return (KXftConfig::SubPixel::Type)t;

    return KXftConfig::SubPixel::None;
}

void KFonts::enableAaWidgets()
{
    aaExcludeRange->setEnabled(useAA);
    aaExcludeFrom->setEnabled(aaExcludeRange->isChecked() && useAA);
    aaExcludeTo->setEnabled(aaExcludeRange->isChecked() && useAA);
    aaExcludeToLabel->setEnabled(aaExcludeRange->isChecked() && useAA);
    aaUseSubPixel->setEnabled(useAA);
    aaSubPixelType->setEnabled(aaUseSubPixel->isChecked() && useAA);
}

// vim:ts=2:sw=2:tw=78
