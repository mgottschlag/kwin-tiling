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
#include <qlistview.h>
#include <qgroupbox.h>

#include <kthemebase.h>
#include <kcmodule.h>

class QCheckBox;
class QRadioButton;
class QButtonGroup;
class QBoxLayout;
class KConfig;


// DF 13-Mar-99
// This class is a part of the "style" tab.
// It's separated from the KGeneral class, in case it has to be moved
// somewhere else later.


class KIconStyle: public QGroupBox
{
    Q_OBJECT

public:
    KIconStyle(QWidget *parent=0L, const char *name=0L);
    ~KIconStyle();

    void load();
    void save();
    void defaults();

signals:
    void changed(bool);

private slots:
    void slotPanel(int);
    void slotKonq(int);
    void slotKDE(int);

private:
    bool bChanged;
    int m_PanelStyle, m_KonqStyle, m_KDEStyle;

    QButtonGroup *panelGroup, *konqGroup, *kdeGroup;
    KConfig *config;
};


/**
 * Mosfet's themelist box.
 */
class KThemeListBox : public QListView
{
    Q_OBJECT

public:
    KThemeListBox(QWidget *parent=0, const char *name=0);
    ~KThemeListBox();

    void save();
    void load();
    void defaults();

private:
    void readThemeDir(const QString &directory);

    QString curName;
    QListViewItem *curItem, *defItem;
    KConfig *kconfig;
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

signals:
    void changed(bool);

private slots:
    void slotChangeStylePlugin(QListViewItem *);
    void slotChangeTbStyle();
    void slotUseResourceManager();
    void slotMacStyle();

private:
    void readSettings();
    void showSettings();

    bool m_bChanged;
    bool useRM, macStyle;
    bool tbUseHilite, tbMoveTransparent;

    int tbUseText;

    QGroupBox *styles, *tbStyle;
    QRadioButton *tbIcon, *tbText, *tbAside, *tbUnder;
    QCheckBox *tbHilite, *tbTransp;
    QCheckBox *cbRes;
    QCheckBox *cbMac;

    GUIStyle applicationStyle;

    KIconStyle * iconStyle;
    KThemeListBox *themeList;
    KConfig *config;
};


#endif
