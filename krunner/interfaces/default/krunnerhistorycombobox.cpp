/***************************************************************************
 *   Copyright 2009 by Jacopo De Simoi <wilderkde@gmail.com>               *
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

#include "krunnerhistorycombobox.h"

#include <QApplication>
#include <QTimer>

#include <KDebug>
#include <KLineEdit>
#include <KStandardShortcut>

KrunnerHistoryComboBox::KrunnerHistoryComboBox(bool useCompletion, QWidget * parent)
    : KHistoryComboBox(useCompletion,  parent),
      m_addingToHistory(false)
{
    setPalette(QApplication::palette());
    setDuplicatesEnabled(false);
    setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);

    // in theory, the widget should detect the direction from the content
    // but this is not available in Qt4.4/KDE 4.2, so the best default for this widget
    // is LTR: as it's more or less a "command line interface"
    // FIXME remove this code when KLineEdit has automatic direction detection of the "paragraph"
    setLayoutDirection(Qt::LeftToRight);
}

KrunnerHistoryComboBox::~KrunnerHistoryComboBox()
{
}

void KrunnerHistoryComboBox::addToHistory(const QString & item)
{
    m_addingToHistory = true;
    KHistoryComboBox::addToHistory(item);
    m_addingToHistory = false;
}

void KrunnerHistoryComboBox::setLineEdit(QLineEdit *e)
{
    if (lineEdit()) {
        disconnect(lineEdit(), 0, this, 0);
    }

    KComboBox::setLineEdit(e);
    QTimer::singleShot(50, this, SLOT(connectLineEdit()));
}

void KrunnerHistoryComboBox::connectLineEdit()
{
    disconnect(this, SIGNAL(currentIndexChanged(QString)), this, SLOT(currentIndexChanged(QString)));
    connect(this, SIGNAL(currentIndexChanged(QString)), this, SLOT(currentIndexChanged(QString)));

    if (lineEdit()) {
        disconnect(lineEdit(), SIGNAL(textEdited(QString)), this, SIGNAL(queryTextEdited(QString)));
        connect(lineEdit(), SIGNAL(textEdited(QString)), this, SIGNAL(queryTextEdited(QString)));
    }
}

void KrunnerHistoryComboBox::focusOutEvent(QFocusEvent *e)
{
    discardCompletion();
    KHistoryComboBox::focusOutEvent(e);
}

void KrunnerHistoryComboBox::wheelEvent(QWheelEvent *e)
{
    KHistoryComboBox::wheelEvent(e);
    emit queryTextEdited(lineEdit()->text());
}

void KrunnerHistoryComboBox::keyPressEvent(QKeyEvent *e)
{
    bool enterPressed = ( e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter );
    if (enterPressed) {
        discardCompletion();
    }

    KHistoryComboBox::keyPressEvent(e);

    int event_key = e->key() | e->modifiers();
    if (KStandardShortcut::rotateUp().contains(event_key) ||
        KStandardShortcut::rotateDown().contains(event_key)) {
        emit queryTextEdited(lineEdit()->text());
    }
}

void KrunnerHistoryComboBox::discardCompletion()
{
    //FIXME: find a reliable way to see if the scene is empty; now defaults to
    //       never complete
    bool emptyScene = false;
    KLineEdit* edit = static_cast<KLineEdit*>(lineEdit());
    bool suggestedCompletion = (edit->text() != edit->userText());

    if (emptyScene &&  suggestedCompletion) {
        // We hit TAB with an empty scene and a suggested completion:
        // Complete but don't lose focus
        edit->setText(edit->text());
    } else if (suggestedCompletion) {
        // We hit TAB with a non-empty scene and a suggested completion:
        // Assume the user wants to switch input to the results scene and discard the completion
        edit->setText(edit->userText());
    }
}

void KrunnerHistoryComboBox::currentIndexChanged(const QString &item)
{
    if (!m_addingToHistory) {
        emit queryTextEdited(item);
    }
}

#include "krunnerhistorycombobox.moc"
