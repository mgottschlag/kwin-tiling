/*****************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2004 Lubos Lunak <l.lunak@kde.org>

You can Freely distribute this program under the GNU General Public
License. See the file "COPYING" for the exact licensing terms.
******************************************************************/

#ifndef KWIN_RULES_H
#define KWIN_RULES_H

#include <qstring.h>
#include <netwm_def.h>
#include <qrect.h>
#include <qvaluevector.h>
#include <kdebug.h>

#include "placement.h"
#include "lib/kdecoration.h"
#include "options.h"
#include "utils.h"

class KConfig;

namespace KWinInternal
{

class Client;
class Rules;

class WindowRules
    : public KDecorationDefines
    {
    public:
        WindowRules( const QValueVector< Rules* >& rules );
        WindowRules();
        void update( Client* );
        void discardTemporary();
        Placement::Policy checkPlacement( Placement::Policy placement ) const;
        QRect checkGeometry( QRect rect, bool init = false ) const;
        // use 'invalidPoint' with checkPosition, unlike QSize() and QRect(), QPoint() is a valid point
        QPoint checkPosition( QPoint pos, bool init = false ) const;
        QSize checkSize( QSize s, bool init = false ) const;
        QSize checkMinSize( QSize s ) const;
        QSize checkMaxSize( QSize s ) const;
        bool checkIgnorePosition( bool ignore ) const;
        int checkDesktop( int desktop, bool init = false ) const;
        NET::WindowType checkType( NET::WindowType type ) const;
        MaximizeMode checkMaximize( MaximizeMode mode, bool init = false ) const;
        bool checkMinimize( bool minimized, bool init = false ) const;
        ShadeMode checkShade( ShadeMode shade, bool init = false ) const;
        bool checkSkipTaskbar( bool skip, bool init = false ) const;
        bool checkSkipPager( bool skip, bool init = false ) const;
        bool checkKeepAbove( bool above, bool init = false ) const;
        bool checkKeepBelow( bool below, bool init = false ) const;
        bool checkFullScreen( bool fs, bool init = false ) const;
        bool checkNoBorder( bool noborder, bool init = false ) const;
        int checkFSP( int fsp ) const;
        bool checkAcceptFocus( bool focus ) const;
        Options::MoveResizeMode checkMoveResizeMode( Options::MoveResizeMode mode ) const;
        bool checkCloseable( bool closeable ) const;
    private:
        MaximizeMode checkMaximizeVert( MaximizeMode mode, bool init ) const;
        MaximizeMode checkMaximizeHoriz( MaximizeMode mode, bool init ) const;
        QValueVector< Rules* > rules;
    };

class Rules
    : public KDecorationDefines
    {
    public:
        Rules();
        Rules( KConfig& );
        Rules( const QString&, bool temporary );
        void write( KConfig& ) const;
        bool update( Client* );
        bool isTemporary() const;
        bool match( const Client* c ) const;
        bool discardTemporary( bool force ); // removes if temporary and forced or too old
        bool applyPlacement( Placement::Policy& placement ) const;
        bool applyGeometry( QRect& rect, bool init ) const;
        // use 'invalidPoint' with applyPosition, unlike QSize() and QRect(), QPoint() is a valid point
        bool applyPosition( QPoint& pos, bool init ) const;
        bool applySize( QSize& s, bool init ) const;
        bool applyMinSize( QSize& s ) const;
        bool applyMaxSize( QSize& s ) const;
        bool applyIgnorePosition( bool& ignore ) const;
        bool applyDesktop( int& desktop, bool init ) const;
        bool applyType( NET::WindowType& type ) const;
        bool applyMaximizeVert( MaximizeMode& mode, bool init ) const;
        bool applyMaximizeHoriz( MaximizeMode& mode, bool init ) const;
        bool applyMinimize( bool& minimized, bool init ) const;
        bool applyShade( ShadeMode& shade, bool init ) const;
        bool applySkipTaskbar( bool& skip, bool init ) const;
        bool applySkipPager( bool& skip, bool init ) const;
        bool applyKeepAbove( bool& above, bool init ) const;
        bool applyKeepBelow( bool& below, bool init ) const;
        bool applyFullScreen( bool& fs, bool init ) const;
        bool applyNoBorder( bool& noborder, bool init ) const;
        bool applyFSP( int& fsp ) const;
        bool applyAcceptFocus( bool& focus ) const;
        bool applyMoveResizeMode( Options::MoveResizeMode& mode ) const;
        bool applyCloseable( bool& closeable ) const;
    private:
        enum // values are saved to the cfg file
            {
            Unused = 0,
            DontAffect, // use the default value
            Force,      // force the given value
            Apply,      // apply only after initial mapping
            Remember   // like apply, and remember the value when the window is withdrawn
            };
        enum SetRule
            {
            UnusedSetRule = Unused,
            SetRuleDummy = 256   // so that it's at least short int
            };
        enum ForceRule
            {
            UnusedForceRule = Unused,
            ForceRuleDummy = 256   // so that it's at least short int
            };
        void readFromCfg( KConfig& cfg );
        static SetRule readSetRule( KConfig&, const QString& key );
        static ForceRule readForceRule( KConfig&, const QString& key );
        static NET::WindowType readType( KConfig&, const QString& key );
        static bool checkSetRule( SetRule rule, bool init );
        static bool checkForceRule( ForceRule rule );
        static bool checkSetStop( SetRule rule );
        static bool checkForceStop( ForceRule rule );
        int temporary_state; // e.g. for kstart
        QCString wmclass;
        bool wmclassregexp;
        bool wmclasscomplete;
        QCString windowrole;
        bool windowroleregexp;
        QString title; // TODO "caption" ?
        bool titleregexp;
        QCString extrarole;
        bool extraroleregexp;
        QCString clientmachine;
        bool clientmachineregexp;
        unsigned long types; // types for matching
        Placement::Policy placement;
        ForceRule placementrule;
        QPoint position;
        SetRule positionrule;
        QSize size;
        SetRule sizerule;
        QSize minsize;
        ForceRule minsizerule;
        QSize maxsize;
        ForceRule maxsizerule;
        bool ignoreposition;
        ForceRule ignorepositionrule;
        int desktop;
        SetRule desktoprule;
        NET::WindowType type; // type for setting
        ForceRule typerule;
        bool maximizevert;
        SetRule maximizevertrule;
        bool maximizehoriz;
        SetRule maximizehorizrule;
        bool minimize;
        SetRule minimizerule;
        bool shade;
        SetRule shaderule;
        bool skiptaskbar;
        SetRule skiptaskbarrule;
        bool skippager;
        SetRule skippagerrule;
        bool above;
        SetRule aboverule;
        bool below;
        SetRule belowrule;
        bool fullscreen;
        SetRule fullscreenrule;
        bool noborder;
        SetRule noborderrule;
        int fsplevel;
        ForceRule fsplevelrule;
        bool acceptfocus;
        ForceRule acceptfocusrule;
        Options::MoveResizeMode moveresizemode;
        ForceRule moveresizemoderule;
        bool closeable;
        ForceRule closeablerule;
        friend kdbgstream& operator<<( kdbgstream& stream, const Rules* );
    };

inline
bool Rules::checkSetRule( SetRule rule, bool init )
    {
    if( rule > ( SetRule )DontAffect) // Unused or DontAffect
        {
        if( rule == ( SetRule )Force || init )
            return true;
        }
    return false;
    }

inline
bool Rules::checkForceRule( ForceRule rule )
    {
    return rule == ( ForceRule )Force;
    }

inline
bool Rules::checkSetStop( SetRule rule )
    {
    return rule != UnusedSetRule;
    }
    
inline
bool Rules::checkForceStop( ForceRule rule )
    {
    return rule != UnusedForceRule;
    }

inline
WindowRules::WindowRules( const QValueVector< Rules* >& r )
    : rules( r )
    {
    }

inline
WindowRules::WindowRules()
    {
    }

#ifdef NDEBUG
inline
kndbgstream& operator<<( kndbgstream& stream, const Rules* ) { return stream; }
#else
kdbgstream& operator<<( kdbgstream& stream, const Rules* );
#endif

} // namespace

#endif
