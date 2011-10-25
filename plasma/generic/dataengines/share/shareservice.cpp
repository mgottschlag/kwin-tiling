/***************************************************************************
 *   Copyright 2010 Artur Duque de Souza <asouza@kde.org>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include <QtCore/QFile>

#include <kross/core/action.h>
#include <kross/core/interpreter.h>
#include <kross/core/manager.h>
#include <kstandarddirs.h>

#include <Plasma/Package>

#include "shareservice.h"
#include "shareprovider.h"


ShareService::ShareService(ShareEngine *engine)
    : Plasma::Service(engine)
{
    setName("share");
}

Plasma::ServiceJob *ShareService::createJob(const QString &operation,
                                            QMap<QString, QVariant> &parameters)
{
    return new ShareJob(destination(), operation, parameters, this);
}

ShareJob::ShareJob(const QString &destination, const QString &operation,
                   QMap<QString, QVariant> &parameters, QObject *parent)
    : Plasma::ServiceJob(destination, operation, parameters, parent),
      m_action(0), m_provider(0), m_package(0)
{
}

ShareJob::~ShareJob()
{
    delete m_action;
    delete m_provider;
    delete m_package;
}

void ShareJob::start()
{
    //KService::Ptr service = KService::serviceByStorageId("plasma-share-pastebincom.desktop");
    KService::Ptr service = KService::serviceByStorageId(destination());
    if (!service) {
        showError(i18n("Could not find the provider with the specified destination"));
        return;
    }

    QString pluginName =
        service->property("X-KDE-PluginInfo-Name", QVariant::String).toString();

    const QString path =
        KStandardDirs::locate("data", "plasma/shareprovider/" + pluginName + '/' );

    if (path.isEmpty()) {
        showError(i18n("Invalid path for the requested provider"));
        return;
    }

    m_package = new Plasma::Package(path, ShareProvider::packageStructure());
    if (m_package->isValid()) {
        const QString mainscript =
            m_package->path() + m_package->structure()->contentsPrefixPaths().at(0) +
            m_package->structure()->path("mainscript");

        if (!QFile::exists(mainscript)) {
            showError(i18n("Selected provider does not have a valid script file"));
            return;
        }

        const QString interpreter =
            Kross::Manager::self().interpreternameForFile(mainscript);

        if (interpreter.isEmpty()) {
            showError(i18n("Selected provider does not provide a supported script file"));
            return;
        }

        m_action = new Kross::Action(parent(), pluginName);
        if (m_action) {
            m_provider = new ShareProvider(this);
            connect(m_provider, SIGNAL(readyToPublish()), this, SLOT(publish()));
            connect(m_provider, SIGNAL(finished(QString)),
                    this, SLOT(showResult(QString)));
            connect(m_provider, SIGNAL(finishedError(QString)),
                    this, SLOT(showError(QString)));

            // automatically connects signals and slots with the script
            m_action->addObject(m_provider, "provider",
                                Kross::ChildrenInterface::AutoConnectSignals);

            // set the main script file and load it
            m_action->setFile(mainscript);
            m_action->trigger();

            // check for any errors
            if(m_action->hadError()) {
                showError(i18n("Error trying to execute script"));
                return;
            }

            // do the work together with the loaded plugin
            const QStringList functions = m_action->functionNames();
            if (!functions.contains("url") || !functions.contains("contentKey") ||
                !functions.contains("setup")) {
                showError(i18n("Could not find all required functions"));
                return;
            }

            // call the methods from the plugin
            const QString url =
                m_action->callFunction("url", QVariantList()).toString();
            m_provider->setUrl(url);

            // setup the method (get/post)
            QVariant vmethod;
            if (functions.contains("method")) {
                vmethod =
                    m_action->callFunction("method", QVariantList()).toString();
            }

            // default is POST (if the plugin does not specify one method)
            const QString method = vmethod.isValid() ? vmethod.toString() : "POST";
            m_provider->setMethod(method);

            // setup the provider
            QVariant setup = m_action->callFunction("setup", QVariantList());

            // get the content from the parameters, set the url and add the file
            // then we can wait the signal to publish the information
            const QString contentKey =
                m_action->callFunction("contentKey", QVariantList()).toString();

            const QString content(parameters()["content"].toString());
            m_provider->addPostFile(contentKey, content);
        }
    }
}

void ShareJob::publish()
{
    m_provider->publish();
}

void ShareJob::showResult(const QString &url)
{
    setResult(url);
}

void ShareJob::showError(const QString &message)
{
    QString errorMsg = message;
    if (errorMsg.isEmpty()) {
        errorMsg = i18n("Unknown Error");
    }

    setError(1);
    setErrorText(message);
    emitResult();
}

#include "shareservice.moc"
