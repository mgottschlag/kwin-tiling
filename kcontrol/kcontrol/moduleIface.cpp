/*
  Copyright (c) 2001 Daniel Molkentin <molkentin@kde.org>
 
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

#include "moduleIface.h"
#include "moduleIface.moc"

ModuleIface::ModuleIface(QObject *parent, const char *name) 
	: QObject(parent, name), DCOPObject(name) {

	_parent = static_cast<QWidget *>(parent);

}

ModuleIface::~ModuleIface() {
}

QFont ModuleIface::getFont() {
	return _parent->font(); 
}

QPalette ModuleIface::getPalette(){
	return _parent->palette();
}

void ModuleIface::invokeHelp() {
	emit helpClicked();
}

