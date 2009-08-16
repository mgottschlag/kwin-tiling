/*
   Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>
   Copyright (C) 2008 Michael Jansen <kde@michael-jansen.biz>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef _TRIGGERS_H_
#define _TRIGGERS_H_

#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QUuid>

#include <KDE/KShortcut>

#include <kdemacros.h>
#include "khotkeysglobal.h"
#include "voicesignature.h"

#include "input.h"
#include "windows_handler.h"
#include "triggers/gestures.h"

class QKeySequence;

namespace KHotKeys
{

class Windowdef_list;
class ActionData;

class TriggerVisitor
    {
public:
    virtual ~TriggerVisitor();
    };


class KDE_EXPORT Trigger
    {
    Q_DISABLE_COPY( Trigger )

    public:

        enum TriggerType
            {
            GestureTriggerType  = 0x01, //!< @see GestureTrigger
            ShortcutTriggerType = 0x02, //!< @see ShortcutTrigger
            WindowTriggerType   = 0x04, //!< @see WindowTrigger
            TriggerListType     = 0x08, //!< @see Trigger_list
            AllTypes            = 0xFF  //!< All types. For convenience.
            };

        Q_DECLARE_FLAGS(TriggerTypes, TriggerType)

        Trigger( ActionData* data_P );
        virtual ~Trigger();
        virtual void cfg_write( KConfigGroup& cfg_P ) const = 0;
        virtual Trigger* copy( ActionData* data_P ) const = 0;
        virtual const QString description() const = 0;
        virtual void activate( bool activate_P ) = 0;

        //! Disable the trigger
        virtual void disable();

        //! Enable the trigger
        virtual void enable();

        /**
         * The trigger will be erased permanently
         */
        virtual void aboutToBeErased();

        /**
         * The actual type for this trigger
         */
        virtual TriggerType type() const = 0;

        /**
         * Acyclic visitor pattern
         */
        virtual void accept(TriggerVisitor&) = 0;

    protected:
        ActionData* const data;
    };

Q_DECLARE_OPERATORS_FOR_FLAGS(Trigger::TriggerTypes)


class KDE_EXPORT Trigger_list
    : public QList< Trigger* >
    {
    Q_DISABLE_COPY( Trigger_list )

    public:
        Trigger_list( const QString& comment = QString() );
        virtual ~Trigger_list();

        void activate( bool activate_P );
        void cfg_write( KConfigGroup& cfg_P ) const;
        //! Some convenience typedef
        typedef QList< Trigger* >::Iterator Iterator;
        typedef QList< Trigger* >::ConstIterator ConstIterator;
        const QString comment() const;
        void set_comment(const QString &);
        Trigger_list* copy( ActionData* data_P ) const;

        //! Disable the trigger
        virtual void disable();

        //! Enable the trigger
        virtual void enable();

        /**
         * @reimp
         */
        void aboutToBeErased();

    private:
        QString _comment;
    };


class ShortcutTrigger;
class ShortcutTriggerVisitor
    {
public:
    virtual ~ShortcutTriggerVisitor();
    virtual void visit(ShortcutTrigger&) = 0;
    };


class KDE_EXPORT ShortcutTrigger
    : public QObject, public Trigger
    {
    Q_OBJECT

    typedef Trigger base;
    public:
        ShortcutTrigger(
            ActionData* data,
            const KShortcut& shortcut = KShortcut(),
            const QUuid &uuid = QUuid::createUuid() );

        virtual ~ShortcutTrigger();
        virtual void cfg_write( KConfigGroup& cfg_P ) const;
        virtual ShortcutTrigger* copy( ActionData* data_P ) const;
        virtual const QString description() const;
        KShortcut shortcut() const;
        virtual void activate( bool activate_P );

        void set_key_sequence( const QKeySequence &seq );

        virtual TriggerType type() const { return ShortcutTriggerType; }

        /**
         * @reimp
         */
        void aboutToBeErased();

        //! Disable the trigger
        virtual void disable();

        //! Enable the trigger
        virtual void enable();

        /**
         * Acyclic visitor pattern
         */
        virtual void accept(TriggerVisitor&);

    Q_SIGNALS:

        //! Emitted when the global shortcut is changed from somewhere else
        //  (Global Shortcuts KCM)
        void globalShortcutChanged(const QKeySequence&);

    public Q_SLOTS:

        void trigger();

    private:

        //! A persistent identifier for this shortcut
        QUuid _uuid;

        //! Are the conditions met?
        bool _active;

        /**
         * The Key Sequence associated with this Trigger. This is needed
         * because a inactive trigger doesn't register it's shortcut with
         * kde's global shortcuts registry so we have to remember the shortcut
         * ourselves. Beware of synchronizing saved state, global shortcuts
         * registry state and state of this var :-) .
         */
        KShortcut    _shortcut;



    };


class WindowTrigger;
class WindowTriggerVisitor
    {
public:
    virtual ~WindowTriggerVisitor();
    virtual void visit(WindowTrigger&) = 0;
    };


class KDE_EXPORT WindowTrigger : public QObject, public Trigger
    {
    Q_OBJECT

    Q_FLAGS(WindowEvents)

    typedef Trigger base;

    public:

        enum window_action_t
            {
            NONE                   = 0,
            WINDOW_APPEARS         = ( 1 << 0 ),        //!< The window is opened
            WINDOW_DISAPPEARS      = ( 1 << 1 ),        //!< The window is closed
            WINDOW_ACTIVATES       = ( 1 << 2 ),        //!< The window gets the focus
            WINDOW_DEACTIVATES     = ( 1 << 3 )         //!< The window loses the focus
            };

        Q_DECLARE_FLAGS(WindowEvents, window_action_t)

        WindowTrigger(
                ActionData* data,
                Windowdef_list* windowslist = NULL,
                WindowEvents window_actions = 0 );

        void setOnWindowEvents(WindowEvents events);

        virtual ~WindowTrigger();
        virtual void cfg_write( KConfigGroup& cfg_P ) const;
        virtual WindowTrigger* copy( ActionData* data_P ) const;
        virtual const QString description() const;
        const Windowdef_list* windows() const;
        void set_window_rules(Windowdef_list *list);
        Windowdef_list* windows();
        bool triggers_on( window_action_t w_action_P ) const;
        virtual void activate( bool activate_P );

        virtual TriggerType type() const { return WindowTriggerType; }



        /**
         * Acyclic visitor pattern
         */
        virtual void accept(TriggerVisitor&);

    protected Q_SLOTS:
        void window_added( WId window_P );
        void window_removed( WId window_P );
        void active_window_changed( WId window_P );
        void window_changed( WId window_P, unsigned int dirty_P );

    private:

        //! Useful code for all constructors
        void init();

        Windowdef_list* _windows;

        WindowEvents window_actions;

        typedef QMap< WId, bool > Windows_map;

        //! Internal cache. Remembers if a window is a match or not,
        Windows_map existing_windows;

        //! The last active window
        WId last_active_window;

        //! Is the trigger active?
        bool active;
    };

Q_DECLARE_OPERATORS_FOR_FLAGS(WindowTrigger::WindowEvents)

/**
 * This class handles the storage of gesture data; it also matches gestures
 * and links the gesture to an action.
 * One object equals one configured gesture.
 */
class GestureTrigger;
class GestureTriggerVisitor
    {
public:
    virtual ~GestureTriggerVisitor();
    virtual void visit(GestureTrigger&) = 0;
    };

class KDE_EXPORT GestureTrigger
    : public QObject, public Trigger
    {
    Q_OBJECT
    typedef Trigger base;
    public:
        GestureTrigger( ActionData* data, const StrokePoints& pointdata_P = StrokePoints() );

        virtual ~GestureTrigger();
        virtual void cfg_write( KConfigGroup& cfg_P ) const;
        virtual Trigger* copy( ActionData* data_P ) const;
        virtual const QString description() const;
        const StrokePoints& pointData() const;

        //! Set the point data of the gesture
        void setPointData(const StrokePoints &data);
        void setPointData(const QStringList &strings);
        void setKDE3Gesture(const QString &gestureCode);

        virtual void activate( bool activate_P );

        virtual TriggerType type() const { return GestureTriggerType; }

        /**
         * Acyclic visitor pattern
         */
        virtual void accept(TriggerVisitor&);

    protected Q_SLOTS:
        void handle_gesture( const StrokePoints& gesture_P );
    Q_SIGNALS:
        void gotScore( ActionData* const data, const qreal score );

    private:
        qreal comparePointData(const StrokePoints &a, const StrokePoints &b) const;
        inline qreal angleSquareDifference(qreal alpha, qreal beta) const;

        StrokePoints _pointdata;
    };


// FIXME: SOUND
#if 0
class KDE_EXPORT Voice_trigger
    : public QObject, public Trigger
    {
    Q_OBJECT
    typedef Trigger base;
    public:
        Voice_trigger( ActionData* data_P, const QString& Voice_P, const VoiceSignature & signature1_P, const VoiceSignature & signature2_P );
        Voice_trigger( KConfigGroup& cfg_P, ActionData* data_P );
        virtual ~Voice_trigger();
        virtual void cfg_write( KConfigGroup& cfg_P ) const;
        virtual Trigger* copy( ActionData* data_P ) const;
        virtual const QString description() const;
        const QString& voicecode() const;
        virtual void activate( bool activate_P );
        VoiceSignature voicesignature( int ech ) const;

        virtual TriggerType type() const { return SoundTrigger; }
    public slots:
        void handle_Voice(  );
    private:
        QString _voicecode;
        VoiceSignature _voicesignature[2];
    };
#endif

// FIXME: SOUND
#if 0
// Voice_trigger
inline
const QString& Voice_trigger::voicecode() const
    {
    return _voicecode;
    }

inline
VoiceSignature Voice_trigger::voicesignature(int ech) const
    {
    return _voicesignature[ech-1];
    }
#endif
} // namespace KHotKeys


#endif
