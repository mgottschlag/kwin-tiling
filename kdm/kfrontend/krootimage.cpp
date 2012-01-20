/*

Copyright (C) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
Copyright (C) 2002,2004 Oswald Buddenhagen <ossi@kde.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; see the file COPYING.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.

*/

#include "krootimage.h"

#include <bgdefaults.h>

#include <kcmdlineargs.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kcomponentdata.h>
#include <klocale.h>

#include <QDesktopWidget>
#include <QFile>
#include <QHash>
#include <QPainter>
#include <QX11Info>

#include <X11/Xlib.h>

#include <stdlib.h>

KVirtualBGRenderer::KVirtualBGRenderer(const KSharedConfigPtr &config)
{
    m_pPixmap = 0;
    m_numRenderers = 0;
    m_scaleX = 1;
    m_scaleY = 1;

    m_pConfig = config;

    initRenderers();
    m_size = QApplication::desktop()->size();
}

KVirtualBGRenderer::~KVirtualBGRenderer()
{
    for (int i = 0; i < m_numRenderers; ++i)
        delete m_renderer[i];

    delete m_pPixmap;
}

KBackgroundRenderer *
KVirtualBGRenderer::renderer(unsigned screen)
{
    return m_renderer[screen];
}

QPixmap
KVirtualBGRenderer::pixmap()
{
    if (m_numRenderers == 1)
        return m_renderer[0]->pixmap();

    return *m_pPixmap;
}

bool
KVirtualBGRenderer::needProgramUpdate()
{
    for (int i = 0; i < m_numRenderers; i++)
        if (m_renderer[i]->backgroundMode() == KBackgroundSettings::Program &&
            m_renderer[i]->KBackgroundProgram::needUpdate())
            return true;
    return false;
}

void
KVirtualBGRenderer::programUpdate()
{
    for (int i = 0; i < m_numRenderers; i++)
        if (m_renderer[i]->backgroundMode() == KBackgroundSettings::Program &&
            m_renderer[i]->KBackgroundProgram::needUpdate())
        {
            m_renderer[i]->KBackgroundProgram::update();
        }
}

bool
KVirtualBGRenderer::needWallpaperChange()
{
    for (int i = 0; i < m_numRenderers; i++)
        if (m_renderer[i]->needWallpaperChange())
            return true;
    return false;
}

void
KVirtualBGRenderer::changeWallpaper()
{
    for (int i = 0; i < m_numRenderers; i++)
        m_renderer[i]->changeWallpaper();
}

void
KVirtualBGRenderer::desktopResized()
{
    m_size = QApplication::desktop()->size();

    if (m_pPixmap) {
        delete m_pPixmap;
        m_pPixmap = new QPixmap(m_size);
        m_pPixmap->fill(Qt::black);
    }

    for (int i = 0; i < m_numRenderers; i++)
        m_renderer[i]->desktopResized();
}


QSize
KVirtualBGRenderer::renderSize(int screen)
{
    return m_bDrawBackgroundPerScreen ?
           QApplication::desktop()->screenGeometry(screen).size() :
           QApplication::desktop()->size();
}


void
KVirtualBGRenderer::initRenderers()
{
    KConfigGroup cg(m_pConfig, "Background Common");
    // Same config for each screen?
    // FIXME: Could use same renderer for identically-sized screens.
    m_bCommonScreen = cg.readEntry("CommonScreen", _defCommonScreen);
    // Do not split one big image over all screens?
    m_bDrawBackgroundPerScreen =
        cg.readEntry("DrawBackgroundPerScreen_0", _defDrawBackgroundPerScreen);

    m_numRenderers = m_bDrawBackgroundPerScreen ? QApplication::desktop()->numScreens() : 1;

    m_bFinished.resize(m_numRenderers);
    m_bFinished.fill(false);

    if (m_numRenderers == m_renderer.size())
        return;

    qDeleteAll(m_renderer);
    m_renderer.resize(m_numRenderers);
    for (int i = 0; i < m_numRenderers; i++) {
        int eScreen = m_bCommonScreen ? 0 : i;
        KBackgroundRenderer *r = new KBackgroundRenderer(eScreen, m_bDrawBackgroundPerScreen, m_pConfig);
        m_renderer.insert(i, r);
        r->setSize(renderSize(i));
        connect(r, SIGNAL(imageDone(int)), SLOT(screenDone(int)));
    }
}

void
KVirtualBGRenderer::load(bool reparseConfig)
{
    initRenderers();

    for (int i = 0; i < m_numRenderers; i++) {
        int eScreen = m_bCommonScreen ? 0 : i;
        m_renderer[i]->load(eScreen, m_bDrawBackgroundPerScreen, reparseConfig);
    }
}

void
KVirtualBGRenderer::screenDone(int screen)
{
    m_bFinished[screen] = true;

    if (m_pPixmap) {
        // There's more than one renderer, so we are drawing each output to our own pixmap

        QRect overallGeometry;
        for (int i = 0; i < QApplication::desktop()->numScreens(); i++)
            overallGeometry |= QApplication::desktop()->screenGeometry(i);

        QPoint drawPos =
            QApplication::desktop()->screenGeometry(screen).topLeft() -
            overallGeometry.topLeft();
        drawPos.setX(int(drawPos.x() * m_scaleX));
        drawPos.setY(int(drawPos.y() * m_scaleY));

        QPixmap source = m_renderer[screen]->pixmap();
        QSize renderSize = this->renderSize(screen);
        renderSize.setWidth(int(renderSize.width() * m_scaleX));
        renderSize.setHeight(int(renderSize.height() * m_scaleY));

        QPainter p(m_pPixmap);

        if (renderSize == source.size())
            p.drawPixmap(drawPos, source);
        else
            p.drawTiledPixmap(drawPos.x(), drawPos.y(),
                              renderSize.width(), renderSize.height(), source);

        p.end();
    }

    for (int i = 0; i < m_bFinished.size(); i++)
        if (!m_bFinished[i])
            return;

    emit imageDone();
}

void
KVirtualBGRenderer::start()
{
    delete m_pPixmap;
    m_pPixmap = 0;

    if (m_numRenderers > 1) {
        m_pPixmap = new QPixmap(m_size);
        // If are screen sizes do not properly tile the overall virtual screen
        // size, then we want the untiled parts to be black for use in desktop
        // previews, etc
        m_pPixmap->fill(Qt::black);
    }

    m_bFinished.fill(false);
    for (int i = 0; i < m_numRenderers; i++)
        m_renderer[i]->start();
}


void KVirtualBGRenderer::stop()
{
    for (int i = 0; i < m_numRenderers; i++)
        m_renderer[i]->stop();
}


void KVirtualBGRenderer::cleanup()
{
    m_bFinished.fill(false);

    for (int i = 0; i < m_numRenderers; i++)
        m_renderer[i]->cleanup();

    delete m_pPixmap;
    m_pPixmap = 0l;
}

void KVirtualBGRenderer::saveCacheFile()
{
    for (int i = 0; i < m_numRenderers; i++)
        m_renderer[i]->saveCacheFile();
}

void KVirtualBGRenderer::enableTiling(bool enable)
{
    for (int i = 0; i < m_numRenderers; i++)
        m_renderer[i]->enableTiling(enable);
}


MyApplication::MyApplication(const char *conf, int &argc, char **argv)
    : QApplication(argc, argv)
    , renderer(KSharedConfig::openConfig(QFile::decodeName(conf)))
{
    connect(&timer, SIGNAL(timeout()), SLOT(slotTimeout()));
    connect(&renderer, SIGNAL(imageDone()), this, SLOT(renderDone()));
    renderer.enableTiling(true); // optimize
    renderer.changeWallpaper(); // cannot do it when we're killed, so do it now
    timer.start(60000);
    renderer.start();
}


void
MyApplication::renderDone()
{
    QPalette palette;
    palette.setBrush(desktop()->backgroundRole(), QBrush(renderer.pixmap()));
    desktop()->setPalette(palette);
    XClearWindow(QX11Info::display(), desktop()->winId());

    renderer.saveCacheFile();
    renderer.cleanup();
    for (unsigned i = 0; i < renderer.numRenderers(); ++i) {
        KBackgroundRenderer *r = renderer.renderer(i);
        if (r->backgroundMode() == KBackgroundSettings::Program ||
            (r->multiWallpaperMode() != KBackgroundSettings::NoMulti &&
             r->multiWallpaperMode() != KBackgroundSettings::NoMultiRandom))
            return;
    }
    quit();
}

void
MyApplication::slotTimeout()
{
    bool change = false;

    if (renderer.needProgramUpdate()) {
        renderer.programUpdate();
        change = true;
    }

    if (renderer.needWallpaperChange()) {
        renderer.changeWallpaper();
        change = true;
    }

    if (change)
        renderer.start();
}

int
main(int argc, char *argv[])
{
    KCmdLineArgs::init(argc, argv, "krootimage", "kdmgreet",
                       ki18n("KRootImage"), QByteArray(),
                       ki18n("Fancy desktop background for kdm"));

    KCmdLineOptions options;
    options.add("+config", ki18n("Name of the configuration file"));
    KCmdLineArgs::addCmdLineOptions(options);

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    if (!args->count())
        args->usage();
    KComponentData inst(KCmdLineArgs::aboutData());
    MyApplication app(args->arg(0).toLocal8Bit(),
                      KCmdLineArgs::qtArgc(), KCmdLineArgs::qtArgv());
    args->clear();

    app.exec();
    app.flush();

    // Keep color resources after termination
    XSetCloseDownMode(QX11Info::display(), RetainTemporary);

    return 0;
}

#include "krootimage.moc"
