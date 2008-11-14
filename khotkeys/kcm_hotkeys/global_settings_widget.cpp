/*
   Copyright (C) 2008 Michael Jansen <kde@michael-jansen.biz>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "global_settings_widget.h"

#include <KDebug>
#include <KDesktopFile>
#include <KGlobal>
#include <KStandardDirs>


GlobalSettingsWidget::GlobalSettingsWidget( QWidget *parent )
    : HotkeysWidgetIFace( parent )
    {
    ui.setupUi(this);

    QString path = KGlobal::dirs()->findResource( "services", "kded/khotkeys.desktop");
    if ( KDesktopFile::isDesktopFile(path) )
        {
        _config = KSharedConfig::openConfig(
                path,
                KConfig::NoGlobals,
                "services" );
        }

    connect(
            ui.enabled, SIGNAL(stateChanged(int)),
            _changedSignals, SLOT(map()) );
    _changedSignals->setMapping(ui.enabled, "enabled" );
    }


GlobalSettingsWidget::~GlobalSettingsWidget()
    {
    }


void GlobalSettingsWidget::doCopyFromObject()
    {
    if ( _config )
        {
        KConfigGroup file(_config, "Desktop Entry");
        ui.enabled->setChecked(file.readEntry("X-KDE-Kded-autoload", false));
        }
    }


void GlobalSettingsWidget::doCopyToObject()
    {
    kDebug();
    if (_config)
        {
        KConfigGroup file(_config, "Desktop Entry");
        file.writeEntry("X-KDE-Kded-autoload", ui.enabled->checkState()==Qt::Checked);
        _config->sync();
        }
    }


bool GlobalSettingsWidget::isChanged() const
    {
    if (_config)
        {
        KConfigGroup file(_config, "Desktop Entry");
        bool enabled = file.readEntry("X-KDE-Kded-autoload", false);

        if ( enabled && ( ui.enabled->checkState() != Qt::Checked ) )
            {
            return true;
            }

        if ( !enabled && ( ui.enabled->checkState() != Qt::Unchecked ) )
            {
            return true;
            }
        }
    return false;
    }


#include "moc_global_settings_widget.cpp"
