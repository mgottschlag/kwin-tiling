/****************************************************************************
**
**
** KRDB - puts current KDE color and font scheme into preprocessor statements
** cats specially written application default files and uses xrdb -merge to
** write to RESOURCE_MANAGER. Thus it gives a  simple way to make non-KDE
** applications fit in with the desktop
**
** Copyright (C) 1998 by Mark Donohoe
** Copyright (C) 1999 by Dirk A. Mueller (reworked for KDE 2.0)
** This application is freely distributable under the GNU Public License.
**
*****************************************************************************/

#include <qdir.h>
#include <qdatastream.h>
#include <qstring.h>
#include <qtextstream.h>

#include <stdlib.h>
#include <time.h>

#include <kapp.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <kprocess.h>

enum FontStyle { Normal, Bold, Italic, Fixed };

// -----------------------------------------------------------------------------

QString fontString( QFont rFont, FontStyle style )
{

  switch(style) {
  case Bold:
    rFont.setBold(true);
    break;
  case Italic:
    rFont.setItalic(true);
    break;
  case Fixed:
    rFont.setFixedPitch(true);
    break;
  case Normal:
    break;
  }

  return rFont.rawName();
}


// -----------------------------------------------------------------------------

void addColorDef(QString& s, const char* n, const QColor& col) 
{
  QString tmp;

  tmp.sprintf("#define %s #%02x%02x%02x\n",
              n, col.red(), col.green(), col.blue());

  s += tmp;
}


// -----------------------------------------------------------------------------

void addFontDef(QString& s, const char* n, const QFont& f, FontStyle fs)
{
  QString tmp;

  tmp.sprintf("#define %s %s\n", n, fontString(f, fs).latin1());
  
  s += tmp;
}


// -----------------------------------------------------------------------------

void copyFile(QFile& tmp, QString const& filename) 
{
  QFile f( filename );
  QString s;
  
  if ( f.open(IO_ReadOnly) ) {
    
    QTextStream t( &f );
    
    while ( !t.eof() ) {
      s += t.readLine() + "\n";
    }
    f.close();
  }
  tmp.writeBlock( s.latin1(), s.length() );
}


// -----------------------------------------------------------------------------

int main( int argc, char ** argv )
{
  KApplication a( argc, argv );
  KGlobal::dirs()->addResourceType("appdefaults", KStandardDirs::kde_default("data") + "kdisplay/app-defaults/");
  QColorGroup cg = a.palette().normal();
  
  QString preproc;
  
  addColorDef(preproc, "FOREGROUND"         , cg.foreground());
  QColor backCol = cg.background();        
  addColorDef(preproc, "BACKGROUND"         , backCol);
  addColorDef(preproc, "HIGHLIGHT"          , backCol.light(100+(2*a.contrast()+4)*16/1));
  addColorDef(preproc, "LOWLIGHT"           , backCol.dark(100+(2*a.contrast()+4)*10));
  addColorDef(preproc, "SELECT_BACKGROUND"  , cg.highlight());
  addColorDef(preproc, "SELECT_FOREGROUND"  , cg.highlightedText());
  addColorDef(preproc, "WINDOW_BACKGROUND"  , cg.base());
  addColorDef(preproc, "WINDOW_FOREGROUND"  , cg.foreground());
  addColorDef(preproc, "INACTIVE_BACKGROUND", a.inactiveTitleColor());
  addColorDef(preproc, "INACTIVE_FOREGROUND", a.inactiveTitleColor());
  //addColorDef(preproc, "INACTIVE_BLEND"     , a.inactiveBlend());
  addColorDef(preproc, "ACTIVE_BACKGROUND"  , a.activeTitleColor());
  addColorDef(preproc, "ACTIVE_FOREGROUND"  , a.activeTitleColor());
  //addColorDef(preproc, "ACTIVE_BLEND"       , a.activeBlend()); 
  
  addFontDef(preproc, "FONT"                , KGlobal::generalFont(), Normal);
  addFontDef(preproc, "BOLD_FONT"           , KGlobal::generalFont(), Bold);
  addFontDef(preproc, "ITALIC_FONT"         , KGlobal::generalFont(), Italic);
  addFontDef(preproc, "FIXED_FONT"          , KGlobal::fixedFont(), Fixed);
  // TITLE_FONT...
  
  // Fontlist
  preproc += "#define FONTLIST FONT,BOLD_FONT=BOLD,ITALIC_FONT=ITALIC\n";
  
  //---------------------------------------------------------------
  
  QStringList list;
  
  QStringList adPaths = KGlobal::dirs()->findDirs("appdefaults", "");
  for (QStringList::ConstIterator it = adPaths.begin(); it != adPaths.end(); it++) {
    QDir dSys( *it );
  
    if ( dSys.exists() ) {
      dSys.setFilter( QDir::Files );
      dSys.setSorting( QDir::Name );
      dSys.setNameFilter("*.ad");
      list += dSys.entryList();
    }
  }

  QString propString;
  
  time_t timestamp;
  ::time( &timestamp );
  
  QString tmpFile;
  tmpFile.sprintf("/tmp/krdb.%d", timestamp);
  
  QFile tmp( tmpFile );
  if ( tmp.open( IO_WriteOnly ) ) {
    tmp.writeBlock( preproc.latin1(), preproc.length() );
  } else {
    debug("Couldn't open temp file");
    exit(0);
  }
  
  for (QStringList::ConstIterator it = list.begin(); it != list.end(); it++)
    copyFile(tmp, locate("appdefaults", *it ));

  // very primitive support for  ~/.Xdefaults by appending it
  copyFile(tmp, QDir::homeDirPath() + "/.Xdefaults");
  
  tmp.close();
  
  KProcess proc;
  
  proc.setExecutable("xrdb");
  proc << "-merge" << tmpFile.latin1();
  
  proc.start( KProcess::Block, KProcess::Stdin );
  
  QDir d("/tmp");
  d.remove( tmpFile );
}
