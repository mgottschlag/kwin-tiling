#ifndef WIDGETEXPLORER_H
#define WIDGETEXPLORER_H

#include <QtGui>
#include <KDE/KDialog>
#include <plasma/framesvg.h>
#include "plasmaappletitemmodel_p.h"

namespace Plasma
{

class Corona;
class Containment;
class Applet;
class WidgetExplorerPrivate;
class WidgetExplorerMainWidgetPrivate;

class WidgetExplorerMainWidget : public QGraphicsWidget
{

    Q_OBJECT

public:
    WidgetExplorerMainWidget(QGraphicsItem *parent = 0);
    ~WidgetExplorerMainWidget();

    void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);

    QString application();
    void setApplication(const QString &application = QString());
    /**
     * Changes the current default containment to add applets to
     *
     * @arg containment the new default
     */
    void setContainment(Plasma::Containment *containment);

    /**
     * @return the current default containment to add applets to
     */
    Containment *containment() const;

public Q_SLOTS:
    /**
     * Adds currently selected applets
     */
    void addApplet();

    /**
     * Adds applet
     */
    void addApplet(PlasmaAppletItem *appletItem);

    /**
     * Destroy all applets with this name
     */
    void destroyApplets(const QString &name);

    /**
     * Launches a download dialog to retrieve new applets from the Internet
     *
     * @arg type the type of widget to download; an empty string means the default
     *           Plasma widgets will be accessed, any other value should map to a
     *           PackageStructure PluginInfo-Name entry that provides a widget browser.
     */
    void downloadWidgets(const QString &type = QString());

    /**
     * Opens a file dialog to open a widget from a local file
     */
    void openWidgetFile();

    /**
     * Shows infos about applets.
     */
    void infoAboutApplet(const QString &name);

    void populateWidgetsMenu();

private:
    Q_PRIVATE_SLOT(d, void appletAdded(Plasma::Applet*))
    Q_PRIVATE_SLOT(d, void appletRemoved(Plasma::Applet*))
    Q_PRIVATE_SLOT(d, void containmentDestroyed())

    WidgetExplorerMainWidgetPrivate * const d;
    Plasma::FrameSvg *m_backgroundSvg;

};

class WidgetExplorer: public QGraphicsScene
{
    Q_OBJECT

public:

    explicit WidgetExplorer(QWidget *parent = 0, Qt::WindowFlags f = 0);
    virtual ~WidgetExplorer();

    void init();

    void setApplication(const QString &application = QString());
    QString application();

    /**
     * Changes the current default containment to add applets to
     *
     * @arg containment the new default
     */
    void setContainment(Plasma::Containment *containment);

    /**
     * @return the current default containment to add applets to
     */
    Containment *containment() const;

private:
    WidgetExplorerMainWidget *m_widget;
};

} // namespace Plasma

#endif // WIDGETEXPLORER_H
