//
// C++ Implementation: kcolortreewidget
//
// Description: 
//
//
// Author: Olivier Goffart
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "kcolortreewidget.h"
#include <klocale.h>
#include <kdebug.h>
#include <kcolordialog.h>

#include <QColor>
#include <QItemDelegate>
#include <QPainter>


//BEGIN KColorTreeWidgetItem
/*
    QListViewItem subclass to display/edit a style, bold/italic is check boxes,
    normal and selected colors are boxes, which will display a color chooser when
    activated.
    The context name for the style will be drawn using the editor default font and
    the chosen colors.
    This widget id designed to handle the default as well as the individual hl style
    lists.
    This widget is designed to work with the KateStyleTreeWidget class exclusively.
    Added by anders, jan 23 2002.
*/
class KColorTreeWidgetItem : public QTreeWidgetItem
{
  public:
    KColorTreeWidgetItem( QTreeWidget *parent, const QString& role , int idx );
    ~KColorTreeWidgetItem() {};

    QString role() const { return text(0); };
    /* only true for a hl mode item using it's default style */
    bool defStyle() const;
    
    int index() { return m_idx; }

    virtual QVariant data( int column, int role ) const;
    virtual void setData( int column, int role, const QVariant& value );
    
    QColor color() {  return data(1 , Qt::BackgroundColorRole).value<QColor>(); }
    void setColor(const QColor& col) { setData( 1 , Qt::BackgroundColorRole , col ); };

  private:
      int m_idx;
};


KColorTreeWidgetItem::KColorTreeWidgetItem( QTreeWidget * parent, const QString & role, int idx )
    : QTreeWidgetItem( parent ), m_idx(idx)
{
    setText(0, role);
}

QVariant KColorTreeWidgetItem::data( int column, int role ) const
{
//     if (column == 1) {
//         switch (role) {
//             case Qt::TextColorRole:
//                 if (style()->hasProperty(QTextFormat::ForegroundBrush))
//                     return style()->foreground().color();
//                 break;
// 
//             case Qt::BackgroundColorRole:
//                 if (style()->hasProperty(QTextFormat::BackgroundBrush))
//                     return style()->background().color();
//                 break;
// 
//             case Qt::FontRole:
//                 return style()->font();
//                 break;
//         }
//     }
// 
//     if (role == Qt::DisplayRole) {
//         switch (column) {
//             case Foreground:
//                 return style()->foreground();
//             case SelectedForeground:
//                 return style()->selectedForeground();
//             case Background:
//                 return style()->background();
//             case SelectedBackground:
//                 return style()->selectedBackground();
//         }
//     }

    return QTreeWidgetItem::data(column, role);
}

void KColorTreeWidgetItem::setData( int column, int role, const QVariant& value )
{
    QTreeWidgetItem::setData(column, role, value);
}


//END


//BEGIN KColorTreeDelegate
class KColorTreeDelegate : public QItemDelegate
{
    public:
        KColorTreeDelegate(QWidget* widget) : m_widget(widget) {};

        virtual void paint( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const;

    private:
        QWidget* m_widget;
};


void KColorTreeDelegate::paint( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    if (index.column() != 1)
        return QItemDelegate::paint(painter, option, index);

    QColor bkColor =  qVariantValue<QBrush>(index.data(Qt::BackgroundColorRole));

    QBrush brush(bkColor);
    QStyleOptionButton opt;
    opt.rect = option.rect;
    opt.palette = m_widget->palette();

    bool set = brush != QBrush();

    if (!set) {
        opt.text = i18nc("No text or background colour set", "None set");
        brush = Qt::white;
    }

    m_widget->style()->drawControl(QStyle::CE_PushButton, &opt, painter, m_widget);

    if (set)
        painter->fillRect(m_widget->style()->subElementRect(QStyle::SE_PushButtonContents, &opt,m_widget), brush);
}

//END




KColorTreeWidget::KColorTreeWidget(QWidget *parent)
  : QTreeWidget(parent)
{
  setItemDelegate(new KColorTreeDelegate(this));

  setColumnCount( 2 );
  QStringList headers;
  headers << i18n("Role") << i18n("Color");
  setHeaderLabels(headers);
}


KColorTreeWidget::~KColorTreeWidget()
{
}

void KColorTreeWidget::addRole( int idx, const QString & role )
{
    m_items[idx] = new KColorTreeWidgetItem( this , role, idx);
}

void KColorTreeWidget::setColor( int idx, const QColor & color )
{
//    kDebug() << k_funcinfo << idx << endl;
    if(!m_items.contains(idx))
        return;
    KColorTreeWidgetItem *item = m_items[idx];
    item->setColor( color );
    emit colorChanged(idx, color);
}

QColor KColorTreeWidget::color( int idx )
{
    if(!m_items.contains(idx))
        return QColor();
    KColorTreeWidgetItem *item = m_items[idx];
    return item->color();
}

bool KColorTreeWidget::edit( const QModelIndex & index, EditTrigger trigger, QEvent * event )
{
    KColorTreeWidgetItem *i = dynamic_cast<KColorTreeWidgetItem*>(itemFromIndex(index));
    if (!i || index.column() != 1)
        return QTreeWidget::edit(index, trigger, event);

    switch (trigger) {
        case QAbstractItemView::DoubleClicked:
        case QAbstractItemView::SelectedClicked:
        case QAbstractItemView::EditKeyPressed:
        {
            QColor c;
            QColor d;
            if ( KColorDialog::getColor( c, d, this ) != QDialog::Accepted) 
                return false;
            setColor( i->index() , c);
            return false;
        }
        default:
            return QTreeWidget::edit(index, trigger, event);
    }

}

#include "kcolortreewidget.moc"


