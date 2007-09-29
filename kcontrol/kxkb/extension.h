/*
 *  Copyright (C) 2003-2006 Andriy Rysin (rysin@kde.org)
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

#ifndef __EXTENSION_H__
#define __EXTENSION_H__

#include <X11/Xlib.h>


class XKBExtension
{
public:
	XKBExtension(Display *display=NULL);
	~XKBExtension();
	bool init();
	void reset();

	static bool setXkbOptions(const QString& options, bool resetOldOptions);
	bool setLayout(const QString& model,
					const QString& layout, const QString& variant,
					const QString& includeGroup, bool useCompiledLayouts=true);
	bool setLayout(const QString& layouts, const QString& variants);
	bool setGroup(unsigned int group);
	unsigned int getGroup() const;

	static bool isGroupSwitchEvent(XEvent* event);
	static bool isLayoutSwitchEvent(XEvent* event);	

    int xkb_opcode;

private:
    Display *m_dpy;
/*	QString m_tempDir;
	static QMap<QString, FILE*> fileCache;*/
	
	bool setLayoutInternal(const QString& model,
				   const QString& layout, const QString& variant,
				   const QString& includeGroup);
// 	bool compileCurrentLayout(const QString& layoutKey);
// 	bool setCompiledLayout(const QString& layoutKey);
	
// 	QString getPrecompiledLayoutFilename(const QString& layoutKey);
//	void deletePrecompiledLayouts();
};

#endif
