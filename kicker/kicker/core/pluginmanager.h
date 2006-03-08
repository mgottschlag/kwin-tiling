/*****************************************************************

Copyright (c) 1996-2000 the kicker authors. See file AUTHORS.

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

#ifndef __PLUGIN_MANAGER_H__
#define __PLUGIN_MANAGER_H__

#include <QMap>
#include <QObject>
#include <QStringList>

#include <kdemacros.h>
#include <kstaticdeleter.h>

#include "appletinfo.h"

class QMenu;
class QStringList;
class AppletContainer;
class ExtensionContainer;
class KPanelApplet;
class KPanelExtension;

class KDE_EXPORT PluginManager : public QObject
{
    Q_OBJECT

public:
    static PluginManager* self();

    static AppletInfo::List applets(bool sort = true, AppletInfo::List* list = 0);
    static AppletInfo::List extensions(bool sort = true, AppletInfo::List* list = 0);
    static AppletInfo::List builtinButtons(bool sort = true, AppletInfo::List* list = 0);
    static AppletInfo::List specialButtons(bool sort = true, AppletInfo::List* list = 0);

    AppletContainer* createAppletContainer(const QString& desktopFile,
                                           bool isStartup,
                                           const QString& configFile,
                                           QMenu* opMenu,
                                           QWidget* parent,
                                           bool isImmutable = false);
    ExtensionContainer* createExtensionContainer(const QString& desktopFile,
                                                 bool isStartup,
                                                 const QString& configFile,
                                                 const QString& extensionId);

    KPanelApplet* loadApplet(const AppletInfo& info, QWidget* parent);
    KPanelExtension* loadExtension(const AppletInfo& info, QWidget* parent);

    bool hasInstance(const AppletInfo&) const;

public Q_SLOTS:
    void clearUntrustedLists();

protected:
    static AppletInfo::List plugins(const QStringList& desktopFiles,
                                    AppletInfo::AppletType,
                                    bool sort,
                                    AppletInfo::List* list);

private Q_SLOTS:
    void slotPluginDestroyed(QObject* plugin);

private:
    friend class KStaticDeleter<PluginManager>;
    PluginManager();
    virtual ~PluginManager();

    static PluginManager* m_self;

    AppletInfo::Dict m_appletDict;
    QStringList m_untrustedApplets;
    QStringList m_untrustedExtensions;

};

class LibUnloader : public QObject
{
    Q_OBJECT
public:
    static void unload( const QString &libName );

private Q_SLOTS:
    void unload();

private:
    LibUnloader( const QString &libName, QObject *parent );

    QString _libName;
};

#endif
