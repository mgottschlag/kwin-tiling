#ifndef XKLAVIER_ADAPTOR_H
#define XKLAVIER_ADAPTOR_H
#include <X11/Xlib.h>

#include <QHash>
#include <QList>


class LayoutUnit;
class XKlavierAdaptorPriv;

class XKlavierAdaptor {

public:
	~XKlavierAdaptor();	
		
	void loadXkbConfig(bool layoutsOnly);

	QHash<QString, QString> getModels();
	QHash<QString, QString> getLayouts();
	QHash<QString, XkbOption> getOptions();
	QHash<QString, XkbOptionGroup> getOptionGroups();
	QHash<QString, QStringList*> getVariants();

	QList<LayoutUnit> getGroupNames();
	
	static XKlavierAdaptor* getInstance(Display* dpy);
		
private:
	XKlavierAdaptor(Display* dpy);

	XKlavierAdaptorPriv* priv;
};

#endif
