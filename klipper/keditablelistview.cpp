/* -------------------------------------------------------------

   keditablelistview.cpp (part of Klipper - Cut & paste history for KDE)

   $Id$

   (C) 2000 by Carsten Pfeiffer <pfeiffer@kde.org>

   Licensed under the Artistic License

 ------------------------------------------------------------- */

#include <qlineedit.h>
#include <qpixmap.h>
#include <qvaluelist.h>

#include "keditablelistview.h"


KEditableListView::KEditableListView( QWidget *parent, const char *name )
    : QListView( parent, name )
{
    myEdit = 0L;
    myCurrentItem = 0L;
    myUnEditableDict.setAutoDelete( true );

    connect( this, SIGNAL( selectionChanged() ), SLOT( slotDestroyEdit() ));
    connect( this, SIGNAL( mouseButtonClicked( int, QListViewItem *, 
					       const QPoint&, int ) ),
	     SLOT( slotItemClicked(int, QListViewItem *, const QPoint&, int)));
}

KEditableListView::~KEditableListView()
{
    delete myEdit;
    myUnEditableDict.clear();
}


void KEditableListView::setEditable( QListViewItem *item, int col, bool enable)
{
    ASSERT( item != 0L );

    ColumnList *list = myUnEditableDict.find( item );
    if ( list && enable )
	list->remove( col );
    else if ( list && !enable && list->find( col ) == list->end() )
	list->append( col );
    else if ( !list && !enable ) {
	list = new ColumnList;
	list->append( col );
	myUnEditableDict.insert( item, list );
    }
}


void KEditableListView::slotItemClicked( int button, QListViewItem *item, 
					 const QPoint&, int col )
{
    QRect r = itemRect( item );
    if ( !r.isValid() ) {
	slotDestroyEdit();
	return;
    }

    bool wasEditing = false;
    if ( myEdit ) {
	slotDestroyEdit();
	wasEditing = true;
    }

    else { // clicked at the plus-sign? -> don't offer an edit-field
	if ( myCurrentItemIsOpen != item->isOpen() ) {
	    myCurrentItemIsOpen = !myCurrentItemIsOpen;
	    return;
	}
    }

    if ( myCurrentItem != item ) {
	myCurrentItem = item;
	myCurrentCol = col;
	return;
    }

    if ( myCurrentCol != col && wasEditing ) {
	myCurrentCol = col;
	setSelected( item, true );
	return;
    }

    myCurrentItem = item;
    myCurrentCol = col;

    // setSelected( item, false );

    if ( button != LeftButton )
	return;
    
    // is the item editable?
    ColumnList *eList = myUnEditableDict.find( item );
    if ( eList && eList->find( col ) != eList->end() )
	return;
	

    myEdit = new RectLineEdit( viewport() );
    connect( myEdit, SIGNAL( returnPressed() ), SLOT( slotReturnPressed() ));
    myEdit->setText( item->text( col ) );
    myEdit->adjustSize();
    int tw = myEdit->fontMetrics().boundingRect( myEdit->text() ).width() + 15;
    if ( item->pixmap( col ) ) // add width of pixmap
    	tw += item->pixmap( col )->width();

    if ( myEdit->width() < tw )
	myEdit->resize( tw, myEdit->height() );

    int x = r.x() + itemMargin() -1;
    for ( int i = 0; i < col; i++ ) {
	x += columnWidth( i );
    }
    if ( col == 0 ) {
	int d = item->depth() + (rootIsDecorated() ? 1 : 0);
	x += (d * treeStepSize());
    }

    int y = r.y() -1;
    if ( y < 0 )
	y = 0;
    myEdit->move( x, y );

    myEdit->show();
    myEdit->setFocus();
}


void KEditableListView::slotReturnPressed()
{
    myCurrentItem->setText( myCurrentCol, myEdit->text() );
    slotDestroyEdit();
    emit itemChanged( myCurrentItem, myCurrentCol );
}


void KEditableListView::slotDestroyEdit()
{
    delete myEdit;
    myEdit = 0L;

    myCurrentItemIsOpen = currentItem()->isOpen();
}


void KEditableListView::keyPressEvent( QKeyEvent *e )
{
    if ( myEdit && myEdit->hasFocus() && e->key() == Key_Escape ) {
	slotDestroyEdit();
	e->accept();
    }
}


#include "keditablelistview.moc"
