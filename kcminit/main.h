/*
  Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>

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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#ifndef KCMINIT_H
#define KCMINIT_H

#include <dcopobject.h>
#include <kservice.h>

class KCmdLineArgs;
class KLibLoader;

class KCMInit : public QObject, public DCOPObject
{
    Q_OBJECT
    K_DCOP
    k_dcop:
        void runPhase1();
        void runPhase2();
    public:
        KCMInit( KCmdLineArgs* args );
        virtual ~KCMInit();
    private:
        bool runModule(const QString &libName, KLibLoader *loader, KService::Ptr service);
        void runModules( int phase );
        KService::List list;
        QStringList alreadyInitialized;
};

#endif
