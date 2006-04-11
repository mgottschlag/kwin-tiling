/*****************************************************************

Copyright (c) 2000 the kicker authors. See file AUTHORS.

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

#include <time.h>

#include <qregexp.h>
#include <qstringlist.h>
#include <QList>
#include <q3tl.h>

#include <dcopclient.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>

#include "kickerSettings.h"

#include "recentapps.h"

RecentlyLaunchedApps& RecentlyLaunchedApps::self()
{
    static RecentlyLaunchedApps obj;
    return obj;
}

RecentlyLaunchedApps::RecentlyLaunchedApps()
{
    // set defaults
    m_nNumMenuItems = 0;
    m_bNeedToUpdate = false;
    m_bInitialised = false;
}

void RecentlyLaunchedApps::init()
{
    if (m_bInitialised)
    {
       return;
    }

    m_nNumMenuItems = 0;
    m_appInfos.clear();

    configChanged();

    QStringList recentApps = KickerSettings::recentAppsStat();

    for (QStringList::ConstIterator it = recentApps.begin(); 
         it != recentApps.end(); ++it )
    {
        QRegExp re( "(\\d*) (\\d*) (.*)" );
        if (re.indexIn(*it) != -1)
        {
            int nCount = re.cap(1).toInt();
            long lTime = re.cap(2).toLong();
            QString szPath = re.cap(3);
            m_appInfos.append(RecentlyLaunchedAppInfo(
                szPath, nCount, time_t(lTime)));
        }
    }

    qHeapSort(m_appInfos);

    m_bInitialised = true;
}

void RecentlyLaunchedApps::configChanged()
{
    qHeapSort(m_appInfos);
}

void RecentlyLaunchedApps::save()
{
    QStringList recentApps;

    for (QList<RecentlyLaunchedAppInfo>::const_iterator it = 
            m_appInfos.constBegin(); it != m_appInfos.constEnd(); ++it)
    {
        recentApps.append(QString("%1 %2 %3").arg((*it).getLaunchCount())
                                             .arg((*it).getLastLaunchTime())
                                             .arg((*it).getDesktopPath()));
    }

    KickerSettings::setRecentAppsStat(recentApps);
    KickerSettings::writeConfig();
}

void RecentlyLaunchedApps::appLaunched(const QString& strApp)
{
    // Inform other applications (like the quickstarter applet)
    // that an application was started
    QByteArray params;
    QDataStream stream(&params, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_3_1);
    stream << launchDCOPSignalSource() << strApp;
    KApplication::kApplication()->dcopClient()->emitDCOPSignal("appLauncher",
        "serviceStartedByStorageId(QString,QString)", params);

    QList<RecentlyLaunchedAppInfo>::iterator itEnd = m_appInfos.end();
    for (QList<RecentlyLaunchedAppInfo>::iterator it = m_appInfos.begin();
         it != itEnd; ++it)
    {
        if ((*it).getDesktopPath() == strApp)
        {
            (*it).increaseLaunchCount();
            (*it).setLastLaunchTime(time(0));
            qHeapSort(m_appInfos);
            return;
        }
    }

    m_appInfos.append(RecentlyLaunchedAppInfo(strApp, 1, time(0)));
    qHeapSort(m_appInfos);
}

void RecentlyLaunchedApps::updateRecentlyUsedApps(KService::Ptr &service)
{
	QString strItem(service->desktopEntryPath());

    // don't add an item from root kmenu level
	if (!strItem.contains('/'))
	{
		return;
	}

    // add it into recent apps list
	appLaunched(strItem);
	save();
	m_bNeedToUpdate = true;
}

void RecentlyLaunchedApps::getRecentApps(QStringList& recentApps)
{
    recentApps.clear();

    int maximumNum = KickerSettings::numVisibleEntries();
    int i = 0;
    QList<RecentlyLaunchedAppInfo>::const_iterator itEnd = m_appInfos.constEnd();
    for (QList<RecentlyLaunchedAppInfo>::const_iterator it = m_appInfos.constBegin();
         it != itEnd && i < maximumNum;
         ++it, ++i)
    {
        recentApps.append((*it).getDesktopPath());
    }
}

void RecentlyLaunchedApps::removeItem( const QString& strName )
{
    QList<RecentlyLaunchedAppInfo>::iterator itEnd = m_appInfos.end();
    for (QList<RecentlyLaunchedAppInfo>::iterator it = m_appInfos.begin();
         it != itEnd; ++it)
    {
        if ((*it).getDesktopPath() == strName)
        {
            m_appInfos.erase(it);
            return;
        }
    }
}

void RecentlyLaunchedApps::clearRecentApps()
{
    m_appInfos.clear();
}

QString RecentlyLaunchedApps::caption() const
{
    return KickerSettings::recentVsOften() ?
           i18n("Recently Used Applications") :
           i18n("Most Used Applications");
}
