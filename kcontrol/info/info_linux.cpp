/*  $Id$

    Linux-specific Information about the Hardware.

    written 1998 by Helge Deller (helge.deller@ruhr-uni-bochum.de)

    To do (maybe?):
    - include Information about XFree86 and/or Accelerated X 
	(needs to change configure-skript, to see, if Header-files are available !)
    - maybe also include information about the video-framebuffer devices (newer Kernels >= 2.1.100)
    - rewrite detection-routines (maybe not to use the /proc-fs)
    - more & better sound-information
 
    /dev/sndstat support added: 1998-12-08 Duncan Haldane (f.d.m.haldane@cwix.com)
*/


#include <unistd.h>
#include <syscall.h>
#include <stdio.h>
#include <fstab.h>
#include <sys/statfs.h>
#include <sys/stat.h>
#include <linux/kernel.h>

#include <kapp.h>
#include <ktablistbox.h>

#define INFO_CPU_AVAILABLE
#define INFO_CPU "/proc/cpuinfo"

#define INFO_IRQ_AVAILABLE
#define INFO_IRQ "/proc/interrupts"

#define INFO_DMA_AVAILABLE
#define INFO_DMA "/proc/dma"

#define INFO_PCI_AVAILABLE
#define INFO_PCI "/proc/pci"

#define INFO_IOPORTS_AVAILABLE
#define INFO_IOPORTS "/proc/ioports"

#define INFO_SOUND_AVAILABLE
#define INFO_DEV_SNDSTAT "/dev/sndstat"
#define INFO_SOUND "/proc/sound" 

#define INFO_DEVICES_AVAILABLE
#define INFO_DEVICES "/proc/devices"
#define INFO_MISC "/proc/misc"

#define INFO_SCSI_AVAILABLE
#define INFO_SCSI "/proc/scsi/scsi"

#define INFO_PARTITIONS_AVAILABLE
#define INFO_PARTITIONS "/proc/partitions"
#define INFO_PARTITIONS_FULL_INFO	// define, to show complete info !

#define INFO_XSERVER_AVAILABLE


#define MAXCOLUMNWIDTH 600

bool GetInfo_ReadfromFile( KTabListBox *lBox, const char *Name, char splitchar  )
{
  QString str;
  char buf[512];

  QFile *file = new QFile(Name);

  if(!file->open(IO_ReadOnly)) {
    delete file; 
    return FALSE;
  }
  
  while (file->readLine(buf,sizeof(buf)-1) > 0) {
      if (strlen(buf))
      {  char *p=buf;
         if (splitchar!=0)    /* remove leading spaces between ':' and the following text */
	     while (*p)
	     {	if (*p==splitchar)
	        { *p++ = ' ';
		  while (*p==' ') ++p;
		  *(--p) = splitchar;
		  ++p;
		}
		else ++p;
	     }
         lBox->setSeparator(splitchar);
         lBox->insertItem(buf);
      }
  }
  file->close();
  delete file;
  return TRUE;
}




bool GetInfo_CPU( KTabListBox *lBox )
{
  lBox->setNumCols(2);
  lBox->setColumn(0,i18n("Information"),150 );
  lBox->setColumn(1,i18n("Value") );
  return GetInfo_ReadfromFile( lBox, INFO_CPU, ':' );
}


bool GetInfo_IRQ( KTabListBox *lBox )
{
  return GetInfo_ReadfromFile( lBox, INFO_IRQ, 0 );
}

bool GetInfo_DMA( KTabListBox *lBox )
{
  lBox->setNumCols(2);
  lBox->setColumn(0,i18n("DMA-Channel"),100 );
  lBox->setColumn(1,i18n("used by") );
  return GetInfo_ReadfromFile( lBox, INFO_DMA, ':' );
}

bool GetInfo_PCI( KTabListBox *lBox )
{
  return GetInfo_ReadfromFile( lBox, INFO_PCI, 0 );
}

bool GetInfo_IO_Ports( KTabListBox *lBox )
{
  lBox->setNumCols(2);
  lBox->setColumn(0,i18n("I/O-Range"),100 );
  lBox->setColumn(1,i18n("used by") );
  return GetInfo_ReadfromFile( lBox, INFO_IOPORTS, ':' );
}

bool GetInfo_Sound( KTabListBox *lBox )
{
  if ( GetInfo_ReadfromFile( lBox, INFO_DEV_SNDSTAT, 0 )) 
    return TRUE;
  else 
    return GetInfo_ReadfromFile( lBox, INFO_SOUND, 0 );
}

bool GetInfo_Devices( KTabListBox *lBox )
{  
  GetInfo_ReadfromFile( lBox, INFO_DEVICES, 0 );
  lBox->insertItem(QString(""));
  // don't use i18n() for "Misc devices", because all other info is english too!
  lBox->insertItem(QString("Misc devices:")); 
  GetInfo_ReadfromFile( lBox, INFO_MISC, 0 );
  return TRUE;
}

bool GetInfo_SCSI( KTabListBox *lBox )
{
  return GetInfo_ReadfromFile( lBox, INFO_SCSI, 0 );
}



#ifndef INFO_PARTITIONS_FULL_INFO

bool GetInfo_Partitions( KTabListBox *lBox )
{
  return GetInfo_ReadfromFile( lBox, INFO_PARTITIONS, 0 );
}

#else // INFO_PARTITIONS_FULL_INFO

// Some Ideas taken from garbazo from his source in info_fbsd.cpp

bool GetInfo_Partitions (KTabListBox *lbox)
{
	#define NUMCOLS 6
	int maxwidth[NUMCOLS]={0,0,0,0,0,0};
	QString Title[NUMCOLS];
	int n;
	
	struct fstab *fstab_ent;
 	struct statfs sfs;
	/* struct stat st; */
	unsigned long total,avail;
	QFontMetrics fm(lbox->tableFont());
	QString str;
	QString MB(i18n("MB"));	// "MB" = "Mega-Byte"
	QString TAB(SEPERATOR);

	if (setfsent() != 0) // Try to open fstab
	    return FALSE;

	MB = QString(" ") + MB;
	Title[0] = i18n("Device");
	Title[1] = i18n("Mount Point");
	Title[2] = i18n("FS Type");
	Title[3] = i18n("Total Size");
	Title[4] = i18n("Free Size");
	Title[5] = i18n("Mount Options");

	lbox->setNumCols(NUMCOLS);
	lbox->setSeparator(SEPERATOR_CHAR);
	for (n=0; n<NUMCOLS; ++n)
        {	maxwidth[n]=fm.width(Title[n]);
		lbox->setColumn(n,Title[n],maxwidth[n] );
	}

	while ((fstab_ent=getfsent())!=NULL) {
		if ( (n=fm.width(fstab_ent->fs_spec)) > maxwidth[0])
			maxwidth[0]=n;

		if ( (n=fm.width(fstab_ent->fs_file)) > maxwidth[1])
			maxwidth[1]=n;
		
		if ( (n=fm.width(fstab_ent->fs_vfstype)) > maxwidth[2])
			maxwidth[2]=n;

		if (!maxwidth[3])
		    maxwidth[3] = maxwidth[4] = fm.width("999999 MB");

		if ( (n=fm.width(fstab_ent->fs_mntops)) > maxwidth[5])
			maxwidth[5]=n;

		str =  QString(fstab_ent->fs_spec) + TAB
		    +  QString(fstab_ent->fs_file) + TAB 
		    +  QString(fstab_ent->fs_vfstype) + TAB;

		total = avail = 0;
		if (statfs(fstab_ent->fs_file,&sfs)==0) {
    		    total = sfs.f_blocks * sfs.f_bsize;
		    avail = (getuid() ? sfs.f_bavail : sfs.f_bfree) * sfs.f_bsize;
		};
		/*
		if (stat(fstab_ent->fs_file,&st)!=0)
			total = 0;
		if (!S_ISDIR(st.st_mode))
			total = 0;
		*/
		if (total)
		    str += TAB
			+  Value((total/1024+512)/1024,6) + MB 
			+  TAB
			+  Value((avail/1024+512)/1024,6) + MB;
		else
		    str += " " + TAB + " " + TAB;

		str +=  TAB + QString(fstab_ent->fs_mntops);

		lbox->insertItem(str);
	}
	endfsent();  // close fstab..

	for (n=0; n<NUMCOLS; ++n)
	    lbox->setColumnWidth(n, maxwidth[n] + PIXEL_ADD);
	    
	return TRUE;
}
#endif	 // INFO_PARTITIONS_FULL_INFO




bool GetInfo_XServer_and_Video( KTabListBox *lBox )
{
  return GetInfo_XServer_Generic( lBox );
}
