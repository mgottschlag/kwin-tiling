/* -------------------------------------------------------------

   toplevel.h (part of Klipper - Cut & paste history for KDE)

   (C) by Andrew Stanley-Jones

   Generated with the KDE Application Generator

   Licensed under the Artistic License

 ------------------------------------------------------------- */


#ifndef _TOPLEVEL_H_
#define _TOPLEVEL_H_

#include <kapplication.h>
#include <kglobalaccel.h>
#include <kpopupmenu.h>
#include <qmap.h>
#include <qpixmap.h>
#include <dcopobject.h>

class QClipboard;
class KToggleAction;
class URLGrabber;
class ClipboardPoll;

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
    QPopupMenu* popup() { return m_popup; }
    KGlobalAccel *globalKeys;

    static void updateXTime();

public slots:
    void saveSession();
    void slotSettingsChanged( int category );
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
    void slotPopupMenu() { showPopupMenu( m_popup ); }
    void showPopupMenu( QPopupMenu * );
    void slotRepeatAction();
    void setURLGrabberEnabled( bool );
    void toggleURLGrabber();
    void disableURLGrabber();
    void cleanAppletMenu();

private slots:
    void newClipData();
    void clickedMenu(int);
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
    QPixmap m_pixmap;
    bool bPopupAtMouse :1;
    bool bClipEmpty    :1;
    bool bKeepContents :1;
    bool bURLGrabber   :1;
    bool bReplayActionInHistory :1;
    bool bUseGUIRegExpEditor    :1;
    bool bNoNullClipboard       :1;
    bool bTearOffHandle         :1;
    bool bIgnoreSelection       :1;
    QString QSempty;
    URLGrabber *myURLGrabber;
    int m_selectedItem;
    int maxClipItems;
    int URLGrabItem;
    KConfig* m_config;
    ClipboardPoll* poll;

    void trimClipHistory(int);
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
protected:
#if QT_VERSION < 0x030200
    virtual void enterEvent( QEvent* );
#endif
};

#endif
