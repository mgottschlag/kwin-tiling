// -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 8; -*-
/* This file is part of the KDE project

   Copyright (C) Andrew Stanley-Jones

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef _APPLET_H_
#define _APPLET_H_

#include <kpanelapplet.h>

#include "toplevel.h"

class KlipperAppletWidget;

class KlipperApplet : public KPanelApplet
{
  Q_OBJECT
public:
    KlipperApplet(const QString& configFile, Type t = Normal, int actions = 0,
                  QWidget *parent = 0, const char *name = 0);
    ~KlipperApplet();

    int widthForHeight(int h) const;
    int heightForWidth(int w) const;
protected:
    void resizeEvent( QResizeEvent* );
    void preferences();
    void help();
    void about();

private:
    void centerWidget();
    KlipperAppletWidget* widget;
};

class KlipperAppletWidget : public KlipperWidget
{
    Q_OBJECT
    K_DCOP
k_dcop:
    int newInstance();
public:
    KlipperAppletWidget( QWidget* parent = NULL );
    virtual ~KlipperAppletWidget();
private:
    void init();
    static DCOPClient* s_dcop;

};

#endif
