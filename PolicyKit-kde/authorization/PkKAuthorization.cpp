/**************************************************************************
 *   Copyright (C) 2008 Daniel Nicoletti <dantti85-pk@yahoo.com.br>        *
 *   Copyright (C) 2008 Dario Freddi <drf54321@gmail.com>                  *
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

#include "PkKAuthorization.h"

#include "PoliciesModel.h"
#include "AuthorizationsFilterModel.h"
#include "pkitemdelegate.h"

#include <QHeaderView>

#include <Context>

#include <KMessageBox>
#include <KDebug>

using namespace PolkitQt;

namespace PolkitKde
{

PkKAuthorization::PkKAuthorization(QWidget *parent)
        : QWidget(parent)
        , m_pkKAction(0)
        , m_displayingAction(false)
{
    kDebug() << "Constructing PolicyKitKDE PkKAuthorization";
    // check if our context was correctly set
    if (Context::instance()->hasError()) {
        KMessageBox::error(this,
                           Context::instance()->lastError(),
                           i18n("Failed to initialize PolicyKit context"));
        return;
    }
    m_pkContext = Context::instance()->getPolKitContext();
    connect(Context::instance(), SIGNAL(configChanged()),
            this, SLOT(UpdateActionTree()));

    setupUi(this);
    treeView->header()->hide();

    m_model = new PoliciesModel(this);
    m_proxyModel = new AuthorizationsFilterModel(this);
    m_proxyModel->setSourceModel(m_model);
    m_proxyModel->setDynamicSortFilter(true);
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    treeView->setModel(m_proxyModel);
    treeView->setSortingEnabled(true);
    treeView->setItemDelegate(new PkItemDelegate(this));
    connect(treeView->selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
            this, SLOT(actionClicked(const QModelIndex &)));

    connect(searchLine, SIGNAL(textChanged(const QString &)),
            this, SLOT(setFilterRegExp(const QString &)));

    polkit_context_set_load_descriptions(m_pkContext);

    m_pkKAction = new PkKAction;

    UpdateActionTree();
}

PkKAuthorization::~PkKAuthorization()
{
    if (m_pkKAction) {
        delete m_pkKAction;
    }
}

void PkKAuthorization::setFilterRegExp(const QString &filter)
{
    m_proxyModel->setFilterRegExp(filter);
    // don't expand the tree if there aren't items
    // not sure this is cool
    if (!filter.isEmpty()) {
        treeView->expandAll();
    }
}

void PkKAuthorization::actionClicked(const QModelIndex &index)
{
//     kDebug() << index << "row:" << index.row() << index.data(Qt::DisplayRole);
    if (index.data(PolkitKde::PoliciesModel::IsGroupRole).toBool() == false) {
        PolKitPolicyFileEntry *pfe;
        pfe = index.data(PolkitKde::PoliciesModel::PolkitEntryRole).value<PolKitPolicyFileEntry *>();
//         kDebug() << "PFE: " << pfe;
        if (pfe) {
//             kDebug() << polkit_policy_file_entry_get_id(pfe);
            m_pkKAction->setPolKitPolicyFileEntry(pfe);
            if (!m_displayingAction) {
                label->hide();
                widget->layout()->addWidget(m_pkKAction);
                m_pkKAction->show();
                m_displayingAction = true;
            }
        }
    } else {
        m_displayingAction = false;
        m_pkKAction->hide();
        widget->layout()->addWidget(label);
        label->show();
    }
}

void PkKAuthorization::newAction(const QString &action)
{
    QModelIndex index = m_model->indexFromId(action);
//     kDebug() << "action: " << action << " modelIndex: " << index << index.data(Qt::DisplayRole);

    treeView->setCurrentIndex(m_proxyModel->mapFromSource(index));
}

polkit_bool_t
PkKAuthorization::buildActionList(PolKitPolicyCache *policy_cache, PolKitPolicyFileEntry *pfe, void *user_data)
{
    Q_UNUSED(policy_cache)

//     kDebug() << "Adding an action";
    QList<PolKitPolicyFileEntry *> *pfeList = (QList<PolKitPolicyFileEntry *> *) user_data;
    pfeList->append(pfe);

    // keep iterating
    return false;
}

void PkKAuthorization::UpdateActionTree()
{
    pkPFileEntry.clear();

    PolKitPolicyCache *pkPCache;
    pkPCache = polkit_context_get_policy_cache(m_pkContext);
    polkit_policy_cache_foreach(pkPCache, buildActionList, &pkPFileEntry);
    m_model->setCurrentEntries(pkPFileEntry);

    //Re selects the current index so the action view is updated
    actionClicked(treeView->selectionModel()->currentIndex());
    // set the filter again as the filter
    // makes empty groups disappear and they might
    // not be fully populated when the filter is applied
    setFilterRegExp(searchLine->text());
}


}

#include "PkKAuthorization.moc"
