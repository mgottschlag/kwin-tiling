//-----------------------------------------------------------------------------
//
// KDE Display fonts, styles setup module
//
// Copyright (c)  Mark Donohoe 1997
//

#ifndef __GENERAL_H__
#define __GENERAL_H__

#include <qdict.h>
#include <qwidget.h>
#include <qstrlist.h>
#include <qbuttongroup.h>
#include <qlistview.h>

#include <kthemebase.h>
#include <kcontrol.h>
#include <kfontdialog.h>

#include "display.h"

class QCheckBox;
class QRadioButton;
class QButtonGroup;
class QBoxLayout;

// DF 13-Mar-99
// This class is a part of the "style" tab.
// It's separated from the KGeneral class, in case it has to be moved
// somewhere else later.

class KIconStyle // : public KDisplayModule
{
    //Q_OBJECT
public:
	KIconStyle( QWidget *parent, QBoxLayout * topLayout );
	~KIconStyle();

	void apply();
	void readSettings();
	
	void writeSettings();
	void setDefaults();

protected:
	static const char *appName [];
	static const int nApp;

	QDict <char> m_dictSettings; // initial setting for each application
	QDict <QRadioButton> m_dictCBNormal; // checkbox 'normal' for each application
	QDict <QRadioButton> m_dictCBLarge;  // checkbox 'large' for each application
};

// mosfet 4/26/99
class KThemeListBox : public QListView
{
    Q_OBJECT
public:
    KThemeListBox(QWidget *parent=0, const char *name=0);
    ~KThemeListBox(){;}
    QString currentFile();
    // This is the currently set name, not selected name.
    QString currentName();
    void writeSettings();
    void apply();
protected:
    void readThemeDir(const QString &directory);
    QString curName;
};


class KGeneral : public KDisplayModule
{
	Q_OBJECT
	
public:
	KGeneral(QWidget *parent, Mode mode);
	~KGeneral();

	virtual void readSettings( int deskNum = 0 );
	virtual void loadSettings() {};
	virtual void applySettings();
	virtual void defaultSettings();
	
	GUIStyle applicationStyle;
	
	Display *kde_display;
	Atom 	KDEChangeGeneral;

protected slots:
    void slotChangeStylePlugin(QListViewItem*);
    void slotChangeTbStyle();
	void slotUseResourceManager();
	void slotMacStyle();

protected:
	void apply();
	void writeSettings();
	void setDefaults();
	
protected:
	QGroupBox *styles, *tbStyle;
	QRadioButton *tbIcon, *tbText, *tbAside, *tbUnder;
	QCheckBox *tbHilite, *tbTransp;
    QCheckBox *cbRes;

	QCheckBox *cbMac;
	bool changed;
	
	bool useRM;
	bool macStyle;
	Window root;
	int screen;

	int tbUseText;
	bool tbUseHilite, tbMoveTransparent;
       
    KIconStyle * iconStyle;
    KThemeListBox *themeList;
};

#endif

