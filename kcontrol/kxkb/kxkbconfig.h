//
// C++ Interface: kxkbconfig
//
// Description: 
//
//
// Author: Andriy Rysin <rysin@kde.org>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef KXKBCONFIG_H
#define KXKBCONFIG_H

#include <QString>
#include <QStringList>
#include <QQueue>
#include <QMap>


/* Utility classes for per-window/per-application layout implementation
*/
enum SwitchingPolicy { 
	SWITCH_POLICY_GLOBAL = 0,
	SWITCH_POLICY_WIN_CLASS = 1,
	SWITCH_POLICY_WINDOW = 2,
	SWITCH_POLICY_COUNT = 3
};



inline QString createPair(QString key, QString value) 
{
	if( value.isEmpty() )
		return key;
	return QString("%1(%2)").arg(key, value);
} 

struct LayoutUnit {
	QString layout;
	QString variant;
	QString includeGroup;
	QString displayName;
 	int defaultGroup;
	
	LayoutUnit() {}
	
	LayoutUnit(QString layout_, QString variant_):
		layout(layout_),
		variant(variant_)
	{}
	
	LayoutUnit(QString pair) {
		setFromPair( pair );
	}
	
	void setFromPair(const QString& pair) {
		layout = parseLayout(pair);
		variant = parseVariant(pair);
	}
	
	QString toPair() const {
		return createPair(layout, variant);
	}
	
	bool operator<(const LayoutUnit& lu) const {
		return layout<lu.layout ||
				(layout==lu.layout && variant<lu.variant);
	}
	
	bool operator!=(const LayoutUnit& lu) const {
		return layout!=lu.layout || variant!=lu.variant;
	}
	
	bool operator==(const LayoutUnit& lu) const {
// 		kDebug() << layout << "==" << lu.layout << "&&" << variant << "==" << lu.variant << endl;
		return layout==lu.layout && variant==lu.variant;
	}
	
//private:
	static const QString parseLayout(const QString &layvar);
	static const QString parseVariant(const QString &layvar);
};

extern const LayoutUnit DEFAULT_LAYOUT_UNIT;
extern const char* DEFAULT_MODEL;


class KxkbConfig
{
public:
	enum { LOAD_INIT_OPTIONS, LOAD_ACTIVE_OPTIONS, LOAD_ALL };
	
	bool m_useKxkb;
	bool m_showSingle;
	bool m_showFlag;
	bool m_enableXkbOptions;
	bool m_resetOldOptions;
	SwitchingPolicy m_switchingPolicy;
	bool m_stickySwitching;
	int m_stickySwitchingDepth;
	
	QString m_model;
	QString m_options;
	QList<LayoutUnit> m_layouts;

	LayoutUnit getDefaultLayout();
	
	bool load(int loadMode);
	void save();
	void setDefaults();
	
	QStringList getLayoutStringList(/*bool compact*/);
	static QString getDefaultDisplayName(const QString& code_);
	static QString getDefaultDisplayName(const LayoutUnit& layoutUnit, bool single=false);

private:	
	static const QMap<QString, QString> parseIncludesMap(const QStringList& pairList);
};


#endif
