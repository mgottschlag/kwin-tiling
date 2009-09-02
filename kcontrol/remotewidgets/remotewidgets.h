/*
 *  Copyright (C) 2009 Dario Freddi <drf@kde.org>
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
 *
 */
#ifndef REMOTEWIDGETS_H
#define REMOTEWIDGETS_H

#include <kcmodule.h>

namespace Ui {
  class MainPage;
}

class RemoteWidgetsModule : public KCModule
{
    Q_OBJECT

public:
    RemoteWidgetsModule(QWidget *parent, const QVariantList &);
  
    void save();
    void load();
    void defaults();
    
private:
    Ui::MainPage *m_ui;

};

#endif // REMOTEWIDGETS_H
