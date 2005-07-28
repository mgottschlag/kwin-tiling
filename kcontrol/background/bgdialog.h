/*
   This file is part of the KDE libraries
   Copyright (c) 2003 Waldo Bastian <bastian@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef _BGDIALOG_H_
#define _BGDIALOG_H_

#include <q3ptrvector.h>
#include <qmap.h>

#include "bgdialog_ui.h"
#include "bgrender.h"
#include "bgsettings.h"
#include "bgdefaults.h"

class BGMonitor;
class KStandardDirs;

class BGDialog : public BGDialog_UI
{
   Q_OBJECT
public:
   BGDialog(QWidget* parent, KConfig* _config, bool _multidesktop = true);
   ~BGDialog();

   void load();
   void save();
   void defaults();

   void makeReadOnly();

   QString quickHelp() const;

signals:
   void changed(bool);

protected:
   void initUI();
   void updateUI();

   void setWallpaper(const QString &);

   void loadWallpaperFilesList();

protected slots:
   void slotSelectDesk(int desk);
   void slotWallpaperTypeChanged(int i);
   void slotWallpaper(int i);
   void slotWallpaperPos(int);
   void slotWallpaperSelection();
   void slotSetupMulti();
   void slotPrimaryColor(const QColor &color);
   void slotSecondaryColor(const QColor &color);
   void slotPattern(int pattern);
   void slotImageDropped(const QString &uri);
   void slotPreviewDone(int);
   void slotAdvanced();
   void slotGetNewStuff();
   void slotBlendMode(int mode);
   void slotBlendBalance(int value);
   void slotBlendReverse(bool b);
   void desktopResized();
   void setBlendingEnabled(bool);

protected:
   KGlobalBackgroundSettings *m_pGlobals;
   KStandardDirs *m_pDirs;
   bool m_multidesktop;

   int m_Max;
   int m_Desk;
   int m_eDesk;
   Q3PtrVector<KBackgroundRenderer> m_Renderer;
   QMap<QString,int> m_Wallpaper;
   QStringList m_Patterns;
   int m_slideShowRandom; // Remembers last Slide Show setting
   int m_wallpaperPos; // Remembers last wallpaper pos

   BGMonitor *m_pMonitor;

   bool m_previewUpdates;
   bool m_copyAllDesktops;
};

#endif
