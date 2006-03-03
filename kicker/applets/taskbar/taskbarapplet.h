/*****************************************************************

Copyright (c) 2001 Matthias Elter <elter@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#ifndef __taskbarapplet_h__
#define __taskbarapplet_h__

#include <kpanelapplet.h>

class TaskBar;

class TaskbarApplet : public KPanelApplet
{
    Q_OBJECT

public:
    TaskbarApplet( const QString& configFile, Plasma::Type t = Plasma::Normal, int actions = 0,
		   QWidget *parent = 0, const char *name = 0 );
    ~TaskbarApplet();

    int widthForHeight( int h ) const;
    int heightForWidth( int w ) const;

    void preferences();

protected:
    void popupDirectionChange( Plasma::Position );
    void orientationChange( Qt::Orientation );

private:
    TaskBar*     container;
};

#endif
