#include <locale.h>

#include <QString>
#include <QStringList>

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

#include <libxklavier/xklavier.h>

#include "rules.h"
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
};

XklConfigRegistry *XKlavierAdaptorPriv::config;



XKlavierAdaptor::XKlavierAdaptor()
{
  priv = new XKlavierAdaptorPriv();
  
  g_type_init();
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
	  kDebug() << "model: " << model << " - " << desc << endl;
#endif

	((XKlavierAdaptorPriv*)userData)->m_models.insert(model, desc);
}


static void processVariants(XklConfigRegistry*, const XklConfigItem* configItem, gpointer userData)
{
	QString variant = configItem->name;
	QString layout = ((XKlavierAdaptorPriv*)userData)->currLayout;

#if VERBOSE == 1
	  kDebug() << "\tvariant: " << variant << " (parent: " << layout << ")" << endl;
#endif

	QStringList* vars = ((XKlavierAdaptorPriv*)userData)->m_variants[layout];
	vars->append(variant);	//TODO: //QString(configItem->description));
}


static void processLayout(XklConfigRegistry*, const XklConfigItem* configItem, gpointer userData)
{
	QString layout = configItem->name;
	QString desc = configItem->description;

	kDebug() << "layout: " << layout << " - " << desc << endl;
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
	  kDebug() << "\toptions: " << option.name << endl;
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
	  kDebug() << "group: " << group.name << " - " << group.description << endl;
#endif

	((XKlavierAdaptorPriv*)userData)->m_optionGroups.insert(group.name, group);

	((XKlavierAdaptorPriv*)userData)->currGroup = 
		  &((XKlavierAdaptorPriv*)userData)->m_optionGroups[group.name];
	xkl_config_registry_foreach_option(XKlavierAdaptorPriv::config, 
					configItem->name, processOptions, userData);
}


void XKlavierAdaptor::loadXkbConfig(Display* dpy, bool layoutsOnly)
{
	QString locale = KGlobal::locale()->language();
	if( ! KGlobal::locale()->country().isEmpty() ) {
//		locale += KGlobal::_locale->country();
//		locale += "UTF-8";
	}
	kDebug() << "Setting LC_MESSAGES for libxklavier: " << locale << endl;
//	setlocale(LC_ALL, locale.toLatin1());
	setlocale(LC_MESSAGES, locale.toLatin1());


	XklEngine *engine = xkl_engine_get_instance(dpy);
	if (engine == NULL) {
		kError() << "XKlavier engine cannot be initialized!" << endl;
		return; // throw
	}
	
	kDebug() << "Xklavier initialized" << endl;
	priv->config = xkl_config_registry_get_instance(engine);

	xkl_config_registry_load(priv->config);
	
	void *userData = priv;

#ifdef HAVE_SET_CUSTOM_CHARSET
	xkl_config_registry_set_custom_charset(priv->config, "UTF-8");
#else
#ifdef __GNUC__
#warning "No xkl_config_registry_set_custom_curset found - local layout names may be corrupted! Consider updating libxklavier"
#endif
#endif	

	
	xkl_config_registry_foreach_layout(priv->config, processLayout, userData);

	if( ! layoutsOnly ) {
	  xkl_config_registry_foreach_model(priv->config, processModel, userData);

	  xkl_config_registry_foreach_option_group(priv->config, processOptionGroup, userData);
	}

	g_object_unref(priv->config);
	g_object_unref(engine);
}

XKlavierAdaptor::~XKlavierAdaptor()
{
//	delete priv;
//	kDebug() << "Finalizer" << endl;
}
