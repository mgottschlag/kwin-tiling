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

#include <qtabbar.h>

#include <kapp.h>
#include <kcharsets.h>
#include <klocale.h>
#include "info.h"		/* include the forward declares... */
#include "info.moc"

#include <X11/Xlib.h>


/* All Functions GetInfo_xyz() can set GetInfo_ErrorString, when a special 
   error-message should be shown to the user....
   If GetInfo_ErrorString is not modified in the function, the default string
   DEFAULT_ERRORSTRING will be used...
*/

QString	GetInfo_ErrorString;

#define DEFAULT_ERRORSTRING \
  i18n("No Information available\n or \nthis System is not yet supported :-(")

/* easier to read with such a define ! */
#define I18N_MAX(txt,in,fm,maxw) \
    { int n = fm.width(txt=in); if (n>maxw) maxw=n; }
    
#define SEPERATOR 	"\t"
#define SEPERATOR_CHAR	'\t'

#define PIXEL_ADD	10	// add x Pixels to multicolumns

static QString Value( int val, int numbers=1 )
{	return QString("%1").arg(val,numbers);
}


static void XServer_fill_screen_info( KTabListBox *lBox, Display *dpy, int scr,
	    QFontMetrics fm, int *maxwidth )
{
    int i;
    double xres, yres;
    int ndepths = 0, *depths = NULL;
    QString txt,txt2;
    QString TAB(SEPERATOR);
    
    xres = ((double)(DisplayWidth(dpy,scr)  * 25.4) / DisplayWidthMM(dpy,scr) );
    yres = ((double)(DisplayHeight(dpy,scr) * 25.4) / DisplayHeightMM(dpy,scr));

    lBox->insertItem("");

    lBox->insertItem( i18n("Screen # %1").arg((int)scr,-1) );

    I18N_MAX(txt,i18n("Dimensions"),fm,*maxwidth);
    txt2 = i18n("%1 x %2 Pixels (%3 x %4 mm)")
		.arg( (int)DisplayWidth(dpy,scr) )
		.arg( (int)DisplayHeight(dpy,scr) )
		.arg( (int)DisplayWidthMM(dpy,scr) )
		.arg( (int)DisplayHeightMM (dpy,scr) );
    lBox->insertItem( QString(" ") + txt + TAB + txt2 );

    I18N_MAX(txt,i18n("Resolution"),fm,*maxwidth);
    txt2 = i18n("%1 x %2 Dots per Inch (dpi)")
		.arg( (int)(xres+0.5) )
		.arg( (int)(yres+0.5) );
    lBox->insertItem( QString(" ") + txt + TAB + txt2 );

    depths = XListDepths (dpy, scr, &ndepths);
    if (!depths) ndepths = 0;
    I18N_MAX(txt,i18n("Depths"),fm,*maxwidth);
    txt =  QString(" ") + txt + QString(" (%1)").arg(ndepths,-1) + TAB;
    
    for (i = 0; i < ndepths; i++) 
    {	
	txt = txt + Value(depths[i]);
	if (i < ndepths - 1)
	    txt = txt + QString(", ");
    }
    lBox->insertItem(txt);
    if (depths) XFree((char *) depths);
}


bool GetInfo_XServer_Generic( KTabListBox *lBox )
{
    /* Parts of this source is taken from the X11-program "xdpyinfo" */

    Display *dpy;
    int i;
    QString str,txt;
    QFontMetrics fm(lBox->tableFont());
    int	maxwidth;
    QString TAB(SEPERATOR);

    dpy = XOpenDisplay(NULL);
    if (!dpy)  return FALSE;

    lBox->setNumCols(2);		// Table-Headers....
    maxwidth = 0;
    I18N_MAX(txt,i18n("Information"),fm,maxwidth);
    lBox->setColumn(0,txt,maxwidth );
    lBox->setColumn(1,i18n("Value") );
    lBox->setSeparator(SEPERATOR_CHAR);

			
    I18N_MAX(txt,i18n("Name of the Display"),fm,maxwidth);
    str = txt + TAB + DisplayString(dpy);
    lBox->insertItem(str);

    I18N_MAX(txt,i18n("Version Number"),fm,maxwidth);
    str = txt 	+ TAB + Value((int)ProtocolVersion(dpy)) 
		+ QString(".") + Value((int)ProtocolRevision(dpy));
    lBox->insertItem(str);

    I18N_MAX(txt,i18n("Vendor String"),fm,maxwidth);
    str = txt + TAB + QString(ServerVendor(dpy));
    lBox->insertItem(str);

    I18N_MAX(txt,i18n("Vendor Release Number"),fm,maxwidth);
    str = txt + TAB + Value((int)VendorRelease(dpy));
    lBox->insertItem(str);

    I18N_MAX(txt,i18n("Default Screen Number"),fm,maxwidth);
    str = txt + TAB + Value((int)DefaultScreen(dpy));
    lBox->insertItem(str);

    I18N_MAX(txt,i18n("Number of Screens"),fm,maxwidth);
    str = txt + TAB + Value((int)ScreenCount(dpy));
    lBox->insertItem(str);

    for (i = 0; i < ScreenCount (dpy); ++i)
	XServer_fill_screen_info (lBox,dpy, i, fm, &maxwidth);

    lBox->setColumnWidth(0,maxwidth+20);
    
    XCloseDisplay (dpy);
    return TRUE;
}



/*
***************************************************************************
***************************************************************************
***************************************************************************
*/

#include <qobjcoll.h>
#include <qwidget.h>
#include <qwidcoll.h>

void set_Status_for_all_Children( QWidget *start, bool wait )
{	const QObjectList *list = start->children();
	QObjectListIt it(*list);
	QObject *obj;
	while ( (obj=it.current()) != 0 )
	{	++it;
		if (obj->inherits("QWidget"))
		{   // set_Status_for_all_Children( (QWidget*)obj, wait ); /*doesn't work ! why ?? */
		    ((QWidget*)obj)->setCursor(wait ? WaitCursor:ArrowCursor);
		}
	}
}

void set_Status_for_all_Widgets( QWidget *start, bool wait )
{   
    set_Status_for_all_Children( start->topLevelWidget(), wait );
}


#define SCREEN_XY_OFFSET 20

void KInfoListWidget::defaultSettings()
{  
    bool ok = FALSE;
    
    if (lBox)	delete lBox;
    lBox 	= new KTabListBox(this);

    if (lBox)
    {   lBox->clear();
	lBox->setMinimumSize( 200,2*SCREEN_XY_OFFSET );
	lBox->setGeometry(SCREEN_XY_OFFSET,SCREEN_XY_OFFSET,
                     width() -2*SCREEN_XY_OFFSET,
		     height()-2*SCREEN_XY_OFFSET);
	lBox->setTableFont(QFont("Courier"));
	lBox->enableKey();
	lBox->setAutoUpdate(TRUE);

	/*  Delete the user-visible ErrorString, before calling the 
	    retrieve-function. If the function wants the widget to show
	    another string, then it should modify GetInfo_ErrorString ! */
	GetInfo_ErrorString = "";
	
        if (getlistbox)
	{   set_Status_for_all_Widgets( lBox, TRUE );
	    ok = (*getlistbox)(lBox);	// retrieve the information !
	    set_Status_for_all_Widgets( lBox, FALSE );
	}

	if (lBox->numCols()<=1)  // if ONLY ONE COLUMN (!), then set title and calculate necessary column-width
	{   QFontMetrics fm(lBox->tableFont());
	    int row, cw, colwidth = 0;
	    row = lBox->numRows();
	    while (row>=0)			// loop through all rows in this single column
	    {  cw = fm.width(lBox->text(row));	// calculate the necessary width
	       if (cw>colwidth) colwidth=cw;
	       --row;
	    }
	    colwidth += 5;
	    if (!title.isEmpty()) 
   	        lBox->setColumn(0,title,colwidth); // set title and width
	    else
   	        lBox->setDefaultColumnWidth(colwidth); // only set width
	}

	if (ok) lBox->show();
    }

    if (!ok)
    {	if (lBox) { delete lBox; lBox = 0; }	    
	if (GetInfo_ErrorString.isEmpty())
	    GetInfo_ErrorString = DEFAULT_ERRORSTRING;
	if (NoInfoText)
	    NoInfoText->setText(GetInfo_ErrorString);
	else
	    NoInfoText = new QLabel(GetInfo_ErrorString,this);
	NoInfoText->setAutoResize(TRUE);
	NoInfoText->setAlignment(AlignCenter); //  | WordBreak);
	NoInfoText->move( width()/2,height()/2 ); // -120 -30
    }
}


KInfoListWidget::KInfoListWidget(QWidget *parent, const char *name, 
		QString _title, bool _getlistbox(KTabListBox *lbox))
  : KConfigWidget(parent, name)
{   
    int pos;
    getlistbox 	= _getlistbox;
    lBox 	= 0;
    NoInfoText  = 0;
    title   	= _title;
    if (title.isEmpty() && name)
	title = QString(name);
    do {	// delete all '&'-chars !
	pos = title.find('&');
	if (pos>=0) title.remove(pos,1);	// delete this char !
    } while (pos>=0);
    GetInfo_ErrorString = "";
    setMinimumSize( 200,6*SCREEN_XY_OFFSET );
    defaultSettings();
}


void KInfoListWidget::resizeEvent( QResizeEvent *re )
{   QSize size = re->size();
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
