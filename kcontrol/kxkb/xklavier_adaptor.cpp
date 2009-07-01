/*
 *  Copyright (C) 2007 Andriy Rysin (rysin@kde.org)
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

#include <locale.h>

#include <QTextDocument> // for Qt::escape

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

#include <libxklavier/xklavier.h>

#include "rules.h"
#include "kxkbconfig.h"
#include "xklavier_adaptor.h"


#define KDE_TRANSLATE 1
#define VERBOSE 0

class XKlavierAdaptorPriv {
public:
	QHash<QString, QString> m_models;
	QHash<QString, QString> m_layouts;
	QHash<QString, QList<XkbVariant>*> m_variants;
	QHash<QString, XkbOption> m_options;
	QHash<QString, XkbOptionGroup> m_optionGroups;

  	static XklConfigRegistry *config;
	
	QString currLayout;
	XkbOptionGroup* currGroup;
	XklEngine *engine;
};

XklConfigRegistry *XKlavierAdaptorPriv::config;



XKlavierAdaptor::XKlavierAdaptor(Display* dpy)
{
    priv = new XKlavierAdaptorPriv();

    g_type_init();

    priv->engine = xkl_engine_get_instance(dpy);
    if (priv->engine == NULL) {
        kError() << "XKlavier engine cannot be initialized!" << endl;
        return; // throw
    }

#if KDE_TRANSLATE == 1
    // If we cannot get XKlavier's own i18n to work,
    // we have to manually translate strings it reports using its catalog.
    // This catalog has to be in the locale paths which KDE will search.
    KGlobal::locale()->insertCatalog("xkeyboard-config");
    // KDE's i18n expects messages to be well-formed XML,
    // so use Qt::escape() to replace < with &lt; etc.
    // Actually we couldn't have done this just like that,
    // as then an escaped message would not be found in the catalog;
    // but, by a lucky fiat, xkeyboard-config too wants its messages as
    // well-formed XML, so they come escaped exactly like this.
    #define I18N_KDE(x) i18n(Qt::escape(QString::fromUtf8(x)).toUtf8())
#endif
}

QHash<QString, QString> XKlavierAdaptor::getModels() { return priv->m_models; }
QHash<QString, QString> XKlavierAdaptor::getLayouts() { return priv->m_layouts; }
QHash<QString, XkbOption> XKlavierAdaptor::getOptions() { return priv->m_options; }
QHash<QString, XkbOptionGroup> XKlavierAdaptor::getOptionGroups() { return priv->m_optionGroups; }
QHash<QString, QList<XkbVariant>*> XKlavierAdaptor::getVariants() { return priv->m_variants; }



static void processModel(XklConfigRegistry*, const XklConfigItem* configItem, gpointer userData)
{
	QString model = QString::fromUtf8(configItem->name);
#if KDE_TRANSLATE == 1
	QString desc = I18N_KDE(configItem->description);
#else
	QString desc = QString::fromUtf8(configItem->description);
#endif

#if VERBOSE == 1
	  kDebug() << "model: " << model << " - " << desc;
#endif

	((XKlavierAdaptorPriv*)userData)->m_models.insert(model, desc);
}


static void processVariants(XklConfigRegistry*, const XklConfigItem* configItem, gpointer userData)
{
	XkbVariant variant;
	variant.name = QString::fromUtf8(configItem->name);
#if KDE_TRANSLATE == 1
	variant.description = I18N_KDE(configItem->description);
#else
	variant.description = QString::fromUtf8(configItem->description);
#endif
	QString layout = ((XKlavierAdaptorPriv*)userData)->currLayout;

#if VERBOSE == 1
	kDebug() << "\tvariant: " << variant.name << "-" << variant.description << " (parent: " << layout << ")";
#endif

	QList<XkbVariant>* vars = ((XKlavierAdaptorPriv*)userData)->m_variants[layout];
	vars->append(variant);
}


static void processLayout(XklConfigRegistry*, const XklConfigItem* configItem, gpointer userData)
{
	QString layout = QString::fromUtf8(configItem->name);
#if KDE_TRANSLATE == 1
	QString desc = I18N_KDE(configItem->description);
#else
	QString desc = QString::fromUtf8(configItem->description);
#endif

#if VERBOSE == 1
	kDebug() << "layout: " << layout << " - " << desc;
#endif
	((XKlavierAdaptorPriv*)userData)->m_layouts.insert(layout, desc);
	((XKlavierAdaptorPriv*)userData)->m_variants.insert(layout, new QList<XkbVariant>());
	
	((XKlavierAdaptorPriv*)userData)->currLayout = layout;
	xkl_config_registry_foreach_layout_variant(XKlavierAdaptorPriv::config, 
					configItem->name, processVariants, userData);
}


static void processOptions(XklConfigRegistry*, const XklConfigItem* configItem, gpointer userData)
{
	XkbOption option;
	
	option.name = QString::fromUtf8(configItem->name);
#if KDE_TRANSLATE == 1
	option.description = I18N_KDE(configItem->description);
#else
	option.description = QString::fromUtf8(configItem->description);
#endif
	option.group = ((XKlavierAdaptorPriv*)userData)->currGroup;

#if VERBOSE == 1
	  kDebug() << "\toptions: " << option.name;
#endif

	((XKlavierAdaptorPriv*)userData)->m_options.insert(option.name, option);
}


static void processOptionGroup(XklConfigRegistry*, const XklConfigItem* configItem, void *userData)
{
	XkbOptionGroup group;
	group.name = QString::fromUtf8(configItem->name);
#if KDE_TRANSLATE == 1
	group.description = I18N_KDE(configItem->description);
#else
	group.description = QString::fromUtf8(configItem->description);
#endif
	group.exclusive = ! GPOINTER_TO_INT (g_object_get_data (G_OBJECT (configItem),
	                                                          XCI_PROP_ALLOW_MULTIPLE_SELECTION));
	
#if VERBOSE == 1
	  kDebug() << "group: " << group.name << " - " << group.description;
#endif

	((XKlavierAdaptorPriv*)userData)->m_optionGroups.insert(group.name, group);

	((XKlavierAdaptorPriv*)userData)->currGroup = 
		  &((XKlavierAdaptorPriv*)userData)->m_optionGroups[group.name];
	xkl_config_registry_foreach_option(XKlavierAdaptorPriv::config, 
					configItem->name, processOptions, userData);
}


static const int LOCALE_CATEGORY = LC_ALL;

void XKlavierAdaptor::loadXkbConfig(bool layoutsOnly)
{
    if( priv->engine == NULL )
        return;
    
    const char* currLocale = setlocale(LOCALE_CATEGORY, NULL);

    QString locale = KGlobal::locale()->language();
    if( locale.indexOf('_') == -1 ) {   // TODO: do we have to do this?
        QString country = KGlobal::locale()->country();
        if( ! country.isEmpty() ) {
            locale += "_";
	    locale += country.toUpper();
        }
    }
//  locale = "uk_UA";   // testing
//  locale = "en_US";
    locale += ".UTF-8";
    kDebug() << "Setting LC_ALL for libxklavier: " << locale;

    const char* newLocale = setlocale(LOCALE_CATEGORY, locale.toLatin1());
    if( newLocale == NULL ) {
        kDebug() << "Setting locale " << locale << " failed - will use 'C' locale";
        setlocale(LC_ALL, "C");
    }

    kDebug() << "Xklavier initialized";
    priv->config = xkl_config_registry_get_instance(priv->engine);

    xkl_config_registry_load(priv->config);
	
    void *userData = priv;

//	xkl_config_registry_set_custom_charset(priv->config, "UTF-8");
	
    xkl_config_registry_foreach_layout(priv->config, processLayout, userData);

    if( ! layoutsOnly ) {
	xkl_config_registry_foreach_model(priv->config, processModel, userData);
	xkl_config_registry_foreach_option_group(priv->config, processOptionGroup, userData);
    }

    kDebug() << priv->m_layouts.count() << "total layouts" << priv->m_models.count() << "models";

    setlocale(LOCALE_CATEGORY, currLocale);

    g_object_unref(priv->config);
}

XKlavierAdaptor::~XKlavierAdaptor()
{
    g_object_unref(priv->engine);
//	delete priv;
//	kDebug() << "Finalizer";
}

XkbConfig 
XKlavierAdaptor::getGroupNames()
{
    XkbConfig xkbConfig;

//    kDebug() << "retrieving active layout from server...";
    XklConfigRec configRec;
    xkl_config_rec_get_from_server(&configRec, priv->engine);

    for(int ii=0; configRec.layouts[ii] != NULL && ii < GROUP_LIMIT; ii++) {
	LayoutUnit lu;
	lu.layout = configRec.layouts[ii];
	lu.variant = configRec.variants[ii];
	xkbConfig.layouts << lu;
	kDebug() << " layout nm:" << lu.layout << "variant:" << lu.variant;
    }

    for(int ii=0; configRec.options[ii] != NULL && ii < 15; ii++) {
	xkbConfig.options << configRec.options[ii];
	kDebug() << " option:" << configRec.options[ii];
    }

//        const char **gn = xkl_engine_get_groups_names(priv->engine);
//	int gt = xkl_engine_get_num_groups(priv->engine);
//	int i;
//	for (i = 0; i < gt; i++)
//	    kDebug() << "group:" << gn[i];

    return xkbConfig;
}

static XKlavierAdaptor* instance = NULL;

XKlavierAdaptor*
XKlavierAdaptor::getInstance(Display* dpy)
{
    if( instance == NULL ) {
	instance = new XKlavierAdaptor(dpy);
    }
    return instance;
}

int
XKlavierAdaptor::startListening()
{
    return xkl_engine_start_listen(priv->engine, XKLL_TRACK_KEYBOARD_STATE);
}

int
XKlavierAdaptor::stopListening()
{
    return xkl_engine_start_listen(priv->engine, XKLL_TRACK_KEYBOARD_STATE);
}

int 
XKlavierAdaptor::filterEvents(XEvent* ev)
{
    return xkl_engine_filter_events(priv->engine, ev);
}
