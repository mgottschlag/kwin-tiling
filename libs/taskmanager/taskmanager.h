/*****************************************************************

Copyright (c) 2000 Matthias Elter <elter@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#ifndef __taskmanager_h__
#define __taskmanager_h__

#include <sys/types.h>

#include <qpoint.h>
#include <qobject.h>
#include <qvaluelist.h>
#include <qlist.h>
#include <qpixmap.h>

#include <dcopobject.h>
#include <kwin.h>

class Task: public QObject
{
    Q_OBJECT

public:
    Task( WId win, QObject * parent, const char *name = 0 );
    virtual ~Task();

    WId window() const { return _win; }
    QString name() { return _info.name; }
    QString visibleName() { return _info.visibleName; }
    QString visibleNameWithState() { return _info.visibleNameWithState(); }
    QString iconName();
    QString visibleIconName();
    QString className();

    QValueList<WId> transients() { return _transients; }

    /**
     * Returns a 16x16 (KIcon::Small) icon for the task. This method will
     * only fall back to a static icon if there is no icon of any size in
     * the WM hints.
     */
    QPixmap pixmap() { return _pixmap; }

    /**
     * Returns the best icon for any of the KIcon::StdSizes. If there is no
     * icon of the specified size specified in the WM hints, it will try to
     * get one using KIconLoader.
     *
     * <pre>
     *   bool gotStaticIcon;
     *   QPixmap icon = myTask->icon( KIcon::SizeMedium, gotStaticIcon );
     * </pre>
     *
     * @param size Any of the constants in KIcon::StdSizes.
     * @param isStaticIcon Set to true if KIconLoader was used, false otherwise.
     */
    QPixmap bestIcon( int size, bool &isStaticIcon );

    /**
     * Tries to find an icon for the task with the specified size. If there
     * is no icon that matches then it will either resize the closest available
     * icon or return a null pixmap depending on the value of allowResize.
     *
     * Note that the last icon is cached, so a sequence of calls with the same
     * parameters will only query the NET properties if the icon has changed or
     * none was found.
     */
    QPixmap icon( int width, int height, bool allowResize = false );

    static bool idMatch(const QString &, const QString &);

    // state
    bool isMaximized() const;
    bool isIconified() const;
    bool isShaded() const;
    bool isActive() const;
    bool isOnCurrentDesktop() const;
    bool isOnAllDesktops() const;
    bool isAlwaysOnTop() const;
    bool isModified() const ;
    int  desktop() const { return _info.desktop; }

    // actions
    void maximize();
    void restore();
    void iconify();
    void close();
    void raise();
    void activate();
    void setAlwaysOnTop(bool);
    void setShaded(bool);
    void toDesktop(int);
    void toCurrentDesktop();
    void publishIconGeometry(QRect);

    // internal
    void refresh(bool icon = false);
    void addTransient( WId w ) { _transients.append( w ); }
    void removeTransient( WId w ) { _transients.remove( w ); }
    bool hasTransient( WId w ) const { return _transients.contains( w ); }
    void setActive(bool a);

    // For thumbnails
    double thumbnailSize() const { return _thumbSize; }
    void setThumbnailSize( double size ) { _thumbSize = size; }

    bool hasThumbnail() const { return !_thumb.isNull(); }
    const QPixmap &thumbnail() const { return _thumb; }

public slots:
    void updateThumbnail();

signals:
    void changed();
    void iconChanged();
    void activated();
    void deactivated();
    void thumbnailChanged();

protected slots:
   void generateThumbnail();

private:
    bool                _active;
    WId                 _win;
    QPixmap             _pixmap;
    KWin::Info          _info;
    QValueList<WId>     _transients;

    int                 _lastWidth;
    int                 _lastHeight;
    bool                _lastResize;
    QPixmap             _lastIcon;

    double _thumbSize;
    QPixmap _thumb;
    QPixmap _grab;
};

class Startup: public QObject
{
    Q_OBJECT

public:
    Startup(const QString &text, const QString &icon, pid_t pid, const QString & bin,
            bool compliant, QObject * parent, const char *name = 0);
    virtual ~Startup();

    QString text() const { return _text; }
    QString bin() const { return _bin; }
    QString icon() const { return _icon; }

    pid_t pid() const { return _pid; }
    bool compliant() const { return _compliant; }

signals:
    void killMe(Startup*);

protected:
    void timerEvent(QTimerEvent *);

private:
    QString     _bin;
    QString     _text;
    pid_t       _pid;
    bool        _compliant;
    QString     _icon;
};

class TaskManager : public QObject, virtual public DCOPObject
{
    Q_OBJECT
    K_DCOP

    k_dcop:
    void clientStarted(QString name, QString icon, pid_t pid, QString bin, bool compliant, int screennumber);
    void clientDied(pid_t pid);

public:
    TaskManager( QObject *parent = 0, const char *name = 0 );
    virtual ~TaskManager();

    QList<Task> tasks() { return _tasks; }
    QList<Startup> startups() { return _startups; }

    QString desktopName(int);
    int numberOfDesktops();
    int currentDesktop();
    bool isOnTop(Task*);

signals:
    void taskAdded(Task*);
    void taskRemoved(Task*);
    void startupAdded(Startup*);
    void startupRemoved(Startup*);
    void desktopChanged(int desktop);
    void windowDesktopChanged(WId);

protected slots:
    void windowAdded(WId);
    void windowRemoved(WId);
    void windowChanged(WId, unsigned int);

    void activeWindowChanged(WId);
    void currentDesktopChanged(int);
    void killStartup(pid_t pid);
    void killStartup(Startup*);

protected:
    Task* findTask(WId w);

private:
    Task*               _active;
    QList<Task>         _tasks;
    QList<Startup>      _startups;
};

#endif
