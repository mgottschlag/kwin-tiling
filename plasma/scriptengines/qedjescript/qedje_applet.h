#ifndef QEDJEAPPLETSCRIPT_HEADER
#define QEDJEAPPLETSCRIPT_HEADER

#include <plasma/scripting/appletscript.h>

// Include qedje stuff
#include <qzion.h>
#include <qedje.h>

#include "ui_qedjeConfig.h"

// Define our plasma AppletScript
class QEdjeAppletScript: public Plasma::AppletScript
{
    Q_OBJECT
public:
    // Basic Create/Destroy
    QEdjeAppletScript(QObject *parent, const QVariantList &args);
    ~QEdjeAppletScript();

    virtual bool init();
    virtual void resizeAll(QSize size);

    // The paintInterface procedure paints the applet to screen
    virtual void paintInterface(QPainter *painter,
                                const QStyleOptionGraphicsItem *option,
                                const QRect& contentsRect);

public Q_SLOTS:
    void showConfigurationInterface();
    void configChanged();
    void groupSelected(int index);

private:
    QEdje *world;
    QZionCanvas *canvas;
    QGraphicsProxyWidget *proxy;

    // everything needed by the fancy setup UI
    Ui::qedjeConfig ui;
    KDialog *dialog;
    QWidget *config_widget;
    QEdje *previewWorld;
    QZionCanvas *previewCanvas;

    QString m_edje_file;
    QString m_edje_group;
    QStringList m_groups_list;
    int currentIndex;

    void setup_canvas();
};

#endif
