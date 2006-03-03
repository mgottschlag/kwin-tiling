/*****************************************************************

Copyright (c) 2000 Matthias Elter

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

#include <QMenu>

#include <kconfig.h>

#include "kpanelextension.h"
#include "kpanelextension.moc"
#include "extensionSettings.h"

class KPanelExtension::Private
{
public:
    Private()
      : position(Plasma::Top),
        alignment(Plasma::LeftTop),
        size(Plasma::SizeNormal),
        config(0),
        settings(0),
        customMenu(0),
        customSize(58),
        reserveStrut(true)
    {}

    Plasma::Position    position;
    Plasma::Alignment   alignment;
    Plasma::Size        size;
    KConfig*            config;
    ExtensionSettings*  settings;
    int                 actions;
    QMenu*  customMenu;
    int     customSize;
    bool    reserveStrut;
};

KPanelExtension::KPanelExtension(const QString& configFile,
                                 int actions, QWidget *parent)
  : QWidget(parent)
{
    d = new Private;
    d->config = new KConfig(configFile);
    d->actions = actions;
    d->settings = new ExtensionSettings(KSharedConfig::openConfig(configFile));

    // if we have an extension, we need to grab the extension-specific
    // defaults for position, size and custom size and override the
    // defaults in the settings object since the extension may differ
    // from the "normal" panels. for example, the universal sidebar's
    // preferred position is the left, not the bottom/top
    KConfigSkeleton::ItemInt* item = dynamic_cast<KConfigSkeleton::ItemInt*>(d->settings->findItem("Position"));
    if (item)
    {
        Plasma::Position p = preferredPosition();
        item->setDefaultValue(p);
    }

    item = dynamic_cast<KConfigSkeleton::ItemInt*>(d->settings->findItem("Size"));
    if (item)
    {
        item->setDefaultValue(size());
    }

    item = dynamic_cast<KConfigSkeleton::ItemInt*>(d->settings->findItem("CustomSize"));
    if (item)
    {
        item->setDefaultValue(customSize());
    }

    if (!d->settings->iExist())
    {
        d->settings->setIExist(true);
        d->settings->writeConfig();
    }
}

KPanelExtension::~KPanelExtension()
{
    delete d;
}

QSize KPanelExtension::sizeHint() const
{
    return QSize(0,0);
}

void KPanelExtension::setPosition(Plasma::Position p)
{
    if( d->position == p ) return;
    d->position = p;
    positionChange(p);
    d->settings->setPosition(p);
}

void KPanelExtension::setAlignment(Plasma::Alignment a)
{
    if( d->alignment == a ) return;
    d->alignment = a;
    alignmentChange(a);
    d->settings->setAlignment(a);
}

void KPanelExtension::setSize(Plasma::Size size, int customSize)
{
    if (d->size == size && d->customSize == customSize)
    {
        return;
    }

    d->size = size;
    d->customSize = customSize;
    d->settings->setSize(size);
    d->settings->setCustomSize(customSize);
    emit updateLayout();
}

void KPanelExtension::action(Plasma::Action a)
{
    if (a & Plasma::About)
    {
       about();
    }
    if (a & Plasma::Help)
    {
        help();
    }
    if (a & Plasma::Preferences)
    {
        preferences();
    }
    if (a & Plasma::ReportBug)
    {
        reportBug();
    }
}

Qt::Orientation KPanelExtension::orientation()
{
    if (d->position == Plasma::Left || d->position == Plasma::Right)
    {
        return Qt::Vertical;
    }
    else
    {
        return Qt::Horizontal;
    }
}

int KPanelExtension::customSize() const
{
    return d->customSize;
}

QMenu* KPanelExtension::customMenu() const
{
    return d->customMenu;
}

void KPanelExtension::setCustomMenu(QMenu* menu)
{
    d->customMenu = menu;
}

bool KPanelExtension::reserveStrut() const
{
    return position() == Plasma::Floating || d->reserveStrut;
}

void KPanelExtension::setReserveStrut(bool reserve)
{
    d->reserveStrut = reserve;
}

KConfig* KPanelExtension::config() const
{
    return d->config;
}

ExtensionSettings* KPanelExtension::settings() const
{
    return d->settings;
}

int KPanelExtension::actions() const
{
    return d->actions;
}

Plasma::Size KPanelExtension::size() const
{
    return d->size;
}

int KPanelExtension::sizeInPixels() const
{
    return Plasma::sizeValue(size());
}

Plasma::Position KPanelExtension::position() const
{
    return d->position;
}

Plasma::Alignment KPanelExtension::alignment() const
{
    return d->alignment;
}

void KPanelExtension::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

