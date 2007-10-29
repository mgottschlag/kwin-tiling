/*
 *  Copyright (C) 2003-2006 Andriy Rysin (rysin@kde.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <QtGui/QWidgetList>
#include <QX11Info>
#include <QRegExp>
#include <QTextStream>
#include <QFile>
#include <QDir>

#include <kstandarddirs.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <config-workspace.h>

#include "x11helper.h"
#include "rules.h"

#ifdef HAVE_XKLAVIER
#include "xklavier_adaptor.h"
#endif

#include "kxkbconfig.h"


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
    XKlavierAdaptor* xklAdaptor = XKlavierAdaptor::getInstance(QX11Info::display());
    xklAdaptor->loadXkbConfig(layoutsOnly);

    m_layouts = xklAdaptor->getLayouts();
    if( layoutsOnly == false ) {
	m_models = xklAdaptor->getModels();
        m_varLists = xklAdaptor->getVariants();
	m_optionGroups = xklAdaptor->getOptionGroups();
	m_options = xklAdaptor->getOptions();
	
        QHashIterator<QString, XkbOption> it( m_options );
        for (; it.hasNext(); ) {
            const XkbOption& option = it.next().value();
            option.group->options.append(option);
        }
    }
}

#else

void XkbRules::loadRules(const QString &file, bool layoutsOnly)
{
    RulesInfo* rules = X11Helper::loadRules(file, layoutsOnly);

    if (rules == NULL) {
	kDebug() << "Unable to load rules";
	return;
    }

    m_layouts= rules->layouts;

    if( layoutsOnly == false ) {
	m_models = rules->models;
//        m_varLists = rules->variants;
	m_options = rules->options;
	m_optionGroups = rules->optionGroups;

        QHashIterator<QString, XkbOption> it( m_options );
        for (; it.hasNext(); ) {
            const XkbOption& option = it.next().value();
            option.group->options.append(option);
        }
    }
}


#endif

QList<XkbVariant>
XkbRules::getAvailableVariants(const QString& layout)
{
    if( layout.isEmpty() || !layouts().contains(layout) )
	  return QList<XkbVariant>();

    QList<XkbVariant>* result1 = m_varLists[layout];

#ifdef HAVE_XKLAVIER
        return *result1;
#else
    if( result1 )
        return *result1;

    QList<XkbVariant>* result = X11Helper::getVariants(layout, X11_DIR);

    m_varLists.insert(layout, result);
    return *result;
#endif

}
