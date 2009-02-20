/*
 *  Copyright (C) 2003-2006 Andriy Rysin (rysin@kde.org)
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

#ifndef __KCM_LAYOUT_H__
#define __KCM_LAYOUT_H__


#include <QHash>
#include <kcmodule.h>

#include "kxkbconfig.h"


class QWidget;
class KActionCollection;
class Ui_LayoutConfigWidget;
class XkbRules;

class SrcLayoutModel;
class DstLayoutModel;
class XkbOptionsModel;

static bool localeAwareLessThan(const QString &s1, const QString &s2)
{
    return QString::localeAwareCompare(s1, s2) < 0;
}

// sort by locale-aware value string
static QList<QString> getKeysSortedByVaue(const QHash<QString, QString>& map)
{
    QList<QString> outList;

    QMap<QString, QString> reverseMap;
    // we have to add nums as translations can be dups and them reverse map will miss items
    int i=0;
    QString fmt("%1%2");
    foreach (const QString& str, map.keys())
        reverseMap.insert(fmt.arg(map[str], QString::number(i++)), str);

    QList<QString> values = reverseMap.keys();
    qSort(values.begin(), values.end(), localeAwareLessThan);

    foreach (const QString& value, values)
        outList << reverseMap[value];
/*
    int diff = map.keys().count() - reverseMap.keys().count();
    if( diff > 0 ) {
        kDebug() << "original keys" << map.keys().count() << "reverse map" << reverseMap.keys().count() 
            << "- translation encoding must have been messed up - padding layouts...";
        for(int i=0; i<diff; i++)
            reverseMap.insert(QString("%1%2").arg("nocrash", i), "nocrash");
    }
*/
    return outList;
}

enum {
    LAYOUT_COLUMN_FLAG = 0,
    LAYOUT_COLUMN_NAME = 1,
    LAYOUT_COLUMN_MAP = 2,
    LAYOUT_COLUMN_VARIANT = 3,
    LAYOUT_COLUMN_DISPLAY_NAME = 4,
    SRC_LAYOUT_COLUMN_COUNT = 3,
    DST_LAYOUT_COLUMN_COUNT = 5
};

enum { TAB_LAYOUTS=0, TAB_OPTIONS=1, TAB_XKB=2 };
enum { BTN_XKB_ENABLE=0, BTN_XKB_INDICATOR=1, BTN_XKB_DISABLE=2 };



class LayoutConfig : public KCModule
{
  Q_OBJECT

public:
  LayoutConfig(QWidget *parent, const QVariantList &args);
  virtual ~LayoutConfig();

  void load();
  void save();
  void defaults();
  void initUI();

protected:
    QString createOptionString();

protected slots:
    void moveUp();
    void moveDown();
    void variantChanged();
    void showFlagChanged(bool on);
    void xkbShortcutPressed();
    void xkbShortcut3dPressed();
    void clearXkbSequence();
    void clearXkb3dSequence();
    void updateShortcutsLabels();
    void xkbOptionsChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight);
    void enableChanged();
    void updateGroupsFromServer();
    void displayNameChanged(const QString& name);
    void layoutSelChanged();
    void loadRules();
    void refreshRulesUI();
    void updateLayoutCommand();
    void updateOptionsCommand();
    void add();
    void remove();

    void changed();

private:
    const QString DEFAULT_VARIANT_NAME;
    Ui_LayoutConfigWidget* widget;

    XkbRules *m_rules;
    KxkbConfig m_kxkbConfig;
    SrcLayoutModel* m_srcModel;
    DstLayoutModel* m_dstModel;
    XkbOptionsModel* m_xkbOptModel;
    KActionCollection* actionCollection;

  void makeOptionsTab();
  void updateStickyLimit();
  void updateAddButton();
  void updateDisplayName();
  void moveSelected(int shift);
  int getSelectedDstLayout();
};

class SrcLayoutModel: public QAbstractTableModel {

Q_OBJECT

public:
    SrcLayoutModel(XkbRules* rules, QObject *parent)
    : QAbstractTableModel(parent)
    { setRules(rules); }
//    bool hasChildren ( const QModelIndex & parent = QModelIndex() ) const { return false; }
    int columnCount(const QModelIndex& parent) const { return !parent.isValid() ? SRC_LAYOUT_COLUMN_COUNT : 0; }
    int rowCount(const QModelIndex&) const { return m_rules->layouts().keys().count(); }
    QVariant data(const QModelIndex& index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    void setRules(XkbRules* rules) { 
        m_rules = rules; 
        m_layoutKeys = getKeysSortedByVaue( m_rules->layouts() );
    }
    QString getLayoutAt(int row) { return m_layoutKeys[row]; }
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);                       
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QMimeData* mimeData(const QModelIndexList &indexes) const;

private:
    XkbRules* m_rules;
    QStringList m_layoutKeys;

signals:
    void layoutRemoved();
};

class DstLayoutModel: public QAbstractTableModel {

Q_OBJECT

public:
    DstLayoutModel(XkbRules* rules, KxkbConfig* kxkbConfig, QObject *parent)
    : QAbstractTableModel(parent),
    m_kxkbConfig(kxkbConfig)
    { setRules(rules); }
    int columnCount(const QModelIndex& parent) const { return !parent.isValid() ? DST_LAYOUT_COLUMN_COUNT : 0; }
    int rowCount(const QModelIndex&) const { return m_kxkbConfig->m_layouts.count(); }
    QVariant data(const QModelIndex& index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    void setRules(XkbRules* rules) { m_rules = rules; }
    void reset() { QAbstractTableModel::reset(); }
    void emitDataChange(int row, int col) { emit dataChanged(createIndex(row,col),createIndex(row,col)); }
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);                       
    Qt::ItemFlags flags(const QModelIndex &index) const;

private:
    XkbRules* m_rules;
    KxkbConfig* m_kxkbConfig;

signals:
    void layoutAdded();
};

#endif

