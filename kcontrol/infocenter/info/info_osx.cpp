/*
 *  Copyright (c) 2003 Benjamin Reed <ranger@befunk.com>
 *
 *  info_osx.cpp is part of the KDE program kcminfo.  Copied wholesale
 *  from info_fbsd.cpp =)
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

#define INFO_CPU_AVAILABLE
//#define INFO_IRQ_AVAILABLE
//#define INFO_DMA_AVAILABLE
//#define INFO_PCI_AVAILABLE
//#define INFO_IOPORTS_AVAILABLE
#define INFO_SOUND_AVAILABLE
#define INFO_DEVICES_AVAILABLE
#define INFO_SCSI_AVAILABLE
#define INFO_PARTITIONS_AVAILABLE
#define INFO_XSERVER_AVAILABLE

/*
 * all following functions should return TRUE, when the Information
 * was filled into the lBox-Widget. Returning FALSE indicates that
 * information was not available.
 */

#include <sys/types.h>
#include <sys/sysctl.h>

#include <fstab.h>
#include <stdio.h>
#include <stdlib.h>

#include <iostream.h>

#include <qdict.h>
#include <qfile.h>
#include <qfontmetrics.h>
#include <qptrlist.h>
#include <qstring.h>
#include <qtextstream.h>

#include <kdebug.h>

#include <mach/mach.h>
#include <mach-o/arch.h>

#ifdef HAVE_COREAUDIO
#include <CoreAudio/CoreAudio.h>
#endif

#include <machine/limits.h>

bool GetInfo_CPU (QListView *lBox)
{

	QString cpustring;

        kern_return_t ret;
        struct host_basic_info basic_info;
        unsigned int count=HOST_BASIC_INFO_COUNT;

        ret=host_info(mach_host_self(), HOST_BASIC_INFO, 
                (host_info_t)&basic_info, &count);
        if (ret != KERN_SUCCESS) {
		kdDebug() << "unable to get host information from mach" << endl;
		return false;
        } else {
		kdDebug() << "got Host Info: (" << basic_info.avail_cpus << ") CPUs available" << endl;
        	const NXArchInfo *archinfo;
        	archinfo=NXGetArchInfoFromCpuType(basic_info.cpu_type, basic_info.cpu_subtype);
		new QListViewItem(lBox, i18n("Kernel is configured for %1 CPUs").arg(basic_info.max_cpus));
		for (int i = 1; i <= basic_info.avail_cpus; i++) {
			cpustring = i18n("CPU %1: %2").arg(i).arg(archinfo->description);
			new QListViewItem(lBox, cpustring);
		}
		return true;
	}
	return false;
}

bool GetInfo_IRQ (QListView *)
{
	return false;
}

bool GetInfo_DMA (QListView *)
{
	return false;
}

bool GetInfo_PCI (QListView *)
{
	return false;
}

bool GetInfo_IO_Ports (QListView *)
{
	return false;
}

bool GetInfo_Sound (QListView *lBox)
{
#ifdef HAVE_COREAUDIO
#define qMaxStringSize 1024
	OSStatus status;
	AudioDeviceID gOutputDeviceID;
	unsigned long propertySize;
	char deviceName[qMaxStringSize];
	char manufacturer[qMaxStringSize];
	propertySize = sizeof(gOutputDeviceID);
	status = AudioHardwareGetProperty(kAudioHardwarePropertyDefaultOutputDevice, &propertySize, &gOutputDeviceID);
	if (status) {
		kdDebug() << "get default output device failed, status = " << (int)status << endl;
		return false;
	}

	if (gOutputDeviceID != kAudioDeviceUnknown) {

		propertySize = qMaxStringSize;

		/* Device Name */
		status = AudioDeviceGetProperty(gOutputDeviceID, 1, 0, kAudioDevicePropertyDeviceName, &propertySize, deviceName);
		if (status) {
			kdDebug() << "get device name failed, status = " << (int)status << endl;
			return false;
		}
		new QListViewItem(lBox, i18n("Device Name: %1").arg(deviceName));

		/* Manufacturer */
		status = AudioDeviceGetProperty(gOutputDeviceID, 1, 0, kAudioDevicePropertyDeviceManufacturer, &propertySize, manufacturer);
		if (status) {
			kdDebug() << "get manufacturer failed, status = " << (int)status << endl;
			return false;
		}
		new QListViewItem(lBox, i18n("Manufacturer: %1").arg(manufacturer));
		return true;
	} else {
		return false;
	}
#else
	return false;
#endif
}

bool GetInfo_SCSI (QListView *lbox)
{
	return false;
}

bool GetInfo_Partitions (QListView *lbox)
{
	return false;
}

bool GetInfo_XServer_and_Video (QListView *lBox)
{
	return GetInfo_XServer_Generic( lBox );
}

bool GetInfo_Devices (QListView *lbox)
{
	return false;
}
