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

class QCheckBox;

class FontUseItem : public QObject
{
  Q_OBJECT

public:
    FontUseItem(QWidget * parent, QLabel * prvw, QString n, QString grp, QString key, QString rc, QFont default_fnt, bool fixed = false);

    QString fontString(QFont rFont);

    void readFont();
    void writeFont();
    void setDefault();
    void setFont(const QFont &fnt ) { _font = fnt; }
    void applyFontDiff(const QFont &fnt, int fontDiffFlags);

    QFont font() { return _font; }
    const QString& rcFile() { return _rcfile; }
    const QString& rcGroup() { return _rcgroup; }
    const QString& rcKey() { return _rckey; }
    const QString& text() { return _text; }
    bool spacing() { return fixed; }

signals:
    void changed();

public slots:
    void choose();

private:
    void updateLabel();
    QWidget * prnt;
    QLabel * preview;
    QString _text;
    QString _rcfile;
    QString _rcgroup;
    QString _rckey;
    QFont _font;
    QFont _default;
    bool fixed;
};


/**
 * The Desktop/fonts tab in kcontrol.
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
    virtual QString quickHelp() const;

    int buttons();

signals:
    void changed(bool);

protected slots:
    void fontChanged();
    void slotApplyFontDiff(); 
    void slotUseAntiAliasing();

private:
    bool _changed;
    bool useAA, useAA_original;
    QCheckBox *cbAA;
    QPtrList <FontUseItem> fontUseList;
};

#endif

