/*
    kshorturifilter.h

    This file is part of the KDE project
    Copyright (C) 2000 Yves Arrouye <yves@realnames.com>
    Copyright (C) 2000 Dawit Alemayehu <adawit@earthlink.net>

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

#ifndef _KSHORTURIFILTER_H_
#define _KSHORTURIFILTER_H_

#include <qmap.h>

#include <dcopobject.h>
#include <klibloader.h>
#include <kurifilter.h>

class KInstance;

class KShortURIFilter : public KURIFilterPlugin , public DCOPObject
{
    K_DCOP

public:
    enum CmdType { URL, Executable, Shell, Help, Error, Unknown };

    KShortURIFilter( QObject *parent = 0, const char *name = 0 );
    virtual ~KShortURIFilter();

    virtual bool filterURI( KURL &uri );
    CmdType type() const { return m_pCmdType; }
    QString errorMessage() const { return m_strError; }

    virtual KCModule *configModule( QWidget *parent = 0, const char *name = 0 ) const;
    virtual QString configName() const;

public:
k_dcop:
    virtual void configure() { return; }

protected:
    /**
     *  Function determines whether what the user entered in the
     *  KShortURIFilter box is executable or not.
     */
    bool isExecutable ( const QString& ) const;

    bool isValidShortURL ( const QString& ) const;

    /**
     * Parses cmd to determine its type
     * Possible return values are:
     * URL - cmd is a URL
     * Executable - cmd is a local executable with no parameters (for now)
     * Shell - anything else should be run through a shell
     * Help - should be passed to kdehelp
     * Error - cmd is invald (for now only if it's ~username and no home dir could be found)
     * in case of an error, cmd is set to the appropriate message, otherwise it's qualified
     */
    KShortURIFilter::CmdType parseCmd( QString& );

private:
    KShortURIFilter::CmdType m_pCmdType;
    QMap<QString, QString> m_urlHints;
    QString m_strError;
};


class KShortURIFilterFactory : public KLibFactory
{
    Q_OBJECT

public:
    KShortURIFilterFactory( QObject *parent = 0, const char *name = 0 );
    ~KShortURIFilterFactory();

    virtual QObject *create( QObject *parent = 0, const char *name = 0, const char* classname = "QObject", const QStringList &args = QStringList() );

    static KInstance *instance();

private:
    static KInstance *s_instance;

};
#endif
