// -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4; -*-
/*  Copyright (C) 2003 Lukas Tinkl <lukas@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "knewthemedlg.h"

#include <QLineEdit>
#include <QTextEdit>

#include <klocale.h>

KNewThemeDlg::KNewThemeDlg( QWidget * parent, const char * name )
    : KDialog( parent )
{
    setObjectName( name );
    setModal( true );
    setCaption( i18n("New Theme") );
    setButtons( Ok|Cancel );

    m_base = new NewThemeWidget( this );
    setMainWidget( m_base );
    connect( m_base->leName, SIGNAL( textChanged ( const QString & ) ), this, SLOT( slotThemeNameChanged( const QString & ) ) );
    slotThemeNameChanged( m_base->leName->text() );
}

KNewThemeDlg::~KNewThemeDlg()
{
    delete m_base;
}

void KNewThemeDlg::slotThemeNameChanged( const QString &_text )
{
     enableButtonOk( !_text.isEmpty() );
}

QString KNewThemeDlg::getName() const
{
    return m_base->leName->text();
}

QString KNewThemeDlg::getAuthor() const
{
    return m_base->leAuthor->text();
}

QString KNewThemeDlg::getEmail() const
{
    return m_base->leEmail->text();
}

QString KNewThemeDlg::getHomepage() const
{
    return m_base->leHomepage->text();
}

QString KNewThemeDlg::getComment() const
{
    return m_base->teComment->text();
}

QString KNewThemeDlg::getVersion() const
{
    return m_base->leVersion->text();
}

void KNewThemeDlg::setAuthor( const QString & author )
{
    m_base->leAuthor->setText( author );
}

void KNewThemeDlg::setEmail( const QString & email )
{
    m_base->leEmail->setText( email );
}

void KNewThemeDlg::setVersion( const QString & version )
{
    m_base->leVersion->setText( version );
}

void KNewThemeDlg::setName( const QString & name )
{
    m_base->leName->setText( name );
}

#include "knewthemedlg.moc"
