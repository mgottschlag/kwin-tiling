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
#include <kcmodule.h>


class FontUseItem
{
public:
    FontUseItem(QString n, QFont default_fnt, bool fixed = false);

    QString fontString(QFont rFont);
    void setRC(QString group, QString key, QString rcfile=QString::null);

    void readFont();
    void writeFont();
    void setDefault();
    void setFont(const QFont &fnt ) { _font = fnt; }

    QFont font() { return _font; }
    const QString& rcFile() { return _rcfile; }
    const QString& rcGroup() { return _rcgroup; }
    const QString& rcKey() { return _rckey; }
    const QString& text() { return _text; }
    bool spacing() { return fixed; }
    void setSelect( bool flag )	{ selected = flag; }
    bool select() { return selected; }

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


/**
 * The Destkop/fonts tab in kcontrol.
 */
class KFonts : public KCModule
{
    Q_OBJECT
	
public:
    KFonts(QWidget *parent, const char *name);
    ~KFonts();

    virtual void load();
    virtual void save();
    virtual void defaults();
    virtual QString quickHelp();

    int buttons();

signals:
    void changed(bool);

private slots:
    void slotSetFont(const QFont &fnt);
    void slotPreviewFont( int index );

private:
    bool useRM;
    bool _changed;
    bool defaultCharset;
    
    QListBox *lbFonts;
    QList <FontUseItem> fontUseList;
    KFontChooser *fntChooser;
};

#endif

