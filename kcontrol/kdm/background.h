/* vi: ts=8 sts=4 sw=4
 *
 * This file is part of the KDE project, module kcmdisplay.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 *
 * You can Freely distribute this program under the GNU General Public
 * License. See the file "COPYING" for the exact licensing terms.
 */

#ifndef __Bgnd_h_Included__
#define __Bgnd_h_Included__

#include <QObject>
#include <QWidget>


class KSimpleConfig;
class BGDialog;
class KGlobalBackgroundSettings;
class QCheckBox;
class QLabel;

class KBackground: public QWidget
{
    Q_OBJECT
public:
    KBackground(QWidget *parent=0);
    ~KBackground();

    void load();
    void save();
    void defaults();
    void makeReadOnly();
Q_SIGNALS:
    void changed(bool);

private Q_SLOTS:
    void slotEnableChanged();
private:
    void init();
    void apply();

    QCheckBox *m_pCBEnable;
    QLabel *m_pMLabel;
    KSimpleConfig *m_simpleConf;
    BGDialog *m_background;
};


#endif // __Bgnd_h_Included__
