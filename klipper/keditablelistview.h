/* -------------------------------------------------------------

   keditablelistview.h (part of Klipper - Cut & paste history for KDE)

   $Id$

   (C) 2000 by Carsten Pfeiffer <pfeiffer@kde.org>

   Licensed under the Artistic License

 ------------------------------------------------------------- */

#ifndef KEDITABLELISTVIEW_H
#define KEDITABLELISTVIEW_H

#include <qlineedit.h>
#include <qlistview.h>
#include <qptrdict.h>

class RectLineEdit;

/**
 * Implements a QListView with editable items. An item can have multiple
 * columns -- you can set any column editable.
 *
 * When the user clicks on a highlighted item, a lineedit will pop up over the
 * item, containing the item's text.
 * Pressing Escape or clicking somewhere in the listview will make the lineedit
 * go away, leaving the item's text as it was before.
 *
 * Pressing Return or Enter in the lineedit will change the item's text to the
 * lineedit's content and destroy the lineedit. The signal @ref itemChanged
 * will be emitted, then.
 *
 * Note: by default, all QListViewItems inserted into this listview are
 * editable in all columns. Use @ref setEditable to change editability.
 *
 * @short a QListView with in-place-editing capabilities
 * @author Carsten Pfeiffer <pfeiffer@kde.org>
 */

class KEditableListView : public QListView
{
    Q_OBJECT

public:
    /**
     * The usual constructor, nothing special here.
     */
    KEditableListView( QWidget *parent = 0L, const char *name = 0L );

    /**
     * The usual destructor, nothing special here.
     */
    ~KEditableListView();

    /**
     * Makes the item in column col editable / not editable
     * By default, all items/all columns are editable (otherwise you wouldn't
     * use this class, would you? :o)
     */
    void setEditable( QListViewItem *item, int col, bool enable );

protected:
    virtual void keyPressEvent( QKeyEvent * );

protected slots:
    void slotItemClicked( int button, QListViewItem *, const QPoint&, int );
    void slotDestroyEdit();
    void slotReturnPressed();


private:
    // a list of columns per item that are NOT editable
    typedef QValueList<int> ColumnList;
    QPtrDict<ColumnList> myUnEditableDict;

    RectLineEdit 	*myEdit;
    QListViewItem 	*myCurrentItem;
    int 	  	myCurrentCol;
    bool 		myCurrentItemIsOpen;


signals:
    /**
     * Emitted, when an item was edited and changed. Parameters are the
     * modified item and the respective column.
     */
    void itemChanged( QListViewItem *, int );

};


// a lineEdit that paints a rectangle instead of the standard frame
// used for in-place editing

class RectLineEdit : public QLineEdit
{
public:
    RectLineEdit( QWidget *parent=0, const char *name=0 )
	: QLineEdit( parent, name ) {
	setFrame( false );
    }

    RectLineEdit( const QString& text, QWidget *parent=0, const char *name=0 )
	: QLineEdit( text, parent, name ) {
	setFrame( false );
    }


protected:
    virtual void paintEvent( QPaintEvent *e ) {
	QLineEdit::paintEvent( e );

	if ( !frame() ) {
	    QPainter p( this );
	    p.setClipRegion( e->region() );
	    p.drawRect( rect() );
	}
    }
};


#endif // KEDITABLELISTVIEW_H
