#include <iostream.h>
/*
 * $Id$
 *
 * info_fbsd.cpp is part of the KDE program kcminfo.  This displays
 * various information about the system (hopefully a FreeBSD system)
 * it's running on.
 */

#define INFO_CPU_AVAILABLE
#define INFO_IRQ_AVAILABLE
#define INFO_DMA_AVAILABLE
#define INFO_PCI_AVAILABLE
#define INFO_IOPORTS_AVAILABLE
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
#include <stdlib.h>

#include <qtextstream.h>
#include <qfile.h>
#include <qfontmetrics.h>

#include <kdebug.h>

bool GetInfo_CPU (QListView *lBox)
{ 
  QString str;

  /* Stuff for sysctl */
  char *buf, *mhz;
  QString cpustring;
  int mib[2], machspeed;
  size_t len;

  mib[0] = CTL_HW;
  mib[1] = HW_MODEL;
  sysctl(mib,2,NULL,&len,NULL,0);
  buf=(char*)malloc(len);
  sysctl(mib,2,buf,&len,NULL,0);
  /*	Get the CPU speed, only on Genuine P5s
		heh, heh, undocumented sysctls rule but I dunno
		if this works on 2.2.x machines. */
  mib[0] = CTL_MACHDEP; mib[1] = 107;
  len=sizeof(machspeed);
  sysctl(mib,2,&machspeed,&len,NULL,0);
  /* Format the integer into correct xxx.xx MHz */
  mhz=(char *)malloc(20);
  snprintf(mhz,20,"%d.%02d",(machspeed+4999)/1000000,
		   ((machspeed+4999)/10000)%100);
  if (strcmp(mhz,"0.00")==0)
 	/* We dunno how fast it's running */
	cpustring = i18n("%1, unknown speed").arg(buf);
  else
	cpustring = i18n("%1 running at %2 MHz").arg(buf).arg(mhz);

  /* Put everything in the listbox */
  new QListViewItem(lBox, cpustring);
  /* Clean up after ourselves, this time I mean it ;-) */
  free(mhz);
  free(buf);

  return true;
}

bool GetInfo_IRQ (QListView *)
{
  /* systat lists the interrupts assigned to devices as well as how many were
     generated.  Parsing its output however is about as fun as a sandpaper
     enema.  The best idea would probably be to rip out the guts of systat.
     Too bad it's not very well commented */
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

bool GetInfo_Sound (QListView *lbox)
{
  QFile *sndstat = new QFile("/dev/sndstat");
  QTextStream *t; QString s;
 
  if (!sndstat->exists()) {
    delete sndstat;
    return false;
  }
  
  if (!sndstat->open(IO_ReadOnly)) {
    delete sndstat;
    return false;
  }
  
  t = new QTextStream(sndstat);

  QListViewItem* olditem = 0;
  while ((s=t->readLine()) != QString::null)
      olditem = new QListViewItem(lbox, olditem, s);

  delete t;
  sndstat->close();
  delete sndstat;
  return true;
}

bool GetInfo_Devices (QListView *lbox)
{
	QFile *dmesg = new QFile("/var/run/dmesg.boot");

	if (!dmesg->exists()) {
		delete dmesg;
		return false;
	}

	if (!dmesg->open(IO_ReadOnly)) {
		delete dmesg;
		return false;
	}

	lbox->clear();

	QTextStream *t = new QTextStream(dmesg);
	QString s;

    QListViewItem* olditem;
    
	while ((s=t->readLine()) != QString::null)
        olditem = new QListViewItem(lbox, olditem, s);

	delete t;
	dmesg->close();
	delete dmesg;
	return true;
}

bool GetInfo_SCSI (QListView *lbox)
{
  /*
   * This code relies on the system at large having "the" CAM (see the FreeBSD
   * 3.0 Release notes for more info) SCSI layer, and not the older one.
   * If someone who has a system with the older SCSI layer and would like to
   * tell me (jazepeda@pacbell.net) how to extract that info, I'd be much
   * obliged.
   */
  FILE *pipe;
  QFile *camcontrol = new QFile("/sbin/camcontrol");
  QTextStream *t;
  QString s;

  if (!camcontrol->exists()) {
    delete camcontrol;
    return false;
  }
  
  /* This prints out a list of all the scsi devies, perhaps eventually we could
     parse it as opposed to schlepping it into a listbox */
  if ((pipe = popen("/sbin/camcontrol devlist", "r")) == NULL) {
    delete camcontrol;
    return false;
  }

  t = new QTextStream(pipe, IO_ReadOnly);
  
  QListViewItem* olditem = 0;

  while (true) {
    s = t->readLine();
    if ( (s == "") || (s == QString::null) )
      break;
    olditem = new QListViewItem(lbox, olditem, s);
  }

  delete t; delete camcontrol; pclose(pipe);
  
  if (!lbox->childCount())
    return false;
  
  return true;
}

bool GetInfo_Partitions (QListView *lbox)
{
	struct fstab *fstab_ent;

	if (setfsent() != 1) /* Try to open fstab */ {
		kdebug(KDEBUG_ERROR, 0, "Ahh couldn't open fstab!");
		return false;
	}

	lbox->addColumn(i18n("Device"));
	lbox->addColumn(i18n("Mount Point"));
	lbox->addColumn(i18n("FS Type"));
	lbox->addColumn(i18n("Mount Options"));


	while ((fstab_ent=getfsent())!=NULL) {
        new QListViewItem(lbox, fstab_ent->fs_spec,
                          fstab_ent->fs_file, fstab_ent->fs_vfstype,
                          fstab_ent->fs_mntops);
        
	}

    lbox->setSorting(0);
    lbox->header()->setClickEnabled(true);

	endfsent(); /* Close fstab */
	return true;
}

bool GetInfo_XServer_and_Video (QListView *lBox)
{
	return GetInfo_XServer_Generic( lBox );
}
