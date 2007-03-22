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


static const QRegExp NON_CLEAN_LAYOUT_REGEXP("[^a-z]");

//bool XkbRules::m_layoutsClean = true;

XkbRules::XkbRules(bool layoutsOnly)
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
#endif

}


#ifdef HAVE_XKLAVIER

void XkbRules::loadNewRules(bool layoutsOnly)
{
	XKlavierAdaptor xklAdaptor;
	xklAdaptor.loadXkbConfig(QX11Info::display(), layoutsOnly);

	m_layouts = xklAdaptor.getLayouts();
	if( layoutsOnly == false ) {
	  m_models = xklAdaptor.getModels();
	  m_optionGroups = xklAdaptor.getOptionGroups();
	  m_options = xklAdaptor.getOptions();
	  m_varLists = xklAdaptor.getVariants();
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
		m_optionGroups = rules->optionGroups;
	}
}


#endif

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

    QStringList* result = X11Helper::getVariants(layout, X11_DIR);

    m_varLists.insert(layout, result);
    return *result;
#endif
}
