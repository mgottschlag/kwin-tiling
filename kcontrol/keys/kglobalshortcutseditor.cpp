/*
 *  Copyright 2008 Michael Jansen <kde@michael-jansen.biz>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include "kglobalshortcutseditor.h"

#include <QStackedWidget>
#include <QHash>

#include "ui_kglobalshortcutseditor.h"
#include "kactioncollection.h"
#include "kshortcut.h"
#include "kdebug.h"
#include <kglobalaccel.h>

//### copied over from kdedglobalaccel.cpp (figure out a header file to put it in!)
enum actionIdFields
{
    ComponentUnique = 0,
    ActionUnique = 1,
    ComponentFriendly = 2,
    ActionFriendly = 3
};

/*
 * README
 *
 * This class was created because the kshortcutseditor class has some shortcomings. That class uses
 * QTreeWidget and therefore makes it impossible for an outsider to switch the models. But the
 * global shortcuts editor did that. Each global component ( kded, krunner, kopete ... ) was
 * destined to be separately edited. If you selected another component the kshortcutseditor was
 * cleared and refilled. But the items take care of undoing. Therefore when switching the component
 * you lost the undo history.
 *
 * To solve that problem this class keeps one kshortcuteditor for each component. That is easier
 * than rewrite that dialog to a model/view framework.
 *
 * It perfectly covers a bug of KExtedableItemDelegate when clearing and refilling the associated
 * model.
 */

struct componentData
{
    KShortcutsEditor *editor;
    QString uniqueName;
};

class KGlobalShortcutsEditor::KGlobalShortcutsEditorPrivate
{
public:

    KGlobalShortcutsEditorPrivate(KGlobalShortcutsEditor *q)
     : q(q),
       stack(0)
    {}

    void initGUI();

    KGlobalShortcutsEditor *q;
    Ui::KGlobalShortcutsEditor ui;
    QStackedWidget *stack;
    KShortcutsEditor::ActionTypes actionTypes;
    QHash<QString, componentData> components;
};


void KGlobalShortcutsEditor::KGlobalShortcutsEditorPrivate::initGUI()
{
    ui.setupUi(q);
    // Create a stacked widget.
    stack = new QStackedWidget(q);
    q->layout()->addWidget(stack);
    // Connect our components
    connect(ui.components, SIGNAL(activated(const QString&)),
            q, SLOT(activateComponent(const QString&)));
}


KGlobalShortcutsEditor::KGlobalShortcutsEditor(QWidget *parent, KShortcutsEditor::ActionTypes actionTypes)
 : QWidget(parent),
   d(new KGlobalShortcutsEditorPrivate(this))
{
    d->actionTypes = actionTypes;
    // Setup the ui
    d->initGUI();
}


KGlobalShortcutsEditor::~KGlobalShortcutsEditor()
{
    // Before closing the door, undo all changes
    undo();
    delete d;
}


void KGlobalShortcutsEditor::activateComponent(const QString &component)
{
    QHash<QString, componentData>::Iterator iter = d->components.find(component);
    if (iter == d->components.end()) {
        // Unknown component. Its a bad bad world
        kWarning() << "The component " << component << " is unknown";
        Q_ASSERT(iter != d->components.end());
        return;
    } else {
        // Known component. Get it.
        d->stack->setCurrentWidget((*iter).editor);
        KGlobalAccel::self()->overrideMainComponentData(KComponentData((*iter).uniqueName.toAscii()));
    }
}


void KGlobalShortcutsEditor::addCollection(KActionCollection *collection, const QStringList &componentId)
{
    kDebug() << "adding collection " << componentId;
    KShortcutsEditor *editor;
    const QString &friendlyName = componentId[ComponentFriendly];
    // Check if this component is known
    QHash<QString, componentData>::Iterator iter = d->components.find(friendlyName);
    if (iter == d->components.end()) {
        // Unknown component. Create an editor.
        editor = new KShortcutsEditor(this, d->actionTypes);
        d->stack->addWidget(editor);
        // Add to the component combobox
        d->ui.components->addItem(friendlyName);
        // Add to our component registry
        componentData cd;
        cd.editor = editor;
        cd.uniqueName = componentId[ComponentUnique];
        d->components.insert(friendlyName, cd);
        connect(editor, SIGNAL(keyChange()), this, SLOT(_k_key_changed()));
    } else {
        // Known component.
        editor = (*iter).editor;
    }

    // Add the collection to the editor of the component
    editor->addCollection(collection, friendlyName);

    if (d->ui.components->count() > -1) {
        kDebug() << "Activate item " << d->ui.components->itemText(0);
        d->ui.components->setCurrentIndex(0);
        activateComponent(d->ui.components->itemText(0));
    }
}


void KGlobalShortcutsEditor::allDefault()
{
    // The editors are responsible for the reset
    kDebug() << "Reset";
    foreach (const componentData &cd, d->components.values()) {
        cd.editor->allDefault();
    }
}


void KGlobalShortcutsEditor::clear()
{
    // Remove all components and their associated editors
    foreach (const componentData &cd, d->components.values()) {
        delete cd.editor;
    }
    d->components.clear();
    d->ui.components->clear();
}



void KGlobalShortcutsEditor::save()
{
    // The editors are responsible for the saving
    kDebug() << "Save the changes";
    foreach (const componentData &cd, d->components.values()) {
        cd.editor->save();
    }
}


void KGlobalShortcutsEditor::importConfiguration(KConfig *config)
{
    // The editors are responsible for the writing of the scheme
    foreach (const componentData &cd, d->components.values()) {
        cd.editor->importConfiguration(config);
    }
}

void KGlobalShortcutsEditor::exportConfiguration(KConfig *config) const
{
    // The editors are responsible for the writing of the scheme
    foreach (const componentData &cd, d->components.values()) {
        cd.editor->exportConfiguration(config);
    }
}


void KGlobalShortcutsEditor::undo()
{
    // The editors are responsible for the undo
    kDebug() << "Undo the changes";
    foreach (const componentData &cd, d->components.values()) {
        cd.editor->undoChanges();
    }
}


bool KGlobalShortcutsEditor::isModified() const
{
    foreach (const componentData &cd, d->components.values()) {
        if (cd.editor->isModified()) {
            return true;
        }
    }
    return false;
}


void KGlobalShortcutsEditor::_k_key_changed()
{
    emit changed(isModified());
}

#include "kglobalshortcutseditor.moc"
