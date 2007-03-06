/*
 *   Copyright (C) 2006 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <QPainter>
#include <QSvgRenderer>
#include <QResizeEvent>

#include <KDebug>

#include "../plasma/lib/theme.h"

#include "krunnerdialog.h"
#include "krunnerapp.h"

KRunnerDialog::KRunnerDialog( QWidget * parent, Qt::WindowFlags f )
    : QDialog( parent, f ),
      m_bgRenderer( 0 ),
      m_renderDirty( true )

{
    m_theme = new Plasma::Theme( this );
    themeChanged();
    connect( m_theme, SIGNAL(changed()), this, SLOT(themeChanged()) );
}

KRunnerDialog::~KRunnerDialog()
{
}

void KRunnerDialog::themeChanged()
{
    delete m_bgRenderer;
    kDebug() << "themeChanged() to " << m_theme->themeName()
             << "and we have " << m_theme->image("/background/dialog") << endl;
    m_bgRenderer = new QSvgRenderer( m_theme->image( "/background/dialog" ), this );
}

void KRunnerDialog::paintEvent(QPaintEvent *e)
{
    kDebug() << "paint event!" << endl;
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setClipRect(e->rect());

    if ( KRunnerApp::s_haveCompositeManager ) {
        //kDebug() << "gots us a compmgr!" << m_haveCompositionManager << endl;
        p.save();
        p.setCompositionMode( QPainter::CompositionMode_Source );
        p.fillRect( rect(), Qt::transparent );
        p.restore();
    }

    if ( m_renderDirty ) {
        m_renderedSvg.fill( Qt::transparent );
        QPainter p( &m_renderedSvg );
        p.setRenderHint( QPainter::Antialiasing );
        m_bgRenderer->render( &p);
        p.end();
        m_renderDirty = false;
    }

    p.drawPixmap( 0, 0, m_renderedSvg );
}

void KRunnerDialog::resizeEvent(QResizeEvent *e)
{
    if ( e->size() != m_renderedSvg.size() ) {
        m_renderedSvg = QPixmap( e->size() );
        m_renderDirty = true;
    }

    QDialog::resizeEvent( e );
}

#include "krunnerdialog.moc"
