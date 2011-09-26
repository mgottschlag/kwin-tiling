/*
 *   Copyright (C) 2008 Montel Laurent <montel@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */
#include "klinespellchecking.h"
#include <QMenu>
#include <KStandardAction>
#include <KActionCollection>
#include <KAction>
#include <QContextMenuEvent>
#include <sonnet/dialog.h>
#include <sonnet/backgroundchecker.h>

KLineSpellChecking::KLineSpellChecking(QWidget* parent)
    : KLineEdit(parent)
{
    KActionCollection *ac = new KActionCollection(this);
    m_spellAction = KStandardAction::spelling( this, SLOT(slotCheckSpelling()), ac );
}

KLineSpellChecking::~KLineSpellChecking()
{
}

void KLineSpellChecking::slotCheckSpelling()
{
    if ( text().isEmpty() ) {
        return;
    }
    Sonnet::Dialog *spellDialog = new Sonnet::Dialog(new Sonnet::BackgroundChecker(this), 0);
    connect(spellDialog, SIGNAL(replace(QString,int,QString)), this, SLOT(spellCheckerCorrected(QString,int,QString)));
    connect(spellDialog, SIGNAL(misspelling(QString,int)), this, SLOT(spellCheckerMisspelling(QString,int)));
    connect(spellDialog, SIGNAL(done(QString)), this, SLOT(slotSpellCheckDone(QString)));
    connect(spellDialog, SIGNAL(cancel()), this, SLOT(spellCheckerFinished()));
    connect(spellDialog, SIGNAL(stop()), this, SLOT(spellCheckerFinished()));
    spellDialog->setBuffer(text());
    spellDialog->show();
}

void KLineSpellChecking::spellCheckerMisspelling( const QString &_text, int pos)
{
    highLightWord( _text.length(),pos );
}

void KLineSpellChecking::highLightWord( unsigned int length, unsigned int pos )
{
    setSelection ( pos, length );
}

void KLineSpellChecking::spellCheckerCorrected( const QString &old, int pos, const QString &corr )
{
    if( old!= corr )
    {
        setSelection ( pos, old.length() );
        insert( corr );
        setSelection ( pos, corr.length() );
    }
}

void KLineSpellChecking::spellCheckerFinished()
{
}

void KLineSpellChecking::slotSpellCheckDone( const QString &s )
{
    if( s != text() )
        setText( s );
}

void KLineSpellChecking::contextMenuEvent(QContextMenuEvent *e)
{
    QMenu* popup = createStandardContextMenu();

    if ( !popup )
        return;

    if (echoMode() == QLineEdit::Normal &&
        !isReadOnly()) {
        popup->addSeparator();

        popup->addAction( m_spellAction );
        m_spellAction->setEnabled( !text().isEmpty() );
    }
    popup->exec(e->globalPos());
    delete popup;
}



#include "klinespellchecking.moc"


