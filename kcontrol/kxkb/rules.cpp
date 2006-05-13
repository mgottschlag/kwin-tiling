#include <qwindowdefs.h>
#include <QFile>
#include <QTextStream>
#include <QRegExp>
#include <QStringList>
#include <QDir>

#include <kstandarddirs.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <config.h>

#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XKBrules.h>

#include "rules.h"
#include <QX11Info>

const char* X11DirList[2] = {"/usr/X11R6/lib/X11/", "/usr/local/X11R6/lib/X11/"};
const char* rulesFileList[2] = {"xkb/rules/xorg", "xkb/rules/xfree86"};

KeyRules::KeyRules():
    m_layouts(90)
{
  for(int ii=0; ii<2; ii++)
    if( QDir(X11DirList[ii]).exists() ) {
	X11_DIR = X11DirList[ii];
	break;
    }

  if( X11_DIR.isEmpty() ) {
    kDebug() << "Cannot find X11 directory!" << endl;
    return;
  }

  // ugly check for XFree 4.3 or older
  if( QDir(X11_DIR + "xkb/symbols/pc").exists() ) {
	m_xfree43 = true;
  }
  else {
	m_xfree43 = false;
  }

  QString rulesFile;
  Display *dpy = QX11Info::display();
  XkbRF_VarDefsRec vd;
  char *tmp= NULL;
  if (XkbRF_GetNamesProp(dpy,&tmp,&vd) && tmp) 
    rulesFile = X11_DIR + QString("xkb/rules/%1").arg(tmp);
  else {
    // old way
    for(int ii=0; ii<2; ii++)
    if( QFile(X11_DIR + QString(rulesFileList[ii])).exists() ) {
	rulesFile = X11_DIR + rulesFileList[ii];
	break;
    }
  }

  if( rulesFile.isEmpty() ) {
    kDebug() << "Cannot find rules file in " << X11_DIR << endl;
    return;
  }

  loadRules(rulesFile);
  loadOldLayouts(rulesFile);
  loadGroups(::locate("config", "kxkb_groups"));
}


void KeyRules::loadRules(QString file)
{
// THIS IS TEMPORARY!!!
// This should be fixed in XFree86 (and actually is fixed in XFree 4.2)
// some handcoded ones, because the X11 rule file doesn't get them correctly, or in case
// the rule file wasn't found
static struct {
    const char * locale;
    const char * layout;
} fixedLayouts[] = {
    { "ben", "Bengali" },
    { "ar", "Arabic" },
    { "ir", "Farsi" },
    { 0, 0 }
};

  XkbRF_RulesPtr rules = XkbRF_Load(QFile::encodeName(file).data(), "", true, true);

  if (rules == NULL) {
     kDebug() << "Unable to load rules" << endl;
     return;
  }

  int i;
  for (i = 0; i < rules->models.num_desc; ++i)
      m_models.replace(rules->models.desc[i].name, qstrdup( rules->models.desc[i].desc ) );
  for (i = 0; i < rules->layouts.num_desc; ++i)
      m_layouts.replace(rules->layouts.desc[i].name, qstrdup( rules->layouts.desc[i].desc ) );
  for (i = 0; i < rules->options.num_desc; ++i)
      m_options.replace(rules->options.desc[i].name, qstrdup( rules->options.desc[i].desc ) );

  // workaround for empty 'compose' options group description
  if( m_options.find("compose:menu") && !m_options.find("compose") ) {
    m_options.replace("compose", "Compose Key");
  }

  // workaround for empty misc options group description in XFree86 4.4.0
  if( m_options.find("numpad:microsoft") && !m_options.find("misc") ) {
    m_options.replace("misc", "Miscellaneous compatibility options" );
  }

  XkbRF_Free(rules, true);

  for(int i=0; fixedLayouts[i].layout != 0; i++ ) {
      if( m_layouts.find(fixedLayouts[i].locale) == 0 )
        m_layouts.insert(fixedLayouts[i].locale, fixedLayouts[i].layout);
  }
}

// check $oldlayouts and $nonlatin groups for XFree 4.3 and later
void KeyRules::loadOldLayouts(QString file)
{
  static const char* oldLayoutsTag = "! $oldlayouts";
  static const char* nonLatinLayoutsTag = "! $nonlatin";
  QFile f(file);
  if (f.open(QIODevice::ReadOnly))
    {
      QTextStream ts(&f);
      QString line;

      m_oldLayouts.clear();
      m_nonLatinLayouts.clear();
      while (!ts.atEnd()) {
	  line = ts.readLine().simplified();

	  if( line.find(oldLayoutsTag) == 0 ) {

	    line = line.mid(strlen(oldLayoutsTag));
	    line = line.mid(line.find('=')+1).simplified();
	    while( !ts.atEnd() && line.endsWith("\\") )
		line = line.left(line.length()-1) + ts.readLine();
	    line = line.simplified();

	    m_oldLayouts = line.split( QRegExp("\\s"),QString::SkipEmptyParts);
//	    kDebug() << "oldlayouts " << m_oldLayouts.join("|") << endl;
	    if( !m_nonLatinLayouts.empty() )
	      break;
	    
	  }
	  else
	  if( line.find(nonLatinLayoutsTag) == 0 ) {

	    line = line.mid(strlen(nonLatinLayoutsTag)+1).simplified();
	    line = line.mid(line.find('=')+1).simplified();
	    while( !ts.atEnd() && line.endsWith("\\") )
		line = line.left(line.length()-1) + ts.readLine();
	    line = line.simplified();

	    m_nonLatinLayouts = line.split(QRegExp( "\\s"),QString::SkipEmptyParts);
//	    kDebug() << "nonlatin " << m_nonLatinLayouts.join("|") << endl;
	    if( !m_oldLayouts.empty() )
	      break;
	    
	  }
      }

      f.close();
    }
}

// for multi-group layouts in XFree 4.2 and older
//    or if layout is present in $oldlayout or $nonlatin groups
void KeyRules::loadGroups(QString file)
{
  QFile f(file);
  if (f.open(QIODevice::ReadOnly))
    {
      QTextStream ts(&f);
      QString locale;
      unsigned int grp;

      while (!ts.atEnd()) {
         ts >> locale >> grp;
	 locale.simplified();

	 if (locale[0] == '#' || locale.left(2) == "//" || locale.isEmpty())
	    continue;

    	 m_initialGroups.insert(locale, grp);
      }
      
      f.close();
    }
}

unsigned int KeyRules::getGroup(const QString& layout, const char* baseGr)
{
// check for new one-group layouts in XFree 4.3 and older
    if( isSingleGroup(layout) ) {
	if( baseGr != 0 && baseGr[0] != '\0' )
	    return 1;
	else
	    return 0;
    }
    
    QMap<QString, unsigned int>::iterator it = m_initialGroups.find(layout);
    return it == m_initialGroups.end() ? 0 : it.data();
}

/*
QStringList KeyRules::rules(QString path)
{
  QStringList result;

  if (path.isEmpty())
    path = X11_DIR + "xkb/rules";

  QDir dir(path);
  dir.setFilter(QDir::Files);
  QStringList list = dir.entryList();
  for (QStringList::Iterator it = list.begin(); it != list.end(); ++it)
    if ((*it).right(4) != ".lst")
      result << *it;

  return result;
}
*/

/* pretty simple algorithm - reads the layout file and
    tries to find "xkb_symbols"
    also checks whether previous line contains "hidden" to skip it
*/
QStringList
KeyRules::getVariants(const QString& layout)
{
    if( layout.isEmpty() || !layouts().find(layout) )
	return QStringList();

  QStringList* result1 = m_varLists[layout];
  if( result1 )
    return *result1;

  QStringList* result = new QStringList();

  QString file = X11_DIR + "xkb/symbols/";
  // workaround for XFree 4.3 new directory for one-group layouts
  if( QDir(file+"pc").exists() && !m_oldLayouts.contains(layout) )
    file += "pc/";
    
  file += layout;

//  kDebug() << "reading variants from " << file << endl;
  
  QFile f(file);
  if (f.open(QIODevice::ReadOnly))
    {
      QTextStream ts(&f);

      QString line;
      QString prev_line;

      while (!ts.atEnd()) {
    	  prev_line = line;
	  line = ts.readLine().simplified();

	    if (line[0] == '#' || line.left(2) == "//" || line.isEmpty())
		continue;

	    int pos = line.find("xkb_symbols");
	    if (pos < 0)
		continue;

	    if( prev_line.find("hidden") >=0 )
		continue;

	    pos = line.find('"', pos) + 1;
	    int pos2 = line.find('"', pos);
	    if( pos < 0 || pos2 < 0 )
		continue;

	    result->append(line.mid(pos, pos2-pos));
//  kDebug() << "adding variant " << line.mid(pos, pos2-pos) << endl;
      }

      f.close();
    }

    m_varLists.insert(layout, result);

    return *result;
}

void KeyRules::parseVariants(const QStringList& vars, Q3Dict<char>& variants, bool chkVars)
{
  static const char* LAYOUT_PATTERN = "[a-z0-9_]*";
  static const char* VARIANT_PATTERN = "\\([a-z0-9_]*\\)";
  for (QStringList::ConstIterator it = vars.begin(); it != vars.end(); ++it)
  {
      QString varLine = (*it).trimmed();
      QRegExp rx(LAYOUT_PATTERN);
      int pos = rx.search(varLine, 0);
      int len = rx.matchedLength();
  // check for errors
      if( pos < 0 || len < 2 )
        continue;
      QString layout = varLine.mid(pos, len);
      rx.setPattern(VARIANT_PATTERN);
      pos = rx.search(varLine, pos+len);
      len = rx.matchedLength();
  // check for errors
      if( pos < 2 || len < 2 )
        continue;
      QString variant_ = varLine.mid(pos+1, len-2);


      QStringList addVars = getVariants(layout);
      if( !chkVars || (!variant_.isEmpty() && addVars.contains(variant_)) )
      {
        variants.replace(layout, strdup(variant_.toLatin1()));
      }
  }
}
