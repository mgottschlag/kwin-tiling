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
    m_mainLayout = new QGraphicsLinearLayout();
    m_mainLayout->setOrientation(Qt::Vertical);

    m_iconWidget   = new Plasma::IconWidget();
    m_nameLabel    = new Plasma::Label();
    m_tabs         = new Plasma::TabBar();

    m_aboutLabel   = new Plasma::Label();
    m_actionsLabel = new Plasma::Label();
    m_authorLabel  = new Plasma::Label();

    // main layout init
    QGraphicsLinearLayout * headerLayout = new QGraphicsLinearLayout();
    headerLayout->setOrientation(Qt::Horizontal);
    headerLayout->addItem(m_iconWidget);
    headerLayout->addItem(m_nameLabel);
    m_mainLayout->addItem(headerLayout);

    m_mainLayout->addItem(m_tabs);

    m_mainLayout->setContentsMargins(0, 0, 0, 0);

    m_tabs->setPreferredSize(250, 150);

    // header init
    m_iconWidget->setAcceptHoverEvents(false);
    m_iconWidget->setAcceptedMouseButtons(false);
    m_iconWidget->setMinimumSize(IconSize(KIconLoader::Desktop), IconSize(KIconLoader::Desktop));
    m_iconWidget->setMaximumSize(IconSize(KIconLoader::Desktop), IconSize(KIconLoader::Desktop));

    QFont nameFont = m_nameLabel->nativeWidget()->font();
    nameFont.setBold(true);
    nameFont.setPointSize(1.2 * nameFont.pointSize());
    m_nameLabel->nativeWidget()->setFont(nameFont);
    m_nameLabel->nativeWidget()->setScaledContents(true);
    m_nameLabel->setMaximumHeight(m_iconWidget->maximumHeight());

    // about tab
    m_tabs->addTab(i18n("About"), m_aboutLabel);

    // actions tab
    m_tabs->addTab(i18n("Actions"), m_actionsLabel);

    // author tab
    m_tabs->addTab(i18n("Author"), m_authorLabel);

    // m_infoButton = new Plasma::IconWidget();
    // m_infoButton->setIcon("help-about");
    // m_infoButton->setMinimumSize(IconSize(KIconLoader::MainToolbar), IconSize(KIconLoader::MainToolbar));
    // m_infoButton->setMaximumSize(IconSize(KIconLoader::MainToolbar), IconSize(KIconLoader::MainToolbar));
    // connect(m_infoButton, SIGNAL(clicked()), this, SLOT(onInfoButtonClick()));

    // m_linearLayout = new QGraphicsLinearLayout();
    // m_linearLayout->setOrientation(Qt::Horizontal);
    // QGraphicsLinearLayout *vLayout = new QGraphicsLinearLayout();
    // vLayout->setOrientation(Qt::Vertical);

    // m_linearLayout->addItem(m_iconWidget);
    // vLayout->addItem(m_nameLabel);
    // vLayout->addItem(m_descriptionLabel);
    // m_linearLayout->addItem(vLayout);
    // m_linearLayout->addItem(m_infoButton);

    setLayout(m_mainLayout);

    //m_mainLayout->setAlignment(m_infoButton, Qt::AlignRight);
}

void AppletInfoWidget::setAppletItem(PlasmaAppletItem *appletItem)
{
    m_appletItem = appletItem;
}

void AppletInfoWidget::updateInfo()
{
    if(m_appletItem != 0) {
        m_iconWidget->setIcon(m_appletItem->icon());
        m_nameLabel->setText(m_appletItem->name());

        m_aboutLabel->setText(
            i18n("Version: %1").arg(m_appletItem->version()) + "\n\n" +
            m_appletItem->description());

        m_authorLabel->setText(
            i18n("<html><p>Author: %1</p><p>License: %2</p><p>Please report bugs to: <a href=\"mailto:%3\">%3</a></p></html>")
                .arg(m_appletItem->author())
                .arg(m_appletItem->license())
                .arg(m_appletItem->email()));

    } else {
        m_iconWidget->setIcon("plasma");
        m_nameLabel->setText("Unknown applet");
    }

    // m_linearLayout->invalidate();
    m_mainLayout->activate();
    // QSize nameSize = m_nameLabel->nativeWidget()->sizeHint();
    // QSize descSize = m_descriptionLabel->nativeWidget()->sizeHint();
    // qDebug() << "size hint " << nameSize << descSize;

    QSizeF prefSize = m_mainLayout->sizeHint(Qt::PreferredSize) + QSizeF(32, 32);
    resize(prefSize);
}

void AppletInfoWidget::onInfoButtonClick()
{
    emit(infoButtonClicked(m_appletItem->pluginName()));
}
