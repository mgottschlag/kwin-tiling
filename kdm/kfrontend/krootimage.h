/* This file is part of the KDE project
   Copyright (C) ???
   Copyright (C) 2002 Oswald Buddenhagen <ossi@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License version 2 as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef __KDMDESKTOP_H__
#define __KDMDESKTOP_H__


#include <kapplication.h>

#include <bgrender.h>


class MyApplication : public KApplication
{
  Q_OBJECT

public:
  MyApplication( const char * );

private slots:
  void renderDone();

private:
  KBackgroundRenderer renderer;
};

#endif
