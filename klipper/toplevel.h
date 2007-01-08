// -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 8; -*-
/* This file is part of the KDE project
   Copyright (C) by Andrew Stanley-Jones
   Copyright (C) 2004  Esben Mose Hansen <kde@mosehansen.dk>

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
#ifndef _TOPLEVEL_H_
#define _TOPLEVEL_H_

#include <kapplication.h>
#include <kglobalaccel.h>
#include <kmenu.h>
#include <QMap>
#include <QPixmap>
//Added by qt3to4:
#include <QMouseEvent>
#include <QPaintEvent>
#include <QTimer>
#include <kglobal.h>

class QClipboard;
class KToggleAction;
class KAboutData;
class URLGrabber;
class ClipboardPoll;
class QTime;
class History;
class QAction;
class QMimeSource;
class HistoryItem;
class KlipperSessionManager;

class KlipperWidget : public QWidget
{
  Q_OBJECT
  Q_CLASSINFO("D-Bus Interface", "org.kde.klipper.klipper")

public Q_SLOTS:
  Q_SCRIPTABLE QString getClipboardContents();
  Q_SCRIPTABLE void setClipboardContents(QString s);
  Q_SCRIPTABLE void clearClipboardContents();
  Q_SCRIPTABLE void clearClipboardHistory();
  Q_SCRIPTABLE QStringList getClipboardHistoryMenu();
  Q_SCRIPTABLE QString getClipboardHistoryItem(int i);

public:
    KlipperWidget( QWidget *parent, KConfig* config );
    ~KlipperWidget();

    virtual void adjustSize();
    KGlobalAccel *globalKeys;

    /**
     * Get clipboard history (the "document")
     */
    History* history() { return m_history; }

    static void updateTimestamp();
    static void createAboutData();
    static void destroyAboutData();
    static KAboutData* aboutData();

public Q_SLOTS:
    void saveSession();
    void slotSettingsChanged( int category );
    void slotHistoryTopChanged();
    void slotConfigure();

protected:
    /**
     * The selection modes
     *
     * Don't use 1, as I use that as a guard against passing
     * a boolean true as a mode.
     */
    enum SelectionMode { Clipboard = 2, Selection = 4 };

    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void readProperties(KConfig *);
    void readConfiguration(KConfig *);

    /**
     * Loads history from disk.
     */
    bool loadHistory();

    /**
     * Save history to disk
     */
    void saveHistory();

    void writeConfiguration(KConfig *);
    /**
     * @returns the contents of the selection or, if empty, the contents of
     * the clipboard.
     */
    QString clipboardContents( bool *isSelection = 0L );

    void removeFromHistory( const QString& text );
    void setEmptyClipboard();

    void clipboardSignalArrived( bool selectionMode );

    /**
     * Check data in clipboard, and if it passes these checks,
     * store the data in the clipboard history.
     */
    void checkClipData( bool selectionMode );

    /**
     * Enter clipboard data in the history.
     */
    void applyClipChanges( const QMimeSource& data );

    void setClipboard( const HistoryItem& item, int mode );
    bool ignoreClipboardChanges() const;

    KConfig* config() const { return m_config; }
    bool isApplet() const { return m_config != KGlobal::config(); }

protected Q_SLOTS:
    void slotPopupMenu();
    void showPopupMenu( QMenu * );
    void slotRepeatAction();
    void setURLGrabberEnabled( bool );
    void toggleURLGrabber();
    void disableURLGrabber();

private Q_SLOTS:
    void newClipData( bool selectionMode );
    void slotClearClipboard();

    void slotSelectionChanged() {
        clipboardSignalArrived( true );
    }
    void slotClipboardChanged() {
        clipboardSignalArrived( false );
    }

    void slotQuit();
    void slotStartHideTimer();
    void slotStartShowTimer();

    void slotClearOverflow();
    void slotCheckPending();

private:

    QClipboard *clip;

    QTime *hideTimer;
    QTime *showTimer;

    QMimeSource* m_lastClipdata;
    int m_lastClipboard;
    int m_lastSelection;
    History* m_history;
    int m_overflowCounter;
    KToggleAction *toggleURLGrabAction;
    QAction* clearHistoryAction;
    QAction* configureAction;
    QAction* quitAction;
    QPixmap m_pixmap;
    bool bPopupAtMouse :1;
    bool bKeepContents :1;
    bool bURLGrabber   :1;
    bool bReplayActionInHistory :1;
    bool bUseGUIRegExpEditor    :1;
    bool bNoNullClipboard       :1;
    bool bTearOffHandle         :1;
    bool bIgnoreSelection       :1;
    bool bSynchronize           :1;
    bool bSelectionTextOnly     :1;
    bool bIgnoreImages          :1;

    /**
     * Avoid reacting to our own changes, using this
     * lock.
     * Don't manupulate this object directly... use the Ignore struct
     * instead
     */
    int locklevel;

    URLGrabber *myURLGrabber;
    QString m_lastURLGrabberTextSelection;
    QString m_lastURLGrabberTextClipboard;
    KConfig* m_config;
    QTimer m_overflowClearTimer;
    QTimer m_pendingCheckTimer;
    bool m_pendingContentsCheck;
    ClipboardPoll* poll;
    static KAboutData* about_data;

    bool blockFetchingNewData();
    KlipperSessionManager* session_managed;
};

class Klipper : public KlipperWidget
{
    Q_OBJECT

public Q_SLOTS:
    Q_SCRIPTABLE int newInstance();
    Q_SCRIPTABLE void quitProcess();

public:
    Klipper( QWidget* parent = NULL );
};

#endif
