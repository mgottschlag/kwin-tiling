/*
 *   Copyright 2009 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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

#ifndef INTERACTIVECONSOLE
#define INTERACTIVECONSOLE

#include <QPointer>
#include <QScriptValue>

#include <KDialog>

#include <KIO/Job>

class QSplitter;

class KAction;
class KFileDialog;
class KTextEdit;
class KTextBrowser;

namespace Plasma
{
    class Corona;
} // namespace Plasma

class ScriptEngine;

class InteractiveConsole : public KDialog
{
    Q_OBJECT

public:
    InteractiveConsole(Plasma::Corona *corona, QWidget *parent = 0);
    ~InteractiveConsole();

    void loadScript(const QString &path);

protected:
    void showEvent(QShowEvent *);

protected Q_SLOTS:
    void print(const QString &string);

private Q_SLOTS:
    void openScriptFile();
    void saveScript();
    void scriptTextChanged();
    void evaluateScript();
    void clearEditor();
    void scriptFileDataRecvd(KIO::Job *job, const QByteArray &data);
    void scriptFileDataReq(KIO::Job *job, QByteArray &data);
    void reenableEditor();
    void saveScriptUrlSelected();
    void openScriptUrlSelected();

private:
    ScriptEngine *m_engine;
    QSplitter *m_splitter;
    KTextEdit *m_editor;
    KTextBrowser *m_output;
    KAction *m_loadAction;
    KAction *m_saveAction;
    KAction *m_clearAction;
    KAction *m_executeAction;

    KFileDialog *m_fileDialog;
    QPointer<KIO::Job> m_job;
};

#endif

