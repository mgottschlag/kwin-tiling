#include <QtGui/QGraphicsProxyWidget>

#include <plasma/package.h>
#include <plasma/applet.h>

#include "qedje_applet.h"

using namespace Plasma;

K_EXPORT_PLASMA_APPLETSCRIPTENGINE(qedjescripts, QEdjeAppletScript)

QEdjeAppletScript::QEdjeAppletScript(QObject *parent, const QVariantList &args)
: Plasma::AppletScript(parent), dialog(0), config_widget(0),
    m_edje_file(""), m_edje_group(""), currentIndex(0)
{
    Q_UNUSED(args);
}

QEdjeAppletScript::~QEdjeAppletScript()
{
    // the proxy gets the ownership of the applet
    // so we need to free the applet in order to avoid segfaults later
    proxy->setWidget(0);

    if (dialog)
        delete dialog;

    delete world;
    delete canvas;
}

void QEdjeAppletScript::resizeAll(QSize size)
{
    // minimum required size
    if (size == QSize(0, 0))
        size = QSize(100, 100);

    // resize the applet and qzion's canvas
    QSizeF new_size = applet()->size() - applet()->contentsRect().size() + size;
    applet()->resize(new_size.toSize());
    canvas->resize(size);
}

void QEdjeAppletScript::setup_canvas()
{
    // we need a proxy so we can put qzion inside the applet
    proxy = new QGraphicsProxyWidget(applet());
    canvas = new QZionCanvas();

    // create the canvasd (qzion) needed by qedje
    proxy->setWidget(canvas->widget());
    canvas->show();

    // minimum size
    canvas->resize(100, 100);
}

bool QEdjeAppletScript::init()
{
    setup_canvas();

    // set plasma options
    applet()->setBackgroundHints(Applet::TranslucentBackground);
    setHasConfigurationInterface(true);

    // get config info
    KConfigGroup cg = applet()->config();
    m_edje_group = cg.readEntry("EdjeGroup", "");

    // setup edje file
    m_edje_file = package()->filePath("edje_file");

    // check groups
    m_groups_list = groupNamesFromFile(m_edje_file);

    if (m_groups_list.count() <= 0)
        return false;

    if (m_edje_group.isEmpty()) {
        m_edje_group = m_groups_list.first();
        currentIndex = 0;
    } else
        currentIndex = m_groups_list.indexOf(m_edje_group);

    // create qedje object
    world = new QEdje(canvas, m_edje_file, m_edje_group);

    // show qedje object and resize applet and plasmoid based on
    // the object's min size
    world->show();
    resizeAll(world->propMin());
    return true;
}

void QEdjeAppletScript::showConfigurationInterface()
{
    if (!dialog) {
        dialog = new KDialog();
        config_widget = new QWidget(dialog);
        previewCanvas = new QZionCanvas(config_widget);

        dialog->setCaption(i18n("QEdje Applet Config"));
        dialog->setButtons(KDialog::Ok | KDialog::Cancel);

        ui.setupUi(config_widget);
        ui.edje_groups->addItems(m_groups_list);
        ui.edje_groups->setCurrentIndex(ui.edje_groups->findText(m_edje_group));
        previewCanvas->widget()->setGeometry(ui.preview->frameGeometry());

        // connect the signals
        connect(dialog, SIGNAL(okClicked()), this, SLOT(configChanged()));
        connect(ui.edje_groups, SIGNAL(activated(int)), this, SLOT(groupSelected(int)));

        // show the config dialog
        dialog->setMainWidget(config_widget);

        previewWorld = new QEdje(previewCanvas, m_edje_file, m_edje_group);
    }

    dialog->show();
    previewWorld->show();
}

void QEdjeAppletScript::groupSelected(int index)
{
    if (index == currentIndex)
        return;

    m_edje_group = m_groups_list[index];
    currentIndex = index;

    previewWorld->hide();
    delete previewWorld;

    previewWorld = new QEdje(previewCanvas, m_edje_file, m_edje_group);
    previewWorld->show();
}

void QEdjeAppletScript::configChanged()
{
    KConfigGroup cg = applet()->config();
    cg.writeEntry("EdjeGroup", m_edje_group);

    world->hide();
    delete world;

    world = new QEdje(canvas, m_edje_file, m_edje_group);
    world->show();
    resizeAll(world->propMin());
}

void QEdjeAppletScript::paintInterface(QPainter *,
                                       const QStyleOptionGraphicsItem *,
                                       const QRect &contentsRect)
{
    // make sure qzion have the correct geometry
    canvas->widget()->setGeometry(contentsRect);
}

#include "qedje_applet.moc"
