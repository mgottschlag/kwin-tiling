/*
  Copyright (c) 2000 Matthias Elter <elter@kde.org>
 
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

#include <stdlib.h> 
#include <unistd.h>
#include <sys/types.h>

#include "global.h"

bool KCGlobal::_root = false;
bool KCGlobal::_system = false;
QString KCGlobal::_uname = "";
QString KCGlobal::_hname = "";

void KCGlobal::init()
{
  QString hostname, username;
  char buf[128];
  char *user = getlogin();
  
  gethostname(buf, 128);
  if (strlen(buf)) hostname = QString("%1").arg(buf); else hostname = "";
  if (!user) user = getenv("LOGNAME");
  if (!user) username = ""; else username = QString("%1").arg(user);

  setHostName(hostname);
  setUserName(username);
  setRoot(getuid() == 0);
}
