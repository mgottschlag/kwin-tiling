#include <QString>
#include <QStringList>

#include <kdebug.h>

#include <libxklavier/xklavier.h>

#include "rules.h"
#include "xklavier_adaptor.h"


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



XKlavierAdaptor::XKlavierAdaptor() {
  priv = new XKlavierAdaptorPriv();
}

QHash<QString, QString> XKlavierAdaptor::getModels() { return priv->m_models; }
QHash<QString, QString> XKlavierAdaptor::getLayouts() { return priv->m_layouts; }
QHash<QString, XkbOption> XKlavierAdaptor::getOptions() { return priv->m_options; }
QHash<QString, XkbOptionGroup> XKlavierAdaptor::getOptionGroups() { return priv->m_optionGroups; }
QHash<QString, QStringList*> XKlavierAdaptor::getVariants() { return priv->m_variants; }



static void processModel(XklConfigRegistry* reg, const XklConfigItem* configItem, gpointer userData) {
	QString model = configItem->name;
	kDebug() << "model: " << model << " - " << configItem->description << endl;
	((XKlavierAdaptorPriv*)userData)->m_models.insert(model, QString(configItem->description));
}

static void processVariants(XklConfigRegistry* reg, const XklConfigItem* configItem, gpointer userData) {
	QString variant = configItem->name;
	QString layout = ((XKlavierAdaptorPriv*)userData)->currLayout;

	kDebug() << "\tvariant: " << variant << " (parent: " << layout << ")" << endl;

	QStringList* vars = ((XKlavierAdaptorPriv*)userData)->m_variants[layout];
	vars->append(variant);	//TODO: //QString(configItem->description));
}

static void processLayout(XklConfigRegistry* reg, const XklConfigItem* configItem, gpointer userData)
{
	QString layout = configItem->name;

	kDebug() << "layout: " << layout << " - " << configItem->description << endl;
	((XKlavierAdaptorPriv*)userData)->m_layouts.insert(layout, QString(configItem->description));
	((XKlavierAdaptorPriv*)userData)->m_variants.insert(layout, new QStringList());
	
	((XKlavierAdaptorPriv*)userData)->currLayout = layout;
	xkl_config_registry_foreach_layout_variant(XKlavierAdaptorPriv::config, 
					configItem->name, processVariants, userData);
}


static void processOptions(XklConfigRegistry* reg, const XklConfigItem* configItem, gpointer userData)
{
	XkbOption option;
	
	option.name = configItem->name;
	option.description = configItem->description;
	option.group = ((XKlavierAdaptorPriv*)userData)->currGroup;
	kDebug() << "\toptions: " << option.name << endl;

	((XKlavierAdaptorPriv*)userData)->m_options.insert(option.name, option);
}

static void processOptionGroup(XklConfigRegistry* reg, const XklConfigItem* configItem, void *userData)
{
	XkbOptionGroup group;
	group.name = configItem->name;
	group.description = configItem->description;
	group.exclusive = ! GPOINTER_TO_INT (g_object_get_data (G_OBJECT (configItem),
	                                                          XCI_PROP_ALLOW_MULTIPLE_SELECTION));
	
	kDebug() << "group: " << group.name << " - " << group.description << endl;
	((XKlavierAdaptorPriv*)userData)->m_optionGroups.insert(group.name, group);

	((XKlavierAdaptorPriv*)userData)->currGroup = 
		  &((XKlavierAdaptorPriv*)userData)->m_optionGroups[group.name];
	xkl_config_registry_foreach_option(XKlavierAdaptorPriv::config, 
					configItem->name, processOptions, userData);
}

void XKlavierAdaptor::xklConfig(Display* dpy) {
//	XklConfigInit();
//	bool res = XklConfigLoadRegistryFromFile("/etc/X11/xkb/rules/xorg.xml");
//	cerr << "XklConfigLoadRegistryFromFile: " << res << endl;
	g_type_init();
	
	XklEngine *engine = xkl_engine_get_instance(dpy);
	if (engine != NULL) {
		kDebug() << "Xklavier initialized" << endl;
		priv->config =
		    xkl_config_registry_get_instance(engine);
		xkl_config_registry_load(priv->config);
	}	
	
	void *userData = priv;

#ifdef HAVE_SET_CUSTOM_CHARSET
	xkl_config_registry_set_custom_charset(priv->config, "UTF-8");
#else
#warning "No xkl_config_registry_set_custom_curset found - local layout names may be corrupted! Consider updating libxklavier"
#endif	

	xkl_config_registry_foreach_model(priv->config, processModel, userData);

	
	xkl_config_registry_foreach_layout(priv->config, processLayout, userData);


	xkl_config_registry_foreach_option_group(priv->config, processOptionGroup, userData);


//	xkl_config_registry_free(config);
//	XklConfigTerm();
}

XKlavierAdaptor::~XKlavierAdaptor() {
	kDebug() << "Finalizer" << endl;
//	XklTerm();
//	XCloseDisplay(display);
}


/*
int main () {

	
	setenv("LC_MESSAGES", "uk_UA", 1);
    setlocale(LC_MESSAGES, "");
//	setenv("LANG", "uk_UA", 1);
	
	XKlavierAdaptor xklTest;
	
	xklTest.xklConfig();

	return 0;	
}
*/
