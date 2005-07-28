/*
    Copyright (C) 2000,2002 Carsten Pfeiffer <pfeiffer@kde.org>


    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/


#ifndef _KNOTIFY_H
#define _KNOTIFY_H

#include <qstring.h>
//Added by qt3to4:
#include <QLabel>

#include <kcmodule.h>
#include <kdialogbase.h>

class QCheckBox;
class QLabel;
class QSlider;

class KAboutData;
class KComboBox;
class KURLRequester;
class PlayerSettingsDialog;
class PlayerSettingsUI;

namespace KNotify
{
    class Application;
    class KNotifyWidget;
}

class KCMKNotify : public KCModule
{
    Q_OBJECT

public:
    KCMKNotify(QWidget *parent, const char *name, const QStringList &);
    virtual ~KCMKNotify();

    virtual void defaults();
    virtual void save();

public slots:
    virtual void load();

private slots:
    void slotAppActivated( const QString& app );
    void slotPlayerSettings();

private:
    KNotify::Application *applicationByDescription( const QString& text );

    KComboBox *m_appCombo;
    KNotify::KNotifyWidget *m_notifyWidget;
    PlayerSettingsDialog *m_playerSettings;

};

class PlayerSettingsDialog : public KDialogBase
{
    Q_OBJECT

public:
    PlayerSettingsDialog( QWidget *parent, bool modal );
    void load();
    void save();

protected slots:
    virtual void slotApply();
    virtual void slotOk();
    void externalToggled( bool on );
    void slotChanged();

private:
    PlayerSettingsUI* m_ui;
    bool dataChanged;
};


#endif
