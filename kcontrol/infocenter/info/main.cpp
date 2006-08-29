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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */



#include "memory.h"
#include <kinstance.h>
 
/* we have to include the info.cpp-file, to get the DEFINES about possible properties.
   example: we need the "define INFO_CPU_AVAILABLE" */
#include "info.cpp"
#include <kgenericfactory.h>
class QStringList;

#define CREATE_FACTORY(type, name, symbol) \
class K##type##InfoWidget; \
typedef KGenericFactory<K##type##InfoWidget> K##type##InfoWidgetFactory; \
class K##type##InfoWidget : public KInfoListWidget \
{ \
    public: \
        K##type##InfoWidget(QWidget *parent, const QStringList &) \
            : KInfoListWidget(K##type##InfoWidgetFactory::instance(), \
                    name, parent, GetInfo_##type) \
        { \
        } \
}; \
K_EXPORT_COMPONENT_FACTORY(symbol, K##type##InfoWidgetFactory("kcminfo"))

#ifdef INFO_CPU_AVAILABLE
CREATE_FACTORY(CPU, i18n("Processor(s)"), cpu)
#endif
#ifdef INFO_IRQ_AVAILABLE
CREATE_FACTORY(IRQ, i18n("Interrupt"), irq)
#endif
#ifdef INFO_PCI_AVAILABLE
CREATE_FACTORY(PCI, i18n("PCI"), pci)
#endif
#ifdef INFO_DMA_AVAILABLE
CREATE_FACTORY(DMA, i18n("DMA-Channel"), dma)
#endif
#ifdef INFO_IOPORTS_AVAILABLE
CREATE_FACTORY(IO_Ports, i18n("I/O-Port"), ioports)
#endif
#ifdef INFO_SOUND_AVAILABLE
CREATE_FACTORY(Sound, i18n("Soundcard"), sound)
#endif
#ifdef INFO_SCSI_AVAILABLE
CREATE_FACTORY(SCSI, i18n("SCSI"), scsi)
#endif
#ifdef INFO_DEVICES_AVAILABLE
CREATE_FACTORY(Devices, i18n("Devices"), devices)
#endif
#ifdef INFO_PARTITIONS_AVAILABLE
CREATE_FACTORY(Partitions, i18n("Partitions"), partitions)
#endif
#ifdef INFO_XSERVER_AVAILABLE
CREATE_FACTORY(XServer_and_Video, i18n("X-Server"), xserver)
#endif
#ifdef INFO_OPENGL_AVAILABLE
CREATE_FACTORY(OpenGL, i18n("OpenGL"), opengl)
#endif
