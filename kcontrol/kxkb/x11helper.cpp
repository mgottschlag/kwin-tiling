#include <qwindowdefs.h>
#include <QDir>
#include <QString>
#include <QString>
#include <QStringList>
#include <QHash>
#include <QRegExp>
#include <QTextStream>

#include <kdebug.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#define explicit int_explicit        // avoid compiler name clash in XKBlib.h
#include <X11/XKBlib.h>
#undef explicit
#include <X11/extensions/XKBrules.h>

#include "x11helper.h"
#include "config.h"

#ifndef HAVE_XKLAVIER

// Compiler will size array automatically.
static const char* X11DirList[] =
    {
        XLIBDIR,
        "/etc/X11/",
        "/usr/share/X11/",
        "/usr/local/share/X11/",
        "/usr/X11R6/lib/X11/",
        "/usr/X11R6/lib64/X11/",
        "/usr/local/X11R6/lib/X11/",
        "/usr/local/X11R6/lib64/X11/",
        "/usr/lib/X11/",
        "/usr/lib64/X11/",
        "/usr/local/lib/X11/",
        "/usr/local/lib64/X11/"
    };

// Compiler will size array automatically.
static const char* rulesFileList[] =
    {
	"xkb/rules/xorg",
	"xkb/rules/xfree86"
    };

// Macro will return number of elements in any static array as long as the
// array has at least one element.
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

static const int X11_DIR_COUNT = ARRAY_SIZE(X11DirList);
static const int X11_RULES_COUNT = ARRAY_SIZE(rulesFileList);

const QString
X11Helper::findX11Dir()
{
	for(int ii=0; ii<X11_DIR_COUNT; ii++) {
		const char* xDir = X11DirList[ii];
		if( xDir != NULL && QDir(QString(xDir) + "xkb").exists() ) {
//			for(int jj=0; jj<X11_RULES_COUNT; jj++) {
//				
//			}
			return QString(xDir);
	    }
	
//	  if( X11_DIR.isEmpty() ) {
//	    return;
//	  }
	}
	return NULL;	
}

const QString
X11Helper::findXkbRulesFile(QString x11Dir, Display *dpy)
{
	QString rulesFile;
	XkbRF_VarDefsRec vd;
	char *tmp = NULL;

	if (XkbRF_GetNamesProp(dpy, &tmp, &vd) && tmp != NULL ) {
// 			kDebug() << "namesprop " << tmp  << endl;
	  	rulesFile = x11Dir + QString("xkb/rules/%1").arg(tmp);
// 			kDebug() << "rulesF " << rulesFile  << endl;
	}
	else {
    // old way
    	for(int ii=0; ii<X11_RULES_COUNT; ii++) {
    		const char* ruleFile = rulesFileList[ii];
     		QString xruleFilePath = x11Dir + ruleFile;
//  			kDebug() << "trying " << xruleFilePath << endl;
		    if( QFile(xruleFilePath).exists() ) {
				rulesFile = xruleFilePath;
			break;
		    }
    	}
    }
	
	return rulesFile;
}

RulesInfo*
X11Helper::loadRules(const QString& file, bool layoutsOnly) 
{
	XkbRF_RulesPtr xkbRules = XkbRF_Load(QFile::encodeName(file).data(), (char*)"", true, true);

	if (xkbRules == NULL) {
// throw Exception
		return NULL;
	}

	RulesInfo* rulesInfo = new RulesInfo();

	for (int i = 0; i < xkbRules->layouts.num_desc; ++i) {
		QString layoutName(xkbRules->layouts.desc[i].name);
		rulesInfo->layouts.insert( layoutName, qstrdup( xkbRules->layouts.desc[i].desc ) );
	}

	if( layoutsOnly == true ) {
		XkbRF_Free(xkbRules, true);
		return rulesInfo;
	}
  
	//TODO: replace -> insert
  for (int i = 0; i < xkbRules->models.num_desc; ++i)
      rulesInfo->models.insert(xkbRules->models.desc[i].name, qstrdup( xkbRules->models.desc[i].desc ) );
  for (int i = 0; i < xkbRules->options.num_desc; ++i)
      rulesInfo->options.insert(xkbRules->options.desc[i].name, qstrdup( xkbRules->options.desc[i].desc ) );

  XkbRF_Free(xkbRules, true);

// workaround for empty 'compose' options group description
   if( rulesInfo->options.contains("compose:menu") && !rulesInfo->options.contains("compose") ) {
     rulesInfo->options.insert("compose", "Compose Key Position");
   }

   for(QHashIterator<QString, QString> it(rulesInfo->options) ; it.hasNext();  ) {
	  it.next();
	   
	  QString option(it.key());
	  int columnPos = option.indexOf(":");
	  
	  if( columnPos != -1 ) {
		  QString group = option.mid(0, columnPos);
		  if( !rulesInfo->options.contains(group) ) {
			  rulesInfo->options.insert(group, group);
			  kDebug() << "Added missing option group: " << group << endl;
		  }
	  }
  }
  
//   // workaround for empty misc options group description in XFree86 4.4.0
//   if( rulesInfo->options.find("numpad:microsoft") && !rulesInfo->options.find("misc") ) {
//     rulesInfo->options.replace("misc", "Miscellaneous compatibility options" );
//   }

  return rulesInfo;
}

// check $oldlayouts and $nonlatin groups for XFree 4.3 and later
OldLayouts*
X11Helper::loadOldLayouts(const QString& rulesFile)
{
  static const char* oldLayoutsTag = "! $oldlayouts";
  static const char* nonLatinLayoutsTag = "! $nonlatin";
  QStringList m_oldLayouts;
  QStringList m_nonLatinLayouts;
  
  QFile f(rulesFile);
  
  if (f.open(IO_ReadOnly))
    {
      QTextStream ts(&f);
      QString line;

	  while ( ts.status() != QTextStream::ReadPastEnd ) {
	  line = ts.readLine().simplified();

	  if( line.indexOf(oldLayoutsTag) == 0 ) {

	    line = line.mid(strlen(oldLayoutsTag));
	    line = line.mid(line.indexOf('=')+1).simplified();
		
		while( ts.status() != QTextStream::ReadPastEnd && line.endsWith("\\") )
			line = line.left(line.length()-1) + ts.readLine();
	    line = line.simplified();

	    m_oldLayouts = line.split(QRegExp("\\s"));
//	    kDebug() << "oldlayouts " << m_oldLayouts.join("|") << endl;
	    if( !m_nonLatinLayouts.empty() )
	      break;
	    
	  }
	  else
	  if( line.indexOf(nonLatinLayoutsTag) == 0 ) {

	    line = line.mid(strlen(nonLatinLayoutsTag)+1).simplified();
	    line = line.mid(line.indexOf('=')+1).simplified();
		
		while( ts.status() != QTextStream::ReadPastEnd && line.endsWith("\\") )
			line = line.left(line.length()-1) + ts.readLine();
	    line = line.simplified();

	    m_nonLatinLayouts = line.split(QRegExp("\\s"));
//	    kDebug() << "nonlatin " << m_nonLatinLayouts.join("|") << endl;
	    if( !m_oldLayouts.empty() )
	      break;
	    
	  }
      }

      f.close();
    }
	
	OldLayouts* oldLayoutsStruct = new OldLayouts();
	oldLayoutsStruct->oldLayouts = m_oldLayouts;
	oldLayoutsStruct->nonLatinLayouts = m_nonLatinLayouts;
	
	return oldLayoutsStruct;
}


/* pretty simple algorithm - reads the layout file and
    tries to find "xkb_symbols"
    also checks whether previous line contains "hidden" to skip it
*/
QStringList*
X11Helper::getVariants(const QString& layout, const QString& x11Dir, bool oldLayouts)
{
  QStringList* result = new QStringList();

  QString file = x11Dir + "xkb/symbols/";
  // workaround for XFree 4.3 new directory for one-group layouts
  if( QDir(file+"pc").exists() && !oldLayouts )
    file += "pc/";
    
  file += layout;

//  kDebug() << "reading variants from " << file << endl;
  
  QFile f(file);
  if (f.open(IO_ReadOnly))
    {
      QTextStream ts(&f);

      QString line;
      QString prev_line;

	  while ( ts.status() != QTextStream::ReadPastEnd ) {
    	  prev_line = line;
	  line = ts.readLine().simplified();

	    if (line[0] == '#' || line.left(2) == "//" || line.isEmpty())
		continue;

	    int pos = line.indexOf("xkb_symbols");
	    if (pos < 0)
		continue;

	    if( prev_line.indexOf("hidden") >=0 )
		continue;

	    pos = line.indexOf('"', pos) + 1;
	    int pos2 = line.indexOf('"', pos);
	    if( pos < 0 || pos2 < 0 )
		continue;

	    result->append(line.mid(pos, pos2-pos));
//  kDebug() << "adding variant " << line.mid(pos, pos2-pos) << endl;
      }

      f.close();
    }

	return result;
}

#endif

const QString X11Helper::X11_WIN_CLASS_ROOT = "<root>";
const QString X11Helper::X11_WIN_CLASS_UNKNOWN = "<unknown>";

QString 
X11Helper::getWindowClass(WId winId, Display* dpy)
{
  unsigned long nitems_ret, bytes_after_ret;
  unsigned char* prop_ret;
  Atom     type_ret;
  int      format_ret;
  Window w = (Window)winId;	// suppose WId == Window
  QString  property;

  if( winId == X11Helper::UNKNOWN_WINDOW_ID ) {
	  kDebug() << "Got window class for " << winId << ": '" << X11_WIN_CLASS_ROOT << "'" << endl;
	  return X11_WIN_CLASS_ROOT;
  }
  
//  kDebug() << "Getting window class for " << winId << endl;
  if((XGetWindowProperty(dpy, w, XA_WM_CLASS, 0L, 256L, 0, XA_STRING,
			&type_ret, &format_ret, &nitems_ret,
			&bytes_after_ret, &prop_ret) == Success) && (type_ret != None)) {
    property = QString::fromLocal8Bit(reinterpret_cast<char*>(prop_ret));
    XFree(prop_ret);
  }
  else {
	  property = X11_WIN_CLASS_UNKNOWN;
  }
  kDebug() << "Got window class for " << winId << ": '" << property << "'" << endl;
  
  return property;
}
