#include "applettooltip.h"

#include <kiconloader.h>

//AppletToolTipWidget

AppletToolTipWidget::AppletToolTipWidget(QWidget *parent, AppletIconWidget *applet)
        : Plasma::Dialog(parent)
{
    m_applet = applet;
    m_widget = new AppletInfoWidget();
    connect(m_widget, SIGNAL(infoButtonClicked(QString)), this, SIGNAL(infoButtonClicked(QString)));
    QGraphicsScene * scene = new QGraphicsScene();
    scene->addItem(m_widget);
    if(m_applet) {
        m_widget->setAppletItem(m_applet->appletItem());
    }
    setGraphicsWidget(m_widget);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);
}

AppletToolTipWidget::~AppletToolTipWidget()
{
}

void AppletToolTipWidget::setAppletIconWidget(AppletIconWidget *applet)
{
    m_applet = applet;
    m_widget->setAppletItem(m_applet->appletItem());
}

void AppletToolTipWidget::updateContent()
{
    m_widget->updateInfo();
}

AppletIconWidget *AppletToolTipWidget::appletIconWidget()
{
    return m_applet;
}

void AppletToolTipWidget::enterEvent(QEvent *event)
{
    Q_UNUSED(event);
    emit(enter());
}

void AppletToolTipWidget::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    emit(leave());
}

//AppletInfoWidget

AppletInfoWidget::AppletInfoWidget(QGraphicsItem *parent, PlasmaAppletItem *appletItem)
        : QGraphicsWidget(parent)
{
    m_appletItem = appletItem;
    init();
}

AppletInfoWidget::~AppletInfoWidget()
{
}

void AppletInfoWidget::init()
{
    m_iconWidget = new Plasma::IconWidget();
    m_iconWidget->setAcceptHoverEvents(false);
    m_iconWidget->setAcceptedMouseButtons(false);
    m_iconWidget->setMinimumSize(IconSize(KIconLoader::Desktop), IconSize(KIconLoader::Desktop));
    m_iconWidget->setMaximumSize(IconSize(KIconLoader::Desktop), IconSize(KIconLoader::Desktop));

    m_nameLabel = new Plasma::Label();
    QFont nameFont = m_nameLabel->nativeWidget()->font();
    nameFont.setBold(true);
    nameFont.setPointSize(1.2 * nameFont.pointSize());
    m_nameLabel->nativeWidget()->setFont(nameFont);
    m_nameLabel->nativeWidget()->setScaledContents(true);
    m_nameLabel->nativeWidget()->setWordWrap(true);
    // m_nameLabel->setMinimumSize(0,0);

    m_descriptionLabel = new Plasma::Label();
    QFont descriptionFont = m_descriptionLabel->nativeWidget()->font();
    descriptionFont.setPointSize(1.2 * descriptionFont.pointSize());
    m_descriptionLabel->nativeWidget()->setFont(descriptionFont);
    m_descriptionLabel->setScaledContents(true);
    m_descriptionLabel->nativeWidget()->setWordWrap(true);
    // m_descriptionLabel->setMinimumSize(0,0);

    m_infoButton = new Plasma::IconWidget();
    m_infoButton->setIcon("help-about");
    m_infoButton->setMinimumSize(IconSize(KIconLoader::MainToolbar), IconSize(KIconLoader::MainToolbar));
    m_infoButton->setMaximumSize(IconSize(KIconLoader::MainToolbar), IconSize(KIconLoader::MainToolbar));
    connect(m_infoButton, SIGNAL(clicked()), this, SLOT(onInfoButtonClick()));

    m_linearLayout = new QGraphicsLinearLayout();
    m_linearLayout->setOrientation(Qt::Horizontal);
    QGraphicsLinearLayout *vLayout = new QGraphicsLinearLayout();
    vLayout->setOrientation(Qt::Vertical);

    m_linearLayout->addItem(m_iconWidget);
    vLayout->addItem(m_nameLabel);
    vLayout->addItem(m_descriptionLabel);
    m_linearLayout->addItem(vLayout);
    m_linearLayout->addItem(m_infoButton);

    setLayout(m_linearLayout);

    m_linearLayout->setAlignment(m_infoButton, Qt::AlignRight);
}

void AppletInfoWidget::setAppletItem(PlasmaAppletItem *appletItem)
{
    m_appletItem = appletItem;
}

void AppletInfoWidget::updateInfo()
{
    if(m_appletItem != 0) {
        m_iconWidget->setIcon(m_appletItem->icon());
        // m_descriptionLabel->nativeWidget()->setMinimumSize(0,0);
        m_descriptionLabel->setText(m_appletItem->description());
        qDebug() << "minimum size hint: " << m_descriptionLabel->nativeWidget()->minimumSizeHint();
        // m_descriptionLabel->nativeWidget()->setMinimumSize(m_descriptionLabel->nativeWidget()->minimumSizeHint());
        // m_descriptionLabel->nativeWidget()->setMaximumSize(m_descriptionLabel->nativeWidget()->minimumSizeHint());
        // m_nameLabel->nativeWidget()->setMinimumSize(0,0);
        m_nameLabel->setText(m_appletItem->name());
        // m_nameLabel->nativeWidget()->setMinimumSize(m_nameLabel->nativeWidget()->minimumSizeHint());
        // m_nameLabel->nativeWidget()->setMaximumSize(m_nameLabel->nativeWidget()->minimumSizeHint());
    } else {
        m_iconWidget->setIcon("clock");
        m_descriptionLabel->setText("applet description");
        m_nameLabel->setText("nameless applet");
    }

    m_linearLayout->invalidate();
    m_linearLayout->activate();
    // QSize nameSize = m_nameLabel->nativeWidget()->sizeHint();
    // QSize descSize = m_descriptionLabel->nativeWidget()->sizeHint();
    // qDebug() << "size hint " << nameSize << descSize;

    QSizeF prefSize = m_linearLayout->sizeHint(Qt::PreferredSize) + QSizeF(20, 20);
    resize(prefSize);
}

void AppletInfoWidget::onInfoButtonClick()
{
    emit(infoButtonClicked(m_appletItem->pluginName()));
}
