/*
 * view1394.h
 *
 *  Copyright (C) 2003 Alexander Neundorf <neundorf@kde.org>
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef VIEW1394_H_
#define VIEW1394_H_

#include <kcmodule.h>
#include <kaboutdata.h>

#include <qvaluelist.h>
#include <qsocketnotifier.h>
#include <qtimer.h>
#include <qmap.h>
#include <qstring.h>

#include "view1394widget.h"

#include <libraw1394/raw1394.h>

class OuiDb
{
   public:
      OuiDb();
      QString vendor(octlet_t guid);
   private:
      QMap<QString, QString> m_vendorIds;
};

class View1394: public KCModule
{
   Q_OBJECT
   public:
      View1394(QWidget *parent = 0L, const char *name = 0L);
      virtual ~View1394();

      int buttons();
      QString quickHelp() const;
      const KAboutData* aboutData() const {return myAboutData; };
   public slots: // Public slots
      void rescanBus();
      void generateBusReset();

   private:
      KAboutData *myAboutData;
      View1394Widget *m_view;
      QValueList<raw1394handle_t> m_handles;
      QPtrList<QSocketNotifier> m_notifiers;
      bool readConfigRom(raw1394handle_t handle, nodeid_t nodeid, quadlet_t& firstQuad, quadlet_t& cap, octlet_t& guid);
      bool m_insideRescanBus;
      QTimer m_rescanTimer;
      OuiDb *m_ouiDb;
   private slots:
      void callRaw1394EventLoop(int fd);
};
#endif
