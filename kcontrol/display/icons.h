/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kcmdisplay.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 * with minor additions and based on ideas from
 * Torsten Rahn <torsten@kde.org>                                               
 * 
 * You can Freely distribute this program under the GNU General Public
 * License. See the file "COPYING" for the exact licensing terms.
 */

#ifndef __Icons_h_Included__
#define __Icons_h_Included__

#include <kcmodule.h>
#include <qvaluelist.h>
#include <qcolor.h>
#include <qdialog.h>

class QColor;
class QWidget;
class QCheckBox;
class QComboBox;
class QListBox;
class QSlider;
class QLabel;
class KConfig;
class KIconEffect;
class KIconTheme;
class KIconLoader;
class KColorButton;

/**
 * The Desktop/icons tab in kcontrol.
 */
class KIconConfig: public KCModule
{
    Q_OBJECT

public:
    KIconConfig(QWidget *parent, const char *name);
    virtual void load();
    virtual void save();
    virtual void defaults();
    void preview();

signals:
    void changed(bool);

private slots:
    void slotEffectSetup();
    void slotUsage(int index);
    void slotState(int index);
    void slotEffect(int index);
    void slotSize(int index);
    void slotDPCheck(bool check);
    void slotSTCheck(bool check);

private:
    void init();
    void read();
    void apply();

    bool mbDP[6], mbChanged[6];
    int mSizes[6];
    QValueList<int> mAvSizes[6];
    int mEffects[6][3];
    float mEffectValues[6][3];
    QColor mEffectColors[6][3];
    bool mEffectTrans[6][3];

    int mUsage, mState;
    QString mTheme, mExample;
    QStringList mGroups, mStates;

    KIconEffect *mpEffect;
    KIconTheme *mpTheme;
    KIconLoader *mpLoader;
    KConfig *mpConfig;

    QLabel *mpPreview;
    QListBox *mpUsageList, *mpStateList;
    QComboBox *mpEffectBox, *mpSizeBox;
    QCheckBox *mpDPCheck;
    QCheckBox *mpSTCheck;
    QPushButton *mpESetupBut;
};

class KIconEffectSetupDialog: public QDialog
{
    Q_OBJECT
     
    public:
    KIconEffectSetupDialog(QColor m_pEfColor, 
    float m_pEfValue, int m_pEfTyp,                                          
    QWidget *parent=0L, char *name=0L);
    QColor fxcolor() { return m_pEfColor; }
    float fxvalue() { return m_pEfValue; }

signals:
    void changed(bool);


public slots:
    void slotHelp();
    void slotOK();    

private:
    QSlider *mpEffectSlider;
    KColorButton *mpEColButton;
    QColor m_pEfColor;
    float m_pEfValue;

private slots:
    void slotEffectValue(int value);
    void slotEffectColor(const QColor &col);

};                      
                      
#endif
