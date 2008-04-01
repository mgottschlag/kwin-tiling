/*
 *   Copyright (C) 2008 Laurent Montel <montel@kde.org>
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

#ifndef __PREFERENCESDLG_h__
#define __PREFERENCESDLG_h__

#include <kpagedialog.h>

class SpellCheckingPage;

namespace Sonnet {
    class ConfigWidget;
}

class PreferencesDialog : public KPageDialog
{
    Q_OBJECT
public:
    PreferencesDialog( QWidget *parent );

protected slots:
    void slotSave();

private:
    SpellCheckingPage *m_pageSpellChecking;
};

class SpellCheckingPage : public QWidget
{
    Q_OBJECT
public:
    SpellCheckingPage( QWidget * );
    void saveOptions();
private:
    Sonnet::ConfigWidget *m_confPage;
};


#endif
