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
#include <qgroupbox.h>

#include <klistview.h>
#include <kcmodule.h>

class QCheckBox;
class QRadioButton;
class QButtonGroup;
class QBoxLayout;
class KConfig;
class KAboutData;


/**
 * Mosfet's themelist box.
 */
class KThemeListBox : public KListView
{
    Q_OBJECT

public:
    KThemeListBox(QWidget *parent=0, const char *name=0);
    ~KThemeListBox();

    void save();
    void load();
    void defaults();
public slots:
    void rescan();

private:
    void readTheme(const QString &file);

    QString localThemeDir;
    QString curTheme;
    QString defName;
    QListViewItem *curItem, *defItem;
};


/**
 * This is the "Style" tab in kcontrol/Desktop.
 */
class KGeneral: public KCModule
{
    Q_OBJECT

public:
    KGeneral(QWidget *parent, const char *name);
    ~KGeneral();

    virtual void load();
    virtual void save();
    virtual void defaults();

    virtual QString quickHelp() const;
    const KAboutData* aboutData() const;

signals:
    void changed(bool);

private slots:
    void slotChangeStylePlugin(QListViewItem *);
    void slotChangeTbStyle();
    void slotChangeEffectStyle();
    void slotUseResourceManager();
    void slotMacStyle();
    void slotRunImporter();
    void slotUseIcons();


private:
    void readSettings();
    void showSettings();

    bool m_bChanged, m_bStyleDirty, m_bToolbarsDirty;
    bool m_bEffectsDirty, m_bMacStyleDirty, m_bIconsDirty;
    bool m_bExportColors;
    bool macStyle, icons;
    bool tbUseHilite, tbMoveTransparent;
    bool effectFadeMenu, effectAnimateMenu, effectAnimateCombo, effectFadeTooltip, effectAnimateTooltip, effectNoTooltip;
    QString tbUseText;

    QGroupBox *styles, *tbStyle;
    /*
    QGroupBox *effectStyleMenu, *effectStyleTooltip;
    QRadioButton *tbIcon, *tbText, *tbAside, *tbUnder;
    QCheckBox *tbHilite, *tbTransp;
    QCheckBox *cbRes;
    */
    QCheckBox *cbMac;
    /*
    QCheckBox *cbIcons;
    QRadioButton *effPlainMenu, *effFadeMenu, *effAnimateMenu, *effPlainTooltip, *effFadeTooltip, *effAnimateTooltip;
    QCheckBox *effAnimateCombo, *effNoTooltip;
*/
    GUIStyle applicationStyle;

    KThemeListBox *themeList;
    KConfig *config;
};


#endif
