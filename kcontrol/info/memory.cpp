/* $Id$ */

#include <qtabbar.h>
#include "memory.h"
#include <klocale.h>
#include <kglobal.h>

#define OFFSET_DX 20
#define STARTY    20
#define DY        24

typedef unsigned long t_memsize;

enum { 			/* entries for Memory_Info[] */
    TOTAL_MEM = 0,	/* total physical memory (without swaps) */
    FREE_MEM,		/* total free physical memory (without swaps) */
    SHARED_MEM,
    BUFFER_MEM,
    SWAP_MEM,		/* total size of all swap-partitions */
    FREESWAP_MEM,	/* free memory in swap-partitions */
    MEM_LAST_ENTRY };
/*
   all update()-functions should write their results OR NO_MEMORY_INFO 
   into Memory_Info[] !
*/
static t_memsize Memory_Info[MEM_LAST_ENTRY];

#define MEMORY(x)	((t_memsize) (x))	  // it's easier...
#define NO_MEMORY_INFO	MEMORY(-1)



/* Implementation */

static QLabel   *MemSizeLabel[MEM_LAST_ENTRY][2];

static QString format( t_memsize value)
{
#ifdef linux
  double   mb = value / 1024000.0; /* with Linux divide by (1024*1000) */
#elif hpux	
  double   mb = value / 1048576.0; /* with hpux divide by (1024*1024) */
#else 	// I don't know for other archs... please fill in !
  double   mb = value / 1048576.0; /* divide by (1024*1024) */
#endif
  return i18n("%1 bytes = %2 MB").arg(value,10).arg(mb,7,'f',2);
}

KMemoryWidget::KMemoryWidget(QWidget *parent, const char *name)
  : KCModule(parent, name)
{
    QFont	fixedfont    = KGlobal::fixedFont();	
    QFont	generalfont  = KGlobal::generalFont();
    QFontInfo	*fontinfo;
    QString	title,initial_str;
    QLabel	*titleWidget = 0;
    int 	i,xs;  

    setButtons(Ok|Help);

    fontinfo = new QFontInfo(fixedfont);	/* make same font-sizes.. */
    i  = fontinfo->pointSize();
    delete fontinfo;
    fontinfo = new QFontInfo(generalfont);
    xs = fontinfo->pointSize();
    delete fontinfo;
    if ( xs > i) i = xs;
    generalfont.setPointSize(i);
    fixedfont.setPointSize(i);

    // Create all Labels...
    Width_Info  = 0;
    initial_str = format(0);
    
    for (i=TOTAL_MEM; i<MEM_LAST_ENTRY; ++i) {
	switch (i) {
	    case TOTAL_MEM: 	title = i18n("Total physical memory");	break;
	    case FREE_MEM:	title = i18n("Free physical memory");	break;
	    case SHARED_MEM:	title = i18n("Shared memory");		break;
	    case BUFFER_MEM:	title = i18n("Buffer memory");		break;
	    case SWAP_MEM:	title = i18n("Total swap memory");	break;
	    case FREESWAP_MEM:	title = i18n("Free swap memory");	break;
	    default:		title = "";				break;
	};
	
	titleWidget = new QLabel(title, this);	/* first create the Information-Widget */
        titleWidget->move(-1000,-1000);	// initially invisible !
        titleWidget->setFont(generalfont);
	titleWidget->setAutoResize(TRUE);
	MemSizeLabel[i][0] = titleWidget;
	xs = titleWidget->sizeHint().width();
	if (xs > Width_Info)  // get maximum size...
		Width_Info = xs;
	
	titleWidget = new QLabel(initial_str, this); /* then the Label for the size */
        titleWidget->move(-1000,-1000);	// initially invisible !
        titleWidget->setFont(fixedfont);
//        titleWidget->setAutoResize(TRUE); /* BUG in QT2.0 ?-> needs adjustSize()! */
        titleWidget->adjustSize();
	MemSizeLabel[i][1] = titleWidget;
  }
  
  Width_Value = titleWidget->sizeHint().width();
  Not_Available_Text = i18n("No information available.");
  /* fill string "Not_Available_Text" with spaces to len of format(0) */
  do {
       Not_Available_Text = " "+Not_Available_Text+" "; // add spaces..
       titleWidget->setText(Not_Available_Text); // get width for the NO-INFO-text
       xs = titleWidget->sizeHint().width();
  } while (xs<Width_Value);

  timer = new QTimer(this);
  timer->start(100);
  QObject::connect(timer, SIGNAL(timeout()), this, SLOT(update_Values()));

  update();
}

KMemoryWidget::~KMemoryWidget()
{
    /* stop the timer */
    timer->stop();
    
    /* Remove all Labels */
    for (int i=TOTAL_MEM; i<MEM_LAST_ENTRY; ++i) {
	delete MemSizeLabel[i][0];
	delete MemSizeLabel[i][1];
    }
}



/* Center all Labels in the widget */
void KMemoryWidget::resizeEvent( QResizeEvent *re )
{   
    QSize size = re->size();
    int addy;
    int maxx = Width_Info + OFFSET_DX + Width_Value;
    int left = (size.width()-maxx) / 2;
    int i;
    for (i=TOTAL_MEM; i<MEM_LAST_ENTRY; ++i) {
	addy = (i>=SWAP_MEM) ? DY : 0;
	MemSizeLabel[i][0]->move(left,
				    addy + STARTY + i*DY);
	MemSizeLabel[i][1]->move(left+Width_Info+OFFSET_DX,
				    addy + STARTY + i*DY + 1);
    }
    setMinimumHeight(addy + STARTY + i*DY);
}



/* update_Values() is the main-loop for updating the Memory-Information */
void KMemoryWidget::update_Values()
{
  int i;
  update();	/* get the Information from memory_linux, memory_fbsd */
  for (i=TOTAL_MEM; i<MEM_LAST_ENTRY; ++i) {
    MemSizeLabel[i][1]->setText(  (Memory_Info[i]!=NO_MEMORY_INFO) ?
	    format(Memory_Info[i]) : Not_Available_Text );
  }
}


/* Include system-specific code */

#ifdef linux
#include "memory_linux.cpp"
#elif sgi
#include "memory_sgi.cpp"
#elif __FreeBSD__
#include "memory_fbsd.cpp"
#elif hpux
#include "memory_hpux.cpp"
#else

/* Default for unsupported systems */
void KMemoryWidget::update()
{
    Memory_Info[TOTAL_MEM]    = NO_MEMORY_INFO; // total physical memory (without swaps)
    Memory_Info[FREE_MEM]     = NO_MEMORY_INFO;	// total free physical memory (without swaps)
    Memory_Info[SHARED_MEM]   = NO_MEMORY_INFO; 
    Memory_Info[BUFFER_MEM]   = NO_MEMORY_INFO; 
    Memory_Info[SWAP_MEM]     = NO_MEMORY_INFO; // total size of all swap-partitions
    Memory_Info[FREESWAP_MEM] = NO_MEMORY_INFO; // free memory in swap-partitions
}

#endif
