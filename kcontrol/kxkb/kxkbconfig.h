/*
 *  Copyright (C) 2006 Andriy Rysin (rysin@kde.org)
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

#ifndef KXKBCONFIG_H
#define KXKBCONFIG_H


#include <QQueue>
#include <QMap>


const int GROUP_LIMIT = 4;
const int MAX_LABEL_LEN = 3;

/* Utility classes for per-window/per-application layout implementation
*/
enum SwitchingPolicy { 
	SWITCH_POLICY_GLOBAL = 0,
	SWITCH_POLICY_DESKTOP = 1,
	SWITCH_POLICY_WIN_CLASS = 2,
	SWITCH_POLICY_WINDOW = 3,
	SWITCH_POLICY_COUNT = 4
};


inline QString createPair(QString key, QString value) 
{
	if( value.isEmpty() )
		return key;
	return QString("%1(%2)").arg(key, value);
} 

struct LayoutUnit {
private:
	QString displayName;
public:
	QString layout;
	QString variant;
	
	LayoutUnit() {}
	
	LayoutUnit(QString layout_, QString variant_):
		layout(layout_),
		variant(variant_)
	{}
	
	LayoutUnit(QString pair) {
		setFromPair( pair );
	}

        void setDisplayName(const QString& name) { displayName = name; }

        QString getDisplayName() const {
            return !displayName.isEmpty() ? displayName : getDefaultDisplayName(layout, variant);
        }
	
	void setFromPair(const QString& pair) {
		layout = parseLayout(pair);
		variant = parseVariant(pair);
	}
	
	QString toPair() const {
		return createPair(layout, variant);
	}
/*	
	bool operator<(const LayoutUnit& lu) const {
		return layout<lu.layout ||
				(layout==lu.layout && variant<lu.variant);
	}
*/		
	bool operator!=(const LayoutUnit& lu) const {
		return layout!=lu.layout || variant!=lu.variant;
	}
	bool operator==(const LayoutUnit& lu) const {
// 		kDebug() << layout << "==" << lu.layout << "&&" << variant << "==" << lu.variant;
		return layout==lu.layout && variant==lu.variant;
	}


        static QString getDefaultDisplayName(const QString& layout, const QString& variant="");
	
//private:
	static const QString parseLayout(const QString &layvar);
	static const QString parseVariant(const QString &layvar);
};

extern const LayoutUnit DEFAULT_LAYOUT_UNIT;
extern const char* DEFAULT_MODEL;

struct XkbConfig {
    QString model;
    QStringList options;
    QList<LayoutUnit> layouts;
};

class KxkbConfig
{
public:
	enum { LOAD_ACTIVE_OPTIONS, LOAD_ALL };
        static const char* OPTIONS_SEPARATOR;
	
	bool m_useKxkb;
	bool m_indicatorOnly;
	bool m_showSingle;
	bool m_showFlag;
//	bool m_enableXkbOptions;
	bool m_resetOldOptions;
	bool m_stickySwitching;
	int m_stickySwitchingDepth;
	SwitchingPolicy m_switchingPolicy;
	
	QString m_model;
	QStringList m_options;
	QList<LayoutUnit> m_layouts;

	KxkbConfig();
	int getDefaultLayout();
	
	bool load(int loadMode);
	void setConfiguredLayouts(XkbConfig xkbConfig);
	void save();
	void setDefaults();
	
	QStringList getLayoutStringList(/*bool compact*/);

	void updateDisplayNames();

private:
	static const QMap<QString, QString> parseIncludesMap(const QStringList& pairList);
};


#endif
