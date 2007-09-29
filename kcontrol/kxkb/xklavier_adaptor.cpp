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

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

#include <libxklavier/xklavier.h>

#include "rules.h"
#include "kxkbconfig.h"
#include "xklavier_adaptor.h"


#define VERBOSE 0

class XKlavierAdaptorPriv {
public:
	QHash<QString, QString> m_models;
	QHash<QString, QString> m_layouts;
	QHash<QString, QStringList*> m_variants;
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

	QString locale = KGlobal::locale()->language();
	if( ! KGlobal::locale()->country().isEmpty() ) {
//		locale += KGlobal::_locale->country();
//		locale += "UTF-8";
	}
	kDebug() << "Setting LC_MESSAGES for libxklavier: " << locale;
//	setlocale(LC_ALL, locale.toLatin1());
	setlocale(LC_MESSAGES, locale.toLatin1());


	priv->engine = xkl_engine_get_instance(dpy);
	if (priv->engine == NULL) {
		kError() << "XKlavier engine cannot be initialized!" << endl;
		return; // throw
	}
	
}

QHash<QString, QString> XKlavierAdaptor::getModels() { return priv->m_models; }
QHash<QString, QString> XKlavierAdaptor::getLayouts() { return priv->m_layouts; }
QHash<QString, XkbOption> XKlavierAdaptor::getOptions() { return priv->m_options; }
QHash<QString, XkbOptionGroup> XKlavierAdaptor::getOptionGroups() { return priv->m_optionGroups; }
QHash<QString, QStringList*> XKlavierAdaptor::getVariants() { return priv->m_variants; }



static void processModel(XklConfigRegistry*, const XklConfigItem* configItem, gpointer userData)
{
	QString model = configItem->name;
	QString desc = configItem->description;

#if VERBOSE == 1
	  kDebug() << "model: " << model << " - " << desc;
#endif

	((XKlavierAdaptorPriv*)userData)->m_models.insert(model, desc);
}


static void processVariants(XklConfigRegistry*, const XklConfigItem* configItem, gpointer userData)
{
	QString variant = configItem->name;
	QString layout = ((XKlavierAdaptorPriv*)userData)->currLayout;

#if VERBOSE == 1
	  kDebug() << "\tvariant: " << variant << " (parent: " << layout << ")";
#endif

	QStringList* vars = ((XKlavierAdaptorPriv*)userData)->m_variants[layout];
	vars->append(variant);	//TODO: //QString(configItem->description));
}


static void processLayout(XklConfigRegistry*, const XklConfigItem* configItem, gpointer userData)
{
	QString layout = configItem->name;
	QString desc = configItem->description;

#if VERBOSE == 1
	kDebug() << "layout: " << layout << " - " << desc;
#endif
	((XKlavierAdaptorPriv*)userData)->m_layouts.insert(layout, desc);
	((XKlavierAdaptorPriv*)userData)->m_variants.insert(layout, new QStringList());
	
	((XKlavierAdaptorPriv*)userData)->currLayout = layout;
	xkl_config_registry_foreach_layout_variant(XKlavierAdaptorPriv::config, 
					configItem->name, processVariants, userData);
}


static void processOptions(XklConfigRegistry*, const XklConfigItem* configItem, gpointer userData)
{
	XkbOption option;
	
	option.name = configItem->name;
	option.description = configItem->description;
	option.group = ((XKlavierAdaptorPriv*)userData)->currGroup;

#if VERBOSE == 1
	  kDebug() << "\toptions: " << option.name;
#endif

	((XKlavierAdaptorPriv*)userData)->m_options.insert(option.name, option);
}


static void processOptionGroup(XklConfigRegistry*, const XklConfigItem* configItem, void *userData)
{
	XkbOptionGroup group;
	group.name = configItem->name;
	group.description = configItem->description;
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


void XKlavierAdaptor::loadXkbConfig(bool layoutsOnly)
{
	if( priv->engine == NULL )
	    return;

	kDebug() << "Xklavier initialized";
	priv->config = xkl_config_registry_get_instance(priv->engine);

	xkl_config_registry_load(priv->config);
	
	void *userData = priv;

	xkl_config_registry_set_custom_charset(priv->config, "UTF-8");
	
	xkl_config_registry_foreach_layout(priv->config, processLayout, userData);

	if( ! layoutsOnly ) {
	  xkl_config_registry_foreach_model(priv->config, processModel, userData);

	  xkl_config_registry_foreach_option_group(priv->config, processOptionGroup, userData);
	}

	g_object_unref(priv->config);
}

XKlavierAdaptor::~XKlavierAdaptor()
{
	g_object_unref(priv->engine);
//	delete priv;
//	kDebug() << "Finalizer";
}

QList<LayoutUnit> 
XKlavierAdaptor::getGroupNames()
{
    QList<LayoutUnit> list;

    XklConfigRec configRec;
    xkl_config_rec_get_from_server(&configRec, priv->engine);

    for(int ii=0; configRec.layouts[ii] != NULL; ii++) {
	LayoutUnit lu;
	lu.layout = configRec.layouts[ii];
	lu.variant = configRec.variants[ii];
	list << lu;
	kDebug() << "layout nm:" << lu.layout << "variant:" << lu.variant;
    }

//        const char **gn = xkl_engine_get_groups_names(priv->engine);
//	int gt = xkl_engine_get_num_groups(priv->engine);
//	int i;
//	for (i = 0; i < gt; i++)
//	    kDebug() << "group:" << gn[i];

    return list;
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
XKlavierAdaptor::filterEvents(XEvent* ev)
{
    return xkl_engine_filter_events(priv->engine, ev);
}
