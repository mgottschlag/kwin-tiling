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
#include <QRadioButton>
#include <QTreeWidget>

#include <keditlistbox.h>
#include <knuminput.h>
#include <KConfigDialog>

#include "urlgrabber.h"

class KConfigSkeleton;
class KShortcutsEditor;
class QPushButton;
class QDialog;
class ConfigDialog;

class GeneralWidget : public QWidget
{
    Q_OBJECT

    friend class ConfigDialog;

public:
    GeneralWidget( QWidget *parent );
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
class AdvancedWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AdvancedWidget( QWidget *parent = 0L );
    ~AdvancedWidget();

    void setWMClasses( const QStringList& items );
    QStringList wmClasses() const { return editListBox->items(); }

private:
    KEditListBox *editListBox;
};

class ActionWidget : public QWidget
{
    Q_OBJECT

    friend class ConfigDialog;

public:
    ActionWidget( const ActionList *list, QWidget *parent );
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
    void slotItemChanged(QTreeWidgetItem *item, int column);
    void slotAdvanced();
    void slotContextMenu(const QPoint& pos);
    void selectionChanged();

private:
    QTreeWidget *treeWidget;
    QStringList m_wmClasses;
    AdvancedWidget *advancedWidget;
    QPushButton *delActionButton;
    QCheckBox *cbUseGUIRegExpEditor;
};

class ConfigDialog : public KConfigDialog
{
    Q_OBJECT

public:
    ConfigDialog( QWidget *parent, KConfigSkeleton *config, const ActionList *list, KActionCollection *collection, bool isApplet );
    ~ConfigDialog();

    ActionList * actionList() const { return m_actionWidget->actionList(); }

    bool keepContents()    const {
	return m_generalWidget->cbSaveContents->isChecked();
    }
    bool popupAtMousePos() const {
	return m_generalWidget->cbMousePos->isChecked();
    }
    bool trimmed() const {
        return m_generalWidget->cbStripWhitespace->isChecked();
    }
    bool replayActionInHistory() const {
	return m_generalWidget->cbReplayAIH->isChecked();
    }
    bool noNullClipboard() const {
        return m_generalWidget->cbNoNull->isChecked();
    }

    int popupTimeout() const {
	return m_generalWidget->popupTimeout->value();
    }
    int maxItems() const {
	return m_generalWidget->maxItems->value();
    }
    bool ignoreSelection() const
    {
        return m_generalWidget->cbIgnoreSelection->isChecked();
    }
    QStringList noActionsFor() const {
	return m_actionWidget->wmClasses();
    }
    bool useGUIRegExpEditor() const
    {
      return m_actionWidget->cbUseGUIRegExpEditor->isChecked();
    }

    bool synchronize() const {
        return m_generalWidget->cbSynchronize->isChecked();
    }
    bool implicitSelection() const {
        return m_generalWidget->cbImplicitSelection->isChecked();
    }

    void setKeepContents( bool enable ) {
	m_generalWidget->cbSaveContents->setChecked( enable );
    }
    void setPopupAtMousePos( bool enable ) {
	m_generalWidget->cbMousePos->setChecked( enable );
    }
    void setStripWhiteSpace( bool enable ) {
        m_generalWidget->cbStripWhitespace->setChecked( enable );
    }
    void setReplayActionInHistory( bool enable ) {
	m_generalWidget->cbReplayAIH->setChecked( enable );
    }
    void setNoNullClipboard( bool enable ) {
        m_generalWidget->cbNoNull->setChecked( enable );
    }
    void setPopupTimeout( int timeout ) {
	m_generalWidget->popupTimeout->setValue( timeout );
    }
    void setMaxItems( int items ) {
	m_generalWidget->maxItems->setValue( items );
    }
    void setIgnoreSelection( bool ignore ) {
        m_generalWidget->cbIgnoreSelection->setChecked( ignore );
    }
    void setSynchronize( bool synchronize ) {
        m_generalWidget->cbSynchronize->setChecked( synchronize );
    }
    void setNoActionsFor( const QStringList& items ) {
	m_actionWidget->setWMClasses( items );
    }
    void setUseGUIRegExpEditor( bool enabled )
    {
	// the checkbox is only hidden explicitly when there's no
	// regexp editor component available.
	if ( !m_actionWidget->cbUseGUIRegExpEditor->isHidden() )
            m_actionWidget->cbUseGUIRegExpEditor->setChecked( enabled );
    }

    void commitShortcuts();

private:
    GeneralWidget *m_generalWidget;
    ActionWidget *m_actionWidget;
    KShortcutsEditor *m_shortcutsWidget;

};

#endif // CONFIGDIALOG_H
