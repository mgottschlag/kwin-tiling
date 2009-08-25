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

#include <QDateTime>
#include <QFile>
#include <QHBoxLayout>
#include <QSplitter>
#include <QVBoxLayout>

#include <KLocale>
#include <KPushButton>
#include <KShell>
#include <KTextEdit>
#include <KTextBrowser>

#include <Plasma/Corona>

#include "scriptengine.h"

//TODO:
// save and restore splitter sizes
// better initial size of editor to output
// use text editor KPart for syntax highlighting?
// interative help?

InteractiveConsole::InteractiveConsole(Plasma::Corona *corona, QWidget *parent)
    : KDialog(parent),
      m_engine(new ScriptEngine(corona, this)),
      m_editor(new KTextEdit(this)),
      m_output(new KTextBrowser(this)),
      m_clearButton(new KPushButton(KIcon("edit-clear"), i18n("&Clear"), this)),
      m_executeButton(new KPushButton(KIcon("system-run"), i18n("&Run Script"), this))
{
    setWindowTitle(KDialog::makeStandardCaption(i18n("Desktop Shell Scripting Console")));
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

    setInitialSize(QSize(500, 400));
    KConfigGroup cg(KGlobal::config(), "InteractiveConsole");
    restoreDialogSize(cg);

    scriptTextChanged();

    connect(m_executeButton, SIGNAL(clicked()), this, SLOT(evaluateScript()));
    connect(m_clearButton, SIGNAL(clicked()), this, SLOT(clearEditor()));
    connect(m_editor, SIGNAL(textChanged()), this, SLOT(scriptTextChanged()));
    connect(m_engine, SIGNAL(print(QString)), this, SLOT(print(QString)));
    connect(m_engine, SIGNAL(printError(QString)), this, SLOT(print(QString)));
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
        m_output->append(i18n("Unable to load script file <b>%1</b>", script));
    }
}

void InteractiveConsole::showEvent(QShowEvent *)
{
    m_editor->setFocus();
}

void InteractiveConsole::print(const QString &string)
{
    m_output->append(string);
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
    m_output->moveCursor(QTextCursor::End);
    QTextCursor cursor = m_output->textCursor();
    m_output->setTextCursor(cursor);

    QTextCharFormat format;
    format.setFontWeight(QFont::Bold);
    format.setFontUnderline(true);

    if (cursor.position() > 0) {
        cursor.insertText("\n\n");
    }

    QDateTime dt = QDateTime::currentDateTime();
    cursor.insertText(i18n("Executing script at %1", KGlobal::locale()->formatDateTime(dt)), format);

    format.setFontWeight(QFont::Normal);
    format.setFontUnderline(false);
    QTextBlockFormat block = cursor.blockFormat();
    block.setLeftMargin(10);
    cursor.insertBlock(block, format);
    QTime t;
    t.start();
    m_engine->evaluateScript(m_editor->toPlainText());

    cursor.insertText("\n\n");
    format.setFontWeight(QFont::Bold);
    cursor.insertText(i18n("Runtime: %1ms", QString::number(t.elapsed())), format);
    block.setLeftMargin(0);
    cursor.insertBlock(block);
    m_output->ensureCursorVisible();
}

void InteractiveConsole::clearEditor()
{
    m_editor->clear();
}

#include "interactiveconsole.moc"

