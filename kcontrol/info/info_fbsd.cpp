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
//#define INFO_PCI_AVAILABLE
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
#include <machine/perfmon.h>

#include <fstab.h>
#include <stdio.h>
#include <stdlib.h>

#include <qtextstream.h>
#include <qfile.h>
#include <qfontmetrics.h>
#include <qstring.h>
#include <qtextstream.h>
#include <qlist.h>
#include <qdict.h>

#include <kdebug.h>

class Device {
public:
	Device (QString n=QString::null, QString d=QString::null) {name=n; description=d;}
	QString name ,description;
};

void ProcessChildren(QString name);
QString GetController(const QString &line);
Device *GetDevice(const QString &line);

bool GetInfo_CPU (QListView *lBox)
{
    // Modified 13 July 2000 for SMP by Brad Hughes - bhughes@trolltech.com

    int sc[2], ncpu, cpuspeed;
    size_t len;

    sc[0] = CTL_HW;
    sc[1] = HW_NCPU;
    len = sizeof(ncpu);
    sysctl(sc, 2, &ncpu, &len, NULL, 0);

    for (int i = ncpu; i > 0; i--) {
	/* Stuff for sysctl */
	char *buf, *mhz;
	QString cpustring;

	// get the processor model
	sc[0] = CTL_HW;
	sc[1] = HW_MODEL;
	sysctl(sc,2,NULL,&len,NULL,0);
	buf = new char[len];
	sysctl(sc,2,buf,&len,NULL,0);

	/*
	  Get the CPU speed, only on Genuine P5s
	  heh, heh, undocumented sysctls rule but I dunno
	  if this works on 2.2.x machines.
	*/
	sc[0] = CTL_MACHDEP;
	sc[1] = 0x6b;

	len = sizeof(cpuspeed);
	cpuspeed = 0;
	sysctl(sc, 2, &cpuspeed, &len, NULL, 0);

	/* Format the integer into correct xxx.xx MHz */
	mhz = new char[20];
	snprintf(mhz,20,"%d.%02d",
		 ((cpuspeed + 4999) / 1000000),
		 ((cpuspeed + 4999) / 10000) % 100);

	if (strcmp(mhz,"0.00") == 0)
	    /* We dunno how fast it's running */
	    cpustring = i18n("CPU %1: %2, unknown speed").arg(i).arg(buf);
	else
	    cpustring = i18n("CPU %1: %2 running at %3 MHz").arg(i).arg(buf).arg(mhz);

	/* Put everything in the listbox */
	new QListViewItem(lBox, cpustring);

	/* Clean up after ourselves, this time I mean it ;-) */

	delete mhz;
	delete buf;
    }

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
	qDebug("camcontrol doesn't exist");

	delete camcontrol;
	return false;
    }

    /* This prints out a list of all the scsi devies, perhaps eventually we could
       parse it as opposed to schlepping it into a listbox */
    if ((pipe = popen("/sbin/camcontrol devlist", "r")) == NULL) {
	qDebug("popen failed");

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
		kdError() << "Ahh couldn't open fstab!" << endl;
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

bool GetInfo_Devices (QListView *lbox)
{
	QFile *f = new QFile("/var/run/dmesg.boot");
	if (f->open(IO_ReadOnly)) {
		QTextStream qts(f);
		QDict<QListViewItem> lv_items;
		Device *dev;
		QString line, controller;
		lbox->setRootIsDecorated(true);
		lbox->addColumn("Device");
		lbox->addColumn("Description");
		while ( (line=qts.readLine()) != QString::null) {
			controller = GetController(line);
			if (controller == QString::null)
				continue;
			dev=GetDevice(line);
			if (!dev)
				continue;
			// Ewww assuing motherboard is the only toplevel controller is rather gross
			if (controller == "motherboard") {
				if (!lv_items[dev->name]) {
					lv_items.insert(dev->name, new QListViewItem(lbox, dev->name, dev->description) );
				}
			} else {
				QListViewItem *parent=lv_items[controller];
				if (parent && !lv_items[dev->name]) {
					lv_items.insert(dev->name, new QListViewItem(parent, dev->name, dev->description) );
				}
			}
		}
		return true;
	}
	return false;
}

QString GetController(const QString &line)
{
		if ( ( (line.startsWith("ad")) || (line.startsWith("afd")) || (line.startsWith("acd")) ) && (line.find(":") < 6) ) {
			QString controller = line;
			controller.remove(0, controller.find(" at ")+4);
			if (controller.find("-slave") != -1) {
				controller.remove(controller.find("-slave"), controller.length());
			} else if (controller.find("-master") != -1) {
				controller.remove(controller.find("-master"), controller.length());
			} else
				controller=QString::null;
			if (controller != QString::null)
				return controller;
		}
		if (line.find(" on ") != -1) {
			QString controller;
			controller = line;
			controller.remove(0, controller.find(" on ")+4);
			if (controller.find(" ") != -1)
				controller.remove(controller.find(" "), controller.length());
			return controller;
		}
			return QString::null;
}

Device *GetDevice(const QString &line)
{
	Device *dev = new Device;
	if (line.find(":") == -1)
		return 0;
	dev->name = line.mid(0, line.find(":"));
	dev->description = line.mid(line.find("<")+1, line.length());
	dev->description.remove(dev->description.find(">"), dev->description.length());
	return dev;
}
