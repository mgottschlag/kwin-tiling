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


#include <QMap>
#include <QFile>
#include <QX11Info>

#include <kdebug.h>
#include <kstandarddirs.h>
#include <kprocess.h>

#include <X11/Xatom.h>
#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XKBfile.h>
//#include <X11/extensions/XKBgeom.h>
#include <X11/extensions/XKM.h>

#include "extension.h"


// QMap<QString, FILE*> XKBExtension::fileCache;	//TODO: move to class?


static QString getLayoutKey(const QString& layout, const QString& variant)
{
	return layout + '.' + variant;
}

// QString XKBExtension::getPrecompiledLayoutFilename(const QString& layoutKey)
// {
// 	QString compiledLayoutFileName = m_tempDir + layoutKey + ".xkm";
// 	return compiledLayoutFileName;
// }

XKBExtension::XKBExtension(Display *d)
{
	if ( d == NULL )
		d = QX11Info::display();
	m_dpy = d;
	
//	QStringList dirs = KGlobal::dirs()->findDirs ( "tmp", "" );
//	m_tempDir = dirs.count() == 0 ? "/tmp/" : dirs[0];
// 	m_tempDir = KStandardDirs::locateLocal("tmp", "");
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

    // Do it, or face horrible memory corrupting bugs
    ::XkbInitAtoms(NULL);

	int eventMask = XkbNewKeyboardNotifyMask | XkbStateNotifyMask;
    if (!XkbSelectEvents(m_dpy, XkbUseCoreKbd, eventMask, eventMask)) {
	   kDebug() << "Couldn't select desired XKB events";
	   return false;
	}

	kDebug() << "XKB inited";

    return true;
}

void XKBExtension::reset()
{
// 	for(QMap<QString, FILE*>::ConstIterator it = fileCache.begin(); it != fileCache.end(); ++it) {
// 		fclose(*it);
// //		remove( QFile::encodeName(getPrecompiledLayoutFileName(*it)) );
// 	}
// 	fileCache.clear();
}

XKBExtension::~XKBExtension()
{
/*	if( m_compiledLayoutFileNames.isEmpty() == false )
		deletePrecompiledLayouts();*/
}

bool XKBExtension::setXkbOptions(const QString& options, bool resetOld)
{
    if (options.isEmpty())
        return true;

    QString exe = KGlobal::dirs()->findExe("setxkbmap");
    if (exe.isEmpty())
        return false;

    KProcess p;
    p << exe;
    if( resetOld )
        p << "-option";
    p << "-option" << options;

    return p.execute() == 0;
}

bool XKBExtension::setLayout(const QString& model,
		const QString& layout, const QString& variant,
		const QString& includeGroup, bool useCompiledLayouts)
{
	if( useCompiledLayouts == false ) {
		return setLayoutInternal( model, layout, variant, includeGroup );
	}
	
	const QString layoutKey = getLayoutKey(layout, variant);
	
	bool res = setLayoutInternal( model, layout, variant, includeGroup );
	kDebug() << "setRawLayout " << layoutKey << ": " << res;
	return res;
}


bool XKBExtension::setLayout(const QString& layouts, const QString& variants)
{
	bool res = setLayoutInternal( "", layouts, variants, "" );
	kDebug() << "setRawLayout " << layouts << ": " << variants << " res: " << res;
	return res;
}

// private
bool XKBExtension::setLayoutInternal(const QString& model,
		const QString& layout, const QString& variant,
		const QString& includeGroup)
{
    if ( layout.isEmpty() )
        return false;

	QString exe = KGlobal::dirs()->findExe("setxkbmap");
	if( exe.isEmpty() ) {
		kError() << "Can't find setxkbmap" << endl;
		return false;
	}

    QString fullLayout = layout;
    QString fullVariant = variant;
	if( includeGroup.isEmpty() == false ) {
        fullLayout = includeGroup;
        fullLayout += ',';
        fullLayout += layout;
		
//    fullVariant = baseVar;
        fullVariant = ",";
        fullVariant += variant;
    }
 
    KProcess p;
    p << exe;
//  p << "-rules" << rule;
	if( model.isEmpty() == false )
		p << "-model" << model;
    p << "-layout" << fullLayout;
    if( !fullVariant.isNull() && !fullVariant.isEmpty() )
        p << "-variant" << fullVariant;

	kDebug() << "Ext: setting " << fullLayout << ", " << fullVariant;
	
    return p.execute() == 0;
}

bool XKBExtension::setGroup(unsigned int group)
{
	kDebug() << "Setting group " << group;
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

    return ( (xkbEvent->any.xkb_type == XkbMapNotify) && (xkbEvent->map.changed & XkbKeySymsMask) ) ||
/*    	  || ( (xkbEvent->any.xkb_type == XkbNamesNotify) && (xkbEvent->names.changed & XkbGroupNamesMask) || )*/
    	   (xkbEvent->any.xkb_type == XkbNewKeyboardNotify);
}
