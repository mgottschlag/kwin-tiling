//-----------------------------------------------------------------------------
//
// kdisplay, fonts tab
//
// Copyright (c)  Mark Donohoe 1997
//                Lars Knoll 1999

#ifndef FONTS_H
#define FONTS_H

#include <QObject>
//Added by qt3to4:
#include <QLabel>
#include <Q3PtrList>

#include <kcmodule.h>
#include <kdialog.h>
#include <kfontdialog.h>
#include <kfontrequester.h>

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

class FontAASettings : public KDialog
{
  Q_OBJECT

public:

    FontAASettings(QWidget *parent);

    bool save( bool useAA );
    bool load();
    void defaults();
    int getIndex(KXftConfig::SubPixel::Type spType);
    KXftConfig::SubPixel::Type getSubPixelType();
#ifdef HAVE_FONTCONFIG
    int getIndex(KXftConfig::Hint::Style hStyle);
    KXftConfig::Hint::Style getHintStyle();
#endif
    void enableWidgets();
    int exec();

protected Q_SLOTS:

    void changed();

private:

    QCheckBox *excludeRange;
    QCheckBox *useSubPixel;
    KDoubleNumInput *excludeFrom;
    KDoubleNumInput *excludeTo;
    QComboBox *subPixelType;
#ifdef HAVE_FONTCONFIG
    QComboBox *hintingStyle;
#endif
    QLabel    *excludeToLabel;
    bool      changesMade;
};

/**
 * The Desktop/fonts tab in kcontrol.
 */
class KFonts : public KCModule
{
    Q_OBJECT

public:
    KFonts(QWidget *parent, const QStringList &);
    ~KFonts();

    virtual void load();
    virtual void save();
    virtual void defaults();

protected Q_SLOTS:
    void fontSelected();
    void slotApplyFontDiff(); 
    void slotUseAntiAliasing();
    void slotCfgAa();

private:
    enum AASetting { AAEnabled, AASystem, AADisabled };
    enum DPISetting { DPINone, DPI96, DPI120 };
    AASetting useAA, useAA_original;
    DPISetting dpi_original;
    QComboBox *cbAA;
    QComboBox* comboForceDpi;
    QPushButton *aaSettingsButton;
    Q3PtrList <FontUseItem> fontUseList;
    FontAASettings *aaSettings;
};

#endif

