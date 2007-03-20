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
#include <kglobalsettings.h>


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
    KColorTreeWidgetItem( QTreeWidget *parent, const QString& role , int fg_idx , int bg_idx );
    ~KColorTreeWidgetItem() {}

    QString role() const { return text(0); }
    /* only true for a hl mode item using it's default style */
    bool defStyle() const;
    
    int bgIndex() { return m_bg_idx; }
    int fgIndex() { return m_fg_idx; }

    virtual QVariant data( int column, int role ) const;
    virtual void setData( int column, int role, const QVariant& value );
    
    QColor textColor() {  return data(1 , Qt::BackgroundColorRole).value<QColor>(); }
    void setTextColor(const QColor& col); 
    QColor bgColor() {  return data(2 , Qt::BackgroundColorRole).value<QColor>(); }
    void setBgColor(const QColor& col);

  private:
      int m_fg_idx;
      int m_bg_idx;
};


KColorTreeWidgetItem::KColorTreeWidgetItem( QTreeWidget * parent, const QString & role, int f_idx, int b_idx )
    : QTreeWidgetItem( parent ), m_fg_idx(f_idx), m_bg_idx(b_idx)
{
    setText(0, role);
}

void KColorTreeWidgetItem::setTextColor( const QColor & col )
{
    setData( 1 , Qt::BackgroundColorRole , col );
    setData( 0 , Qt::TextColorRole , col );
}

void KColorTreeWidgetItem::setBgColor( const QColor & col )
{
    setData( 2 , Qt::BackgroundColorRole , col );
    setData( 0 , Qt::BackgroundColorRole , col );
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
        KColorTreeDelegate(QWidget* widget) : m_widget(widget) {}

        virtual void paint( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const;

    private:
        QWidget* m_widget;
};


void KColorTreeDelegate::paint( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    if (index.column() == 0)
        return QItemDelegate::paint(painter, option, index);

    QBrush brush =  qVariantValue<QBrush>(index.data(Qt::BackgroundColorRole));

    QStyleOptionButton opt;
    opt.rect = option.rect;
    opt.palette = m_widget->palette();

    if(brush != QBrush())
    {
        m_widget->style()->drawControl(QStyle::CE_PushButton, &opt, painter, m_widget);
        painter->fillRect(m_widget->style()->subElementRect(QStyle::SE_PushButtonContents, &opt,m_widget), brush);
    }
    else
    {
        painter->fillRect(option.rect,QBrush(KGlobalSettings::textColor(), Qt::FDiagPattern) );
    }

//     if (!set) {
//         opt.text = i18nc("No text or background colour set", "None set");
//         //brush = Qt::white;
//         brush = QBrush(Qt::white, Qt::DiagCrossPattern);
//     }

}

//END




KColorTreeWidget::KColorTreeWidget(QWidget *parent)
  : QTreeWidget(parent)
{
  setItemDelegate(new KColorTreeDelegate(this));

  setColumnCount( 3 );
  QStringList headers;
  headers << i18n("Role") << i18n("Foreground") << i18n("Background");
  setHeaderLabels(headers);
}


KColorTreeWidget::~KColorTreeWidget()
{
}

void KColorTreeWidget::addRole( int idx_f , int idx_g , const QString & role )
{
    KColorTreeWidgetItem* i =  new KColorTreeWidgetItem( this , role, idx_f, idx_g);
    m_bgItems[idx_g] = i;
    m_fgItems[idx_f] = i;
}

void KColorTreeWidget::setColor( int idx, const QColor & color )
{
//    kDebug() << k_funcinfo << idx << endl;
    if(m_bgItems.contains(idx))
    {
        m_bgItems[idx]->setBgColor( color );
        emit colorChanged(idx, color);
    }
    else if(m_fgItems.contains(idx))
    {
        m_fgItems[idx]->setTextColor( color );
        emit colorChanged(idx, color);
    }
        
}

QColor KColorTreeWidget::color( int idx )
{
    if(m_bgItems.contains(idx))
        return m_bgItems[idx]->bgColor();
    else if(m_fgItems.contains(idx))
        return m_fgItems[idx]->textColor();

    return QColor();
}

bool KColorTreeWidget::edit( const QModelIndex & index, EditTrigger trigger, QEvent * event )
{
    KColorTreeWidgetItem *i = dynamic_cast<KColorTreeWidgetItem*>(itemFromIndex(index));
    if (!i || index.column() == 0)
        return QTreeWidget::edit(index, trigger, event);
    
    if( (index.column() == 1 && i->fgIndex() == -1)  ||
            (index.column() == 2 && i->bgIndex() == -1) )
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
            setColor( (index.column() == 1) ? i->fgIndex() : i->bgIndex() , c);
            return false;
        }
        default:
            return QTreeWidget::edit(index, trigger, event);
    }

}


#include "kcolortreewidget.moc"


