//-----------------------------------------------------------------------------
//
// KDE xscreensaver launcher
//
// Copyright (c)  Martin R. Jones <mjones@kde.org> 1999
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation;
// version 2 of the License.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; see the file COPYING.  If not, write to
// the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <qlist.h>
#include <qtextstream.h>
#include <kapp.h>
#include <kconfig.h>
#include <kstddirs.h>
#include "kxsitem.h"

#define MAX_ARGS  20

template class QList<KXSConfigItem>;

//===========================================================================

void usage(const char *name)
{
  printf("kxsconfig - xscreensaver launcher\n");
  printf("Usage: %s xscreensaver-filename [xscreensaver-options]\n", name);
}

//===========================================================================
int main(int argc, char *argv[])
{
  KApplication app(argc, argv, "KXSConfig", false);

  if (argc < 2 || argv[1][0] == '-')
  {
    usage(argv[0]);
    exit(1);
  }

  QString filename(argv[1]);
  QString configFile(filename);

  // Get the config filename
  int slash = filename.findRev('/');
  if (slash >= 0)
  {
    configFile = filename.mid(slash+1);
  }

  configFile += "rc";

  // read configuration args
  KConfig config(configFile);

  QList<KXSConfigItem> configItemList;
  int idx = 0;

  while (true)
  {
    QString group = QString("Arg%1").arg(idx);
    if (config.hasGroup(group))
    {
      config.setGroup(group);
      QString type = config.readEntry("Type");
      if (type == "Range")
      {
        KXSRangeItem *rc = new KXSRangeItem(group, config);
        configItemList.append(rc);
      }
      else if (type == "DoubleRange")
      {
        KXSDoubleRangeItem *rc = new KXSDoubleRangeItem(group, config);
        configItemList.append(rc);
      }
      else if (type == "Check")
      {
        KXSBoolItem *cc = new KXSBoolItem(group, config);
        configItemList.append(cc);
      }
      else if (type == "DropList")
      {
        KXSSelectItem *si = new KXSSelectItem(group, config);
        configItemList.append(si);
      }
    }
    else
    {
      break;
    }
    idx++;
  }

  // find the xscreensaver executable
  QString exeFile = KStandardDirs::findExe(filename);

  if (!exeFile.isEmpty())
  {
    char *sargs[MAX_ARGS];
    sargs[0] = new char [strlen(filename.ascii())+1];
    strcpy(sargs[0], filename.ascii());

    // add the command line options
    QString cmd;
    int i;
    for (i = 2; i < argc; i++)
    {
      cmd += " " + QString(argv[i]);
    }

    // add the config options
    KXSConfigItem *item;
    for (item = configItemList.first(); item != 0; item = configItemList.next())
    {
      cmd += " " + item->command();
    }

    debug("Command: %s", cmd.ascii());

    // put into char * array for execv
    QTextStream ts(&cmd, IO_ReadOnly);
    QString word;
    i = 1;
    while (!ts.atEnd() && i < MAX_ARGS-1)
    {
      ts >> word;
      word = word.stripWhiteSpace();
      if (!word.isEmpty())
      {
        sargs[i] = new char [strlen(word.ascii())+1];
        strcpy(sargs[i], word.ascii());
        i++;
      }
    }

    sargs[i] = 0;

    // here goes...
    execv(exeFile.ascii(), sargs);
  }
}


