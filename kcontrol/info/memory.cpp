/*
 *  memory.cpp
 *
 *  prints memory-information and shows a graphical display.
 *
 *  Copyright (c) 1999 Helge Deller   (deller@gmx.de)
 *
 *
 *  Requires the Qt widget libraries, available at no cost at
 *  http://www.troll.no/
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 *  $Id$ 
 */

#include <sys/param.h>	/* for BSD */

#include <qtabbar.h>
#include <qlayout.h>
#include <qpainter.h>

#include <klocale.h>
#include <kglobal.h>

#include "memory.h"

enum { 			/* entries for Memory_Info[] */
    TOTAL_MEM = 0,	/* total physical memory (without swaps) */
    FREE_MEM,		/* total free physical memory (without swaps) */
#if !defined(__svr4__) || !defined(sun)
#ifndef __NetBSD__
    SHARED_MEM,
    BUFFER_MEM,
#else
    ACTIVE_MEM,
    INACTIVE_MEM,
#endif
#endif
    SWAP_MEM,		/* total size of all swap-partitions */
    FREESWAP_MEM,	/* free memory in swap-partitions */
    MEM_LAST_ENTRY };
/*
   all update()-functions should write their results OR NO_MEMORY_INFO 
   into Memory_Info[] !
*/

static t_memsize Memory_Info[MEM_LAST_ENTRY];

#define MEMORY(x)	((t_memsize) (x))	  // it's easier...
#define NO_MEMORY_INFO	MEMORY(-1)	/*  DO NOT CHANGE */



/******************/
/* Implementation */
/******************/

static QLabel   *MemSizeLabel[MEM_LAST_ENTRY][2];

enum { MEM_RAM_AND_HDD, MEM_RAM, MEM_HDD, MEM_LAST };
static QWidget	*Graph[MEM_LAST];
static QLabel	*GraphLabel[MEM_LAST];

#define SPACING 16

static QString format_MB( t_memsize value)
{
#ifdef __linux__
  double   mb = value / 1024000.0; /* with Linux divide by (1024*1000) */
#elif hpux	
  double   mb = value / 1048576.0; /* with hpux divide by (1024*1024) */
#else 	// I don't know for other archs... please fill in !
  double   mb = value / 1048576.0; /* divide by (1024*1024) */
#endif
  return i18n("%1 MB").arg(KGlobal::locale()->formatNumber(mb, 2));
}

KMemoryWidget::KMemoryWidget(QWidget *parent, const char *name)
  : KCModule(parent, name)
{
    QString	title,initial_str;
    QLabel	*Widget = 0;
    int 	i,j;

    setButtons(Help);

    /* default string for no Information... */
    Not_Available_Text = i18n("Not available.");

    QVBoxLayout *top = new QVBoxLayout(this, 10, 10);
    
    Widget = new QLabel(i18n("Memory Information"), this);
    Widget->setAlignment(AlignCenter);
    QFont font(Widget->font());
    font.setUnderline(true);
    font.setPointSize(3*font.pointSize()/2);
    Widget->setFont(font);
    top->addWidget(Widget);
    top->addSpacing(SPACING);
    
    QHBoxLayout *hbox = new QHBoxLayout();
    top->addLayout(hbox);

    /* stretch the left side */
    hbox->addStretch();
		
    /* first create the Informationtext-Widget */
    QVBoxLayout *vbox = new QVBoxLayout(hbox,0);
    for (i=TOTAL_MEM; i<MEM_LAST_ENTRY; ++i) {
	switch (i) {
	    case TOTAL_MEM: 	title = i18n("Total physical memory");	break;
	    case FREE_MEM:	title = i18n("Free physical memory");	break;
#if !defined(__svr4__) || !defined(sun)
#ifndef __NetBSD__
	    case SHARED_MEM:	title = i18n("Shared memory");		break;
	    case BUFFER_MEM:	title = i18n("Buffer memory");		break;
#else
	    case ACTIVE_MEM:	title = i18n("Active memory");		break;
	    case INACTIVE_MEM:	title = i18n("Inactive memory");	break;
#endif
#endif
	    case SWAP_MEM:	vbox->addSpacing(SPACING);
				title = i18n("Total swap memory");	break;
	    case FREESWAP_MEM:	title = i18n("Free swap memory");	break;
	    default:		title = "";				break;
	};
	Widget = new QLabel(title, this);
	Widget->setAlignment(AlignLeft);
	vbox->addWidget(Widget,1);
    }	

    /* then the memory-content-widgets */
    for (j=0; j<2; j++) {
	vbox = new QVBoxLayout(hbox,0);
        for (i=TOTAL_MEM; i<MEM_LAST_ENTRY; ++i) {
	    if (i==SWAP_MEM)
	    	vbox->addSpacing(SPACING);
    	    Widget = new QLabel("",this); 
	    Widget->setAlignment(AlignRight);
	    MemSizeLabel[i][j] = Widget;
	    vbox->addWidget(Widget,1);
	}
    }

    /* stretch the right side */
    hbox->addStretch();

    QFrame* tmpQFrame = new QFrame( this );
    tmpQFrame->setFrameStyle( QFrame::HLine | QFrame::Sunken );
    tmpQFrame->setMinimumHeight(SPACING/2);
    top->addWidget(tmpQFrame);
    
    /* the middle stretch... */
//    top->addStretch(1);

    /* now the Graphics */
    hbox = new QHBoxLayout(top,1);
    for (i=MEM_RAM_AND_HDD; i<MEM_LAST; i++) {
	hbox->addSpacing(SPACING);
	vbox = new QVBoxLayout(hbox);
	
	switch (i) {
	    case MEM_RAM_AND_HDD:title= i18n("Total memory");		break;
	    case MEM_RAM:	title = i18n("Physical memory");	break;
	    case MEM_HDD:	title = i18n("Virtual memory");		break;
	    default:		title = "";				break;
	};
	Widget = new QLabel(title, this);
	Widget->setAlignment(AlignCenter);
	vbox->addWidget(Widget);
    	vbox->addSpacing(SPACING/2);

	QWidget *g = new QWidget(this);
	g->setMinimumWidth(2*SPACING);
	g->setMinimumHeight(3*SPACING);
	g->setBackgroundMode(NoBackground);
	Graph[i] = g;
	vbox->addWidget(g,2);
    	vbox->addSpacing(SPACING/2);

        Widget = new QLabel(this);  /* xx MB used. */
	Widget->setAlignment(AlignCenter);
	GraphLabel[i] = Widget;
	vbox->addWidget(Widget);
    }
    hbox->addSpacing(SPACING);


    /* the bottom stretch... */
//    top->addStretch(1);

    timer = new QTimer(this);
    timer->start(100);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(update_Values()));
    
    update();
}

KMemoryWidget::~KMemoryWidget()
{
    /* stop the timer */
    timer->stop();
}


/* Graphical Memory Display */    
bool KMemoryWidget::Display_Graph( 
	    int widgetindex, bool highlight,
	    t_memsize total, t_memsize avail )
{
    QWidget	*graph = Graph[widgetindex];
    QPainter	paint(graph);
    QPen	pen(QColor(0,0,0));
    
    if (total == 0 || total < avail ||
	total == NO_MEMORY_INFO ||
	avail == NO_MEMORY_INFO) {
	paint.fillRect(1,1,graph->width()-2,graph->height()-2,
			QBrush(QColor(128,128,128)));
	paint.setPen(pen);
	paint.drawRect(graph->rect());
	GraphLabel[widgetindex]->setText( Not_Available_Text );
	return false;
    }
    
    int percent     = (int) ((((double)avail) * 100) / total);
    int localheight = ((graph->height()-2) * percent) / 100;
    int color	    = 250 - (highlight ? 0 : 30);
    
    /* available mem in green */
    paint.fillRect(1,1,graph->width()-2,localheight,
			QBrush(QColor(0,color,0)));
    /* used mem in red */
    paint.fillRect(1,localheight+1,
			graph->width()-2,graph->height()-2,
			QBrush(QColor(color,0,0)));
    paint.setPen(pen);
    paint.drawRect(graph->rect());
    /* print percentage into box */
    int dy, boxheight = (graph->height()-localheight);
    if (boxheight < SPACING) dy = SPACING; else dy = 0;
    paint.drawText( 0, graph->height(),
		    graph->width(),
		    -(boxheight+dy),
		    AlignCenter,
		    QString("%1%").arg(100-percent) );
    GraphLabel[widgetindex]->setText( 
	i18n("%1 used").arg(format_MB(total-avail)) );
    return true;
}

/* update_Values() is the main-loop for updating the Memory-Information */
void KMemoryWidget::update_Values()
{
    int i;
    bool ok1,ok2;
    QLabel *label;

    update();	/* get the Information from memory_linux, memory_fbsd */

    /* now update the byte-strings */
    for (i=TOTAL_MEM; i<MEM_LAST_ENTRY; i++) {
	label = MemSizeLabel[i][0];
        if (Memory_Info[i] == NO_MEMORY_INFO)
	    label->clear();
	else
	    label->setText(
		i18n("%1 bytes =").arg(KGlobal::locale()->formatNumber(Memory_Info[i], 0)));
    }

    /* now update the MB-strings */
    for (i=TOTAL_MEM; i<MEM_LAST_ENTRY; i++) {
	label = MemSizeLabel[i][1];
	label->setText( (Memory_Info[i] != NO_MEMORY_INFO)
		    ? format_MB(Memory_Info[i])
		    : Not_Available_Text );
    }

    /* display graphical output (ram, hdd, at last: HDD+RAM) */
    /* be careful ! Maybe we have not all info available ! */
    ok1 = Display_Graph( MEM_RAM, false, Memory_Info[TOTAL_MEM], Memory_Info[FREE_MEM] );
    ok2 = Display_Graph( MEM_HDD, false, Memory_Info[SWAP_MEM],  Memory_Info[FREESWAP_MEM] );
    if (!ok2)
	ok2 = ( Memory_Info[SWAP_MEM] != NO_MEMORY_INFO &&
		Memory_Info[FREESWAP_MEM] != NO_MEMORY_INFO);
    Display_Graph( MEM_RAM_AND_HDD, true,
	(ok1 && ok2)? Memory_Info[TOTAL_MEM]+ Memory_Info[SWAP_MEM]
		    : NO_MEMORY_INFO,
		Memory_Info[FREE_MEM] + Memory_Info[FREESWAP_MEM] );
}


/* Include system-specific code */

#ifdef __linux__
#include "memory_linux.cpp"
#elif sgi
#include "memory_sgi.cpp"
#elif defined(__svr4__) && defined(sun)
#include "memory_solaris.cpp"
#elif __FreeBSD__
#include "memory_fbsd.cpp"
#elif hpux
#include "memory_hpux.cpp"
#elif __NetBSD__
#include "memory_netbsd.cpp"
#elif __osf__
#include "memory_tru64.cpp"
#else

/* Default for unsupported systems */
void KMemoryWidget::update()
{
    Memory_Info[TOTAL_MEM]    = NO_MEMORY_INFO; // total physical memory (without swaps)
    Memory_Info[FREE_MEM]     = NO_MEMORY_INFO;	// total free physical memory (without swaps)
#if !define(__svr4__) || !defined(sun)
#ifndef __NetBSD__
    Memory_Info[SHARED_MEM]   = NO_MEMORY_INFO; 
    Memory_Info[BUFFER_MEM]   = NO_MEMORY_INFO; 
#else
    Memory_Info[ACTIVE_MEM]   = NO_MEMORY_INFO; 
    Memory_Info[INACTIVE_MEM] = NO_MEMORY_INFO; 
#endif
#endif
    Memory_Info[SWAP_MEM]     = NO_MEMORY_INFO; // total size of all swap-partitions
    Memory_Info[FREESWAP_MEM] = NO_MEMORY_INFO; // free memory in swap-partitions
}

#endif
#include "memory.moc"
