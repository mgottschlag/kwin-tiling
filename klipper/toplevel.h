/* -------------------------------------------------------------

   toplevel.h (part of Klipper - Cut & paste history for KDE)

   (C) by Andrew Stanley-Jones

   Generated with the KDE Application Generator

   Licensed under the GNU GPL Version 2

 ------------------------------------------------------------- */


#ifndef _TOPLEVEL_H_
#define _TOPLEVEL_H_

#include <kapplication.h>
#include <kglobalaccel.h>
#include <kpopupmenu.h>
#include <qmap.h>
#include <qpixmap.h>
#include <dcopobject.h>
#include <qtimer.h>

class QClipboard;
class KToggleAction;
class KAboutData;
class URLGrabber;
class ClipboardPoll;
class QTime;
class History;
class KAction;

class KlipperWidget : public QWidget, public DCOPObject
{
  Q_OBJECT
  K_DCOP

k_dcop:
    QString getClipboardContents();
    void setClipboardContents(QString s);
    void clearClipboardContents();
    void clearClipboardHistory();
    QStringList getClipboardHistoryMenu();
    QString getClipboardHistoryItem(int i);

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

public slots:
    void saveSession();
    void slotSettingsChanged( int category );
    void slotHistoryChanged();
    void slotConfigure();

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void readProperties(KConfig *);
    void readConfiguration(KConfig *);
    void writeConfiguration(KConfig *);
    /**
     * @returns the contents of the selection or, if empty, the contents of
     * the clipboard.
     */
    QString clipboardContents( bool *isSelection = 0L );

    void removeFromHistory( const QString& text );
    void setEmptyClipboard();

    void clipboardSignalArrived( bool selectionMode );
    void checkClipData( const QString& text, bool selectionMode );
    void applyClipChanges( const QString& text );

    void setClipboard( const QString& text, int mode );
    void setClipboard( const QString& text, bool selectionMode );
    bool ignoreClipboardChanges() const;

    KConfig* config() const { return m_config; }
    bool isApplet() const { return m_config != kapp->config(); }

protected slots:
    void slotPopupMenu();
    void showPopupMenu( QPopupMenu * );
    void slotRepeatAction();
    void setURLGrabberEnabled( bool );
    void toggleURLGrabber();
    void disableURLGrabber();

private slots:
    void newClipData();
    void slotClearClipboard();

    void slotSelectionChanged() {
        clipboardSignalArrived( true );
    }
    void slotClipboardChanged() {
        clipboardSignalArrived( false );
    }

    void slotQuit();
    void slotAboutToHideMenu();
    
    void slotClearOverflow();
    void slotCheckPending();

private:
    enum SelectionMode { Clipboard = 1, Selection = 2 };

    QClipboard *clip;

    QTime *menuTimer;

    QString m_lastString;
    QString m_lastClipboard, m_lastSelection;
    History* m_history;
    int m_overflowCounter;
    KToggleAction *toggleURLGrabAction;
    KAction* clearHistoryAction;
    KAction* configureAction;
    KAction* quitAction;
    QPixmap m_pixmap;
    bool bPopupAtMouse :1;
    bool bKeepContents :1;
    bool bURLGrabber   :1;
    bool bReplayActionInHistory :1;
    bool bUseGUIRegExpEditor    :1;
    bool bNoNullClipboard       :1;
    bool bTearOffHandle         :1;
    bool bIgnoreSelection       :1;
    URLGrabber *myURLGrabber;
    KConfig* m_config;
    QTimer m_overflowClearTimer;
    QTimer m_pendingCheckTimer;
    bool m_pendingContentsCheck;
    ClipboardPoll* poll;
    static KAboutData* about_data;

    bool blockFetchingNewData();
};


class Klipper : public KlipperWidget
{
    Q_OBJECT
    K_DCOP
k_dcop:
    int newInstance();
    void quitProcess(); // not ASYNC
public:
    Klipper( QWidget* parent = NULL );
};

#endif
