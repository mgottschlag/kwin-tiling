/* 	$Id$
	
	!!!!! this file will be included by info.cpp !!!!!
	
	Mostly written by Helge Deller (deller@gmx.de),
	with some pieces of code from Aubert Pierre.

	Last modified:	done:
	1999-06-18	added support for 64-Bit HP-UX in CPU-detection (deller)
	1999-05-04	added audio(alib)-support (deller)
	1999-04-27	[tested with HP-UX 10.20 (HP9000/715/64-EISA)]
			added support for nearly all categories 
			    (means: not finished!)
	1998-11-01	first, nearly empty version (deller)
			with a little source for CPU from Aubert Pierre
*/

#include <unistd.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/pstat.h>
#include <sys/utsname.h>
#include <sys/statvfs.h>
#include <sys/vfs.h>
#include <fstab.h>
#include <stdlib.h>
#include <qfile.h>
#include <qfontmetrics.h>
#include <qtextstream.h>


#define INFO_CPU_AVAILABLE
#	define INFO_CPU_MODEL		"/bin/model" 	// as pipe !!

#define INFO_IRQ_AVAILABLE

#define INFO_DMA_AVAILABLE

#define INFO_PCI_AVAILABLE
#	define INFO_PCI			""	// Please, who know it ????
#	define INFO_PCI_EISA		"/etc/eisa/system.sci" // File !

#define INFO_IOPORTS_AVAILABLE
#	define INFO_IOPORTS_1		"/etc/dmesg"		// as pipe !
#	define INFO_IOPORTS_2		"/usr/sbin/dmesg"	// as pipe !

#define INFO_SOUND_AVAILABLE

#define INFO_DEVICES_AVAILABLE
#	define INFO_DEVICES		"/etc/ioscan" 	// as pipe !!

#define INFO_SCSI_AVAILABLE

#define INFO_PARTITIONS_AVAILABLE
#	define INFO_PARTITIONS_1 	FSTAB	// = "/etc/fstab" (defined in fstab.h)
#	define INFO_PARTITIONS_2 	"/etc/checklist"

#define INFO_XSERVER_AVAILABLE



/*	The following table is from an HP-UX 10.20 System
	build out of the file "/usr/lib/sched.models"....
	
	If you have more entries, then please add them !!
	(Helge Deller,  deller@gmx.de)
*/

// entries for PA_REVISION[]
enum V_ENTRIES
    {	V_1x0,  
	V_1x1, V_1x1a, V_1x1b, V_1x1c, V_1x1d, V_1x1e, 
	V_2x0, 
	V_LAST };

static char PA_REVISION[V_LAST][8]
    = { "1.0", 
        "1.1", "1.1a", "1.1b", "1.1c", "1.1d", "1.1e",
	"2.0" };

// entries for PA_NAME[]
enum PA_ENTRIES 
    {	PA7000,
	PA7100, PA7100LC, PA7200, PA7300,
	PA8000,
	PARISC_PA_LAST };
			
static char PA_NAME[PARISC_PA_LAST][12]
    = { "PA7000",
	"PA7100", "PA7100LC", "PA7200", "PA7300",
	"PA8000" };

struct _type_LOOKUPTABLE {
	 char 			Name[8]; 
	 enum V_ENTRIES 	parisc_rev; 
	 enum PA_ENTRIES 	parisc_name; 
};
	 
static struct _type_LOOKUPTABLE PA_LOOKUPTABLE[] = {	
/* VERSION A.00.07    (taken from file: /usr/lib/sched.models) */
{ "600"		,V_1x0	,PA7000		},
{ "635"		,V_1x0	,PA7000		},
{ "645"		,V_1x0	,PA7000		},
{ "700"		,V_1x1	,PA7000		},
{ "705"		,V_1x1a	,PA7000		},
{ "715"		,V_1x1c	,PA7100LC	},
{ "710"		,V_1x1a	,PA7000		},
{ "712"		,V_1x1c	,PA7100LC	},
{ "720"		,V_1x1a	,PA7000		},
{ "722"		,V_1x1c	,PA7100LC	},
{ "725"		,V_1x1c	,PA7100LC	},
{ "728"		,V_1x1d	,PA7200		},
{ "730"		,V_1x1a	,PA7000		},
{ "735"		,V_1x1b	,PA7100		},
{ "743"		,V_1x1b	,PA7100		},
{ "745"		,V_1x1b	,PA7100		},
{ "747"		,V_1x1b	,PA7100		},
{ "750"		,V_1x1a	,PA7000		},
{ "755"		,V_1x1b	,PA7100		},
{ "770"		,V_1x1d	,PA7200		},
{ "800"		,V_1x0	,PA7000		},
{ "801"		,V_1x1c	,PA7100LC	},
{ "806"		,V_1x1c	,PA7100LC	},
{ "807"		,V_1x1a	,PA7000		},
{ "808"		,V_1x0	,PA7000		},
{ "809"		,V_1x1d	,PA7200		},
{ "811"		,V_1x1c	,PA7100LC	},
{ "815"		,V_1x0	,PA7000		},
{ "816"		,V_1x1c	,PA7100LC	},
{ "817"		,V_1x1a	,PA7000		},
{ "819"		,V_1x1d	,PA7200		},
{ "821"		,V_1x1d	,PA7200		},
{ "822"		,V_1x0	,PA7000		},
{ "825"		,V_1x0	,PA7000		},
{ "826"		,V_1x1c	,PA7100LC	},
{ "827"		,V_1x1a	,PA7000		},
{ "829"		,V_1x1d	,PA7200		},
{ "831"		,V_1x1d	,PA7200		},
{ "832"		,V_1x0	,PA7000		},
{ "834"		,V_1x0	,PA7000		},
{ "835"		,V_1x0	,PA7000		},
{ "837"		,V_1x1a	,PA7000		},
{ "839"		,V_1x1d	,PA7200		},
{ "840"		,V_1x0	,PA7000		},
{ "841"		,V_1x1d	,PA7200		},
{ "842"		,V_1x0	,PA7000		},
{ "845"		,V_1x0	,PA7000		},
{ "845"		,V_1x0	,PA7000		},
{ "847"		,V_1x1a	,PA7000		},
{ "849"		,V_1x1d	,PA7200		},
{ "850"		,V_1x0	,PA7000		},
{ "851"		,V_1x1d	,PA7200		},
{ "852"		,V_1x0	,PA7000		},
{ "855"		,V_1x0	,PA7000		},
{ "856"		,V_1x1c	,PA7100LC	},
{ "857"		,V_1x1a	,PA7000		},
{ "859"		,V_1x1d	,PA7200		},
{ "860"		,V_1x0	,PA7000		},
{ "861"		,V_2x0	,PA8000		},
{ "865"		,V_1x0	,PA7000		},
{ "867"		,V_1x1a	,PA7000		},
{ "869"		,V_1x1d	,PA7200		},
{ "870"		,V_1x0	,PA7000		},
{ "871"		,V_2x0	,PA8000		},
{ "877"		,V_1x1a	,PA7000		},
{ "879"		,V_2x0	,PA8000		},
{ "887"		,V_1x1b	,PA7100		},
{ "889"		,V_2x0	,PA8000		},
{ "890"		,V_1x0	,PA7000		},
{ "897"		,V_1x1b	,PA7100		},
{ "900"		,V_1x0	,PA7000		},
{ "F10"		,V_1x1a	,PA7000		},
{ "F20"		,V_1x1a	,PA7000		},
{ "H20"		,V_1x1a	,PA7000		},
{ "F30"		,V_1x1a	,PA7000		},
{ "G30"		,V_1x1a	,PA7000		},
{ "H30"		,V_1x1a	,PA7000		},
{ "I30"		,V_1x1a	,PA7000		},
{ "G40"		,V_1x1a	,PA7000		},
{ "H40"		,V_1x1a	,PA7000		},
{ "I40"		,V_1x1a	,PA7000		},
{ "G50"		,V_1x1b	,PA7100		},
{ "H50"		,V_1x1b	,PA7100		},
{ "I50"		,V_1x1b	,PA7100		},
{ "G60"		,V_1x1b	,PA7100		},
{ "H60"		,V_1x1b	,PA7100		},
{ "I60"		,V_1x1b	,PA7100		},
{ "G70"		,V_1x1b	,PA7100		},
{ "H70"		,V_1x1b	,PA7100		},
{ "I70"		,V_1x1b	,PA7100		},
{ "E25"		,V_1x1c	,PA7100LC	},
{ "E35"		,V_1x1c	,PA7100LC	},
{ "E45"		,V_1x1c	,PA7100LC	},
{ "E55"		,V_1x1c	,PA7100LC	},
{ "T500"	,V_1x1c	,PA7100LC	},
{ "K100"	,V_1x1d	,PA7200		},
{ "K200"	,V_1x1d	,PA7200		},
{ "K210"	,V_1x1d	,PA7200		},
{ "K230"	,V_1x1d	,PA7200		},
{ "K400"	,V_1x1d	,PA7200		},
{ "K410"	,V_1x1d	,PA7200		},
{ "K430"	,V_1x1d	,PA7200		},
{ "DXO"		,V_1x1c	,PA7100LC	},
{ "DX5"		,V_1x1c	,PA7100LC	},
{ "D200"	,V_1x1d	,PA7200		},
{ "D400"	,V_1x1d	,PA7200		},
{ "D210"	,V_1x1d	,PA7200		},
{ "D410"	,V_1x1d	,PA7200		},
{ "J200"	,V_1x1d	,PA7200		},
{ "J210"	,V_1x1d	,PA7200		},
{ "C100"	,V_1x1d	,PA7200		},
{ "J220"	,V_2x0	,PA8000		},
{ "S715"	,V_1x1e	,PA7300		},
{ "S760"	,V_1x1e	,PA7300		},
{ "D650"	,V_2x0	,PA8000		},
{ "J410"	,V_2x0	,PA8000		},
{ "J400"	,V_2x0	,PA8000		},
{ "J210XC"	,V_1x1d	,PA7200		},
{ "C140"	,V_2x0	,PA8000		},
{ "C130"	,V_2x0	,PA8000		},
{ "C120"	,V_1x1e	,PA7300		},
{ "C115"	,V_1x1e	,PA7300		},
{ "B120"	,V_1x1e	,PA7300		},
{ "B115"	,V_1x1e	,PA7300		},
{ "S700i"	,V_1x1e	,PA7300		},
{ "S744"	,V_1x1e	,PA7300		},
{ "D330"	,V_1x1e	,PA7300		},
{ "D230"	,V_1x1e	,PA7300		},
{ "D320"	,V_1x1e	,PA7300		},
{ "D220"	,V_1x1e	,PA7300		},
{ "D360"	,V_1x1d	,PA7200		},
{ "K460"	,V_2x0	,PA8000		},
{ "K260"	,V_2x0	,PA8000		},
{ "D260"	,V_1x1d	,PA7200		},
{ "D270"	,V_2x0	,PA8000		},
{ "D370"	,V_2x0	,PA8000		},
{ ""		,V_LAST	,PARISC_PA_LAST	}  // Last Entry has to be empty !!!!
};




/* Helper-Functions */

/* easier to read with such a define ! */
#define I18N_MAX(txt,in,fm,maxw) \
    { int n = fm.width(txt=in); if (n>maxw) maxw=n; }

// Value() is defined in info.cpp !!!

static bool Find_in_LOOKUPTABLE( KTabListBox *lBox, char *machine )
{	char *Machine;
	int  len;
	struct _type_LOOKUPTABLE *Entry = PA_LOOKUPTABLE;
	QString str;
        QString TAB(SEPERATOR);
	
	Machine = machine;
	while ((*Machine) && (*Machine!='/')) ++Machine; 
	if (*Machine) ++Machine; else Machine=machine;
	len = strlen(Machine);
	
	while (strlen(Entry->Name))
	{	if (strncmp(Entry->Name,Machine,len)==0)
		{
		    str = i18n("PA-RISC Processor") + TAB 
			+ QString(PA_NAME[Entry->parisc_name]);
		    lBox->insertItem( str );
		    str = i18n("PA-RISC Revision") + TAB 
			+ QString("PA-RISC ") 
			+ QString(PA_REVISION[Entry->parisc_rev]);
		    lBox->insertItem( str );
		    return TRUE;
		}
		else
		    ++Entry;	// next Entry !
	}
	
	return FALSE;
}


/*  all following functions should return TRUE, when the Information 
    was filled into the lBox-Widget.
    returning FALSE indicates, that information was not available.
*/


bool GetInfo_CPU( KTabListBox *lBox )
{
  FILE *pipe;
  QFile *model;
     
  struct pst_dynamic	psd;
  struct pst_static	pst;
  struct pst_processor	pro;
  struct utsname	info;
  QString TAB(SEPERATOR);
  QString str,str2;
  QFontMetrics fm(lBox->tableFont());
  int	maxwidth,m;
			
  lBox->setNumCols(2);		// Table-Headers....
  maxwidth = 0;
  I18N_MAX(str,i18n("Information"),fm,maxwidth);
  lBox->setColumn(0,str,maxwidth );
  lBox->setColumn(1,i18n("Value") );
  lBox->setSeparator(SEPERATOR_CHAR);
  
  uname(&info);
  I18N_MAX(str,i18n("Machine"),fm,maxwidth);
  str += TAB + QString(info.machine);
  lBox->insertItem( str );
  if (psd.psd_proc_cnt<=0) psd.psd_proc_cnt = 1;
  
  model = new QFile(INFO_CPU_MODEL);  
  if (model->exists()) 
  {	if ((pipe = popen(INFO_CPU_MODEL, "r"))) 
	{	QTextStream *t = new QTextStream(pipe, IO_ReadOnly);
		str = t->readLine();
                m = fm.width(str); if (m>maxwidth) maxwidth=m;
		str = i18n("Model") + TAB + str;	// Machine Model...
		lBox->insertItem(str);
		delete t;
	}
	delete pipe;
  }
  delete model;
  
  I18N_MAX(str,i18n("Machine Identification Number"),fm,maxwidth);
  str += TAB + (strlen(info.__idnumber) ? QString(info.__idnumber) : i18n("(none)") );
  lBox->insertItem( str );

  lBox->insertItem( "" );
  I18N_MAX(str,i18n("Number of active Processors"),fm,maxwidth);
  if (psd.psd_proc_cnt<=0) psd.psd_proc_cnt=1; // Minimum one CPU !
  str += TAB + Value(psd.psd_proc_cnt);
  lBox->insertItem( str );

  pstat_getprocessor( &pro, sizeof(pro), 1, 0 );
  I18N_MAX(str,i18n("CPU Clock"),fm,maxwidth);
  str += TAB + Value(pro.psp_iticksperclktick/10000) 
             + QString(" ") + i18n("MHz");
  lBox->insertItem(str);

  switch(sysconf(_SC_CPU_VERSION))
  {	case CPU_HP_MC68020:	str2 = "Motorola 68020";	break;
	case CPU_HP_MC68030:	str2 = "Motorola 68030";	break;
	case CPU_HP_MC68040:	str2 = "Motorola 68040";	break;
	case CPU_PA_RISC1_0:	str2 = "PA-RISC 1.0";		break;
	case CPU_PA_RISC1_1:	str2 = "PA-RISC 1.1";		break;
	case CPU_PA_RISC1_2:	str2 = "PA-RISC 1.2";		break;
	case CPU_PA_RISC2_0:	
#if defined(_SC_KERNEL_BITS)
              			switch (sysconf(_SC_KERNEL_BITS)) {
              			    case 64: str2 = "PA-RISC 2.0w (64 bit)";	break;
              			    case 32: str2 = "PA-RISC 2.0n (32 bit)";	break;
              			    default: str2 = "PA-RISC 2.0"; 		break;
              			};				break;
#else  /* !defined(_SC_KERNEL_BITS) */
				str2 = "PA-RISC 2.0";		break;
#endif
	default:		str2 = i18n("(unknown)"); 	break;
  }
  I18N_MAX(str,i18n("CPU Architecture"),fm,maxwidth);
  str += TAB + str2;
  lBox->insertItem(str);
  
  Find_in_LOOKUPTABLE( lBox, info.machine );	// Get extended Information !

  for (i=PS_PA83_FPU; i<=PS_PA89_FPU; ++i)
  {	I18N_MAX(str,i18n("Numerical Coprocessor (FPU)"),fm,maxwidth);
	if ((1<<(i-1)) & pro.psp_coprocessor.psc_present)
	{ 	str += TAB 
		    + QString( (i==PS_PA83_FPU) ? "PS_PA83_FPU":"PS_PA89_FPU" )
		    + QString(" (")
		    + QString(((1<<(i-1))&pro.psp_coprocessor.psc_enabled) ? 
				    i18n("enabled") : i18n("disabled")	)
		    + QString(")");
		lBox->insertItem( str );
	}
  }// for(coprocessor..)
  
  lBox->insertItem( "" );
  I18N_MAX(str,i18n("Total Physical Memory"),fm,maxwidth);
  str += TAB 
      + Value(((pst.physical_memory*pst.page_size)/1024/1024)) 
      + QString(" ")
      + i18n("MB");	// Mega-Byte
  lBox->insertItem(str);

  I18N_MAX(str,i18n("Size of one Page"),fm,maxwidth);	// ..of one Memory Page.
  str += TAB + Value(pst.page_size) + QString(" ") + i18n("Bytes");
  lBox->insertItem(str);

   
  lBox->setColumnWidth(0, maxwidth + PIXEL_ADD);
  
  return TRUE;
}

bool GetInfo_ReadfromFile( KTabListBox *lBox, char *Name )
{
  char buf[2048];

  QFile *file = new QFile(Name);

  if(!file->open(IO_ReadOnly)) {
    delete file; 
    return FALSE;
  }
  
  while (file->readLine(buf,sizeof(buf)-1) >= 0) 
  {	if (strlen(buf))
	    lBox->insertItem(buf);
  }
  
  file->close();
  delete file;
  return (lBox->count());
}


static bool GetInfo_ReadfromPipe( KTabListBox *lBox, char *FileName )
{
    FILE *pipe;
    QFile *devices = new QFile(FileName);
    QTextStream *t;
    QString s;
  
    if (!devices->exists()) 
    {	delete devices;
	return FALSE;
    }
  
    if ((pipe = popen(FileName, "r")) == NULL) 
    {   delete devices;
	pclose(pipe);
	return FALSE;
    }

    t = new QTextStream(pipe, IO_ReadOnly);

    while (!t->eof())
	if ((s=t->readLine())!="")
	    lBox->insertItem(s);
  
    delete t; 
    delete devices; 
    pclose(pipe);
  
    return (lBox->count());
}


bool GetInfo_IRQ( KTabListBox *lBox )
{	lBox = lBox;
	return FALSE;
}

bool GetInfo_DMA( KTabListBox *lBox )
{	lBox = lBox;
	return FALSE;
}

bool GetInfo_PCI( KTabListBox *lBox )
{	
    return(	GetInfo_ReadfromFile(lBox,INFO_PCI)    + 
		GetInfo_ReadfromFile(lBox,INFO_PCI_EISA) );
}

bool GetInfo_IO_Ports( KTabListBox *lBox )
{
	if (GetInfo_ReadfromPipe( lBox, INFO_IOPORTS_1 ))
	    return TRUE;
	else
	    return GetInfo_ReadfromPipe( lBox, INFO_IOPORTS_2 );
}


bool GetInfo_Devices( KTabListBox *lBox )
{
	return GetInfo_ReadfromPipe( lBox, INFO_DEVICES );
}

bool GetInfo_SCSI( KTabListBox *lBox )
{	
    return GetInfo_Devices( lBox );
}


/* Parts taken from fsusage.c from the Midnight Commander (mc)

   Copyright (C) 1991, 1992 Free Software Foundation, In

   Return the number of TOSIZE-byte blocks used by
   BLOCKS FROMSIZE-byte blocks, rounding away from zero.
   TOSIZE must be positive.  Return -1 if FROMSIZE is not positive.  */

static long fs_adjust_blocks(long blocks, int fromsize, int tosize)
{
    if (tosize <= 0)
	abort ();
    if (fromsize <= 0)
	return -1;

    if (fromsize == tosize)	/* E.g., from 512 to 512.  */
	return blocks;
    else if (fromsize > tosize)	/* E.g., from 2048 to 512.  */
	return blocks * (fromsize / tosize);
    else			/* E.g., from 256 to 512.  */
	return (blocks + (blocks < 0 ? -1 : 1)) / (tosize / fromsize);
}

/* Fill in the fields of FSP with information about space usage for
   the filesystem on which PATH resides.
   Return 0 if successful, -1 if not. */

#define CONVERT_BLOCKS(b) fs_adjust_blocks ((b), fsd.f_bsize, 512)

static int get_fs_usage (char *path, long *l_total, long *l_avail)
{   struct statfs fsd;    /* 4.3BSD, SunOS 4, HP-UX, AIX.  */
    unsigned long fsu_blocks,fsu_bfree,fsu_bavail;

   *l_total = *l_avail = 0;
    if (statfs (path, &fsd) < 0)
	return -1;

    fsu_blocks = CONVERT_BLOCKS (fsd.f_blocks);
    fsu_bfree = CONVERT_BLOCKS (fsd.f_bfree);
    fsu_bavail = CONVERT_BLOCKS (fsd.f_bavail);

    *l_avail = getuid () ? fsu_bavail/2 : fsu_bfree/2;
    *l_total = fsu_blocks/2;

    return 0;
}


// Some Ideas taken from garbazo from his source in info_fbsd.cpp

bool GetInfo_Partitions (KTabListBox *lbox)
{
	#define NUMCOLS 5
	int maxwidth[NUMCOLS]={0,0,0,0,0};
	QString Title[NUMCOLS];
	int n;
	
	struct fstab *fstab_ent;
	struct statvfs svfs;
	long total,avail;
	QFontMetrics fm(lbox->tableFont());
	QString str;
	QString MB(i18n("MB"));	// International Text for MB=Mega-Byte
	QString TAB(SEPERATOR);

	if (setfsent() != 1) // Try to open fstab 
	    return FALSE;

	Title[0] = i18n("Device");
	Title[1] = i18n("Mount Point");
	Title[2] = i18n("FS Type");
	Title[3] = i18n("Total Size");
	Title[4] = i18n("Free Size");

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

		/* fstab_ent->fs_type holds only "rw","xx","ro"... */
		memset(&svfs,0,sizeof(svfs)); 
		statvfs(fstab_ent->fs_file,&svfs);
		get_fs_usage(fstab_ent->fs_file, &total, &avail);
		
		if ( (n=fm.width(svfs.f_basetype)) > maxwidth[2])
			maxwidth[2]=n;

		if (!maxwidth[3])
		    maxwidth[3] = maxwidth[4] = fm.width("999999 MB");

		if (!strcmp(fstab_ent->fs_type,FSTAB_XX))	// valid drive ?
			svfs.f_basetype[0] = 0;
			    		
		str =  QString(fstab_ent->fs_spec) + TAB
		    +  QString(fstab_ent->fs_file) + TAB 
		    +  QString("  ")
		    +  (svfs.f_basetype[0] ? QString(svfs.f_basetype) : i18n("n/a"));
		if (svfs.f_basetype[0])
		    str += TAB
			+  Value((total+512)/1024,6) + MB + TAB
			+  Value((avail+512)/1024,6) + MB;
		lbox->insertItem(str);
	}
	endfsent(); 

	for (n=0; n<NUMCOLS; ++n)
	    lbox->setColumnWidth(n, maxwidth[n] + PIXEL_ADD);
	    
	return TRUE;
}


bool GetInfo_XServer_and_Video( KTabListBox *lBox )
{	lBox = lBox;
	return GetInfo_XServer_Generic( lBox );
}





#ifndef HAVE_ALIB_H

bool GetInfo_Sound( KTabListBox *lBox )
{	lBox = lBox;
	GetInfo_ErrorString = i18n("Sorry, Audio-Support (Alib) was disabled during configuration and compile-time !");
	return FALSE;
}

#else // defined(HAVE_ALIB_H)

#include "Alib.h"

static char *formatNames[] = {  
    "ADFUnknown",	"ADFMuLaw",	"ADFALaw",
    "ADFLin16",  	"ADFLin8",  	"ADFLin8Offset" };

/* handle typo in 1st release of Alib.h */
#ifndef ARightOutputChMask
#define ARightOutputChMask ARighOutputChMask
#endif


bool GetInfo_Sound( KTabListBox *lBox )
{	
    Audio	*audio;
    long	status;
    char	server[80];
    int		i;

    QString 	TAB(QString(" ")+QString(SEPERATOR));
    QString 	str,str2;
    QFontMetrics fm(lBox->tableFont());
    int		maxwidth;

    // server = Hostname....
    server[0] = 0;    
    audio = AOpenAudio( server, &status );
    if( status ) {
	GetInfo_ErrorString = i18n("Unable to open Audio-Server (Alib) !");
	return FALSE;
    }
    
    lBox->setNumCols(2);		// Table-Headers....
    maxwidth = 0;
    I18N_MAX(str,i18n("Information"),fm,maxwidth);
    lBox->setColumn(0,str,maxwidth );
    lBox->setColumn(1,i18n("Value") );
    lBox->setSeparator(SEPERATOR_CHAR);
    
    I18N_MAX(str,i18n("Audio Name"),fm,maxwidth);
    lBox->insertItem(str + TAB + QString(audio->audio_name));

    I18N_MAX(str,i18n("Vendor"),fm,maxwidth);
    lBox->insertItem(str + TAB + QString(audio->vendor));
    
    I18N_MAX(str,i18n("Alib Version"),fm,maxwidth);    
    lBox->insertItem(str+TAB+Value(audio->alib_major_version) + QString(".") + Value(audio->alib_minor_version));
    
    I18N_MAX(str,i18n("Protocol Revision"),fm,maxwidth);
    str += TAB + Value(audio->proto_major_version) 
	+ QString(".") + Value(audio->proto_minor_version);
    lBox->insertItem(str);

    /* printf( "fd %d\n", audio->fd ); */
    
    I18N_MAX(str,i18n("Vendor Number"),fm,maxwidth);
    lBox->insertItem(str + TAB + Value(audio->vnumber));

    I18N_MAX(str,i18n("Release"),fm,maxwidth);
    lBox->insertItem(str + TAB + Value(audio->release));
    
    lBox->insertItem("");
    I18N_MAX(str,i18n("Byte Order"),fm,maxwidth);
    str += TAB + QString(  
	    (audio->byte_order==ALSBFirst)? i18n("ALSBFirst (LSB)"):(
	    (audio->byte_order==AMSBFirst)? i18n("AMSBFirst (MSB)"):
					    i18n("Invalid Byteorder !")) );
    lBox->insertItem(str);

    I18N_MAX(str,i18n("Bit Order"),fm,maxwidth);
    str += TAB + QString(  
	    (audio->sound_bit_order==ALeastSignificant)? i18n("ALeastSignificant (LSB)"):(
	    (audio->sound_bit_order==AMostSignificant) ? i18n("AMostSignificant (MSB)"):
					    i18n("Invalid Bitorder !")) );
    lBox->insertItem(str);

    lBox->insertItem("");
    I18N_MAX(str,i18n("Data Formats"),fm,maxwidth);
    for ( i = 0;  i < audio->n_data_format; i++ ) {
	if (audio->data_format_list[i] <= ADFLin8Offset)
	    lBox->insertItem(str + TAB + QString(formatNames[audio->data_format_list[i]]));
	str = "";
    }

    lBox->insertItem("");
    I18N_MAX(str,i18n("Sampling Rates"),fm,maxwidth);    
    for ( i = 0;  i < audio->n_sampling_rate; i++ ) {
	lBox->insertItem( str + TAB + Value(audio->sampling_rate_list[i]));
	str = "";
    }

    lBox->insertItem("");
    I18N_MAX(str,i18n("Input Sources"),fm,maxwidth);    
    str += TAB;
    if ( audio->input_sources & AMonoMicrophoneMask ) 
    { lBox->insertItem(str+i18n("Mono-Microphone")); str = TAB; }
    if ( audio->input_sources & AMonoAuxiliaryMask )
    { lBox->insertItem(str+i18n("Mono-Auxiliary"));  str = TAB; }
    if ( audio->input_sources & ALeftMicrophoneMask )
    { lBox->insertItem(str+i18n("Left-Microphone")); str = TAB; }
    if ( audio->input_sources & ARightMicrophoneMask )
    { lBox->insertItem(str+i18n("Right-Microphone"));str = TAB; }
    if ( audio->input_sources & ALeftAuxiliaryMask )
    { lBox->insertItem(str+i18n("Left-Auxiliary"));  str = TAB; }
    if ( audio->input_sources & ARightAuxiliaryMask )
    { lBox->insertItem(str+i18n("Right-Auxiliary")); str = TAB; }

    lBox->insertItem("");
    I18N_MAX(str,i18n("Input Channels"),fm,maxwidth);    
    str += TAB;
    if ( audio->input_channels & AMonoInputChMask )
    { lBox->insertItem(str+i18n("Mono-Channel"));  str = TAB; }
    if ( audio->input_channels & ALeftInputChMask )
    { lBox->insertItem(str+i18n("Left-Channel"));  str = TAB; }
    if ( audio->input_channels & ARightInputChMask )
    { lBox->insertItem(str+i18n("Right-Channel")); str = TAB; }

    lBox->insertItem("");
    I18N_MAX(str,i18n("Output Destinations"),fm,maxwidth);    
    str += TAB;
    if ( audio->output_destinations & AMonoIntSpeakerMask )
    { lBox->insertItem(str+i18n("Mono-InternalSpeaker")); str = TAB; }
    if ( audio->output_destinations & AMonoJackMask )
    { lBox->insertItem(str+i18n("Mono-Jack")); 	str = TAB; }
    if ( audio->output_destinations & ALeftIntSpeakerMask )
    { lBox->insertItem(str+i18n("Left-InternalSpeaker")); str = TAB; }
    if ( audio->output_destinations & ARightIntSpeakerMask )
    { lBox->insertItem(str+i18n("Right-InternalSpeaker")); str = TAB; }
    if ( audio->output_destinations & ALeftJackMask )
    { lBox->insertItem(str+i18n("Left-Jack")); str = TAB; }
    if ( audio->output_destinations & ARightJackMask )
    { lBox->insertItem(str+i18n("Right-Jack")); str = TAB; }

    lBox->insertItem("");
    I18N_MAX(str,i18n("Output Channels"),fm,maxwidth);    
    str += TAB;
    if ( audio->output_channels & AMonoOutputChMask )
    { lBox->insertItem(str+i18n("Mono-Channel")); str = TAB; }
    if ( audio->output_channels & ALeftOutputChMask )
    { lBox->insertItem(str+i18n("Left-Channel")); str = TAB; }
    if ( audio->output_channels & ARightOutputChMask )
    { lBox->insertItem(str+i18n("Right-Channel")); str = TAB; }


    lBox->insertItem("");
    I18N_MAX(str,i18n("Input Gain Limits"),fm,maxwidth);    
    lBox->insertItem(str+TAB+Value(audio->min_input_gain) + QString(" ") + Value(audio->max_input_gain));
    I18N_MAX(str,i18n("Output Gain Limits"),fm,maxwidth);    
    lBox->insertItem(str+TAB+Value(audio->min_output_gain) + QString(" ") + Value(audio->max_output_gain));
    I18N_MAX(str,i18n("Monitor Gain Limits"),fm,maxwidth);    
    lBox->insertItem(str+TAB+Value(audio->min_monitor_gain) + QString(" ") + Value(audio->max_monitor_gain));
    I18N_MAX(str,i18n("Gain Restricted"),fm,maxwidth);    
    lBox->insertItem(str+TAB+Value(audio->gm_gain_restricted));
    /* printf( "sample rate tolerance  %f %f\n", audio->sample_rate_lo_tolerance,audio->sample_rate_hi_tolerance ); */
    I18N_MAX(str,i18n("Lock"),fm,maxwidth);    
    lBox->insertItem(str+TAB+Value(audio->lock));
    I18N_MAX(str,i18n("Queue Length"),fm,maxwidth);    
    lBox->insertItem(str+TAB+Value(audio->qlen));
    I18N_MAX(str,i18n("Block Size"),fm,maxwidth);    
    lBox->insertItem(str+TAB+Value(audio->block_size));
    I18N_MAX(str,i18n("Stream Port (decimal)"),fm,maxwidth);    
    lBox->insertItem(str+TAB+Value(audio->stream_port));
    I18N_MAX(str,i18n("Ev Buffer Size"),fm,maxwidth);    
    lBox->insertItem(str+TAB+Value(audio->ev_buf_size));
    I18N_MAX(str,i18n("Ext Number"),fm,maxwidth);    
    lBox->insertItem(str+TAB+Value(audio->ext_number));

    ACloseAudio( audio, &status );
   
    lBox->setColumnWidth(0, maxwidth + PIXEL_ADD);
    return TRUE;
}

#endif // defined(HAVE_ALIB_H)
