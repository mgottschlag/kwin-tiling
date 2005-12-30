#include "desktop.h"

#ifndef QT_NO_OPENGL
#include <qgl.h>
#endif

#include <QtDebug>

#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>


namespace Plasma
{

Desktop::Desktop()
{
    initializeDisplay();
}

Desktop::~Desktop()
{
}

void Desktop::manage()
{

}

void Desktop::initializeDisplay()
{
#ifndef QT_NO_OPENGL
    QGLFormat format = QGLContext::currentContext()->format();
    qDebug()<<"direct rendering = " << format.directRendering();
#else

#endif
}

}

