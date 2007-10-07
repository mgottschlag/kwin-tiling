/*
    Copyright (C) 2001, S.R.Haque <srhaque@iee.org>. 
	Copyright (C) 2006, Andriy Rysin <rysin@kde.org>. Derived from an
    original by Matthias H�zer-Klpfel released under the QPL.
    This file is part of the KDE project

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

DESCRIPTION

    KDE Keyboard Tool. Manages XKB keyboard mappings.
*/
#ifndef __KXKB_COMPONENT_H__
#define __KXKB_COMPONENT_H__

#include <QLibrary>

/**
    Lightweigth (no pars or io libs required) Kxkb component
*/
extern "C" KDE_EXPORT void* kxkb_create_component(int controlType, void* parentWidget);
typedef void* (*kxkb_create_component_t)(int, void*);
enum { KXKB_INDICATOR_ONLY=1, KXKB_NO_MENU = 2, KXKB_MENU_LAYOUTS_ONLY = 3, KXKB_MENU_FULL=4 };

#define KXKB_CREATE_COMPONENT(type, parent) { \
    QLibrary* kxkbLib = new QLibrary("kdeinit4_kxkb"); \
    kxkb_create_component_t kxkb_create_component = (kxkb_create_component_t)kxkbLib->resolve("kxkb_create_component"); \
    if ( kxkb_create_component ) { \
	QObject* kxkbObject = (QObject*)kxkb_create_component((type), (parent)); \
	QWidget* kxkbWidget = dynamic_cast<QWidget*>(kxkbWidget); \
	if( kxkbWidget == NULL ) \
	    kError() << "failed to create kxkb component"; \
    } \
    else \
	kDebug() << "can't load kxkb component library"; \
    delete kxkbLib; \
  }

#endif
