    /*

    Merge some options from kdmrc into kdm-config

    $Id$

    Copyright (C) 2001 Oswald Buddenhagen <ossi@kde.org>


    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>

#include <qfile.h>

#include <kapp.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <ksimpleconfig.h>

#include <X11/Xlib.h>
#include <X11/Xresource.h>

static XrmQuark XrmQString;

static void PrintBindingQuarkList(XrmBindingList bindings, XrmQuarkList quarks)
{
    Bool	firstNameSeen;

    for (firstNameSeen = False; *quarks; bindings++, quarks++) {
	if (*bindings == XrmBindLoosely) {
	    (void) putchar('*');
	} else if (firstNameSeen) {
	    (void) putchar('.');
	}
	firstNameSeen = True;
	(void) fputs(XrmQuarkToString(*quarks), stdout);
    }
}

/*ARGSUSED*/
static Bool DumpEntry(
    XrmDatabase		*db,
    XrmBindingList      bindings,
    XrmQuarkList	quarks,
    XrmRepresentation   *type,
    XrmValuePtr		value,
    XPointer		data)
{
    register unsigned int	i;
    register char		*s;
    register char		c;

    if (*type != XrmQString)
	return False;
    PrintBindingQuarkList(bindings, quarks);
    s = value->addr;
    i = value->size;
    if (i)
	i--;
    (void) fputs(":\t", stdout);
    if (i && (*s == ' ' || *s == '\t'))
	(void) putchar('\\'); /* preserve leading whitespace */
    while (i--) {
	c = *s++;
	if (c == '\n') {
	    if (i)
		(void) fputs("\\n\\\n", stdout);
	    else
		(void) fputs("\\n", stdout);
	} else if (c == '\\')
	    (void) fputs("\\\\", stdout);
	else if ((c < ' ' && c != '\t') ||
		 ((unsigned char)c >= 0x7f && (unsigned char)c < 0xa0))
	    (void) printf("\\%03o", (unsigned char)c);
	else
	    (void) putchar(c);
    }
    (void) putchar('\n');
    return False;
}

int main(int argc, char **argv)
{
    XrmDatabase DB;
    XrmValue value;
    char *type;

    if (argc != 3) {
	fprintf(stderr, "This program is part of kdm and should not be run manually.\n");
	return 1;
    }

    XrmInitialize ();
    XrmQString = XrmPermStringToQuark("String");
    if (!(DB = XrmGetFileDatabase (argv[1])))
	return 10;

    int margc = 1;
    char *margv[2] = {argv[0], 0};
    KApplication app(margc, margv, QCString("kdm_getcfg"), false, false);

    QString fn = argv[2][0] ? 
	QFile::decodeName(argv[2]) : 
	KGlobal::dirs()->resourceDirs("config").last() + 
		QString::fromLatin1("kdmrc");

    QCString ffn = QFile::encodeName(fn);
    struct stat st;
    if (stat(ffn.data(), &st))
	return 11;
    printf("%s %ld\n", ffn.data(), st.st_mtime);

#   define putr(k, v) \
	XrmPutStringResource (&DB, "DisplayManager." k, v);

    KSimpleConfig c(fn, true);
    c.setGroup("KDM");

    if (c.readBoolEntry( "AutoLoginEnable", false)) {
	QCString autoUser = c.readEntry( "AutoLoginUser" ).local8Bit();
	putr("_0.autoUser", autoUser.data());
	putr("_0.autoPass", "");
	putr("_0.autoString", "");
	QCString autoLogin1st = c.readEntry( "AutoLogin1st", 
				    QString::fromLatin1("true")).local8Bit();
	putr("_0.autoLogin1st", autoLogin1st.data());
    }

    if (c.readBoolEntry( "NoPassEnable", false)) {
	XrmGetResource(DB, "DisplayManager._0.noPassUsers",
			"DisplayManager._0.NoPassUsers", &type, &value);
	QCString noPassUsers = (c.readEntry( "NoPassUsers" ) + 
	    QString::fromLatin1(",") + 
	    QString::fromLocal8Bit(value.addr)).local8Bit();
	putr("_0.noPassUsers", noPassUsers.data());
    }

    if (c.readBoolEntry( "AutoReLogin", false))
	putr("?.autoReLogin", "true");

    XrmQuark empty = NULLQUARK;
    if (XrmEnumerateDatabase(DB, &empty, &empty, XrmEnumAllLevels,
			     DumpEntry, (XPointer) 0))
    return 0;
}
