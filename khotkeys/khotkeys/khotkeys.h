/****************************************************************************

 KHotKeys -  (C) 2000 Lubos Lunak <l.lunak@email.cz>

 khotkeys.h  -

 $Id$

****************************************************************************/

#ifndef __khotkeys_H
#define __khotkeys_H

#include <kuniqueapp.h>
#include <qtimer.h>
#include <dcopobject.h>
#include <kurifilter.h>
#include <kglobalaccel.h>

#include "khotkeysglobal.h"

class KHotKeysApp
    : public KUniqueApplication
    {
    Q_OBJECT
    K_DCOP
    public:
        KHotKeysApp();
        virtual ~KHotKeysApp();
    protected:
        //virtual bool x11EventFilter(XEvent *);
        KGlobalAccel* accel;
        KHotData_dict data;
        void start_general( const QString& action_P );
        void start_menuentry( const QString& action_P );
    protected slots:
        void accel_activated( const QString& action_P, const QString&, const KKeySequence& );
    public:
    k_dcop:
        virtual ASYNC reread_configuration();
    };

//****************************************************************************
// Inline
//****************************************************************************

#endif
