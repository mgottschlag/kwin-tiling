////////////////////////////////////////////////////////////////////////////////
//
// Class Names   : KFI::CFontViewerApp, KFI::CFontViewerAppMainWindow
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
// Creation Date : 30/04/2004
// Version       : $Revision$ $Date$
//
////////////////////////////////////////////////////////////////////////////////
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2004
////////////////////////////////////////////////////////////////////////////////

#include "FontViewerApp.h"
#include "KfiConstants.h"
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klibloader.h>
#include <klocale.h>
#include <kglobal.h>
#include <kfiledialog.h>
#include <kconfig.h>

#define CFG_GROUP    "FontViewer Settings"
#define CFG_SIZE_KEY "Window Size"

namespace KFI
{

CFontViewerAppMainWindow::CFontViewerAppMainWindow()
                        : KParts::MainWindow((QWidget *)0L)
{
    KLibFactory *factory=KLibLoader::self()->factory("libkfontviewpart");

    if(factory)
    {
        KStdAction::open(this, SLOT(fileOpen()), actionCollection());
        KStdAction::quit(kapp, SLOT(quit()), actionCollection());

        itsPreview=(KParts::ReadOnlyPart *)factory->create(this, "fontvier", "KParts::ReadOnlyPart");

        KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
        KURL         openURL;

        if(args->count() > 0)
        {
            KURL url(args->url(args->count() - 1));

            if(url.isValid())
                openURL = url;
        }

        setCentralWidget(itsPreview->widget());
        createGUI(itsPreview);

        if(!openURL.isEmpty())
            itsPreview->openURL(openURL);

        QSize             defSize(450, 380);
        KConfigGroupSaver saver(kapp->config(), CFG_GROUP);

        resize(kapp->config()->readSizeEntry(CFG_SIZE_KEY, &defSize));
        show();
    }
    else
        exit(0);
}

CFontViewerAppMainWindow::~CFontViewerAppMainWindow()
{
    KConfigGroupSaver saver(kapp->config(), CFG_GROUP);
    kapp->config()->writeEntry(CFG_SIZE_KEY, size());
    kapp->config()->sync();
}

void CFontViewerAppMainWindow::fileOpen()
{
    KURL url(KFileDialog::getOpenURL(QString::null, "application/x-font-ttf application/x-font-otf "
                                                    "application/x-font-ttc application/x-font-type1 "
                                                    "application/x-font-bdf application/x-font-pcf ",
                                     this, i18n("Select Font to View")));
    if(url.isValid())
        itsPreview->openURL(url);
}

CFontViewerApp::CFontViewerApp()
{
    KGlobal::locale()->insertCatalog(KFI_CATALOGUE);
    setMainWidget(new CFontViewerAppMainWindow());
}

}

static KCmdLineOptions options[] =
{
    { "+[URL]", I18N_NOOP("URL to open"), 0 },
    KCmdLineLastOption
};

static KAboutData aboutData("kfontview", I18N_NOOP("Font Viewer"), 0, I18N_NOOP("Simple font viewer"),
                            KAboutData::License_GPL,
                            I18N_NOOP("(c) Craig Drummond, 2004"));

int main(int argc, char **argv)
{
    KCmdLineArgs::init(argc, argv, &aboutData);
    KCmdLineArgs::addCmdLineOptions(options);
    KCmdLineArgs::addStdCmdLineOptions();

    KFI::CFontViewerApp app;

    return app.exec();
}

#include "FontViewerApp.moc"
