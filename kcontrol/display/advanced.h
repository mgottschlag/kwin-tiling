/* vi: ts=8 sts=4 sw=4
 *
 * $Id: $
 *
 * This file is part of the KDE project, module kdesktop.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 * 
 * You can Freely distribute this program under the GNU General Public
 * License. See the file "COPYING" for the exact licensing terms.
 */

#ifndef __Advanced_h_Included__
#define __Advanced_h_Included__

#include <qobject.h>
#include "display.h"

class QCheckBox;
class QSpinBox;
class KGlobalBackgroundSettings;

/**
 * The "advanced" tab in kcmdisplay.
 */
class KAdvanced: public KDisplayModule
{
    Q_OBJECT

public:
    KAdvanced(QWidget *parent, Mode mode);

    virtual void loadSettings();
    virtual void applySettings();
    virtual void defaultSettings();

private slots:
    void slotLimitCache(bool);
    void slotCacheSize(int);
    void slotExportBackground(bool);
    void slotDockPanel(bool);

private:
    void apply();

    QCheckBox *m_pCBLimit, *m_pCBExport;
    QCheckBox *m_pCBDock;
    QSpinBox *m_pCacheBox;

    KGlobalBackgroundSettings *m_pSettings;
};


#endif // __Advanced_h_Included__
