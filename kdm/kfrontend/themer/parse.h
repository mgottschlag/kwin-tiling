/*
 *  Copyright (C) 2003 by Unai Garro <ugarro@users.sourceforge.net>
 *  Copyright (C) 2004 by Enrico Ros <rosenric@dei.unipd.it>
 *  Copyright (C) 2004 by Stephan Kulow <coolo@kde.org>
 *  Copyright (C) 2004,2006 by Oswald Buddenhagen <ossi@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef PARSE_H
#define PARSE_H

#include <kdebug.h>

#include <QFont>
#include <QPalette>

class QString;
class QColor;
class QDomElement;

enum DataType { DTnone, DTpixel, DTnpixel, DTpercent, DTbox, DTscale };
struct DataPoint {
	int val, levels;
	DataType type;
};

struct FontType {
	QFont font;
	bool present;

	FontType() : present( false )
	{
	}
};

struct StyleType {
	QPalette palette;
	FontType font, editfont;
	bool frame;
};

void parseSize( const QString &, DataPoint & );
void parseFont( const QString &, FontType & );
void parseFont( const QDomElement &, FontType & );
bool parseColor( const QString &color, const QString &alpha, QColor & );
void parseColor( const QDomElement &, QColor & );
void parseStyle( const QDomElement &, StyleType & );

void setWidgetAttribs( QWidget *widget, const StyleType &style, bool frame );

enum NoSpaceDebug { NoSpace };
enum SpaceDebug { Space };
static inline QDebug operator<<(QDebug ds, NoSpaceDebug) { return ds.nospace(); }
static inline QDebug operator<<(QDebug ds, SpaceDebug) { return ds.space(); }
QDebug enter( const char *fct );
QDebug debug();
QDebug leave();

#endif
