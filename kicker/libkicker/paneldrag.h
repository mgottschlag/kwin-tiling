/*****************************************************************
Copyright (c) 2004 Aaron J. Seigo <aseigo@kde.org>
              2004 Stephen Depooter <sbdep@woot.net>

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

#ifndef _paneldrag_h_
#define _paneldrag_h_

#include <q3dragobject.h>

#include <kdemacros.h>

#include "appletinfo.h"

class BaseContainer;

class KDE_EXPORT PanelDrag : public Q3DragObject
{
    public:
        PanelDrag(BaseContainer* container, QWidget *dragSource);
        ~PanelDrag();

        virtual const char * format(int i = 0) const;
        virtual QByteArray encodedData(const char *) const;

        static bool canDecode(const QMimeSource * e);
        static bool decode(const QMimeSource* e, BaseContainer** container);

    private:
        QByteArray a;
};

class KDE_EXPORT AppletInfoDrag : public Q3DragObject
{
    public:
        AppletInfoDrag(const AppletInfo& container, QWidget *dragSource);
        ~AppletInfoDrag();

        virtual const char * format(int i = 0) const;
        virtual QByteArray encodedData(const char *) const;

        static bool canDecode(const QMimeSource * e);
        static bool decode(const QMimeSource* e, AppletInfo& container);

    private:
        QByteArray a;
};

#endif

