/***************************************************************************
 *   Copyright (c) 2008  Laurent Montel <montel@kde.org>                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#ifndef KEPHAL_EXPORT_H
#define KEPHAL_EXPORT_H


#include <qglobal.h>

#ifdef Q_WS_WIN
# if defined(MAKE_KEPHAL_LIB)
#  define KEPHAL_EXPORT Q_DECL_EXPORT
# else
#  define KEPHAL_EXPORT Q_DECL_IMPORT
# endif
#else
# define KEPHAL_EXPORT Q_DECL_EXPORT
#endif 

#endif
