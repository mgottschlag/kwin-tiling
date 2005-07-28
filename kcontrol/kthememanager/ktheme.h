// -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4; -*-
/*  Copyright (C) 2003 Lukas Tinkl <lukas@kde.org>

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
    Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef KTHEME_H
#define KTHEME_H

#include <qdom.h>
#include <qpointer.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qwidget.h>

#include <kurl.h>

class KStandardDirs;
class KConfig;

/// defines the syntax version used by the XML
#define SYNTAX_VERSION 1

/**
 * The base class representing a KDE theme. The data are stored internally
 * in a DOM tree and accessed using the member methods.
 *
 * @brief Class representing a theme
 * @author Lukas Tinkl <lukas@kde.org>
 */
class KTheme
{
public:
    /**
     * Constructs KTheme using an installed theme
     * @param xmlFile The theme's XML file
     */
    KTheme( QWidget *parent, const QString & xmlFile );

    /**
     * Constructs an empty theme, to be used with
     * #createYourself()
     * @param create Whether to start the DOM tree
     */
    KTheme( QWidget *parent, bool create = false );

    /**
     * Destructor
     */
    ~KTheme();

    /**
     * Download from @p url, unpack the tarball and load the theme definition
     *
     * @return true on success
     */
    bool load( const KURL & url );

    /**
     * Creates a snapshot of the current configuration in the work directory
     * (used for getting the defaults or to create a new user theme).
     * @param pack Whether to also pack the theme in tar.gz format
     * @return The path to the newly created tarball with theme (if @p pack == true)
     */
    QString createYourself( bool pack = false );

    /**
     * Apply the theme to the system, ie. set the config variables and
     * adjust file paths
     */
    void apply();

    /**
     * Uninstall the theme from the system
     * @param name The name of the theme
     * @return true on success
     */
    static bool remove( const QString & name );

    /**
     * @return the theme name
     */
    QString name() const { return m_name; }
    /**
     * Set the theme name
     */
    void setName( const QString & name );

    QString author() const {
        return getProperty( "author" );
    }
    void setAuthor( const QString & author );

    QString email() const {
        return getProperty( "email" );
    }
    void setEmail( const QString & email );

    QString homepage() const {
        return getProperty( "homepage" );
    }
    void setHomepage( const QString & homepage );

    QString comment() const {
        return getProperty( "comment" );
    }
    void setComment ( const QString & comment );

    QString version() const {
        return getProperty( "version" );
    }
    void setVersion ( const QString & version );

    /**
     * Creates a preview file called theme_name.preview.png
     * (i.e. takes a snapshot of the current desktop)
     */
    void addPreview();

private:
    /**
     * Create a property with @p name, value @p value
     * and append it to @p parent element
     */
    void setProperty( const QString & name,
                      const QString & value,
                      QDomElement parent );
    /**
     * Get a simple property from the "general" section of the DOM tree
     */
    QString getProperty( const QString & name ) const;

    /**
     * Get a property from the DOM tree, based on:
     * @param parent Parent tag
     * @param tag From the this tag
     * @param attr From this attribute
     */
    QString getProperty( QDomElement parent, const QString & tag,
                         const QString & attr ) const;

    /**
     * Creates a list of "icon" elements based on:
     * @param group The group in the KConfig object @p cfg
     * @param object Specifier (similiar, but not identical to @p group)
     * @param parent Parent element to append to
     * @param cfg The KConfig object to work with
     */
    void createIconElems( const QString & group, const QString & object,
                          QDomElement parent, KConfig * cfg );

    /**
     * Creates a color DOM element @p name, with a specifier @p object,
     * appends it to @p parent; used when creating themes
     * @param cfg The KConfig object to work with
     */
    void createColorElem( const QString & name, const QString & object,
                          QDomElement parent, KConfig * cfg );
    /**
     * Creates a list of "event" elements based on:
     * @param events The list of events to work on
     * @param object Specifier (currently "global" or "kwin")
     * @param parent Parent element to append to
     * @param cfg The KConfig object to work with
     */
    void createSoundList( const QStringList & events, const QString & object,
                          QDomElement parent, KConfig * cfg );

    /**
     * Tries to find out absolute path to a resource and copy it to the theme's temp dir;
     * used when creating themes
     * @param section The theme section to work on, corresponds to toplevel XML tags
     * @param path The original path, relative or absolute
     * @return an internal path suitable for writing into the XML file or QString::null
     * in case the resource couldn't be found
     */
    QString processFilePath( const QString & section, const QString & path );

    /**
     * Converts an internal theme:/ representation of a resource
     * to a real path
     */
    QString unprocessFilePath( const QString & section, QString path );

    /**
     * Wrapper around KIO::NetAccess::file_copy
     */
    bool copyFile( const QString & from, const QString & to );

    /**
     * Wrapper around KGlobal::dirs()->findResource()
     * @param section Section to work on (desktop, sounds, panel etc)
     * @param path The file to find
     */
    QString findResource( const QString & section, const QString & path );

    /// name of the theme
    QString m_name;

    /// DOM holding the theme
    QDomDocument m_dom;
    /// the DOM root element
    QDomElement m_root;
    /// "general" section
    QDomElement m_general;

    KStandardDirs * m_kgd;

    QPointer<QWidget> m_parent;
};

#endif
