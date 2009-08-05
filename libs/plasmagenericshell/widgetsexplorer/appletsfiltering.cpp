#include "appletsfiltering.h"

#include <kglobalsettings.h>
#include <klineedit.h>

#include <plasma/theme.h>

//FilteringList

FilteringList::FilteringList(QGraphicsItem * parent, Qt::WindowFlags wFlags)
    : QGraphicsWidget(parent, wFlags)
{
    init();
    connect(m_treeView->nativeWidget(), SIGNAL(clicked(const QModelIndex &)), this, SLOT(filterChanged(const QModelIndex &)));
}

FilteringList::~FilteringList()
{
}

void FilteringList::init()
{
    m_treeView = new Plasma::TreeView();
    m_treeView->nativeWidget()->setAttribute(Qt::WA_NoSystemBackground);
    m_treeView->nativeWidget()->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_treeView->nativeWidget()->setRootIsDecorated(false);
    m_treeView->nativeWidget()->setAttribute(Qt::WA_TranslucentBackground);

    QFont listFont = m_treeView->nativeWidget()->font();
    listFont.setPointSize(KGlobalSettings::smallestReadableFont().pointSize());
    m_treeView->nativeWidget()->setFont(listFont);

    QColor textColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    QPalette plasmaPalette = QPalette();
    plasmaPalette.setColor(QPalette::Base,
                           Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor));
    plasmaPalette.setColor(QPalette::Text, textColor);

    m_treeView->setPalette(plasmaPalette);
    m_treeView->nativeWidget()->setAutoFillBackground(true);

    QGraphicsLinearLayout *linearLayout = new QGraphicsLinearLayout();
    linearLayout->addItem(m_treeView);
    setLayout(linearLayout);
    m_treeView->nativeWidget()->header()->setVisible(false);
    //m_treeView->nativeWidget()->setPalette((new Plasma::IconWidget())->palette());
}

void FilteringList::setModel(QStandardItemModel *model)
{
    m_model = model;
    m_treeView->setModel(m_model);
}


void FilteringList::filterChanged(const QModelIndex & index)
{
    emit(filterChanged(index.row()));
}

//FilteringWidget

FilteringWidget::FilteringWidget(QGraphicsItem * parent, Qt::WindowFlags wFlags)
    : QGraphicsWidget(parent, wFlags)
{
    m_backgroundSvg = new Plasma::FrameSvg(this);
    m_backgroundSvg->setImagePath("widgets/background");

    init();
}

FilteringWidget::~FilteringWidget(){
}

void FilteringWidget::init()
{
    m_filterLabel = new Plasma::Label();
    m_filterLabel->nativeWidget()->setText("Filter");

    QFont labelFont = m_filterLabel->font();
    labelFont.setPointSize(KGlobalSettings::smallestReadableFont().pointSize());
    m_filterLabel->setFont(labelFont);

    m_textSearch = new Plasma::LineEdit();
    m_textSearch->nativeWidget()->setClickMessage(/*i18n(*/"Type search"/*)*/);
    m_textSearch->setFocus();
    m_textSearch->setAttribute(Qt::WA_NoSystemBackground);
    m_textSearch->setMaximumHeight(30);
    m_textSearch->setMinimumHeight(30);

    m_categoriesList = new FilteringList();

    QGraphicsLinearLayout *linearLayout = new QGraphicsLinearLayout(Qt::Vertical);

    linearLayout->addItem(m_filterLabel);
    linearLayout->addItem(m_textSearch);
    linearLayout->addItem(m_categoriesList);

    this->setLayout(linearLayout);
    linearLayout->setContentsMargins(15,15,15,15);
}

void FilteringWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
 {
     QGraphicsWidget::paint(painter, option, widget);
     m_backgroundSvg->resizeFrame(contentsRect().size());
     m_backgroundSvg->paintFrame(painter, contentsRect().topLeft());
 }

FilteringList *FilteringWidget::categoriesList()
{
    return m_categoriesList;
}

Plasma::LineEdit *FilteringWidget::textSearch()
{
    return m_textSearch;
}
