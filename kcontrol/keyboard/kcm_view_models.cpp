/*
 *  Copyright (C) 2010 Andriy Rysin (rysin@kde.org)
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

#include "kcm_view_models.h"

#include <kdebug.h>
#include <klocalizedstring.h>
#include <QtGui/QTreeView>
#include <QtGui/QComboBox>

#include "keyboard_config.h"
#include "xkb_rules.h"
#include "flags.h"
#include "x11_helper.h"


static const int MAP_COLUMN = 0;
static const int LAYOUT_COLUMN = 1;
static const int VARIANT_COLUMN = 2;
static const int DISPLAY_NAME_COLUMN = 3;


LayoutsTableModel::LayoutsTableModel(Rules* rules_, Flags *flags_, KeyboardConfig* keyboardConfig_, QObject* parent):
	QAbstractTableModel(parent),
	keyboardConfig(keyboardConfig_),
	rules(rules_),
	countryFlags(flags_)
{
}

void LayoutsTableModel::refresh()
{
	beginResetModel();
	endResetModel();
}

int LayoutsTableModel::rowCount(const QModelIndex &/*parent*/) const
{
	return keyboardConfig->layouts.count();
}

int LayoutsTableModel::columnCount(const QModelIndex&) const
{
	return 4;
}

Qt::ItemFlags LayoutsTableModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return 0;

	Qt::ItemFlags flags = QAbstractTableModel::flags(index);

	if( index.column() == DISPLAY_NAME_COLUMN ) {
		flags |= Qt::ItemIsEditable;
	}

	return flags;
}

QVariant LayoutsTableModel::data(const QModelIndex &index, int role) const
{
     if (!index.isValid())
         return QVariant();

     if (index.row() >= keyboardConfig->layouts.size())
         return QVariant();

	 const LayoutConfig& layoutConfig = keyboardConfig->layouts.at(index.row());

     if (role == Qt::DecorationRole) {
    	 switch( index.column() ) {
    	 case MAP_COLUMN: {
//    			const QPixmap* pixmap = flags->getPixmap(layoutConfig.layout);
//    			return pixmap != NULL ? *pixmap : QVariant();
    			return countryFlags->getIcon(layoutConfig.layout);
    	 }
    	 break;
    	 }
     }
     else
     if (role == Qt::DisplayRole) {
    	 switch( index.column() ) {
    	 case MAP_COLUMN:
    		 return layoutConfig.layout;
    	 break;
    	 case LAYOUT_COLUMN: {
    		 const LayoutInfo* layoutInfo = rules->getLayoutInfo(layoutConfig.layout);
             return layoutInfo != NULL ? layoutInfo->description : layoutConfig.layout;
    	 }
    	 case VARIANT_COLUMN: {
    		 if( layoutConfig.variant.isEmpty() )
    			 return QVariant();
    		 const LayoutInfo* layoutInfo = rules->getLayoutInfo(layoutConfig.layout);
    		 if( layoutInfo == NULL )
    			 return QVariant();
    		 const VariantInfo* variantInfo = layoutInfo->getVariantInfo(layoutConfig.variant);
    		 return variantInfo != NULL ? variantInfo->description : layoutConfig.variant;
    	 }
         break;
    	 case DISPLAY_NAME_COLUMN:
    		 return layoutConfig.getDisplayName();
    	 break;
    	 }
     }
     else if (role==Qt::EditRole ) {
    	 switch( index.column() ) {
    	 case DISPLAY_NAME_COLUMN:
    		 return layoutConfig.getDisplayName();
    	 break;
    	 default:;
    	 }
     }
     return QVariant();
}

QVariant LayoutsTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
     if (role != Qt::DisplayRole)
         return QVariant();

     if (orientation == Qt::Horizontal) {
	 const QString headers[] = {i18nc("layout map name", "Map"), i18n("Layout"), i18n("Variant"), i18n("Label")};
         return headers[section];
     }

     return QVariant();
}

bool LayoutsTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole || index.column() != DISPLAY_NAME_COLUMN )
        return false;

    if (index.row() >= keyboardConfig->layouts.size())
        return false;

	 LayoutConfig& layoutConfig = keyboardConfig->layouts[index.row()];
	 QString displayText = value.toString().left(3);
	 layoutConfig.setDisplayName(displayText);

//    TreeItem *item = getItem(index);
//    bool result = item->setData(index.column(), value);
//
//    if (result)
	 kDebug() << "new display text" << displayText << layoutConfig.getDisplayName();
        emit dataChanged(index, index);

    return true; //result;
}



//
// Xkb Options Tree View
//

int XkbOptionsTreeModel::rowCount(const QModelIndex& parent) const {
    if( ! parent.isValid() )
        return rules->optionGroupInfos.count();
    if( ! parent.parent().isValid() )
        return rules->optionGroupInfos[parent.row()]->optionInfos.count();
    return 0;
}

QVariant XkbOptionsTreeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int row = index.row();

    if (role == Qt::DisplayRole) {
        if( ! index.parent().isValid() ) {
            return rules->optionGroupInfos[row]->description;
        }
        else {
            int groupRow = index.parent().row();
            const OptionGroupInfo* xkbGroup = rules->optionGroupInfos[groupRow];
            return xkbGroup->optionInfos[row]->description;
        }
    }
    else if (role==Qt::CheckStateRole && index.parent().isValid()) {
        int groupRow = index.parent().row();
        const OptionGroupInfo* xkbGroup = rules->optionGroupInfos[groupRow];
        const QString& xkbOptionName = xkbGroup->optionInfos[row]->name;
        return keyboardConfig->xkbOptions.indexOf(xkbOptionName) == -1
        		? Qt::Unchecked : Qt::Checked;
    }
    return QVariant();
}

bool XkbOptionsTreeModel::setData(const QModelIndex & index, const QVariant & value, int role) {
    int groupRow = index.parent().row();
    if( groupRow < 0 ) return false;

    const OptionGroupInfo* xkbGroup = rules->optionGroupInfos[groupRow];
    const OptionInfo* option = xkbGroup->optionInfos[index.row()];

    if( value.toInt() == Qt::Checked ) {
        if( xkbGroup->exclusive ) {
            // clear if exclusive (TODO: radiobutton)
            int idx = keyboardConfig->xkbOptions.indexOf(QRegExp(xkbGroup->name + ".*"));
            if( idx >= 0 ) {
                for(int i=0; i<xkbGroup->optionInfos.count(); i++)
                    if( xkbGroup->optionInfos[i]->name == keyboardConfig->xkbOptions[idx] ) {
                        setData(createIndex(i, index.column(), (quint32)index.internalId()-index.row()+i), Qt::Unchecked, role);
                        break;
                    }
            //    m_kxkbConfig->m_options.removeAt(idx);
            //    idx = m_kxkbConfig->m_options.indexOf(QRegExp(xkbGroupNm+".*"));
            }
        }
        if( keyboardConfig->xkbOptions.indexOf(option->name) < 0 ) {
            keyboardConfig->xkbOptions.append(option->name);
        }
    }
    else {
        keyboardConfig->xkbOptions.removeAll(option->name);
    }

    emit dataChanged(index, index);
    return true;
}

void XkbOptionsTreeModel::gotoGroup(const QString& groupName, QTreeView* view) {
	const OptionGroupInfo* optionGroupInfo = rules->getOptionGroupInfo(groupName);
    int index = rules->optionGroupInfos.indexOf((OptionGroupInfo*)optionGroupInfo);
    if( index != -1 ) {
        QModelIndex modelIdx = createIndex(index,0);
//            view->selectionModel()->setCurrentIndex(createIndex(index,0), QItemSelectionModel::NoUpdate);
        view->setExpanded(modelIdx, true);
        view->scrollTo(modelIdx, QAbstractItemView::PositionAtTop);
        view->selectionModel()->setCurrentIndex(modelIdx, QItemSelectionModel::Current);
        view->setFocus(Qt::OtherFocusReason);
    }
//    else {
//        kDebug() << "can't scroll to group" << group;
//    }
}

