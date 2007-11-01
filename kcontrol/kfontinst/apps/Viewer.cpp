/*
 * KFontInst - KDE Font Installer
 *
 * Copyright 2003-2007 Craig Drummond <craig@kde.org>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "Viewer.h"
#include "KfiConstants.h"
#include <KDE/KCmdLineArgs>
#include <KDE/KAboutData>
#include <KDE/KApplication>
#include <KDE/KPluginLoader>
#include <KDE/KPluginFactory>
#include <KDE/KLocale>
#include <KDE/KGlobal>
#include <KDE/KFileDialog>
#include <KDE/KConfig>
#include <KDE/KStandardAction>
#include <KDE/KActionCollection>
#include <KDE/KParts/BrowserExtension>

#define CFG_GROUP    "FontViewer Settings"
#define CFG_SIZE_KEY "Window Size"

namespace KFI
{

CViewer::CViewer()
{
    KPluginFactory *factory=KPluginLoader("libkfontviewpart").factory();

    if(factory)
    {
        itsPreview=factory->create<KParts::ReadOnlyPart>(this);

        actionCollection()->addAction(KStandardAction::Open, this, SLOT(fileOpen()));
        actionCollection()->addAction(KStandardAction::Quit, kapp, SLOT(quit()));
        itsPrintAct=actionCollection()->addAction(KStandardAction::Print, itsPreview, SLOT(print()));

        itsPrintAct->setEnabled(false);

        if(itsPreview->browserExtension())
            connect(itsPreview->browserExtension(), SIGNAL(enableAction(const char *, bool)),
                    this, SLOT(enableAction(const char *, bool)));

        setCentralWidget(itsPreview->widget());
        createGUI(itsPreview);

        KCmdLineArgs *args(KCmdLineArgs::parsedArgs());

        if(args->count() > 0)
        {
            KUrl url(args->url(args->count() - 1));

            if(url.isValid())
                itsPreview->openUrl(url);
        }

        QSize defSize(440, 530);

        resize(KGlobal::config()->group(CFG_GROUP).readEntry(CFG_SIZE_KEY, defSize));
    }
    else
        exit(0);
}

CViewer::~CViewer()
{
    KGlobal::config()->group(CFG_GROUP).writeEntry(CFG_SIZE_KEY, size());
    KGlobal::config()->sync();
}

void CViewer::fileOpen()
{
    KUrl url(KFileDialog::getOpenUrl(KUrl(), "application/x-font-ttf application/x-font-otf "
                                             "application/x-font-type1 "
                                             "application/x-font-bdf application/x-font-pcf ",
                                     this, i18n("Select Font to View")));
    if(url.isValid())
        itsPreview->openUrl(url);
}

void CViewer::enableAction(const char *name, bool enable)
{
    if(0==qstrcmp("print", name))
        itsPrintAct->setEnabled(enable);
}

}

static KAboutData aboutData("kfontview", KFI_CATALOGUE, ki18n("Font Viewer"), "1.1", ki18n("Simple font viewer"),
                            KAboutData::License_GPL, ki18n("(C) Craig Drummond, 2004-2007"));

int main(int argc, char **argv)
{
    KCmdLineArgs::init(argc, argv, &aboutData);

    KCmdLineOptions options;
    options.add("+[URL]", ki18n("URL to open"));
    KCmdLineArgs::addCmdLineOptions(options);

    KApplication app;

    KFI::CViewer *viewer=new KFI::CViewer;

    viewer->show();
    return app.exec();
}

#include "Viewer.moc"
