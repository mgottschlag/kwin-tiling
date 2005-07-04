/*
 * main.cpp
 *
 * Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
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
 *  Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.
 */



#include "memory.h"

 
/* we have to include the info.cpp-file, to get the DEFINES about possible properties.
   example: we need the "define INFO_CPU_AVAILABLE" */
#include "info.cpp"


extern "C"
{

  KDE_EXPORT KCModule *create_cpu(QWidget *parent, const char * /*name*/)
  { 
#ifdef INFO_CPU_AVAILABLE
    return new KInfoListWidget(i18n("Processor(s)"), parent, "kcminfo", GetInfo_CPU);
#else
    return 0;
#endif
  }

  KDE_EXPORT KCModule *create_irq(QWidget *parent, const char * /*name*/)
  { 
#ifdef INFO_IRQ_AVAILABLE
    return new KInfoListWidget(i18n("Interrupt"), parent, "kcminfo", GetInfo_IRQ);
#else
    return 0;
#endif
  }

  KDE_EXPORT KCModule *create_pci(QWidget *parent, const char * /*name*/)
  { 
#ifdef INFO_PCI_AVAILABLE
    return new KInfoListWidget(i18n("PCI"), parent, "kcminfo", GetInfo_PCI);
#else
    return 0;
#endif
  }

  KDE_EXPORT KCModule *create_dma(QWidget *parent, const char * /*name*/)
  { 
#ifdef INFO_DMA_AVAILABLE
    return new KInfoListWidget(i18n("DMA-Channel"), parent, "kcminfo", GetInfo_DMA);
#else
    return 0;
#endif
  }

  KDE_EXPORT KCModule *create_ioports(QWidget *parent, const char * /*name*/)
  { 
#ifdef INFO_IOPORTS_AVAILABLE
    return new KInfoListWidget(i18n("I/O-Port"), parent, "kcminfo", GetInfo_IO_Ports);
#else
    return 0;
#endif
  }

  KDE_EXPORT KCModule *create_sound(QWidget *parent, const char * /*name*/)
  { 
#ifdef INFO_SOUND_AVAILABLE
    return new KInfoListWidget(i18n("Soundcard"), parent, "kcminfo", GetInfo_Sound);
#else
    return 0;
#endif
  }

  KDE_EXPORT KCModule *create_scsi(QWidget *parent, const char * /*name*/)
  { 
#ifdef INFO_SCSI_AVAILABLE
    return new KInfoListWidget(i18n("SCSI"), parent, "kcminfo", GetInfo_SCSI);
#else
    return 0;
#endif
  }

  KDE_EXPORT KCModule *create_devices(QWidget *parent, const char * /*name*/)
  { 
#ifdef INFO_DEVICES_AVAILABLE
    return new KInfoListWidget(i18n("Devices"), parent, "kcminfo", GetInfo_Devices);
#else
    return 0;
#endif
  }

  KDE_EXPORT KCModule *create_partitions(QWidget *parent, const char * /*name*/)
  { 
#ifdef INFO_PARTITIONS_AVAILABLE
    return new KInfoListWidget(i18n("Partitions"), parent, "kcminfo", GetInfo_Partitions);
#else
    return 0;
#endif
  }

  KDE_EXPORT KCModule *create_xserver(QWidget *parent, const char * /*name*/)
  { 
#ifdef INFO_XSERVER_AVAILABLE
    return new KInfoListWidget(i18n("X-Server"), parent, "kcminfo", GetInfo_XServer_and_Video);
#else
    return 0;
#endif
  }

  KDE_EXPORT KCModule *create_memory(QWidget *parent, const char * /*name*/)
  { 
    return new KMemoryWidget(parent, "kcminfo");
  }

  KDE_EXPORT KCModule *create_opengl(QWidget *parent, const char * )
  { 
#ifdef INFO_OPENGL_AVAILABLE
    return new KInfoListWidget(i18n("OpenGL"), parent, "kcminfo", GetInfo_OpenGL);
#else
    return 0;
#endif
  }
  

}
