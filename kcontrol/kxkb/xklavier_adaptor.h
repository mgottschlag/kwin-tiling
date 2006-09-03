#include <X11/Xlib.h>

#include <QHash>
#include <QString>

class XKlavierAdaptorPriv;

class XKlavierAdaptor {
public:
	XKlavierAdaptor();
	~XKlavierAdaptor();	
		
	void loadXkbConfig(Display* dpy, bool layoutsOnly);

	QHash<QString, QString> getModels();
	QHash<QString, QString> getLayouts();
	QHash<QString, XkbOption> getOptions();
	QHash<QString, XkbOptionGroup> getOptionGroups();
	QHash<QString, QStringList*> getVariants();
	
private:
	XKlavierAdaptorPriv* priv;
};
