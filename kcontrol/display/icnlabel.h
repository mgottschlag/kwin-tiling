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

#ifndef __icnlabel_h__
#define __icnlabel_h__

#include <kcmodule.h>
#include <qvaluelist.h>
#include <qcolor.h>
#include <qdialog.h>

class QColor;
class QWidget;
class QCheckBox;
class QLabel;
class QIconView;
class QIconViewItem;                                                            
class QRadioButton;
class QPushButton;

class KConfig;
class KIconEffect;
class KIconLoader;
class KColorButton;

/**
 * The  Icon-Label tab in kcontrol.
 */
class KIconConfigLabel: public QWidget
{
    Q_OBJECT

public:
    KIconConfigLabel(QWidget *parent=0, const char *name=0);
    virtual void load();                                                        
    virtual void save();
    virtual void defaults();

signals:
    void changed(bool);

private slots:

    void slotBgColorChanged(const QColor &);
    void slotNormalColorChanged(const QColor &);
    void slotHiliteColorChanged(const QColor &);
    void slotUnderline(bool);
    void slotWrap(bool);     
    void slotUsage(int index);                                                   
    void slotChangeTbStyle(); 

private:
    void init();
    void read();
    void apply();
    void preview();

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
                                                    
    QIconView *mpPreview;     
    QIconViewItem *mpPreviewItem;     

    QPushButton *mpDTfont, *mpTBfont;
    QRadioButton *tbIcon, *tbText, *tbAside, *tbUnder;                          
    QListBox *mpUsageList;
    QCheckBox *wordWrapCB, *underlineCB;

    bool wrap, underline;
    QColor bgColor, normalColor, hiliteColor;                                   
};

                      
#endif
