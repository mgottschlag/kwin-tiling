#include <string.h>
#include <errno.h>

#include <QString>
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
#include <X11/extensions/XKBgeom.h>
#include <X11/extensions/XKM.h>

#include "extension.h"


QMap<QString, FILE*> XKBExtension::fileCache;	//TODO: move to class?


static QString getLayoutKey(const QString& layout, const QString& variant)
{
	return layout + "." + variant;
}

QString XKBExtension::getPrecompiledLayoutFilename(const QString& layoutKey)
{
	QString compiledLayoutFileName = m_tempDir + layoutKey + ".xkm";
	return compiledLayoutFileName;
}

XKBExtension::XKBExtension(Display *d)
{
	if ( d == NULL )
		d = QX11Info::display();
	m_dpy = d;
	
	QStringList dirs = KGlobal::dirs()->findDirs ( "tmp", "" );
	m_tempDir = dirs.count() == 0 ? "/tmp/" : dirs[0];
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

void XKBExtension::reset()
{
	for(QMap<QString, FILE*>::ConstIterator it = fileCache.begin(); it != fileCache.end(); it++) {
		fclose(*it);
//		remove( QFile::encodeName(getPrecompiledLayoutFileName(*it)) );
	}
	fileCache.clear();
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

    p.start(KProcess::Block);

    return p.normalExit() && (p.exitStatus() == 0);
}

bool XKBExtension::setLayout(const QString& model,
		const QString& layout, const QString& variant,
		const QString& includeGroup, bool useCompiledLayouts)
{
	if( useCompiledLayouts == false ) {
		return setLayoutInternal( model, layout, variant, includeGroup );
	}
	
	const QString layoutKey = getLayoutKey(layout, variant);
	
	bool res;
	if( fileCache.contains(layoutKey) ) {
		res = setCompiledLayout( layoutKey );
		kDebug() << "setCompiledLayout " << layoutKey << ": " << res << endl;

		if( res )
			return res;
	}
//	else {
		res = setLayoutInternal( model, layout, variant, includeGroup );
		kDebug() << "setRawLayout " << layoutKey << ": " << res << endl;
		if( res )
			compileCurrentLayout( layoutKey );
		
//	}
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
        fullLayout += ",";
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

    if (p.start(KProcess::Block) && p.normalExit() && (p.exitStatus() == 0)) {
		return true; //setGroup( group );
    }
    else {
        return false;
    }
}

bool XKBExtension::setGroup(unsigned int group)
{
	kDebug() << "Setting group " << group << endl;
	return XkbLockGroup( m_dpy, XkbUseCoreKbd, group );
}

unsigned int XKBExtension::getGroup() const
{
	XkbStateRec xkbState;
	XkbGetState( m_dpy, XkbUseCoreKbd, &xkbState );
	return xkbState.group;
}

/**
 * @brief Gets the current layout in its binary compiled form
 *		and write it to the file specified by 'fileName'
 * @param[in] fileName file to store compiled layout to
 * @return true if no problem, false otherwise
 */
bool XKBExtension::compileCurrentLayout(const QString &layoutKey)
{
    XkbFileInfo result;
    memset(&result, 0, sizeof(result));
    result.type = XkmKeymapFile;
    XkbReadFromServer(m_dpy, XkbAllMapComponentsMask, XkbAllMapComponentsMask, &result);
	 
	const QString fileName = getPrecompiledLayoutFilename(layoutKey);

	kDebug() << "compiling layout " << this << " cache size: " << fileCache.count() << endl;
	if( fileCache.contains(layoutKey) ) {
		kDebug() << "trashing old compiled layout for " << fileName << endl;
		if( fileCache[ layoutKey ] != NULL )
			fclose( fileCache[ layoutKey ] );	// recompiling - trash the old file
		fileCache.remove(fileName);
	}

	FILE *output = fopen(QFile::encodeName(fileName), "w");
		
    if ( output == NULL )
    {
		kWarning() << "Could not open " << fileName << " to precompile: " << strerror(errno) << endl;
        XkbFreeKeyboard(result.xkb, XkbAllControlsMask, True);
        return false;
    }

	if( !XkbWriteXKMFile(output, &result) ) {
		kWarning() << "Could not write compiled layout to " << fileName << endl;
		fclose(output);
        return false;
	}
	
	fclose(output);	// TODO: can we change mode w/out reopening?
	FILE *input = fopen(QFile::encodeName(fileName), "r");
	fileCache[ layoutKey ] = input;

	XkbFreeKeyboard(result.xkb, XkbAllControlsMask, True);
    return true;
}

/**
 * @brief takes layout from its compiled binary snapshot in file 
 *	and sets it as current
 * TODO: cache layout in memory rather than in file
 */
bool XKBExtension::setCompiledLayout(const QString &layoutKey)
{
	FILE *input = NULL;
	
	if( fileCache.contains(layoutKey) ) {
		input = fileCache[ layoutKey ];
	}
	
	if( input == NULL ) {
		kWarning() << "setCompiledLayout trying to reopen xkb file" << endl;	// should never happen
		const QString fileName = getPrecompiledLayoutFilename(layoutKey);
		input = fopen(QFile::encodeName(fileName), "r");
		
		// 	FILE *input = fopen(QFile::encodeName(fileName), "r");
		if ( input == NULL ) {
			kDebug() << "Unable to open " << fileName << ": " << strerror(errno) << endl;
			fileCache.remove(layoutKey);
			return false;
		}
	}
	else {
		rewind(input);
	}

    XkbFileInfo result;
    memset(&result, 0, sizeof(result));
	if ((result.xkb = XkbAllocKeyboard())==NULL) {
		kWarning() << "Unable to allocate memory for keyboard description" << endl;
//      fclose(input);
//		fileCache.remove(layoutKey);
    	return false;
	}
	
    unsigned retVal = XkmReadFile(input, 0, XkmKeymapLegal, &result);
    if (retVal == XkmKeymapLegal)
    {
        // this means reading the Xkm didn't manage to read any section
        kWarning() << "Unable to load map from file" << endl;
        XkbFreeKeyboard(result.xkb, XkbAllControlsMask, True);
        fclose(input);
		fileCache.remove(layoutKey);
		return false;
    }

	//    fclose(input);	// don't close - goes in cache

    if (XkbChangeKbdDisplay(m_dpy, &result) == Success)
    {
        if (!XkbWriteToServer(&result))
        {
            kWarning() << "Unable to write the keyboard layout to X display" << endl;
            XkbFreeKeyboard(result.xkb, XkbAllControlsMask, True);
            return false;
        }
    }
    else
    {
        kWarning() << "Unable prepare the keyboard layout for X display" << endl;
    }

    XkbFreeKeyboard(result.xkb, XkbAllControlsMask, True);
    return true;
}


// Deletes the precompiled layouts stored in temporary files
// void XKBExtension::deletePrecompiledLayouts()
// {
// 	QMapConstIterator<LayoutUnit, QString> it, end;
// 	end = m_compiledLayoutFileNames.end();
// 	for (it = m_compiledLayoutFileNames.begin(); it != end; ++it)
// 	{
// 		unlink(QFile::encodeName(it.data()));
// 	}
// 	m_compiledLayoutFileNames.clear();
// }
