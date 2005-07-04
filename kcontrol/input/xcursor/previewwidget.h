/*
 * Copyright (C) 2003 Fredrik Höglund <fredrik@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */


#ifndef __CURSORPREVIEW_H
#define __CURSORPREVIEW_H


class PreviewCursor;


class PreviewWidget : public QWidget
{
	public:
		PreviewWidget( QWidget *parent = NULL, const char *name = NULL );
		~PreviewWidget();

		void setTheme( const QString & );

		void paintEvent( QPaintEvent * );
		void mouseMoveEvent( QMouseEvent * );

	private:
		PreviewCursor **cursors;
		int current;
}; // class CursorPreview



#endif

// vim: set noet ts=4 sw=4:
