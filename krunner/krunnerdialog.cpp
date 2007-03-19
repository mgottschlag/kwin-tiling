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

#include "../plasma/lib/svg.h"

#include "krunnerdialog.h"
#include "krunnerapp.h"

KRunnerDialog::KRunnerDialog( QWidget * parent, Qt::WindowFlags f )
    : QDialog( parent, f )
{
    m_background = new Plasma::Svg( "/background/dialog", this );
    connect( m_background, SIGNAL(repaintNeeded()), this, SLOT(update()) );
}

KRunnerDialog::~KRunnerDialog()
{
}

void KRunnerDialog::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setClipRect(e->rect());
    //kDebug() << "clip rect set to: " << e->rect() << endl;

    if ( KRunnerApp::s_haveCompositeManager ) {
        //kDebug() << "gots us a compmgr!" << m_haveCompositionManager << endl;
        p.setCompositionMode( QPainter::CompositionMode_Source );
        p.fillRect( rect(), Qt::transparent );
    }

    m_background->paint( &p, 0, 0 );
}

void KRunnerDialog::resizeEvent(QResizeEvent *e)
{
    m_background->resize( e->size() );
    QDialog::resizeEvent( e );
}

#include "krunnerdialog.moc"
