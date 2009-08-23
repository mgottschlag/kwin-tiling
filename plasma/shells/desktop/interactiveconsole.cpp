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

#include "interactiveconsole.h"

#include <QFile>
#include <QHBoxLayout>
#include <QScriptEngine>
#include <QSplitter>
#include <QVBoxLayout>

#include <KPushButton>
#include <KShell>
#include <KTextEdit>
#include <KTextBrowser>

#include <Plasma/Corona>

QScriptValue constructQRectFClass(QScriptEngine *engine);

//TODO:
// save and restore splitter sizes
// better initial size of editor to output
// use text editor KPart for syntax highlighting?
// interative help?

InteractiveConsole::InteractiveConsole(Plasma::Corona *corona, QWidget *parent)
    : KDialog(parent),
      m_corona(corona),
      m_engine(new QScriptEngine(this)),
      m_scriptSelf(m_engine->newQObject(this, QScriptEngine::QtOwnership,
                                        QScriptEngine::ExcludeSuperClassProperties |
                                        QScriptEngine::ExcludeSuperClassMethods)),
      m_editor(new KTextEdit(this)),
      m_output(new KTextBrowser(this)),
      m_clearButton(new KPushButton(KIcon("edit-clear"), i18n("&Clear"), this)),
      m_executeButton(new KPushButton(KIcon("system-run"), i18n("&Run Script"), this))
{
    Q_ASSERT(m_corona);
    setAttribute(Qt::WA_DeleteOnClose);
    setButtons(KDialog::None);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QSplitter *splitter = new QSplitter(Qt::Vertical, this);
    mainLayout->addWidget(splitter);

    QWidget *widget = new QWidget(splitter);
    QVBoxLayout *editorLayout = new QVBoxLayout(widget);
    editorLayout->addWidget(m_editor);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch(10);
    buttonLayout->addWidget(m_clearButton);
    buttonLayout->addWidget(m_executeButton);
    editorLayout->addLayout(buttonLayout);

    splitter->addWidget(widget);
    splitter->addWidget(m_output);
    setMainWidget(splitter);
    m_editor->setFocus();

    setInitialSize(QSize(500, 400));
    KConfigGroup cg(KGlobal::config(), "InteractiveConsole");
    restoreDialogSize(cg);

    setupEngine();
    scriptTextChanged();

    connect(m_executeButton, SIGNAL(clicked()), this, SLOT(evaluateScript()));
    connect(m_clearButton, SIGNAL(clicked()), this, SLOT(clearEditor()));
    connect(m_editor, SIGNAL(textChanged()), this, SLOT(scriptTextChanged()));
    connect(m_engine, SIGNAL(signalHandlerException(QScriptValue)), this, SLOT(exception(QScriptValue)));
}

InteractiveConsole::~InteractiveConsole()
{
    KConfigGroup cg(KGlobal::config(), "InteractiveConsole");
    saveDialogSize(cg);
}

void InteractiveConsole::loadScript(const QString &script)
{
    QFile file(KShell::tildeExpand(script));
    if (file.open(QIODevice::ReadOnly | QIODevice::Text) ) {
        m_editor->setText(file.readAll());
    } else {
        m_output->append(i18n("Unable to load script file %1", script));
    }
}

void InteractiveConsole::print(const QString &string)
{
    m_output->append(string);
}

int InteractiveConsole::screenCount() const
{
    return m_corona->numScreens();
}

QRectF InteractiveConsole::screenGeometry(int screen) const
{
    return m_corona->screenGeometry(screen);
}

void InteractiveConsole::setupEngine()
{
    m_engine->setGlobalObject(m_scriptSelf);
    m_scriptSelf.setProperty("QRectF", constructQRectFClass(m_engine));
}

void InteractiveConsole::scriptTextChanged()
{
    const bool enable = !m_editor->document()->isEmpty();
    m_clearButton->setEnabled(enable);
    m_executeButton->setEnabled(enable);
}

void InteractiveConsole::evaluateScript()
{
    //kDebug() << "evaluating" << m_editor->toPlainText();
    m_engine->evaluate(m_editor->toPlainText());
    if (m_engine->hasUncaughtException()) {
        kDebug() << "catch the exception!";
        QString error = i18n("Error: %1 at line %2<p>Backtrace:<br>%3",
                             m_engine->uncaughtException().toString(),
                             QString::number(m_engine->uncaughtExceptionLineNumber()),
                             m_engine->uncaughtExceptionBacktrace().join("<br>"));
        m_output->append(error);
    }
}

void InteractiveConsole::clearEditor()
{
    m_editor->clear();
}

void InteractiveConsole::exception(const QScriptValue &value)
{
    //kDebug() << "exception caught!" << value.toVariant();
    m_output->append(value.toVariant().toString());
}

#include "interactiveconsole.moc"

