/*

Config for kdm

Copyright (C) 1997, 1998 Steffen Hansen <hansen@kde.org>
Copyright (C) 2000-2003 Oswald Buddenhagen <ossi@kde.org>


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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include "kdmconfig.h"
#include "kdm_greet.h"
#include "utils.h"

#include <kconfiggroup.h>
#include <kglobal.h>
#include <klocale.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/utsname.h>

#define WANT_GREET_DEFS
#include <config.ci>

CONF_GREET_DEFS

QString _stsFile;
bool _isLocal;
bool _isReserve;
bool _authorized;
int _grabInput;

static QString
getCfgQStr(int id)
{
    return qString(getCfgStr(id));
}

static QStringList
getCfgQStrList(int id)
{
    return qStringList(getCfgStrArr(id, 0));
}

// Based on kconfiggroupgui.cpp
static QFont *
str2Font(const QString &aValue)
{
    QFont *aRetFont = new QFont();

    QStringList sl = aValue.split(QString::fromLatin1(","), QString::SkipEmptyParts);
    if (sl.count() == 1) {
        /* X11 font spec */
        aRetFont->setRawMode(true);
        aRetFont->setRawName(aValue);
    } else {
        aRetFont->fromString(aValue);
    }
    aRetFont->setStyleStrategy((QFont::StyleStrategy)
       (QFont::PreferMatch |
        (_antiAliasing ? QFont::PreferAntialias : QFont::NoAntialias)));

    return aRetFont;
}

extern "C"
void initConfig(void)
{
    CONF_GREET_INIT

    _isLocal = getCfgInt(C_isLocal);
    _isReserve = _isLocal && getCfgInt(C_isReserve);
    _hasConsole = _hasConsole && _isLocal && getCfgInt(C_hasConsole);
    _authorized = getCfgInt(C_isAuthorized);
    _grabInput =
        (_grabInputPre == GRAB_NEVER) ? 0 :
        (_grabInputPre == GRAB_ALWAYS) ? 1 :
        !_authorized;

    QByteArray dd = _dataDir.toUtf8();
    if (access(dd.constData(), W_OK))
        logError("Data directory %\"s not accessible: %m\n", dd.constData());
    _stsFile = _dataDir + "/kdmsts";

    // Greet String
    char hostname[256], *ptr;
    hostname[0] = '\0';
    if (!gethostname(hostname, sizeof(hostname)))
        hostname[sizeof(hostname)-1] = '\0';
    struct utsname tuname;
    uname(&tuname);
    QString gst = _greetString;
    _greetString.clear();
    int i, j, l = gst.length();
    for (i = 0; i < l; i++) {
        if (gst[i] == '%') {
            switch (gst[++i].cell()) {
            case '%': _greetString += gst[i]; continue;
            case 'd': ptr = dname; break;
            case 'h': ptr = hostname; break;
            case 'n': ptr = tuname.nodename;
                for (j = 0; ptr[j]; j++)
                    if (ptr[j] == '.') {
                        ptr[j] = 0;
                        break;
                    }
                break;
            case 's': ptr = tuname.sysname; break;
            case 'r': ptr = tuname.release; break;
            case 'm': ptr = tuname.machine; break;
            default: _greetString += i18nc("@item:intext substitution for "
                                           "an undefined %X placeholder wrongly "
                                           "given in the config file 'kdmrc', "
                                           "telling the user to fix it",
                                           "[fix kdmrc]"); continue;
            }
            _greetString += QString::fromLocal8Bit(ptr);
        } else {
            _greetString += gst[i];
        }
    }
}

void initQAppConfig(void)
{
    CONF_GREET_INIT_QAPP

    KConfigGroup cfg(KGlobal::config(), "General");
    cfg.writeEntry("nopaletteChange", true);
    cfg.writeEntry("font", *_normalFont);
    if (!_GUIStyle.isEmpty())
        cfg.writeEntry("widgetStyle", _GUIStyle);
}

