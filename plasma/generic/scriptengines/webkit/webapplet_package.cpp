/*
Copyright (c) 2008 Petri Damstén <damu@iki.fi>

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
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "webapplet_package.h"

#include <Plasma/Applet>

WebAppletPackage::WebAppletPackage(QObject *parent, QVariantList args)
: Plasma::PackageStructure(parent, "Web")
{
    Q_UNUSED(args)
    // copy the main applet structure
    Plasma::PackageStructure::operator=(*Plasma::Applet::packageStructure());
    addFileDefinition("mainscript", "code/main.html", i18n("Main Script File"));
    setRequired("mainscript", true);

    // For Webapplet::init()
    addDirectoryDefinition("html", "code/", i18n("Root HTML directory"));
}

void WebAppletPackage::pathChanged()
{
    KDesktopFile config(path() + "/metadata.desktop");
    KConfigGroup cg = config.desktopGroup();
    QString mainScript = cg.readEntry("X-Plasma-MainScript", QString());
    if (!mainScript.isEmpty()) {
        addFileDefinition("mainscript", mainScript, i18n("Main Script File"));
        setRequired("mainscript", true);
    }
}

