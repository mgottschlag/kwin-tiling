/*
 *   Copyright (C) 2006 Aaron Seigo <aseigo@kde.org>
 *   Copyright (C) 2010 Marco Martin <notmart@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "windowedwidgetsrunner.h"

#include <QProcess>

#include <KIcon>

#include <KDebug>
#include <KLocale>
#include <KService>

#include <Plasma/Applet>

WindowedWidgetsRunner::WindowedWidgetsRunner(QObject *parent, const QVariantList &args)
    : Plasma::AbstractRunner(parent, args)
{
    Q_UNUSED(args)

    setObjectName("WindowedWidgets");
    setPriority(AbstractRunner::HighestPriority);

    addSyntax(Plasma::RunnerSyntax(":q:", i18n("Finds Plasma widgets whose name or description match :q:")));
    setDefaultSyntax(Plasma::RunnerSyntax(i18nc("Note this is a KRunner keyword", "mobile applications"), i18n("list all Plasma widgets that can run as standalone applications")));
}

WindowedWidgetsRunner::~WindowedWidgetsRunner()
{
}

void WindowedWidgetsRunner::match(Plasma::RunnerContext &context)
{
    const QString term = context.query();
    if (term.length() <  3) {
        return;
    }


   QList<Plasma::QueryMatch> matches;

    foreach (const KPluginInfo &info, Plasma::Applet::listAppletInfo()) {
        KService::Ptr service = info.service();

        //TODO: distinguish between exact and partial
        if ((service->name().contains(term) ||
             service->genericName().contains(term) ||
             service->comment().contains(term)) &&
             service->categories().contains(term) &&
            service->property("X-Plasma-StandAlone", QVariant::Bool).toBool()) {
            Plasma::QueryMatch match(this);
            match.setType(Plasma::QueryMatch::ExactMatch);
            setupMatch(service, match);
            match.setRelevance(1);
            matches << match;
        }
    }

    if (!context.isValid()) {
        return;
    }

    context.addMatches(term, matches);
}

void WindowedWidgetsRunner::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    Q_UNUSED(context);
    KService::Ptr service = KService::serviceByStorageId(match.data().toString());
    if (service) {
        QProcess::startDetached("plasma-windowed", QStringList() << service->property("X-KDE-PluginInfo-Name", QVariant::String).toString());
    }
}

void WindowedWidgetsRunner::setupMatch(const KService::Ptr &service, Plasma::QueryMatch &match)
{
    const QString name = service->name();

    match.setText(name);
    match.setData(service->storageId());

    if (!service->genericName().isEmpty() && service->genericName() != name) {
        match.setSubtext(service->genericName());
    } else if (!service->comment().isEmpty()) {
        match.setSubtext(service->comment());
    }

    if (!service->icon().isEmpty()) {
        match.setIcon(KIcon(service->icon()));
    }
}

#include "windowedwidgetsrunner.moc"

