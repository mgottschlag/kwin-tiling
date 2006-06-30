#include <qwindowdefs.h>
#include <QX11Info>
#include <QRegExp>
#include <QStringList>
#include <QTextStream>
#include <QFile>
#include <QDir>

#include <kstandarddirs.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <config.h>

#include "x11helper.h"
#include "rules.h"


XkbRules::XkbRules(bool layoutsOnly):
    m_layouts()
{
	X11_DIR = X11Helper::findX11Dir();

   	if( X11_DIR == NULL ) {
        kError() << "Cannot find X11 directory!" << endl;
//        throw Exception();
		return;
   	}

	QString rulesFile = X11Helper::findXkbRulesFile(X11_DIR, QX11Info::display());
	
	if( rulesFile.isEmpty() ) {
  		kError() << "Cannot find rules file in " << X11_DIR << endl;
//		throw Exception();
		return;
	}

	loadRules(rulesFile, layoutsOnly);
	loadOldLayouts(rulesFile);
	loadGroups(::locate("config", "kxkb_groups"));
}


void XkbRules::loadRules(QString file, bool layoutsOnly)
{
	RulesInfo* rules = X11Helper::loadRules(file, layoutsOnly);

	if (rules == NULL) {
		kDebug() << "Unable to load rules" << endl;
		return;
	}

	m_layouts= rules->layouts;
	if( layoutsOnly == false ) {
		m_models = rules->models;
		m_options = rules->options;
	}

	//  fixLayouts();
}

// void XkbRules::fixLayouts() {
// // THIS IS TEMPORARY!!!
// // This should be fixed in XFree86 (and actually is fixed in XFree 4.2)
// // some handcoded ones, because the X11 rule file doesn't get them correctly, or in case
// // the rule file wasn't found
// 	static struct {
// 		const char * locale;
// 		const char * layout;
// 	} fixedLayouts[] = {
// 		{ "ben", "Bengali" },
// 		{ "ar", "Arabic" },
// 		{ "ir", "Farsi" },
// 		{ 0, 0 }
// 	};
// 	
// 	for(int i=0; fixedLayouts[i].layout != 0; i++ ) {
// 		if( m_layouts.find(fixedLayouts[i].locale) == 0 )
// 			m_layouts.insert(fixedLayouts[i].locale, fixedLayouts[i].layout);
// 	}
// }

bool XkbRules::isSingleGroup(const QString& layout)
{
	  return X11Helper::areSingleGroupsSupported()
			  && !m_oldLayouts.contains(layout)
			  && !m_nonLatinLayouts.contains(layout);
}


// check $oldlayouts and $nonlatin groups for XFree 4.3 and later
void XkbRules::loadOldLayouts(QString rulesFile)
{
	OldLayouts* oldLayoutsStruct = X11Helper::loadOldLayouts( rulesFile );
	m_oldLayouts = oldLayoutsStruct->oldLayouts;
	m_nonLatinLayouts = oldLayoutsStruct->nonLatinLayouts;
}

// for multi-group layouts in XFree 4.2 and older
//    or if layout is present in $oldlayout or $nonlatin groups
void XkbRules::loadGroups(QString file)
{
  QFile f(file);
  if (f.open(IO_ReadOnly))
    {
      QTextStream ts(&f);
      QString locale;
      unsigned int grp;

	  while ( ts.status() != QTextStream::ReadPastEnd ) {
         ts >> locale >> grp;
	 locale.simplified();

	 if (locale[0] == '#' || locale.left(2) == "//" || locale.isEmpty())
	    continue;

    	 m_initialGroups.insert(locale, grp);
      }
      
      f.close();
    }
}

unsigned int 
XkbRules::getDefaultGroup(const QString& layout, const QString& includeGroup)
{
// check for new one-group layouts in XFree 4.3 and older
    if( isSingleGroup(layout) ) {
		if( includeGroup.isEmpty() == false )
			return 1;
		else
			return 0;
    }
    
    QMap<QString, unsigned int>::iterator it = m_initialGroups.find(layout);
    return it == m_initialGroups.end() ? 0 : it.data();
}


QStringList
XkbRules::getAvailableVariants(const QString& layout)
{
    if( layout.isEmpty() || !layouts().find(layout) )
	return QStringList();

    QStringList* result1 = m_varLists[layout];
    if( result1 )
        return *result1;

    bool oldLayouts = m_oldLayouts.contains(layout);
    QStringList* result = X11Helper::getVariants(layout, X11_DIR, oldLayouts);

    m_varLists.insert(layout, result);

    return *result;
}

