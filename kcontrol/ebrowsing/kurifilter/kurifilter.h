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

#ifndef __kurifilter_h__
#define __kurifilter_h__ "$Id$"

#include <qlist.h>

#include <kurifilterplugin.h>

class KURIFilterPluginList : public QList<KURIFilterPlugin> {
public:
    virtual int compareItems(Item a, Item b) {
	double diff = ((KURIFilterPlugin *) a)->priority() - ((KURIFilterPlugin *) b)->priority();
	return diff < 0 ? -1 : (diff > 0 ? 1 : 0);
    }
};

/**
 * Manage the filtering of a URI.
 *
 * The KURIFilter class applies a number of filters to a URI, and returns
 * the filtered URI. The filters are implemented by plugins that provide
 * easy extensibility of the filtering mechanism.
 *
 */

class KURIFilter {
public:
    /**
     * Create a KURIFIlter.
     * The filter will be initialized with all the plugins it can find.
     */
    KURIFilter();

    /**
     * Return a static instance of KURIFilter.
     */
    static KURIFilter *filter();

    /**
     * Filter a URI.
     * @param uri The URI to filter.
     * @return A boolean indicating whether the URI has been changed
     * or not.
     */
    bool filterURI(QString &uri);

    /**
     * Return a filtered URI.
     * @param uri The URI to filter.
     * @return The filtered URI.
     */
    QString filteredURI(const QString &uri);

protected:
    void loadPlugins();

private:
    static KURIFilter *ms_pFilter;
    KURIFilterPluginList m_lstPlugins;
};

#endif

