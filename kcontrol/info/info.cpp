/* 	
	$Id$
	
	Main Widget for showing system-dependent information.
	
	** main.cpp includes this file ! **

	This source-file includes another system-dependet sourcefile called
	info_<systemname>.cpp
	which should define one or more of the following defines to
	indicate, that this information is really available.
	
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
	
	right now, there is the problem, that also the .kdelnk-files should 
	depend on the systemname, so that only available .kdelnk-files will 
	be copied to kde/applnk/Settings/Information !!


	Last modified:	by:
	21.05.1999	deller
			minor fixes, sprintf()->arg() for QT 2.x
	24.04.1999	deller
			changed i18n()'s in Get_X_Server, because there was 
			some really ugly layout-problems in some languages !
	01.11.1998	first version [Helge Deller, deller@gmx.de]
	
*/

#include <qheader.h>

#include <kapp.h>
#include <kglobal.h>
#include <kcharsets.h>
#include <klocale.h>
#include "info.h"		/* include the forward declares... */

#include <X11/Xlib.h>

/* All Functions GetInfo_xyz() can set GetInfo_ErrorString, when a special 
   error-message should be shown to the user....
   If GetInfo_ErrorString is not modified in the function, the default string
   DEFAULT_ERRORSTRING will be used...
*/

static QString	GetInfo_ErrorString;
static bool	sorting_allowed;	/* is sorting allowed by user ? */

#define DEFAULT_ERRORSTRING \
  i18n("No Information available\n or \nthis System is not yet supported :-(")

/* easier to read with such a define ! */
#define I18N_MAX(txt,in,fm,maxw) \
    { int n = fm.width(txt=in); if (n>maxw) maxw=n; }
    
#define PIXEL_ADD	20	// add x Pixel to multicolumns..

static QString Value( int val, int numbers=1 )
{	return QString("%1").arg(val,numbers);
}


static QListViewItem* XServer_fill_screen_info( QListView *lBox, QListViewItem* last, Display *dpy, int scr)
{
    int i;
    double xres, yres;
    int ndepths = 0, *depths = 0;
    
    xres = ((double)(DisplayWidth(dpy,scr)  * 25.4) / DisplayWidthMM(dpy,scr) );
    yres = ((double)(DisplayHeight(dpy,scr) * 25.4) / DisplayHeightMM(dpy,scr));
    
    QListViewItem* item;
    item = new QListViewItem(lBox, last, i18n("Screen # %1").arg((int)scr,-1) );
    
    last = new QListViewItem(item, i18n("Dimensions"),
		i18n("%1 x %2 Pixel (%3 x %4 mm)")
		.arg( (int)DisplayWidth(dpy,scr) )
		.arg( (int)DisplayHeight(dpy,scr) )
		.arg( (int)DisplayWidthMM(dpy,scr) )
		.arg( (int)DisplayHeightMM (dpy,scr) ));
    
    last = new QListViewItem(item, last, i18n("Resolution"), 
		i18n("%1 x %2 dpi")
		.arg( (int)(xres+0.5) )
		.arg( (int)(yres+0.5) ));
    
    depths = XListDepths (dpy, scr, &ndepths);
    if (depths) {
	QString txt;
    
        for (i = 0; i < ndepths; i++) {	
            txt = txt + Value(depths[i]);
            if (i < ndepths - 1)
                txt = txt + QString(", ");
        }
    
        new QListViewItem(item, last, i18n("Depths (%1)").arg(ndepths,-1), txt);
        XFree((char *) depths);
    }

    item->setOpen(true);
    return item;
}


bool GetInfo_XServer_Generic( QListView *lBox )
{
    /* Parts of this source is taken from the X11-program "xdpyinfo" */

    Display *dpy;
    int i;
    QString str,txt;

    dpy = XOpenDisplay(0);
    if (!dpy)
        return false;

    lBox->addColumn(i18n("Information") );
    lBox->addColumn(i18n("Value") );
    sorting_allowed = false;
			
    QListViewItem *last;
    last = new QListViewItem(lBox, i18n("Name of the Display"), DisplayString(dpy));

    last = new QListViewItem(lBox, last, i18n("Version Number"),
                      Value((int)ProtocolVersion(dpy)) + QString(".")
                      + Value((int)ProtocolRevision(dpy)));

    last = new QListViewItem(lBox, last, i18n("Vendor String"), QString(ServerVendor(dpy)));
    last = new QListViewItem(lBox, last, i18n("Vendor Release Number"),
                      Value((int)VendorRelease(dpy)));
    last = new QListViewItem(lBox, last, i18n("Default Screen Number") + "   ",
                      Value((int)DefaultScreen(dpy)));
    last = new QListViewItem(lBox, last, i18n("Number of Screens") ,
                      Value((int)ScreenCount(dpy)));

    for (i = 0; i < ScreenCount (dpy); ++i)
        last = XServer_fill_screen_info (lBox, last, dpy, i);

    XCloseDisplay (dpy);
    return true;
}



/*
***************************************************************************
***************************************************************************
***************************************************************************
*/

#include <qlayout.h>
#include <qobjcoll.h>
#include <qwidget.h>
#include <qwidcoll.h>


#define SCREEN_XY_OFFSET 20

void KInfoListWidget::defaults()
{  
    bool ok = false;

    delete lBox;
    lBox  = new QListView(this);

    if (lBox) {
	lBox->setFont(KGlobal::generalFont()); // default font
        lBox->setAllColumnsShowFocus(true);
        setMinimumSize( 200,6*SCREEN_XY_OFFSET );
        lBox->setGeometry(SCREEN_XY_OFFSET,SCREEN_XY_OFFSET,
                          width() -2*SCREEN_XY_OFFSET,
                          height()-2*SCREEN_XY_OFFSET);
        
	/*  Delete the user-visible ErrorString, before calling the 
	    retrieve-function. If the function wants the widget to show
	    another string, then it should modify GetInfo_ErrorString ! */
        GetInfo_ErrorString = "";
	sorting_allowed = true; 	// the functions may set that !
        lBox->setSorting(-1);   	// No Sorting per default
        
        if (getlistbox)
            ok = (*getlistbox)(lBox);	// retrieve the information !

        if (lBox->header()->count()<=1) 
            lBox->addColumn(title);
        if (ok)
            lBox->show();

	/* is the user allowed to use sorting ? */
        lBox->header()->setClickEnabled(sorting_allowed); 
    }

    if (!ok) {
        if (lBox) {
            delete lBox;
            lBox = 0;
        }	    
        if (GetInfo_ErrorString.isEmpty())
            GetInfo_ErrorString = DEFAULT_ERRORSTRING;
        if (NoInfoText)
            NoInfoText->setText(GetInfo_ErrorString);
        else
            NoInfoText = new QLabel(GetInfo_ErrorString,this);
        NoInfoText->setAutoResize(true);
        NoInfoText->setAlignment(AlignCenter); //  | WordBreak);
        NoInfoText->move( width()/2,height()/2 ); // -120 -30
        NoInfoText->adjustSize();
    }
}


KInfoListWidget::KInfoListWidget(QWidget *parent, const char *name, 
                                 bool _getlistbox(QListView *lbox))
    : KCModule(parent, name)
{   
    int pos;
    getlistbox 	= _getlistbox;
    lBox 	= 0;
    NoInfoText  = 0;
    GetInfo_ErrorString = "";
    defaults();

    setButtons(Ok|Help);
}


void KInfoListWidget::resizeEvent( QResizeEvent *re )
{
    QSize size = re->size();
    if (lBox)
        lBox->setGeometry(SCREEN_XY_OFFSET,SCREEN_XY_OFFSET,
                          size.width() -2*SCREEN_XY_OFFSET,
                          size.height()-2*SCREEN_XY_OFFSET);
    if (NoInfoText) {
        QSize s = NoInfoText->sizeHint();
    	NoInfoText->move(   (size.width()-s.width())/2,
                            (size.height()-s.height())/2 );
    }
}

/*
***************************************************************************
**  Include system-specific code					 **
***************************************************************************
*/

#ifdef linux
#include "info_linux.cpp"
#elif sgi || sun
#include "info_sgi.cpp"
#elif __FreeBSD__
#include "info_fbsd.cpp"
#elif hpux
#include "info_hpux.cpp"
#elif __svr4__
#include "info_svr4.cpp"
#else
#include "info_generic.cpp"	/* Default for unsupportet systems.... */
#endif

/*
***************************************************************************
**  End of: Include system-specific code				 **
***************************************************************************
*/
