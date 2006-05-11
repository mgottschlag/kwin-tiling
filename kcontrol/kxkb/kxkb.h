/*
    Copyright (C) 2001, S.R.Haque <srhaque@iee.org>. Derived from an
    original by Matthias Hölzer-Klüpfel released under the QPL.
    This file is part of the KDE project

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

DESCRIPTION

    KDE Keyboard Tool. Manages XKB keyboard mappings.
*/
#ifndef __K_XKB_H__
#define __K_XKB_H__

#include <ksystemtray.h>
#include <kuniqueapplication.h>
#include <QString>
#include <qstringlist.h>
#include <q3dict.h>
#include <QQueue>
//Added by qt3to4:
#include <QMouseEvent>

class QWidget;
class XKBExtension;
class KeyRules;
class KGlobalAccel;
class KWinModule;

/* This class is responsible for displaying flag/label for the layout,
    catching keyboard/mouse events and displaying menu when selected
*/

class TrayWindow : public KSystemTray
{
    Q_OBJECT

public:

    TrayWindow(QWidget *parent=0, const char *name=0);

    void setLayouts(const QStringList& layouts, const KeyRules& rule);
    void setCurrentLayout(const QString& layout);
    void setError(const QString& layout="");
    void setShowFlag(bool showFlag) { m_showFlag = showFlag; }

    KMenu* contextMenu() { return KSystemTray::contextMenu(); };

Q_SIGNALS:

    void toggled();

protected:

    void mouseReleaseEvent(QMouseEvent *);

private:

    int mPrevMenuCount;
    QMap<QString, QString> mDescriptionMap;
    bool m_showFlag;
};

/* Utility classes for per-window/per-application layout implementation
*/


    enum SwitchingPolicy { 
	swpGlobal,
	swpWinClass,
	swpWindow
    }; 

    struct LayoutInfo {

	QString layout;
	int group;
	QQueue<QString *> *lastLayout;

	LayoutInfo()
	 {}	
	LayoutInfo(const QString& layout_, int group_, QQueue<QString *> *lastLayout_):
	 layout(layout_),
	 group(group_),
	 lastLayout(lastLayout_)
  	 {}
	
	LayoutInfo(const LayoutInfo& info2) {
	 layout = info2.layout; group = info2.group;
	}
	 
	QQueue<QString *> *getLastLayout() const { return lastLayout; }

	private:
	 bool operator==(const LayoutInfo& info2)
	 { return layout == info2.layout && group == info2.group; }
    };

    class LayoutMap {
	typedef QMap<WId, LayoutInfo> WinLayoutMap;
	typedef QMap<QString, LayoutInfo> WinClassLayoutMap;

     public:

      void setMode(SwitchingPolicy mode);
      SwitchingPolicy getMode();
      void setLayout(WId winId, const LayoutInfo& layoutInfo);
      const LayoutInfo& getLayout(WId winId);

     private:

	KWinModule* kWinModule;

    // pseudo-union
	WinLayoutMap m_winLayouts;
	WinClassLayoutMap m_appLayouts;
	
	SwitchingPolicy m_ownerMode;
    };



/* This is the main Kxkb class responsible for reading options
    and switching layouts
*/

class KXKBApp : public KUniqueApplication
{
    Q_OBJECT
    K_DCOP

public:

    KXKBApp(bool allowStyles=true, bool GUIenabled=true);
    ~KXKBApp();

    virtual int newInstance();
    
k_dcop:
    bool setLayout(const QString& layout);
    QString getCurrentLayout() { return m_layout; }
    QStringList getLayoutsList() { return m_list; }
    void forceSetXKBMap( bool set );

protected Q_SLOTS:

    void menuActivated(int id);
    void toggled();
    void windowChanged(WId winId);

    void slotSettingsChanged(int category);

protected:

    // Read settings, and apply them.
    bool settingsRead();
    void layoutApply();
    
private:

    void precompileLayouts();
    void deletePrecompiledLayouts();

private:
    
    WId prevWinId;	// for tricky part of saving xkb group
    LayoutMap m_layoutOwnerMap;

    bool m_resetOldOptions;
//    QString m_rule;
    QString m_model;
    QString m_layout;
    QString m_options;
    QString m_defaultLayout;
    Q3Dict<char> m_variants;
    Q3Dict<char> m_includes;
    unsigned int m_group;
    QStringList m_list;
    QMap<QString,QString> m_compiledLayoutFileNames;
    bool m_stickySwitching;
    QQueue<QString *> *m_lastLayout;
    int m_stickySwitchingDepth;
    
    XKBExtension *m_extension;
    KeyRules *m_rules;
    TrayWindow *m_tray;
    KGlobalAccel *keys;
    KWinModule* kWinModule;
    bool m_forceSetXKBMap;
};

#endif
