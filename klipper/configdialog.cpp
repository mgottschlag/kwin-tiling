// -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 8; -*-
/* This file is part of the KDE project
   Copyright (C) 2000 by Carsten Pfeiffer <pfeiffer@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include <qlabel.h>
#include <qlayout.h>
#include <q3listview.h>
#include <qpushbutton.h>
#include <qtooltip.h>

//Added by qt3to4:
#include <QPixmap>
#include <Q3Frame>
#include <Q3VButtonGroup>

#include <assert.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kmenu.h>
#include <kwinmodule.h>
#include <kregexpeditorinterface.h>
#include <kparts/componentfactory.h>
#include <kvbox.h>

#include "configdialog.h"

ConfigDialog::ConfigDialog( const ActionList *list, KGlobalAccel *accel,
                            bool isApplet )
    : KDialogBase( Tabbed, i18n("Configure"),
                    Ok | Cancel | Help,
                    Ok, 0L, "config dialog" )
{
    if ( isApplet )
        setHelp( QString::null, "klipper" );

    KVBox *w = 0L; // the parent for the widgets

    w = addVBoxPage( i18n("&General") );
    generalWidget = new GeneralWidget( w, "general widget" );

    w = addVBoxPage( i18n("Ac&tions") );
    actionWidget = new ActionWidget( list, this, w, "actions widget" );

    w = addVBoxPage( i18n("&Shortcuts") );
    keysWidget = new KKeyChooser( accel, w );
}


ConfigDialog::~ConfigDialog()
{
}

// prevent huge size due to long regexps in the action-widget
void ConfigDialog::show()
{
    if ( !isVisible() ) {
	KWinModule module(0, KWinModule::INFO_DESKTOP);
	QSize s1 = sizeHint();
	QSize s2 = module.workArea().size();
	int w = s1.width();
	int h = s1.height();

	if ( s1.width() >= s2.width() )
	    w = s2.width();
	if ( s1.height() >= s2.height() )
	    h = s2.height();

 	resize( w, h );
    }

    KDialogBase::show();
}

void ConfigDialog::commitShortcuts()
{
    keysWidget->commitChanges();
}

/////////////////////////////////////////
////


GeneralWidget::GeneralWidget( QWidget *parent, const char *name )
    : Q3VBox( parent, name )
{
    setSpacing(KDialog::spacingHint());

    cbMousePos = new QCheckBox( i18n("&Popup menu at mouse-cursor position"),
                                this );
    cbSaveContents = new QCheckBox( i18n("Save clipboard contents on e&xit"),
                                    this );
    cbStripWhitespace = new QCheckBox( i18n("Remove whitespace when executing actions"), this );
    cbStripWhitespace->setWhatsThis(
                     i18n("Sometimes, the selected text has some whitespace at the end, which, if loaded as URL in a browser would cause an error. Enabling this option removes any whitespace at the beginning or end of the selected string (the original clipboard contents will not be modified).") );

    cbReplayAIH = new QCheckBox( i18n("&Replay actions on an item selected from history"),
                                    this );

    cbNoNull = new QCheckBox( i18n("Pre&vent empty clipboard"), this );
    cbNoNull->setWhatsThis(
                     i18n("Selecting this option has the effect, that the "
                          "clipboard can never be emptied. E.g. when an "
                          "application exits, the clipboard would usually be "
                          "emptied.") );

    cbIgnoreSelection = new QCheckBox( i18n("&Ignore selection"), this );
    cbIgnoreSelection->setWhatsThis(
                     i18n("This option prevents the selection being recorded "
                          "in the clipboard history. Only explicit clipboard "
                          "changes are recorded.") );

    Q3VButtonGroup *group = new Q3VButtonGroup( i18n("Clipboard/Selection Behavior"), this );
    group->setExclusive( true );

    group->setWhatsThis(
     i18n("<qt>There are two different clipboard buffers available:<br><br>"
          "<b>Clipboard</b> is filled by selecting something "
          "and pressing Ctrl+C, or by clicking \"Copy\" in a toolbar or "
          "menubar.<br><br>"
          "<b>Selection</b> is available immediately after "
          "selecting some text. The only way to access the selection "
          "is to press the middle mouse button.<br><br>"
          "You can configure the relationship between Clipboard and Selection."
          "</qt>" ));

    cbSynchronize = new QRadioButton(
        i18n("Sy&nchronize contents of the clipboard and the selection"),
        group );
    cbSynchronize->setWhatsThis(
      i18n("Selecting this option synchronizes these two buffers, so they "
           "work the same way as in KDE 1.x and 2.x.") );

    cbSeparate = new QRadioButton(
        i18n("Separate clipboard and selection"), group );
    cbSeparate->setWhatsThis(
        i18n("Using this option will only set the selection when highlighting "
             "something and the clipboard when choosing e.g. \"Copy\" "
             "in a menubar.") );

    cbSeparate->setChecked( !cbSynchronize->isChecked() );

    popupTimeout = new KIntNumInput( this );
    popupTimeout->setLabel( i18n( "Tim&eout for action popups:" ) );
    popupTimeout->setRange( 0, 200, 1, true );
    popupTimeout->setSuffix( i18n( " sec" ) );
    popupTimeout->setToolTip( i18n("A value of 0 disables the timeout") );

    maxItems = new KIntNumInput( this );
    maxItems->setLabel(  i18n( "C&lipboard history size:" ) );
    maxItems->setRange( 2, 2048, 1, true );
    connect( maxItems, SIGNAL( valueChanged( int )),
             SLOT( historySizeChanged( int ) ));

    connect( group, SIGNAL( clicked( int )),
             SLOT( slotClipConfigChanged() ));
    slotClipConfigChanged();

    // Add some spacing at the end
    QWidget *dummy = new QWidget( this );
    setStretchFactor( dummy, 1 );
}

GeneralWidget::~GeneralWidget()
{
}

void GeneralWidget::historySizeChanged( int value )
{
    // Note there is no %n in this string, because value is not supposed
    // to be put into the suffix of the spinbox.
    maxItems->setSuffix( i18n( " entry", " entries", value ) );
}

void GeneralWidget::slotClipConfigChanged()
{
    cbIgnoreSelection->setEnabled( !cbSynchronize->isChecked() );
}

/////////////////////////////////////////
////

void ListView::rename( Q3ListViewItem* item, int c )
{
  bool gui = false;
  if ( item->childCount() != 0 && c == 0) {
    // This is the regular expression
    if ( _configWidget->useGUIRegExpEditor() ) {
      gui = true;
    }
  }

  if ( gui ) {
    if ( ! _regExpEditor )
      _regExpEditor = KParts::ComponentFactory::createInstanceFromQuery<QDialog>( "KRegExpEditor/KRegExpEditor", QString::null, this );
#warning "qt_cast doesn't exist anymore, but tried qobject_cast but without success"
#if 0
    KRegExpEditorInterface *iface = static_cast<KRegExpEditorInterface *>( _regExpEditor->qt_cast( "KRegExpEditorInterface" ) );
    
    assert( iface );
    iface->setRegExp( item->text( 0 ) );

    bool ok = _regExpEditor->exec();
    if ( ok )
      item->setText( 0, iface->regExp() );
#endif
  }
  else
    KListView::rename( item ,c );
}


ActionWidget::ActionWidget( const ActionList *list, ConfigDialog* configWidget, QWidget *parent,
                            const char *name )
    : Q3VBox( parent, name ),
      advancedWidget( 0L )
{
    Q_ASSERT( list != 0L );

    QLabel *lblAction = new QLabel(
	  i18n("Action &list (right click to add/remove commands):"), this );

    listView = new ListView( configWidget, this );
    lblAction->setBuddy( listView );
    listView->addColumn( i18n("Regular Expression (see http://doc.trolltech.com/qregexp.html#details)") );
    listView->addColumn( i18n("Description") );

    listView->setRenameable(0);
    listView->setRenameable(1);
    listView->setItemsRenameable( true );
    listView->setItemsMovable( false );
//     listView->setAcceptDrops( true );
//     listView->setDropVisualizer( true );
//     listView->setDragEnabled( true );

    listView->setRootIsDecorated( true );
    listView->setMultiSelection( false );
    listView->setAllColumnsShowFocus( true );
    listView->setSelectionMode( Q3ListView::Single );
    connect( listView, SIGNAL(executed( Q3ListViewItem*, const QPoint&, int )),
             SLOT( slotItemChanged( Q3ListViewItem*, const QPoint& , int ) ));
    connect( listView, SIGNAL( selectionChanged ( Q3ListViewItem * )),
             SLOT(selectionChanged ( Q3ListViewItem * )));
    connect(listView,
            SIGNAL(contextMenu(KListView *, Q3ListViewItem *, const QPoint&)),
            SLOT( slotContextMenu(KListView*, Q3ListViewItem*, const QPoint&)));

    ClipAction *action   = 0L;
    ClipCommand *command = 0L;
    Q3ListViewItem *item  = 0L;
    Q3ListViewItem *child = 0L;
    Q3ListViewItem *after = 0L; // QListView's default inserting really sucks
    ActionListIterator it( *list );

    const QPixmap& doc = SmallIcon( "misc" );
    const QPixmap& exec = SmallIcon( "exec" );

    for ( action = it.current(); action; action = ++it ) {
        item = new Q3ListViewItem( listView, after,
                                  action->regExp(), action->description() );
        item->setPixmap( 0, doc );

        Q3PtrListIterator<ClipCommand> it2( action->commands() );
        for ( command = it2.current(); command; command = ++it2 ) {
            child = new Q3ListViewItem( item, after,
                                       command->command, command->description);
        if ( command->pixmap.isEmpty() )
            child->setPixmap( 0, exec );
        else
            child->setPixmap( 0, SmallIcon( command->pixmap ) );
            after = child;
        }
        after = item;
    }

    listView->setSorting( -1 ); // newly inserted items just append unsorted

    cbUseGUIRegExpEditor = new QCheckBox( i18n("&Use graphical editor for editing regular expressions" ), this );
    if ( KTrader::self()->query("KRegExpEditor/KRegExpEditor").isEmpty() )
    {
	cbUseGUIRegExpEditor->hide();
	cbUseGUIRegExpEditor->setChecked( false );
    }

    Q3HBox *box = new Q3HBox( this );
    box->setSpacing( KDialog::spacingHint() );
    QPushButton *button = new QPushButton( i18n("&Add Action"), box );
    connect( button, SIGNAL( clicked() ), SLOT( slotAddAction() ));

    delActionButton = new QPushButton( i18n("&Delete Action"), box );
    connect( delActionButton, SIGNAL( clicked() ), SLOT( slotDeleteAction() ));

    QLabel *label = new QLabel(i18n("Click on a highlighted item's column to change it. \"%s\" in a command will be replaced with the clipboard contents."), box);
    label->setAlignment( Qt::TextWordWrap | Qt::AlignLeft | Qt::AlignVCenter );

    box->setStretchFactor( label, 5 );

    box = new Q3HBox( this );
    QPushButton *advanced = new QPushButton( i18n("Advanced..."), box );
    advanced->setFixedSize( advanced->sizeHint() );
    connect( advanced, SIGNAL( clicked() ), SLOT( slotAdvanced() ));
    (void) new QWidget( box ); // spacer

    delActionButton->setEnabled(listView->currentItem () !=0);
}

ActionWidget::~ActionWidget()
{
}

void ActionWidget::selectionChanged ( Q3ListViewItem * item)
{
    delActionButton->setEnabled(item!=0);
}

void ActionWidget::slotContextMenu( KListView *, Q3ListViewItem *item,
                                    const QPoint& pos )
{
    if ( !item )
        return;

    KMenu *menu = new KMenu;
    QAction* addCmd = menu->addAction( i18n("Add Command") );
    QAction* rmCmd = menu->addAction( i18n("Remove Command") );
    if ( !item->parent() ) {// no "command" item
        rmCmd->setEnabled( false );
        item->setOpen( true );
    }

    QAction* executed = menu->exec( pos );
    if ( executed == addCmd ) {
        Q3ListViewItem *p = item->parent() ? item->parent() : item;
        Q3ListViewItem *cmdItem = new Q3ListViewItem( p, item,
                         i18n("Click here to set the command to be executed"),
                         i18n("<new command>") );
        cmdItem->setPixmap( 0, SmallIcon( "exec" ) );
    }
    else if ( executed == rmCmd )
        delete item;

    delete menu;
}

void ActionWidget::slotItemChanged( Q3ListViewItem *item, const QPoint&, int col )
{
    if ( !item->parent() || col != 0 )
        return;
    ClipCommand command( item->text(0), item->text(1) );
        item->setPixmap( 0, SmallIcon( command.pixmap.isEmpty() ?
                                                   "exec" : command.pixmap ) );
}

void ActionWidget::slotAddAction()
{
    Q3ListViewItem *item = new Q3ListViewItem( listView );
    item->setPixmap( 0, SmallIcon( "misc" ));
    item->setText( 0, i18n("Click here to set the regexp"));
    item->setText( 1, i18n("<new action>"));
}


void ActionWidget::slotDeleteAction()
{
    Q3ListViewItem *item = listView->currentItem();
    if ( item && item->parent() )
        item = item->parent();
    delete item;
}


ActionList * ActionWidget::actionList()
{
    Q3ListViewItem *item = listView->firstChild();
    Q3ListViewItem *child = 0L;
    ClipAction *action = 0L;
    ActionList *list = new ActionList;
    list->setAutoDelete( true );
    while ( item ) {
        action = new ClipAction( item->text( 0 ), item->text( 1 ) );
        child = item->firstChild();

        // add the commands
        while ( child ) {
            action->addCommand( child->text( 0 ), child->text( 1 ), true );
            child = child->nextSibling();
        }

        list->append( action );
        item = item->nextSibling();
    }

    return list;
}

void ActionWidget::slotAdvanced()
{
    KDialogBase dlg( 0L, "advanced dlg", true,
                     i18n("Advanced Settings"),
                     KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok );
    KVBox *box = dlg.makeVBoxMainWidget();
    AdvancedWidget *widget = new AdvancedWidget( box );
    widget->setWMClasses( m_wmClasses );

    dlg.resize( dlg.sizeHint().width(),
                dlg.sizeHint().height() +40); // or we get an ugly scrollbar :(

    if ( dlg.exec() == QDialog::Accepted ) {
        m_wmClasses = widget->wmClasses();
    }
}

AdvancedWidget::AdvancedWidget( QWidget *parent, const char *name )
    : Q3VBox( parent, name )
{
    editListBox = new KEditListBox( i18n("D&isable Actions for Windows of Type WM_CLASS"), this, "editlistbox", true, KEditListBox::Add | KEditListBox::Remove );

    editListBox->setWhatsThis(
          i18n("<qt>This lets you specify windows in which Klipper should "
	       "not invoke \"actions\". Use<br><br>"
	       "<center><b>xprop | grep WM_CLASS</b></center><br>"
	       "in a terminal to find out the WM_CLASS of a window. "
	       "Next, click on the window you want to examine. The "
	       "first string it outputs after the equal sign is the one "
	       "you need to enter here.</qt>"));

    editListBox->setFocus();
}

AdvancedWidget::~AdvancedWidget()
{
}

void AdvancedWidget::setWMClasses( const QStringList& items )
{
    editListBox->clear();
    editListBox->insertStringList( items );
}



///////////////////////////////////////////////////////
//////////

/*
KeysWidget::KeysWidget( KAccelActions &keyMap, QWidget *parent, const char *name)
    : QVBox( parent, name )
{
    keyChooser = new KKeyChooser( keyMap, this );
}

KeysWidget::~KeysWidget()
{
}
*/

#include "configdialog.moc"
