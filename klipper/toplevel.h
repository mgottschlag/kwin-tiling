/* -------------------------------------------------------------

   toplevel.h (part of Klipper - Cut & paste history for KDE)

   (C) by Andrew Stanley-Jones

   Generated with the KDE Application Generator

 ------------------------------------------------------------- */


#ifndef _TOPLEVEL_H_
#define _TOPLEVEL_H_

#include <kapp.h>
#include <kglobalaccel.h>
#include <kmainwindow.h>
#include <kpopupmenu.h>
#include <qintdict.h>
#include <qtimer.h>
#include <qpixmap.h>

class QClipboard;
class KToggleAction;
class URLGrabber;

class TopLevel : public KMainWindow
{
  Q_OBJECT

public:
    TopLevel();
    ~TopLevel();

    KGlobalAccel *globalKeys;

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
    QString clipboardContents();

    void removeFromHistory( const QString& text );
    void setEmptyClipboard();

protected slots:
    void slotPopupMenu() { showPopupMenu( m_popup ); }
    void showPopupMenu( QPopupMenu * );
    void saveProperties();
    void slotRepeatAction();
    void setURLGrabberEnabled( bool );
    void toggleURLGrabber();

private slots:
    void newClipData();
    void clickedMenu(int);
    void slotConfigure();
    void slotClearClipboard();
    void slotClipboardChanged( const QString& newContents );

    void slotMoveSelectedToTop();
    
private:
    QClipboard *clip;

    QString m_lastString;
    QString m_lastClipboard, m_lastSelection;
    KPopupMenu *m_popup;
    KToggleAction *toggleURLGrabAction;
    QIntDict<QString> *m_clipDict;
    QTimer *m_checkTimer;
    QPixmap *m_pixmap;
    bool bPopupAtMouse, bClipEmpty, bKeepContents, bURLGrabber, bReplayActionInHistory, bUseGUIRegExpEditor;
    QString QSempty;
    URLGrabber *myURLGrabber;
    int m_selectedItem;
    int maxClipItems;
    int URLGrabItem;

    void trimClipHistory(int);
};

#endif
