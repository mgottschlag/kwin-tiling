/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kcmdisplay.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 * 
 * You can Freely distribute this program under the GNU General Public
 * License. See the file "COPYING" for the exact licensing terms.
 */

#ifndef __Advanced_h_Included__
#define __Advanced_h_Included__

#include <qobject.h>
#include <kcmodule.h>

class QCheckBox;
class QSpinBox;
class KGlobalBackgroundSettings;

/**
 * The Desktop/Advanced tab in kcontrol.
 * TODO: Merge with background settings.
 */
class KAdvanced: public KCModule
{
    Q_OBJECT

public:
    KAdvanced(QWidget *parent, const char *name);

    virtual void load();
    virtual void save();
    virtual void defaults();

    int buttons();

signals:
    void changed(bool);

private slots:
    void slotLimitCache(bool);
    void slotCacheSize(int);

private:
    void apply();

    QCheckBox *m_pCBLimit;
    QSpinBox *m_pCacheBox;

    KGlobalBackgroundSettings *m_pSettings;
};


#endif // __Advanced_h_Included__
