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
#ifndef klinespellchecking_h
#define klinespellchecking_h

#include <KLineEdit>

class KAction;

class KLineSpellChecking : public KLineEdit
{
    Q_OBJECT
public:
    KLineSpellChecking( QWidget *parent = 0 );
    ~KLineSpellChecking();

    void highLightWord( unsigned int length, unsigned int pos );

protected:
    virtual void contextMenuEvent(QContextMenuEvent *e);

private slots:
    void slotCheckSpelling();
    void slotSpellCheckDone( const QString &s );
    void spellCheckerMisspelling( const QString &text, int pos);
    void spellCheckerCorrected( const QString &, int, const QString &);
    void spellCheckerFinished();

private:
    KAction *m_spellAction;
};


#endif
