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

#ifdef HAVE_XKLAVIER
#include "xklavier_adaptor.h"
#endif

XkbRules::XkbRules(bool layoutsOnly):
    m_layouts()
{
#ifdef HAVE_XKLAVIER
	loadNewRules(layoutsOnly);
#else
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
	loadGroups(KStandardDirs::locate("config", "kxkb_groups"));
#endif
}

#ifdef HAVE_XKLAVIER

void XkbRules::loadNewRules(bool layoutsOnly)
{
	XKlavierAdaptor xklAdaptor;
	xklAdaptor.xklConfig(QX11Info::display());

	m_layouts = xklAdaptor.getLayouts();
	if( layoutsOnly == false ) {
	  m_models = xklAdaptor.getModels();
	  m_options = xklAdaptor.getOptions();
	}
}

#else

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
}

#endif

bool XkbRules::isSingleGroup(const QString& layout)
{
#ifdef HAVE_XKLAVIER
	return true;
#else
	  return X11Helper::areSingleGroupsSupported()
			  && !m_oldLayouts.contains(layout)
			  && !m_nonLatinLayouts.contains(layout);
#endif
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

    QMap<QString, unsigned int>::const_iterator it = m_initialGroups.find(layout);
    return it == m_initialGroups.constEnd() ? 0 : it.value();
}


QStringList
XkbRules::getAvailableVariants(const QString& layout)
{
    if( layout.isEmpty() || !layouts().contains(layout) )
	return QStringList();

    QStringList* result1 = m_varLists[layout];

#ifdef HAVE_XKLAVIER
        return *result1;
#else
    if( result1 )
        return *result1;

    bool oldLayouts = m_oldLayouts.contains(layout);
    QStringList* result = X11Helper::getVariants(layout, X11_DIR, oldLayouts);

    m_varLists.insert(layout, result);
    return *result;
#endif
}



#ifndef HAVE_XKLAVIER

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

#endif
