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
#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QCheckBox>
#include <QEvent>
#include <q3header.h>
#include <QRadioButton>

#include <kdialogbase.h>
#include <keditlistbox.h>
#include <kkeydialog.h>
#include <k3listview.h>
#include <knuminput.h>
#include <kvbox.h>

#include "urlgrabber.h"

class KGlobalAccel;
class KKeyChooser;
class K3ListView;
class QPushButton;
class QDialog;
class ConfigDialog;

class GeneralWidget : public KVBox
{
    Q_OBJECT

    friend class ConfigDialog;

public:
    GeneralWidget( QWidget *parent, const char *name );
    ~GeneralWidget();

private Q_SLOTS:
    void historySizeChanged( int value );
    void slotClipConfigChanged();

private:
    QCheckBox *cbMousePos, *cbSaveContents, *cbReplayAIH, *cbNoNull;
    QCheckBox *cbIgnoreSelection, *cbStripWhitespace;
    QRadioButton *cbSynchronize, *cbImplicitSelection, *cbSeparate;
    KIntNumInput *popupTimeout, *maxItems;

};


// only for use inside ActionWidget
class AdvancedWidget : public KVBox
{
    Q_OBJECT

public:
    AdvancedWidget( QWidget *parent = 0L, const char *name = 0L );
    ~AdvancedWidget();

    void setWMClasses( const QStringList& items );
    QStringList wmClasses() const { return editListBox->items(); }

private:
    KEditListBox *editListBox;
};

class ActionWidget : public KVBox
{
    Q_OBJECT

    friend class ConfigDialog;

public:
    ActionWidget( const ActionList *list, ConfigDialog* configWidget, QWidget *parent, const char *name );
    ~ActionWidget();

    /**
     * Creates a list of actions from the listView and returns a pointer to
     * the list.
     * Make sure to free that pointer when you don't need it anymore.
     */
    ActionList * actionList();

    void setWMClasses( const QStringList& items ) { m_wmClasses = items; }
    QStringList wmClasses() const                 { return m_wmClasses; }

private Q_SLOTS:
    void slotAddAction();
    void slotDeleteAction();
    void slotItemChanged( Q3ListViewItem *, const QPoint& , int );
    void slotAdvanced();
    void slotContextMenu( K3ListView *, Q3ListViewItem *, const QPoint& );
    void selectionChanged ( Q3ListViewItem *);

private:
    K3ListView *listView;
    QStringList m_wmClasses;
    AdvancedWidget *advancedWidget;
    QPushButton *delActionButton;
    QCheckBox *cbUseGUIRegExpEditor;
};

/*class KeysWidget : public QVBox
{
    Q_OBJECT

    friend class ConfigDialog;

public:
    KeysWidget( KAccelActions &keyMap, QWidget *parent, const char *name );
    ~KeysWidget();

private:
    KKeyChooser *keyChooser;
};*/


class ConfigDialog : public KDialogBase
{
    Q_OBJECT

public:
    ConfigDialog( const ActionList *list, KGlobalAccel *accel, bool isApplet );
    ~ConfigDialog();

    ActionList * actionList() const { return actionWidget->actionList(); }

    bool keepContents()    const {
	return generalWidget->cbSaveContents->isChecked();
    }
    bool popupAtMousePos() const {
	return generalWidget->cbMousePos->isChecked();
    }
    bool trimmed() const {
        return generalWidget->cbStripWhitespace->isChecked();
    }
    bool replayActionInHistory() const {
	return generalWidget->cbReplayAIH->isChecked();
    }
    bool noNullClipboard() const {
        return generalWidget->cbNoNull->isChecked();
    }

    int popupTimeout() const {
	return generalWidget->popupTimeout->value();
    }
    int maxItems() const {
	return generalWidget->maxItems->value();
    }
    bool ignoreSelection() const
    {
        return generalWidget->cbIgnoreSelection->isChecked();
    }
    QStringList noActionsFor() const {
	return actionWidget->wmClasses();
    }
    bool useGUIRegExpEditor() const
    {
      return actionWidget->cbUseGUIRegExpEditor->isChecked();
    }

    bool synchronize() const {
        return generalWidget->cbSynchronize->isChecked();
    }
    bool implicitSelection() const {
        return generalWidget->cbImplicitSelection->isChecked();
    }

    void setKeepContents( bool enable ) {
	generalWidget->cbSaveContents->setChecked( enable );
    }
    void setPopupAtMousePos( bool enable ) {
	generalWidget->cbMousePos->setChecked( enable );
    }
    void setStripWhiteSpace( bool enable ) {
        generalWidget->cbStripWhitespace->setChecked( enable );
    }
    void setReplayActionInHistory( bool enable ) {
	generalWidget->cbReplayAIH->setChecked( enable );
    }
    void setNoNullClipboard( bool enable ) {
        generalWidget->cbNoNull->setChecked( enable );
    }
    void setPopupTimeout( int timeout ) {
	generalWidget->popupTimeout->setValue( timeout );
    }
    void setMaxItems( int items ) {
	generalWidget->maxItems->setValue( items );
    }
    void setIgnoreSelection( bool ignore ) {
        generalWidget->cbIgnoreSelection->setChecked( ignore );
    }
    void setSynchronize( bool synchronize ) {
        generalWidget->cbSynchronize->setChecked( synchronize );
    }
    void setNoActionsFor( const QStringList& items ) {
	actionWidget->setWMClasses( items );
    }
    void setUseGUIRegExpEditor( bool enabled )
    {
	// the checkbox is only hidden explicitly when there's no
	// regexp editor component available.
	if ( !actionWidget->cbUseGUIRegExpEditor->isHidden() )
            actionWidget->cbUseGUIRegExpEditor->setChecked( enabled );
    }

    virtual void show();
    void commitShortcuts();

private:
    GeneralWidget *generalWidget;
    ActionWidget *actionWidget;
    KKeyChooser *keysWidget;

};

class ListView : public K3ListView
{
public:
    ListView( ConfigDialog* configWidget, QWidget *parent )
	: K3ListView( parent ), _configWidget( configWidget ),
          _regExpEditor(0L) {}
    // QListView has a weird idea of a sizeHint...
    virtual QSize sizeHint () const {
	int w = minimumSizeHint().width();
	int h = header()->height();
	h += viewport()->sizeHint().height();
	h += horizontalScrollBar()->height();
	
	Q3ListViewItem *item = firstChild();
	while ( item ) {
	    h += item->totalHeight();
	    item = item->nextSibling();
	}

	return QSize( w, h );
    }

protected:
    virtual void rename( Q3ListViewItem* item, int c );
private:
    ConfigDialog* _configWidget;
    QDialog* _regExpEditor;
};

#endif // CONFIGDIALOG_H
