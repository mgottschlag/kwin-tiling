/* vi: ts=8 sts=4 sw=4
 *
 *
 *
 * This file is part of the KDE project, module kcontrol.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 *
 * You can Freely distribute this program under the GNU General Public
 * License. See the file "COPYING" for the exact licensing terms.
 *
 * Based on kcontrol1 energy.h, Copyright (c) 1999 Tom Vijlbrief.
 */

#ifndef __Energy_h_Included__
#define __Energy_h_Included__

#include <qobject.h>
#include <kcmodule.h>

class QCheckBox;
class KIntNumInput;
class KConfig;
class KInstance;

extern "C" void init_energy();

/**
 * The Desktop/Energy tab in kcontrol.
 */
class KEnergy: public KCModule
{
    Q_OBJECT

public:
    KEnergy(KInstance *inst, QWidget *parent);
    ~KEnergy();

    virtual void load();
    virtual void save();
    virtual void defaults();

private slots:
    void slotChangeEnable(bool);
    void slotChangeStandby(int);
    void slotChangeSuspend(int);
    void slotChangeOff(int);
    void openURL(const QString &);

private:
    void readSettings();
    void writeSettings();
    void showSettings();

    static void applySettings(bool, int, int, int);
    friend void init_energy();

    bool m_bChanged, m_bDPMS, m_bEnabled, m_bMaintainSanity;
    int m_Standby, m_Suspend, m_Off;
    int m_StandbyDesired, m_SuspendDesired, m_OffDesired;

    QCheckBox *m_pCBEnable;
    KIntNumInput *m_pStandbySlider;
    KIntNumInput *m_pSuspendSlider;
    KIntNumInput *m_pOffSlider;
    KConfig *m_pConfig;
};

#endif // __Energy_h_Included__
