//-----------------------------------------------------------------------------
//
// KDE Display fonts, styles setup module
//
// Copyright (c)  Mark Donohoe 1997
//

#ifndef __GENERAL_H__
#define __GENERAL_H__

#include <kthemebase.h>
#include <qwidget.h>
#include <qpushbutton.h>
#include <qlistbox.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qpainter.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <qstrlist.h>
#include <kspinbox.h>
#include <kcontrol.h>
#include <qbuttongroup.h>
#include <qgroupbox.h>
#include <ktablistbox.h>
#include <kfontdialog.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include "display.h"

class FontUseItem
{
public:
    FontUseItem( const QString& n, QFont default_fnt, bool fixed = false );
	QString fontString( QFont rFont );
	void setRC( const QString& group, const QString& key, const QString& rc = QString::null );
	void readFont();
	void writeFont();
	void setDefault();
	void setFont(const QFont &fnt ) { _font = fnt; }
	QFont font() { return _font; }
	const QString& rcFile() { return _rcfile; }
	const QString& rcGroup() { return _rcgroup; }
	const QString& rcKey() { return _rckey; }
	const QString& text()		{ return _text; }
	bool spacing() { return fixed; }
	void	setSelect( bool flag )	{ selected = flag; }
	bool	select()		{ return selected; }

private:
	QString _text;
	QString _rcfile;
	QString _rcgroup;
	QString _rckey;
	QFont _font;
	QFont _default;
	bool fixed;
	bool selected;
};

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
class KThemeListBox : public KTabListBox
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
    QStrList fileList;
    QString curName;
};


class KGeneral : public KDisplayModule
{
	Q_OBJECT
	
public:
	KGeneral( QWidget *parent, int mode, int desktop = 0 );
	~KGeneral();

	virtual void readSettings( int deskNum = 0 );
	virtual void apply( bool Force = FALSE);
	virtual void loadSettings() {};
	virtual void applySettings();
	virtual void defaultSettings();
	
	GUIStyle applicationStyle;
	
	Display *kde_display;
	Atom 	KDEChangeGeneral;

protected slots:
        void slotChangeStylePlugin(int, int);
        void slotChangeTbStyle();//CT 05Apr1999
	void slotUseResourceManager();
	void slotMacStyle();//CT
	void slotApply();
	void slotHelp();

protected:
	void writeSettings();
	void setDefaults();
	
protected:
	//CT 04Apr1999
	QGroupBox *styles, *tbStyle;
	//QRadioButton *MStyle, *WStyle, *PStyle;
	QRadioButton *tbIcon, *tbText, *tbAside, *tbUnder;
	QCheckBox *tbHilite, *tbTransp;
      	QCheckBox *cbRes;
	//CT

	QCheckBox *cbMac;//CT
	Bool changed;
	
	bool useRM;
	bool macStyle;//CT
	Window root;
	int screen;

	int tbUseText;//CT 05Apr1999
	bool tbUseHilite, tbMoveTransparent;//CT 04Apr1999
       
        KIconStyle * iconStyle;
        KThemeListBox *themeList;
};

class KFonts : public KDisplayModule
{
	Q_OBJECT
	
public:
	KFonts( QWidget *parent, int mode, int desktop = 0 );
	~KFonts();

	virtual void readSettings( int deskNum = 0 );
	virtual void apply( bool Force = FALSE);
	virtual void loadSettings() {};
	virtual void applySettings();
	virtual void defaultSettings();
	
	Display *kde_display;
	Atom 	KDEChangeGeneral;

protected slots:
	void slotApply();
	void slotSetFont(const QFont &fnt);
	void slotPreviewFont( int index );
	void slotHelp();

protected:
	void writeSettings();
	void setDefaults();
	
protected:
	KFontChooser *fntChooser;
	QListBox *lbFonts;
	bool useRM;
	
	Bool changed;
	
	QList <FontUseItem> fontUseList;
	
	Bool defaultCharset;
	Window root;
	int screen;
};

#endif

