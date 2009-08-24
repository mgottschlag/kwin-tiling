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

#include <QScriptValue>

#include <KDialog>

class ScriptEngine;

class KPushButton;
class KTextEdit;
class KTextBrowser;

namespace Plasma
{
    class Corona;
} // namespace Plasma

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
    void scriptTextChanged();
    void evaluateScript();
    void clearEditor();

private:
    ScriptEngine *m_engine;
    KTextEdit *m_editor;
    KTextBrowser *m_output;
    KPushButton *m_clearButton;
    KPushButton *m_executeButton;
};

#endif

