//-----------------------------------------------------------------------------
//
// kdisplay, fonts tab
//
// Copyright (c)  Mark Donohoe 1997
//                Lars Knoll 1999

#ifndef FONTS_H
#define FONTS_H

#include <qobject.h>
#include <kfontdialog.h>

#include "display.h"

#include <X11/X.h>

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

class KFonts : public KDisplayModule
{
	Q_OBJECT
	
public:
	KFonts(QWidget *parent, Mode mode);
	~KFonts();

	virtual void readSettings( int deskNum = 0 );
	virtual void loadSettings() {};
	virtual void applySettings();
	virtual void defaultSettings();
	
	Display *kde_display;
	Atom 	KDEChangeGeneral;

protected slots:
	void slotSetFont(const QFont &fnt);
	void slotPreviewFont( int index );

protected:
	void apply();
	void writeSettings();
	void setDefaults();
	
protected:
	KFontChooser *fntChooser;
	QListBox *lbFonts;
	bool useRM;
	
	bool changed;
	
	QList <FontUseItem> fontUseList;
	
	bool defaultCharset;
	Window root;
	int screen;
};

#endif

