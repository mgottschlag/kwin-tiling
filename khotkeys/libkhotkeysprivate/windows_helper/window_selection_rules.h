#ifndef WINDOW_SELECTION_RULES_H
#define WINDOW_SELECTION_RULES_H
/* Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>
   Copyright (C) 2008 Michael Jansen <kde@michael-jansen.biz>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   Version 2 as published by the Free Software Foundation;

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "windows_helper/window_selection_interface.h"


namespace KHotKeys {

/**
 * Rules to match a windows.
 */
class KDE_EXPORT Windowdef_simple : public Windowdef
    {

    typedef Windowdef base;

    public:

        /**
         * How to compare.
         */
        enum substr_type_t
            {
            NOT_IMPORTANT,
            CONTAINS,
            IS,
            REGEXP,
            CONTAINS_NOT,
            IS_NOT,
            REGEXP_NOT
            };

        /**
         * Window Type
         */
        enum window_type_t
            {
            WINDOW_TYPE_NORMAL     = ( 1 << NET::Normal ),
            WINDOW_TYPE_DESKTOP    = ( 1 << NET::Desktop ),
            WINDOW_TYPE_DOCK       = ( 1 << NET::Dock ),
            WINDOW_TYPE_DIALOG     = ( 1 << NET::Dialog )
            };

        /**
         * Constructor
         */
        Windowdef_simple(
                const QString& comment_P = QString(),
                const QString& title_P = QString(),
                substr_type_t _title_type_P = NOT_IMPORTANT,
                const QString& wclass_P = QString(),
                substr_type_t wclass_type_P = NOT_IMPORTANT,
                const QString& role_P = QString(),
                substr_type_t role_type_P = NOT_IMPORTANT,
                int window_types_P = 0 );

        /**
         * Create from a configuration file.
         */
        Windowdef_simple( KConfigGroup& cfg_P );

        /**
         * Match agains window @p window_P
         */
        virtual bool match( const Window_data& window_P );

        /**
         * Write to configuration file @p cfg_P
         */
        virtual void cfg_write( KConfigGroup& cfg_P ) const;

        /**
         * The string to compare with the window title
         */
        const QString& title() const;
        void set_title(const QString &title);

        /**
         * How to match the window title
         */
        substr_type_t title_match_type() const;
        void set_title_match_type(const substr_type_t &type);

        /**
         * The string to compare with the window class
         */
        const QString& wclass() const;
        void set_wclass(const QString &wclass);

        /**
         * How to match the window class
         */
        substr_type_t wclass_match_type() const;
        void set_wclass_match_type(const substr_type_t &);

        /**
         * The string to compare with the window role
         */
        const QString& role() const;
        void set_role(const QString &role);

        /**
         * How to match the window type
         */
        substr_type_t role_match_type() const;
        void set_role_match_type(const substr_type_t &type);

        /**
         * The window types to match
         */
        int window_types() const;
        void set_window_types(const int types);

        /**
         *
         */
        bool type_match( window_type_t type_P ) const;

        /**
         */
        bool type_match( NET::WindowType type_P ) const;

        /**
         * Create a copy
         */
        virtual Windowdef_simple* copy( /*ActionDataBase* data_P*/ ) const;

        /**
         * The description of this rule.
         *
         * @todo: Move to base class?
         */
        virtual const QString description() const;

    protected:

        /**
         */
        bool is_substr_match(
                const QString& str1_P,
                const QString& str2_P,
                substr_type_t type_P );

    private:

        //! The title string
        QString _title;

        //! How to use the title string
        substr_type_t _title_match_type;

        //! The class string
        QString _wclass;

        //! How to use the class string
        substr_type_t _wclass_match_type;

        //! The role string
        QString _role;

        //! How to use the role string
        substr_type_t _role_match_type;

        //! Which window types to match
        int _window_types;
    };


} // namespace KHotKeys


#endif /* #ifndef WINDOW_SELECTION_RULES_H */
