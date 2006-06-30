#ifndef X11HELPER_H_
#define X11HELPER_H_

#include <QHash>
#include <QStringList>

#include <kwin.h>


struct RulesInfo {
	QHash<QString, QString> models;
	QHash<QString, QString> layouts;
	QHash<QString, QString> options;
};

struct OldLayouts {
	QStringList oldLayouts;
	QStringList nonLatinLayouts;
};

class X11Helper
{
	static bool m_layoutsClean;

public:
	static const WId UNKNOWN_WINDOW_ID = (WId) 0;
	static const QString X11_WIN_CLASS_ROOT;
	static const QString X11_WIN_CLASS_UNKNOWN;
	/**
	 * Tries to find X11 xkb config dir
	 */
	static const QString findX11Dir();
	static const QString findXkbRulesFile(QString x11Dir, Display* dpy);
	static QString getWindowClass(WId winId, Display* dpy);
	static QStringList* getVariants(const QString& layout, const QString& x11Dir, bool oldLayouts=false);
	static RulesInfo* loadRules(const QString& rulesFile, bool layoutsOnly=false);
	static OldLayouts* loadOldLayouts(const QString& rulesFile);
	
	static bool areLayoutsClean() { return m_layoutsClean; }
	static bool areSingleGroupsSupported();
};

#endif /*X11HELPER_H_*/
