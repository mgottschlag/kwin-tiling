/* This file is part of the KDE libraries
 *  Copyright (C) 2000 Yves Arrouye <yves@realnames.com>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#ifndef __kurifilterplugin_h__
#define __kurifilterplugin_h__ "$Id$"

#include <qobject.h>
#include <kurl.h>

class QWidget;
class KCModule;

/**
 * Filters a URI.
 *
 * The KURIFilterPlugin class applies a single filter to a URI.
 *
 */

class KURIFilterPlugin : public QObject {
    Q_OBJECT

public:
    /**
     * Constructor to create a filter plugin with a given name and
     * priority.
     * @param pname The name of the plugin.
     * @param pri The priority of the plugin.
     *
     */
    KURIFilterPlugin(QObject *parent = 0, const char *name = 0, const QString &pname = QString::null, double pri = 1.0);

    /**
     * Return the filter's name.
     * @return A string naming the filter.
     */
    virtual QString name() const;

    /**
     * Return the filter's .
     * Each filter has an assigned priority, a float from 0 to 1. Filters
     * with the lowest priority are first given a chance to filter a URI.
     * @return The priority of the filter.
     */
    virtual double priority() const;

    /**
     * Filter a URI.
     * @param uri The URI to be filtered.
     * @return A boolean indicating whether the URI has been changed
     * or not.
     */
    virtual bool filterURI(KURL &uri) const = 0;
    /**
     * Filter a string representing a URI.
     * @param uri The URI to be filtered.
     * @return A boolean indicating whether the URI has been changed
     * or not.
     */
    virtual bool filterURI(QString &uri) const;

    /**
     * Return a configuration module for the filter.
     * It is the responsability of the caller to delete the module
     * once it is not needed anymore.
     * @return A configuration module, or 0 if the filter isn't
     * configurable.
     */
    virtual KCModule *configModule(const QWidget *parent = 0, const char *name = 0) const;

    /**
     * Return a configuration module for the filter.
     * It is the responsability of the caller to delete the module
     * once it is not needed anymore.
     * @return A configuration module, or 0 if the filter isn't
     * configurable.
     */
    virtual QString configName() const;

protected:
    QString m_strName;
    double m_dblPriority;
};

#endif

