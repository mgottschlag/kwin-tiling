/*

 This is a private header from the Qt library for the use of
 qx11embed_x11.cpp.  It should not be used by any other KDE code.

 The remainder of this file is unchanged from the version in Qt 4.3.2

*/
/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QOBJECT_P_H
#define QOBJECT_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qapplication_*.cpp, qwidget*.cpp and qfiledialog.cpp.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#include "QtCore/qobject.h"
#include "QtCore/qpointer.h"
#include "QtCore/qcoreevent.h"
#include "QtCore/qlist.h"
#include "QtCore/qvector.h"
#include "QtCore/qreadwritelock.h"
#include "QtCore/qvariant.h"

class QVariant;
class QThreadData;

/* mirrored in QtTestLib, DON'T CHANGE without prior warning */
struct QSignalSpyCallbackSet
{
    typedef void (*BeginCallback)(QObject *caller, int method_index, void **argv);
    typedef void (*EndCallback)(QObject *caller, int method_index);
    BeginCallback signal_begin_callback,
                    slot_begin_callback;
    EndCallback signal_end_callback,
                slot_end_callback;
};
void Q_CORE_EXPORT qt_register_signal_spy_callbacks(const QSignalSpyCallbackSet &callback_set);

extern QSignalSpyCallbackSet Q_CORE_EXPORT qt_signal_spy_callback_set;

inline QObjectData::~QObjectData() {}

enum { QObjectPrivateVersion = QT_VERSION };

class Q_CORE_EXPORT QObjectPrivate : public QObjectData
{
    Q_DECLARE_PUBLIC(QObject)

public:
    // use this lock when implementing thread-safe QObject things (e.g. postEvent())
    static QReadWriteLock *readWriteLock();
    // note: must lockForRead() before calling isValidObject()
    static bool isValidObject(QObject *object);

    QObjectPrivate(int version = QObjectPrivateVersion);
    virtual ~QObjectPrivate();

#ifdef QT3_SUPPORT
    QList<QObject *> pendingChildInsertedEvents;
    void sendPendingChildInsertedEvents();
    void removePendingChildInsertedEvents(QObject *child);
#else
    // preserve binary compatibility with code compiled without Qt 3 support
    QList<QObject *> unused;
#endif

    // id of the thread that owns the object
    QThreadData *threadData;
    void moveToThread_helper();
    void setThreadData_helper(QThreadData *currentData, QThreadData *targetData);
    void _q_reregisterTimers(void *pointer);

    // object currently activating the object
    union {
        QObject *currentSender;
        QObject *currentChildBeingDeleted;
    };
    int currentSenderSignalIdStart;
    int currentSenderSignalIdEnd;

    bool isSender(const QObject *receiver, const char *signal) const;
    QObjectList receiverList(const char *signal) const;
    QObjectList senderList() const;

    QList<QPointer<QObject> > eventFilters;

    void setParent_helper(QObject *);

    void deleteChildren();

    static void clearGuards(QObject *);

    struct ExtraData
    {
#ifndef QT_NO_USERDATA
        QVector<QObjectUserData *> userData;
#endif
        QList<QByteArray> propertyNames;
        QList<QVariant> propertyValues;
    };
    ExtraData *extraData;
    mutable quint32 connectedSignals;

    QString objectName;
#if defined(Q_WS_X11)
    virtual void checkWindowRole();
#endif
};

class QSemaphore;
class Q_CORE_EXPORT QMetaCallEvent : public QEvent
{
public:
    QMetaCallEvent(int id, const QObject *sender = 0,
                   int nargs = 0, int *types = 0, void **args = 0, QSemaphore *semaphore = 0);
    QMetaCallEvent(int id, const QObject *sender, int idFrom, int idTo,
                   int nargs = 0, int *types = 0, void **args = 0, QSemaphore *semaphore = 0);
    ~QMetaCallEvent();

    inline int id() const { return id_; }
    inline const QObject *sender() const { return sender_; }
    inline int signalIdStart() const { return idFrom_; }
    inline int signalIdEnd() const { return idTo_; }
    inline void **args() const { return args_; }

    virtual int placeMetaCall(QObject *object);

private:
    int id_;
    const QObject *sender_;
    int idFrom_;
    int idTo_;
    int nargs_;
    int *types_;
    void **args_;
    QSemaphore *semaphore_;
};

class Q_CORE_EXPORT QBoolBlocker
{
public:
    inline QBoolBlocker(bool &b):block(b), reset(b){block = true;}
    inline ~QBoolBlocker(){block = reset; }
private:
    bool &block;
    bool reset;
};

#endif // QOBJECT_P_H
