/*
    kshorturifilter.h

    This file is part of the KDE project
    Copyright (C) 2000 Yves Arrouye <yves@realnames.com>
    Copyright (C) 2000 Dawit Alemayehu <adawit@earthlink.net>
    Copyright (C) 2000 Malte Starostik <starosti@zedat.fu-berlin.de>

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
#include <klocale.h>
#include <klibloader.h>
#include <kurifilter.h>


class KInstance;

class KShortURIFilter : public KURIFilterPlugin , public DCOPObject
{
    K_DCOP
public:
    KShortURIFilter( QObject *parent = 0, const char *name = 0 );

    /**
     * Destructor
     */
    virtual ~KShortURIFilter() {};

    /**
     * Converts short URIs into fully qualified valid URI.
     *
     * Parses any given invalid URI to determine whether it
     * is a known short URI and converts it to its fully
     * qualified version.
     *
     * @param data the data to be filtered
     * @return true if the url has been filtered
     */
    virtual bool filterURI( KURIFilterData &data ) const;
    virtual QString configName() const { return i18n("ShortURIFilter"); }

public:
k_dcop:
    virtual void configure() { return; }

protected:

    /**
     * Determines the validity of the URI from
     *
     * Parses any given invalid URI to determine whether its
     * signature is a possible match for a short URI.
     *
     * @param
     * @param
     * @return
     */
    bool isValidShortURL ( const QString& ) const;

private:
    QMap<QString, QString> m_urlHints;
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
