/*
 *   Copyright 2008 Ryan P. Bitanga <ryan.bitanga@gmail.com>
 *   Copyright 2009 Marco Martin <notmart@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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

#ifndef KRUNNERCONFIG_H
#define KRUNNERCONFIG_H

#include <KPluginSelector>



class KPluginSelector;

namespace Plasma {
    class RunnerManager;
}

class RunnersConfig : public KPluginSelector
{
Q_OBJECT
    public:
        RunnersConfig(Plasma::RunnerManager *manager, QWidget *parent = 0);
        ~RunnersConfig();

    public slots:
        void accept();

    private slots:
        void updateRunner(const QByteArray& runnerName);

    private:
        void init();

        KPluginSelector *m_sel;
        Plasma::RunnerManager *m_manager;
};

#endif

