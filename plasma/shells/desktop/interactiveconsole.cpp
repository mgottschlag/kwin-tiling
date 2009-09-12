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
#include <QLabel>
#include <QSplitter>
#include <QVBoxLayout>

#include <KFileDialog>
#include <KLocale>
#include <KAction>
#include <KShell>
#include <KServiceTypeTrader>
#include <KStandardAction>
#include <KTextBrowser>
#include <KTextEdit>
#include <KTextEditor/Document>
#include <KTextEditor/View>
#include <KToolBar>

#include <Plasma/Corona>

#include "scriptengine.h"

//TODO:
// use text editor KPart for syntax highlighting?
// interative help?

InteractiveConsole::InteractiveConsole(Plasma::Corona *corona, QWidget *parent)
    : KDialog(parent),
      m_engine(new ScriptEngine(corona, this)),
      m_splitter(new QSplitter(Qt::Vertical, this)),
      m_editorPart(0),
      m_editor(0),
      m_loadAction(KStandardAction::open(this, SLOT(openScriptFile()), this)),
      m_saveAction(KStandardAction::save(this, SLOT(saveScript()), this)),
      m_clearAction(KStandardAction::clear(this, SLOT(clearEditor()), this)),
      m_executeAction(new KAction(KIcon("system-run"), i18n("&Execute"), this)),
      m_fileDialog(0)
{
    setWindowTitle(KDialog::makeStandardCaption(i18n("Desktop Shell Scripting Console")));
    setAttribute(Qt::WA_DeleteOnClose);
    setButtons(KDialog::None);

    QWidget *widget = new QWidget(m_splitter);
    QVBoxLayout *editorLayout = new QVBoxLayout(widget);

    QLabel *label = new QLabel(i18n("Editor"), widget);
    QFont f = label->font();
    f.setBold(true);
    label->setFont(f);
    editorLayout->addWidget(label);

    KToolBar *toolBar = new KToolBar(this, true, false);
    toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toolBar->addAction(m_loadAction);
    toolBar->addAction(m_saveAction);
    toolBar->addAction(m_clearAction);
    toolBar->addAction(m_executeAction);
    editorLayout->addWidget(toolBar);

    KService::List offers = KServiceTypeTrader::self()->query("KTextEditor/Document");
    foreach (const KService::Ptr service, offers) {
        m_editorPart = service->createInstance<KTextEditor::Document>(widget);
        if (m_editorPart) {
            editorLayout->addWidget(m_editorPart->createView(widget));
            connect(m_editorPart, SIGNAL(textChanged(KTextEditor::Document*)),
                    this, SLOT(scriptTextChanged()));
            break;
        }
    }

    if (!m_editorPart) {
        m_editor = new KTextEdit(widget);
        editorLayout->addWidget(m_editor);
        connect(m_editor, SIGNAL(textChanged()), this, SLOT(scriptTextChanged()));
    }

    m_splitter->addWidget(widget);

    widget = new QWidget(m_splitter);
    QVBoxLayout *outputLayout = new QVBoxLayout(widget);

    label = new QLabel(i18n("Output"), widget);
    f = label->font();
    f.setBold(true);
    label->setFont(f);
    outputLayout->addWidget(label);

    m_output = new KTextBrowser(widget);
    outputLayout->addWidget(m_output);
    m_splitter->addWidget(widget);

    setMainWidget(m_splitter);

    setInitialSize(QSize(500, 400));
    KConfigGroup cg(KGlobal::config(), "InteractiveConsole");
    restoreDialogSize(cg);
    m_splitter->restoreState(cg.readEntry("SplitterState", QByteArray()));

    scriptTextChanged();

    connect(m_engine, SIGNAL(print(QString)), this, SLOT(print(QString)));
    connect(m_engine, SIGNAL(printError(QString)), this, SLOT(print(QString)));
    connect(m_executeAction, SIGNAL(triggered()), this, SLOT(evaluateScript()));

    m_executeAction->setShortcut(Qt::CTRL + Qt::Key_E);
}

InteractiveConsole::~InteractiveConsole()
{
    KConfigGroup cg(KGlobal::config(), "InteractiveConsole");
    saveDialogSize(cg);
    cg.writeEntry("SplitterState", m_splitter->saveState());
}

void InteractiveConsole::loadScript(const QString &script)
{
    if (m_editorPart) {
        m_editorPart->closeUrl(false);
        if (m_editorPart->openUrl(script)) {
            return;
        }
    } else {
        QFile file(KShell::tildeExpand(script));
        if (file.open(QIODevice::ReadOnly | QIODevice::Text) ) {
            m_editor->setText(file.readAll());
            return;
        }
    }


    m_output->append(i18n("Unable to load script file <b>%1</b>", script));
}

void InteractiveConsole::showEvent(QShowEvent *)
{
    if (m_editorPart) {
        m_editorPart->activeView()->setFocus();
    } else {
        m_editor->setFocus();
    }
}

void InteractiveConsole::print(const QString &string)
{
    m_output->append(string);
}

void InteractiveConsole::scriptTextChanged()
{
    const bool enable = m_editorPart ? !m_editorPart->isEmpty() : !m_editor->document()->isEmpty();
    m_saveAction->setEnabled(enable);
    m_clearAction->setEnabled(enable);
    m_executeAction->setEnabled(enable);
}

void InteractiveConsole::openScriptFile()
{
    if (m_fileDialog) {
        delete m_fileDialog;
    }

    m_fileDialog = new KFileDialog(KUrl(), QString(), 0);
    m_fileDialog->setOperationMode(KFileDialog::Opening);
    m_fileDialog->setCaption(i18n("Open Script File"));

    QStringList mimetypes;
    mimetypes << "application/javascript";
    m_fileDialog->setMimeFilter(mimetypes);

    connect(m_fileDialog, SIGNAL(finished()), this, SLOT(openScriptUrlSelected()));
    m_fileDialog->show();
}

void InteractiveConsole::openScriptUrlSelected()
{
    if (!m_fileDialog) {
        return;
    }

    KUrl url = m_fileDialog->selectedUrl();
    m_fileDialog->deleteLater();
    m_fileDialog = 0;

    if (url.isEmpty()) {
        return;
    }

    if (m_editorPart) {
        m_editorPart->openUrl(url);
    } else {
        m_editor->clear();
        m_editor->setEnabled(false);

        if (m_job) {
            m_job->kill();
        }

        m_job = KIO::get(url, KIO::Reload, KIO::HideProgressInfo);
        connect(m_job, SIGNAL(data(KIO::Job*,QByteArray)), this, SLOT(scriptFileDataRecvd(KIO::Job*,QByteArray)));
        connect(m_job, SIGNAL(result(KJob*)), this, SLOT(reenableEditor()));
    }
}

void InteractiveConsole::scriptFileDataRecvd(KIO::Job *job, const QByteArray &data)
{
    Q_ASSERT(m_editor);

    if (job == m_job) {
        m_editor->insertPlainText(data);
    }
}

void InteractiveConsole::saveScript()
{
    if (m_fileDialog) {
        delete m_fileDialog;
    }

    m_fileDialog = new KFileDialog(KUrl(), QString(), 0);
    m_fileDialog->setOperationMode(KFileDialog::Saving);
    m_fileDialog->setCaption(i18n("Open Script File"));

    QStringList mimetypes;
    mimetypes << "application/javascript";
    m_fileDialog->setMimeFilter(mimetypes);

    connect(m_fileDialog, SIGNAL(finished()), this, SLOT(saveScriptUrlSelected()));
    m_fileDialog->show();
}

void InteractiveConsole::saveScriptUrlSelected()
{
    if (!m_fileDialog) {
        return;
    }

    KUrl url = m_fileDialog->selectedUrl();
    if (url.isEmpty()) {
        return;
    }

    if (m_editorPart) {
        m_editorPart->saveAs(url);
    } else {
        m_editor->setEnabled(false);

        if (m_job) {
            m_job->kill();
        }

        m_job = KIO::put(url, -1, KIO::HideProgressInfo);
        connect(m_job, SIGNAL(dataReq(KIO::Job*,QByteArray&)), this, SLOT(scriptFileDataReq(KIO::Job*,QByteArray&)));
        connect(m_job, SIGNAL(result(KJob*)), this, SLOT(reenableEditor()));
    }
}

void InteractiveConsole::scriptFileDataReq(KIO::Job *job, QByteArray &data)
{
    Q_ASSERT(m_editor);

    if (!m_job || m_job != job) {
        return;
    }

    data.append(m_editor->toPlainText().toLocal8Bit());
    m_job = 0;
}

void InteractiveConsole::reenableEditor()
{
    Q_ASSERT(m_editor);
    m_editor->setEnabled(true);
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
    m_engine->evaluateScript(m_editorPart ? m_editorPart->text() : m_editor->toPlainText());

    cursor.insertText("\n\n");
    format.setFontWeight(QFont::Bold);
    // xgettext:no-c-format
    cursor.insertText(i18n("Runtime: %1ms", QString::number(t.elapsed())), format);
    block.setLeftMargin(0);
    cursor.insertBlock(block);
    m_output->ensureCursorVisible();
}

void InteractiveConsole::clearEditor()
{
    if (m_editorPart) {
        m_editorPart->closeUrl(false);
    } else {
        m_editor->clear();
    }
}

#include "interactiveconsole.moc"

