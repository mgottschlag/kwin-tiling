#include <qimage.h>
#include <qfont.h>
#include <qpainter.h>
#include <qregexp.h>
#include <q3dict.h>
//Added by qt3to4:
#include <QPixmap>

#include <kstandarddirs.h>
#include <klocale.h>

#include "pixmap.h"


Q3Dict<QPixmap> LayoutIcon::pixmaps(80);
const QString LayoutIcon::flagTemplate("l10n/%1/flag.png");
bool LayoutIcon::cacheWithFlags = true;


const QPixmap&
LayoutIcon::findPixmap(const QString& code_, bool showFlag)
{
    if( showFlag != cacheWithFlags ) {
	cacheWithFlags = showFlag;
	pixmaps.clear();
    }

  QPixmap* pm = pixmaps[code_];
  if( pm )
    return *pm;

  if( code_ == "error" ) {
    pm = new QPixmap(21, 14);
    pm->fill(Qt::white);

    QPainter p(pm);
  
    QFont font("sans");
    font.setPixelSize(10);
    font.setWeight(QFont::Bold);
    p.setFont(font);
    p.setPen(Qt::red);
    p.drawText(2, 1, pm->width(), pm->height()-2, Qt::AlignCenter, "err");
    p.setPen(Qt::blue);
    p.drawText(1, 0, pm->width(), pm->height()-2, Qt::AlignCenter, "err");
    pixmaps.insert(code_, pm);

    return *pm;
  }


  QString code = code_.mid(0, code_.find(QRegExp("[()+]")));	//co_foo,co(foo),co+foo
  code = code.mid(code.find('/') + 1);	//NEC/jp

  QString flag;
  int pos = code.length();
  if( code_ == "ar" )	// Arabic - not argentina
    flag = locate("locale", flagTemplate.arg("C"));
  else
  if( code_ == "sr" )	// Serbian language - Yugoslavia
    flag = locate("locale", flagTemplate.arg("yu"));
  else
  if( code_ == "bs" )	// Bosnian language - Bosnia
    flag = locate("locale", flagTemplate.arg("ba"));
  else
  if( code_ == "la" )	// Latin America
    flag = locate("locale", flagTemplate.arg("C"));
  else
  if( code_ == "lo" )	// Lao
    flag = locate("locale", flagTemplate.arg("la"));
  else
  if( code_ == "ml" || code_ == "dev" || code_ == "gur" 
	|| code_ == "guj" || code_ == "kan" || code_ == "ori" 
	|| code_ == "tel" || code_ == "tml" || code_ == "ben" ) // some Indian languages
    flag = locate("locale", flagTemplate.arg("in"));
  else {
    flag = locate("locale", flagTemplate.arg(code.toLower()));
    if (flag.isEmpty()) {
      pos = code.find("_");
      if (pos > 0 && code.find("intl") < 1) { // so "us_intl" != "us" layout
        flag = locate("locale", flagTemplate.arg(code.mid(pos+1).toLower()));
        if (flag.isEmpty())
        flag = locate("locale", flagTemplate.arg(code.left(pos).toLower()));
      }
      //this is a tiny patch for the el (Hellenic <=> Greek) keyboard
      //which is not named after the country, but after the language
      //unlike all others.
      //remove if it causes trouble, but pls contact first
      // d.kamenopoulos@mail.ntua.gr
      else if (code.toLower() == "el")
        flag = locate("locale", flagTemplate.arg("gr"));
      //end of patch
    }
  }
  
  if (flag.isEmpty())
    flag = locate("locale", flagTemplate.arg("C"));

  if (flag.isEmpty() || !showFlag ) {
    pm = new QPixmap(21, 14);
    pm->fill(Qt::white);
  }
  else
    pm = new QPixmap(flag);


  QImage image = pm->toImage();
  for (int y=0; y<image.height(); y++)
    for(int x=0; x<image.width(); x++)
      {
	QRgb rgb = image.pixel(x,y);
	image.setPixel(x,y,qRgb(qRed(rgb)*3/4,qGreen(rgb)*3/4,qBlue(rgb)*3/4));
      }
  pm->convertFromImage(image);

  QPainter p(pm);
  
  // some workaround for layouts which have poor representation with two letters
  if( ( code.length() == 3 && code.find(QRegExp("[^a-zA-Z0-9]")) == -1 )
	|| code_ == "dvorak" ) {
    code = code.left(3);
    QFont font("sans");
    font.setPixelSize(10);
    font.setWeight(QFont::Bold);
    p.setFont(font);
    p.setPen(Qt::black);
    p.drawText(2, 1, pm->width(), pm->height()-2, Qt::AlignCenter, code);
    p.setPen(Qt::white);
    p.drawText(1, 0, pm->width(), pm->height()-2, Qt::AlignCenter, code);
  }
  else {
    code = code.left(pos).right(3);
    QFont font("sans");
    font.setPixelSize(10);
    font.setWeight(QFont::Bold);
    p.setFont(font);
    p.setPen(Qt::black);
    p.drawText(1, 1, pm->width(), pm->height()-2, Qt::AlignCenter, code);
    p.setPen(Qt::white);
    p.drawText(0, 0, pm->width(), pm->height()-2, Qt::AlignCenter, code);
  }

  pixmaps.insert(code_, pm);

  return *pm;
}


// Note: this seems stupid, but allows for translations
#if 0
   I18N_NOOP("Belgian");
   I18N_NOOP("Bulgarian");
   I18N_NOOP("Brazilian");
   I18N_NOOP("Canadian");
   I18N_NOOP("Czech");
   I18N_NOOP("Czech (qwerty)");
   I18N_NOOP("Danish");
   I18N_NOOP("Estonian");
   I18N_NOOP("Finnish");
   I18N_NOOP("French");
   I18N_NOOP("German");
   I18N_NOOP("Hungarian");
   I18N_NOOP("Hungarian (qwerty)");
   I18N_NOOP("Italian");
   I18N_NOOP("Japanese");
   I18N_NOOP("Lithuanian");
   I18N_NOOP("Norwegian");
   I18N_NOOP("PC-98xx Series");
   I18N_NOOP("Polish");
   I18N_NOOP("Portuguese");
   I18N_NOOP("Romanian");
   I18N_NOOP("Russian");
   I18N_NOOP("Slovak");
   I18N_NOOP("Slovak (qwerty)");
   I18N_NOOP("Spanish");
   I18N_NOOP("Swedish");
   I18N_NOOP("Swiss German");
   I18N_NOOP("Swiss French");
   I18N_NOOP("Thai");
   I18N_NOOP("United Kingdom");
   I18N_NOOP("U.S. English");
   I18N_NOOP("U.S. English w/ deadkeys");
   I18N_NOOP("U.S. English w/ISO9995-3");

  //lukas: these seem to be new in XF 4.0.2
   I18N_NOOP("Armenian");
   I18N_NOOP("Azerbaijani");
   I18N_NOOP("Icelandic");
   I18N_NOOP("Israeli");
   I18N_NOOP("Lithuanian azerty standard");
   I18N_NOOP("Lithuanian querty \"numeric\"");	     //for bw compatibility
   I18N_NOOP("Lithuanian querty \"programmer's\"");
   I18N_NOOP("Macedonian");
   I18N_NOOP("Serbian");
   I18N_NOOP("Slovenian");
   I18N_NOOP("Vietnamese");

  //these seem to be new in XFree86 4.1.0
   I18N_NOOP("Arabic");
   I18N_NOOP("Belarusian");
   I18N_NOOP("Bengali");
   I18N_NOOP("Croatian");
   I18N_NOOP("Greek");
   I18N_NOOP("Latvian");
   I18N_NOOP("Lithuanian qwerty \"numeric\"");
   I18N_NOOP("Lithuanian qwerty \"programmer's\"");
   I18N_NOOP("Turkish");
   I18N_NOOP("Ukrainian");

  //these seem to be new in XFree86 4.2.0
   I18N_NOOP("Albanian");
   I18N_NOOP("Burmese");
   I18N_NOOP("Dutch");
   I18N_NOOP("Georgian (latin)");
   I18N_NOOP("Georgian (russian)");
   I18N_NOOP("Gujarati");
   I18N_NOOP("Gurmukhi");
   I18N_NOOP("Hindi");
   I18N_NOOP("Inuktitut");
   I18N_NOOP("Iranian");
//   I18N_NOOP("Iranian"); // should be not Iranian but Farsi
   I18N_NOOP("Latin America");
   I18N_NOOP("Maltese");
   I18N_NOOP("Maltese (US layout)");
   I18N_NOOP("Northern Saami (Finland)");
   I18N_NOOP("Northern Saami (Norway)");
   I18N_NOOP("Northern Saami (Sweden)");
   I18N_NOOP("Polish (qwertz)");
   I18N_NOOP("Russian (cyrillic phonetic)");
   I18N_NOOP("Tajik");
   I18N_NOOP("Turkish (F)");
   I18N_NOOP("U.S. English w/ ISO9995-3");
   I18N_NOOP("Yugoslavian");

  //these seem to be new in XFree86 4.3.0
   I18N_NOOP("Bosnian");
   I18N_NOOP("Croatian (US)");
   I18N_NOOP("Dvorak");
   I18N_NOOP("French (alternative)");
   I18N_NOOP("French Canadian");
   I18N_NOOP("Kannada");
   I18N_NOOP("Lao");
   I18N_NOOP("Malayalam");
   I18N_NOOP("Mongolian");
   I18N_NOOP("Ogham");
   I18N_NOOP("Oriya");
   I18N_NOOP("Syriac");
   I18N_NOOP("Telugu");
   I18N_NOOP("Thai (Kedmanee)");
   I18N_NOOP("Thai (Pattachote)");
   I18N_NOOP("Thai (TIS-820.2538)");

  //these seem to be new in XFree86 4.4.0
   I18N_NOOP("Uzbek");
   I18N_NOOP("Faroese");
#endif
