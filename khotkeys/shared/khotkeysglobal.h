/****************************************************************************

 KHotKeys -  (C) 2000 Lubos Lunak <l.lunak@email.cz>

 khotkeysglobal.h  -
 
 $Id$

****************************************************************************/

#ifndef __khotkeysglobal_H
#define __khotkeysglobal_H

#include <qdict.h>
#include <qtimer.h>
#include <kconfigbase.h>
#include <ksimpleconfig.h>

class KHotData_dict;

struct KHotData
    {
    KHotData( const QString& shortcut_P, const QString& run_P );
    QString shortcut;
    QString run;
    QTimer timeout;
    };

class KHotData_dict
    : public QDict< KHotData >
    {
    public:
        KHotData_dict( int size=17, bool caseSensitive=TRUE );
        bool read_config( KConfigBase& cfg_P );
        void write_config( KSimpleConfig& cfg_P ) const;
        typedef QDictIterator< KHotData > Iterator;
    };
    
    
//*****************************************************************************
// Inline
//*****************************************************************************

inline
KHotData::KHotData( const QString& shortcut_P, const QString& run_P )
    : shortcut( shortcut_P ), run( run_P )
    {
    }
    
inline
KHotData_dict::KHotData_dict( int size, bool caseSensitive )
    : QDict< KHotData >( size, caseSensitive )
    {
    setAutoDelete( true );
    }


#endif
