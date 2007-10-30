/*
 *  Copyright (C) 2003-2006 Andriy Rysin (rysin@kde.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <string.h>
#include <errno.h>


#include <QX11Info>

#include <kdebug.h>
#include <kstandarddirs.h>
#include <kprocess.h>

#include <X11/Xatom.h>
#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#ifndef HAVE_XKLAVIER
#include <X11/extensions/XKBfile.h>
#endif

#include "extension.h"


static const char* SETXKBMAP_SEPARATOR=",";

QString XKBExtension::m_setxkbmap_exe = "";

XKBExtension::XKBExtension(Display *d)
{
    if ( d == NULL )
        d = QX11Info::display();
    m_dpy = d;
}

bool XKBExtension::init()
{
    // Verify the Xlib has matching XKB extension.

    int major = XkbMajorVersion;
    int minor = XkbMinorVersion;
	
    if (!XkbLibraryVersion(&major, &minor))
    {
        kError() << "Xlib XKB extension " << major << '.' << minor <<
            " != " << XkbMajorVersion << '.' << XkbMinorVersion << endl;
        return false;
    }

    // Verify the X server has matching XKB extension.

    int opcode_rtrn;
    int error_rtrn;
//    int xkb_opcode;
    if (!XkbQueryExtension(m_dpy, &opcode_rtrn, &xkb_opcode, &error_rtrn,
                         &major, &minor))
    {
        kError() << "X server XKB extension " << major << '.' << minor <<
            " != " << XkbMajorVersion << '.' << XkbMinorVersion << endl;
        return false;
    }

#ifndef HAVE_XKLAVIER
    // Do it, or face horrible memory corrupting bugs
    ::XkbInitAtoms(NULL);
#endif

    int eventMask = XkbNewKeyboardNotifyMask | XkbStateNotifyMask;
    if( !XkbSelectEvents(m_dpy, XkbUseCoreKbd, eventMask, eventMask) ) {
	kDebug() << "Couldn't select desired XKB events";
	return false;
    }

    kDebug() << "XKB inited";

    return true;
}

XKBExtension::~XKBExtension()
{
}

QString XKBExtension::getSetxkbmapExe()
{
    if( m_setxkbmap_exe.isEmpty() ) {
        m_setxkbmap_exe = KGlobal::dirs()->findExe("setxkbmap");
        if( m_setxkbmap_exe.isEmpty() )
	    kError() << "Can't find setxkbmap" << endl;
    }
    return m_setxkbmap_exe;
}

QString XKBExtension::getXkbOptionsCommand(const QStringList& options, bool resetOld)
{
    if( options.empty() && ! resetOld )
        return "";

    QString cmd = "setxkbmap";
    if( resetOld )
        cmd += " -option";

    if( ! options.empty() ) {
        cmd += " -option ";
        cmd += options.join(SETXKBMAP_SEPARATOR);
    }
    return cmd;
}

bool XKBExtension::setXkbOptions(const QStringList& options, bool resetOld)
{
    kDebug() << "Setting XKB options " << options.join(SETXKBMAP_SEPARATOR);

    if( options.empty() && ! resetOld )
        return true;

    getSetxkbmapExe();
    if( m_setxkbmap_exe.isEmpty() )
        return false;

    KProcess p;
    p << m_setxkbmap_exe;

    if( resetOld )
        p << "-option";
    p << "-option" << options.join(SETXKBMAP_SEPARATOR);

    kDebug() << "executing" << p.program().join(" ");

    return p.execute() == 0;
}

QString XKBExtension::getLayoutGroupsCommand(const QString& model, const QStringList& layouts, const QStringList& variants)
{
    if( layouts.empty() )
        return "";

    QString cmd = "setxkbmap";
    
    if( ! model.isEmpty() ) {
        cmd += " -model ";
        cmd += model;
    }
        
    cmd += " -layout ";
    cmd += layouts.join(SETXKBMAP_SEPARATOR);

    if( ! variants.empty() ) {
        cmd += " -variant ";
        cmd += variants.join(SETXKBMAP_SEPARATOR);
    }
    return cmd;
}

bool XKBExtension::setLayoutGroups(const QString& model, const QStringList& layouts, const QStringList& variants)
{
    if( layouts.empty() )
        return false;

    getSetxkbmapExe();
    if( m_setxkbmap_exe.isEmpty() )
	return false;

    KProcess p;
    p << m_setxkbmap_exe;
    
    if( ! model.isEmpty() )
        p << "-model" << model;
        
    p << "-layout" << layouts.join(SETXKBMAP_SEPARATOR);

    if( ! variants.empty() )
        p << "-variant" << variants.join(SETXKBMAP_SEPARATOR);

    kDebug() << "executing" << p.program().join(" ");
	
    return p.execute() == 0;
}

bool XKBExtension::setGroup(unsigned int group)
{
//	kDebug() << "Setting group " << group;
	return XkbLockGroup( m_dpy, XkbUseCoreKbd, group );
}

unsigned int XKBExtension::getGroup() const
{
	XkbStateRec xkbState;
	XkbGetState( m_dpy, XkbUseCoreKbd, &xkbState );
	return xkbState.group;
}

bool XKBExtension::isGroupSwitchEvent(XEvent* event)
{
    XkbEvent *xkbEvent = (XkbEvent*) event;
#define GROUP_CHANGE_MASK \
    ( XkbGroupStateMask | XkbGroupBaseMask | XkbGroupLatchMask | XkbGroupLockMask )
														  
    return xkbEvent->any.xkb_type == XkbStateNotify && xkbEvent->state.changed & GROUP_CHANGE_MASK;
}

bool XKBExtension::isLayoutSwitchEvent(XEvent* event)
{
    XkbEvent *xkbEvent = (XkbEvent*) event;

    return //( (xkbEvent->any.xkb_type == XkbMapNotify) && (xkbEvent->map.changed & XkbKeySymsMask) ) ||
/*    	  || ( (xkbEvent->any.xkb_type == XkbNamesNotify) && (xkbEvent->names.changed & XkbGroupNamesMask) || )*/
    	   (xkbEvent->any.xkb_type == XkbNewKeyboardNotify);
}
