/**
 *
 * Copyright (c) 2002-2004 George Staikos <staikos@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#include "xineramawidget.h"

XineramaWidget::XineramaWidget( QWidget* parent )
    : QWidget( parent ), Ui_XineramaWidget()
{
    setupUi( this );
    connect(_enableXinerama, SIGNAL(toggled(bool)), _enableResistance, SLOT(setEnabled(bool)));
    connect(_enableXinerama, SIGNAL(toggled(bool)), _enablePlacement, SLOT(setEnabled(bool)));
    connect(_enableXinerama, SIGNAL(toggled(bool)), _enableMaximize, SLOT(setEnabled(bool)));
    connect(_enableXinerama, SIGNAL(toggled(bool)), TextLabel2, SLOT(setEnabled(bool)));
    connect(_enableXinerama, SIGNAL(toggled(bool)), _unmanagedDisplay, SLOT(setEnabled(bool)));
    connect(_enableXinerama, SIGNAL(clicked()), _unmanagedDisplay, SLOT(emitConfigChanged()));
    connect(_enableResistance,SIGNAL(clicked()), this, SLOT(emitConfigChanged()));
    connect(_enablePlacement,SIGNAL(clicked()), this, SLOT(emitConfigChanged()));
    connect(_enableMaximize,SIGNAL(clicked()), this, SLOT(emitConfigChanged()));
    connect(_unmanagedDisplay,SIGNAL(activated(int)),this,SLOT(emitConfigChanged()));
    connect(_enableXinerama, SIGNAL(toggled(bool)), TextLabel2_2, SLOT(setEnabled(bool)));
    connect(_enableXinerama, SIGNAL(toggled(bool)), _ksplashDisplay, SLOT(setEnabled(bool)));
    connect(_ksplashDisplay, SIGNAL(activated(int)), this, SLOT(emitConfigChanged()));
    connect(_enableFullscreen,SIGNAL(clicked()), this, SLOT(emitConfigChanged()));
    connect(_enableXinerama,SIGNAL(toggled(bool)), _enableFullscreen, SLOT(setEnabled(bool)));
}


void XineramaWidget::emitConfigChanged()
{
emit configChanged();
}

#include "xineramawidget.moc"
