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
#include <kfontrequester.h>
#include <kdialogbase.h>
#include "kxftconfig.h"

class QCheckBox;
class QComboBox;
class KDoubleNumInput;
class FontAASettings;

class FontUseItem : public KFontRequester
{
  Q_OBJECT

public:
    FontUseItem(QWidget * parent, const QString &name, const QString &grp, 
        const QString &key, const QString &rc, const QFont &default_fnt, 
        bool fixed = false);

    void readFont();
    void writeFont();
    void setDefault();
    void applyFontDiff(const QFont &fnt, int fontDiffFlags);

    const QString& rcFile() { return _rcfile; }
    const QString& rcGroup() { return _rcgroup; }
    const QString& rcKey() { return _rckey; }

private:
    QString _rcfile;
    QString _rcgroup;
    QString _rckey;
    QFont _default;
};

class FontAASettings : public KDialogBase
{
  Q_OBJECT

public:

    FontAASettings(QWidget *parent);

    bool save();
    void load();
    void defaults();
    int getIndex(KXftConfig::SubPixel::Type spType);
    KXftConfig::SubPixel::Type getSubPixelType();
    void enableWidgets();
    int exec();

protected slots:

    void changed();

private:

    QCheckBox *excludeRange;
    QCheckBox *useSubPixel;
    KDoubleNumInput *excludeFrom;
    KDoubleNumInput *excludeTo;
    QComboBox *subPixelType;
    QComboBox *hintingStyle;
    QLabel    *excludeToLabel;
};

/**
 * The Desktop/fonts tab in kcontrol.
 */
class KFonts : public KCModule
{
    Q_OBJECT

public:
    KFonts(QWidget *parent, const char *name, const QStringList &);
    ~KFonts();

    virtual void load();
    virtual void save();
    virtual void defaults();
    virtual QString quickHelp() const;

    int buttons();

protected slots:
    void fontSelected();
    void slotApplyFontDiff(); 
    void slotUseAntiAliasing();
    void slotCfgAa();

private:
    bool _changed;
    bool useAA, useAA_original;
    QCheckBox *cbAA;
    QPtrList <FontUseItem> fontUseList;
    FontAASettings *aaSettings;
};

#endif

