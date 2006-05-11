#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#include <errno.h>

#include <qwindowdefs.h>
#include <qglobal.h>
#include <QFile>

#include <kapplication.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kprocess.h>

#include <X11/Xatom.h>
#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XKBfile.h>
#include <X11/extensions/XKBgeom.h>
#include <X11/extensions/XKM.h>

#include "extension.h"
#include <QX11Info>

XKBExtension::XKBExtension(Display *d)
{
    if (!d)
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
    int xkb_opcode;
    if (!XkbQueryExtension(m_dpy, &opcode_rtrn, &xkb_opcode, &error_rtrn,
                         &major, &minor))
    {
        kError() << "X server XKB extension " << major << '.' << minor <<
            " != " << XkbMajorVersion << '.' << XkbMinorVersion << endl;
        return false;
    }
    
    // Do it, or face horrible memory corrupting bugs
    ::XkbInitAtoms(NULL);

    return true;
}

XKBExtension::~XKBExtension()
{
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

    p.start(KProcess::Block);

    return p.normalExit() && (p.exitStatus() == 0);
}

bool XKBExtension::setLayout(const QString& model, 
		const QString& layout, const char* variant, unsigned int group,
		const char* baseGr)
{
    if (/*rule.isEmpty() ||*/ model.isEmpty() || layout.isEmpty())
        return false;

    QString exe = KGlobal::dirs()->findExe("setxkbmap");
    if (exe.isEmpty())
        return false;

    QString fullLayout = layout;
    QString fullVariant = variant;
    if( baseGr != 0 && baseGr[0] != '\0' ) {
        fullLayout = baseGr;
        fullLayout += ",";
        fullLayout += layout;
//    fullVariant = baseVar;
        fullVariant = ",";
        fullVariant += variant;
    }
    
    KProcess p;
    p << exe;
//  p << "-rules" << rule;
    p << "-model" << model;
    p << "-layout" << fullLayout;
    if( !fullVariant.isNull() && !fullVariant.isEmpty() )
        p << "-variant" << fullVariant;

    if (p.start(KProcess::Block) && p.normalExit() && (p.exitStatus() == 0)) {

	if( baseGr != 0 && baseGr[0] != '\0' )
	    group = 1;

        return XkbLockGroup( m_dpy, XkbUseCoreKbd, group );
    }
    else {
        return false;
    }
}

bool XKBExtension::setGroup(unsigned int group)
{
    return XkbLockGroup( m_dpy, XkbUseCoreKbd, group );
}

unsigned int XKBExtension::getGroup()
{
    XkbStateRec xkbState;
    XkbGetState( m_dpy, XkbUseCoreKbd, &xkbState );
    return xkbState.group;
}

// Get the current layout in its binary compiled form.
// and write it to the file specified by 'fileName'
bool XKBExtension::getCompiledLayout(const QString &fileName)
{
    XkbFileInfo result;
    memset(&result, 0, sizeof(result));
    result.type = XkmKeymapFile;
    XkbReadFromServer(m_dpy, XkbAllMapComponentsMask, XkbAllMapComponentsMask, &result);

    FILE *output = fopen(QFile::encodeName(fileName), "w");
    if (!output)
    {
        XkbFreeKeyboard(result.xkb, XkbAllControlsMask, True);
        return false;
    }

    if( !XkbWriteXKMFile(output, &result) )
        return false;

    fclose(output);
    XkbFreeKeyboard(result.xkb, XkbAllControlsMask, True);
    return true;
}

// Sets the current layout to the given binary compiled layout
// from the file specified by 'fileName'
bool XKBExtension::setCompiledLayout(const QString &fileName)
{
    FILE *input = fopen(QFile::encodeName(fileName), "r");
    if (!input)
    {
        kDebug() << "Unable to open " << fileName << ": " << strerror(errno) << endl;
        return false;
    }

    XkbFileInfo result;
    memset(&result, 0, sizeof(result));
    if ((result.xkb = XkbAllocKeyboard())==NULL)
    {
         kWarning() << "Unable to allocate memory for keyboard description." << endl;
         fclose(input);
         return false;
    }
    unsigned retVal = XkmReadFile(input, 0, XkmKeymapLegal, &result);
    if (retVal == XkmKeymapLegal)
    {
        // this means reading the Xkm didn't manage to read any section
        kWarning() << "Unable to load map from file." << endl;
        XkbFreeKeyboard(result.xkb, XkbAllControlsMask, True);
        fclose(input);
        return false;
    }

    fclose(input);

    if (XkbChangeKbdDisplay(m_dpy, &result) == Success)
    {
        if (!XkbWriteToServer(&result))
        {
            kWarning() << "Unable to write the keyboard layout to X display." << endl;
            XkbFreeKeyboard(result.xkb, XkbAllControlsMask, True);
            return false;
        }
    }
    else
    {
        kWarning() << "Unable prepare the keyboard layout for X display." << endl;
    }
    
    XkbFreeKeyboard(result.xkb, XkbAllControlsMask, True);
    return true;
}
