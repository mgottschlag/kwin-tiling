/* -------------------------------------------------------------

   toplevel.h (part of Klipper - Cut & paste history for KDE)

   (C) by Andrew Stanley-Jones

   Generated with the KDE Application Generator

 ------------------------------------------------------------- */


#ifndef _TOPLEVEL_H_
#define _TOPLEVEL_H_

#include <kapplication.h>
#include <kglobalaccel.h>
#include <kpopupmenu.h>
#include <ksystemtray.h>
#include <qmap.h>
#include <qtimer.h>
#include <qpixmap.h>
#include <dcopobject.h>

class QClipboard;
class KToggleAction;
class URLGrabber;

class TopLevel : public KSystemTray, public DCOPObject
{
  Q_OBJECT
  K_DCOP

k_dcop:
    void quitProcess(); // not ASYNC
    int newInstance();
    QString getClipboardContents();
    void setClipboardContents(QString s);
    void clearClipboardContents();

public:
    TopLevel( QWidget *parent = 0L, bool applet = false );
    ~TopLevel();

    virtual void adjustSize();

    KGlobalAccel *globalKeys;

public slots:
    void saveSession();


protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    // we don't use any of KSystemTray's features, beside the X11 focus/event
    // handling (without, our tooltip will never show), so we just skip
    // KSystemTray's mouseReleaseEvent handler.
    void mouseReleaseEvent(QMouseEvent *e) { QWidget::mouseReleaseEvent( e ); }
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

protected slots:
    void slotPopupMenu() { showPopupMenu( m_popup ); }
    void showPopupMenu( QPopupMenu * );
    void slotRepeatAction();
    void setURLGrabberEnabled( bool );
    void toggleURLGrabber();

private slots:
    void newClipData();
    void clickedMenu(int);
    void slotConfigure();
    void slotClearClipboard();

    void slotMoveSelectedToTop();

    void slotSelectionChanged() {
        clipboardSignalArrived( true );
    }
    void slotClipboardChanged() {
        clipboardSignalArrived( false );
    }


private:
    enum SelectionMode { Clipboard = 1, Selection = 2 };

    QClipboard *clip;

    QString m_lastString;
    QString m_lastClipboard, m_lastSelection;
    KPopupMenu *m_popup;
    KToggleAction *toggleURLGrabAction;
    QMap<long,QString> m_clipDict;
    QTimer *m_checkTimer;
    QPixmap m_pixmap;
    bool bPopupAtMouse, bClipEmpty, bKeepContents, bURLGrabber;
    bool bReplayActionInHistory, bUseGUIRegExpEditor;
    bool bNoNullClipboard;
    QString QSempty;
    URLGrabber *myURLGrabber;
    int m_selectedItem;
    int maxClipItems;
    int URLGrabItem;
    bool isApplet() const { return m_config != kapp->config(); }
    KConfig* m_config;
    DCOPClient* m_dcop;
    bool bTearOffHandle;

    void trimClipHistory(int);
};

#endif
