/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kcmdisplay.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 * 
 * You can Freely distribute this program under the GNU General Public
 * License. See the file "COPYING" for the exact licensing terms.
 */

#ifndef __Icons_h_Included__
#define __Icons_h_Included__

#include <kcmodule.h>
#include <qvaluelist.h>
#include <qcolor.h>

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
class QIconView;
class QIconViewItem;

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

signals:
    void changed(bool);

private slots:
    void slotUsage(int index);
    void slotState(int index);
    void slotEffect(int index);
    void slotEffectValue(int value);
    void slotSize(int index);
    void slotDPCheck(bool check);

    void slotBgColorChanged(const QColor &);
    void slotNormalColorChanged(const QColor &);
    void slotHiliteColorChanged(const QColor &);
    void slotUnderline(bool);
    void slotWrap(bool);
    
private:
    void init();
    void read();
    void apply();
    void preview();

    bool mbDP[4], mbChanged[4];
    int mSizes[4];
    QValueList<int> mAvSizes[4];
    int mEffects[4][3];
    float mEffectValues[4][3];

    int mUsage, mState;
    QString mTheme, mExample;
    QStringList mGroups, mStates;

    KIconEffect *mpEffect;
    KIconTheme *mpTheme;
    KIconLoader *mpLoader;
    KConfig *mpConfig;

    QIconView *mpPreview;
    QIconViewItem *mpPreviewItem;
    
    QListBox *mpUsageList, *mpStateList;
    QComboBox *mpEffectBox, *mpSizeBox;
    QSlider *mpEffectSlider;
    QCheckBox *mpDPCheck, *wordWrapCB, *underlineCB;

    bool wrap, underline;	
    QColor bgColor, normalColor, hiliteColor;
};

#endif
