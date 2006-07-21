#ifndef X11HELPER_H_
#define X11HELPER_H_

#include <QHash>
#include <QStringList>

#include <kwin.h>


struct XkbOptionGroup {
  QString name;
  QString description;
  bool exclusive;
};

struct XkbOption {
  QString name;
  QString description;
  XkbOptionGroup* group;
};

struct RulesInfo {
	QHash<QString, QString> models;
	QHash<QString, QString> layouts;
	QHash<QString, XkbOption> options;
	QHash<QString, XkbOptionGroup> optionGroups;
};

struct OldLayouts {
	QStringList oldLayouts;
	QStringList nonLatinLayouts;
};

class X11Helper
{
public:
	static const WId UNKNOWN_WINDOW_ID = (WId) 0;
	static const QString X11_WIN_CLASS_ROOT;
	static const QString X11_WIN_CLASS_UNKNOWN;

	static QString getWindowClass(WId winId, Display* dpy);

	static bool areSingleGroupsSupported() { return true; } // assume not ancient xorg
#ifndef HAVE_XKLAVIER
	/**
	 * Tries to find X11 xkb config dir
	 */
	static const QString findX11Dir();
	static const QString findXkbRulesFile(QString x11Dir, Display* dpy);
	static QStringList* getVariants(const QString& layout, const QString& x11Dir, bool oldLayouts=false);
	static RulesInfo* loadRules(const QString& rulesFile, bool layoutsOnly=false);
	static OldLayouts* loadOldLayouts(const QString& rulesFile);
private:

	static XkbOptionGroup createMissingGroup(const QString& groupName);
	static bool isGroupExclusive(const QString& groupName);
#endif

};

#endif /*X11HELPER_H_*/
