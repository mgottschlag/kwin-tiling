/***************************************************************************
 *   Copyright 2008 by Montel Laurent <montel@kde.org>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef SHELLCONFIG_H
#define SHELLCONFIG_H

#include <KCModule>
#include "ui_shellOptions.h"


class ShellConfigForm : public QWidget, public Ui::shellOptions
{
    Q_OBJECT
    public:
        explicit ShellConfigForm(QWidget* parent);
};

class ShellConfig : public KCModule
{
    Q_OBJECT
    public:
        explicit ShellConfig(QWidget* parent = 0, const QVariantList& args = QVariantList());
        ~ShellConfig();

    public slots:
        void save();
        void load();
        void defaults();
        void setRunInTerminal(bool);

    private:
        ShellConfigForm* m_ui;
        bool m_inTerminal;
};

#endif
