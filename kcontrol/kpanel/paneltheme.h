#ifndef __PANELTHEME_H
#define __PANELTHEME_H

/*
 * KPanel theme configuration.
 *
 * (C) Daniel M. Duley 1999
 */

#include <qcombobox.h>
#include <kcontrol.h>
#include <qstring.h>
#include <kiconloaderdialog.h>
#include <kcolorbtn.h>
#include <qlabel.h>
#include "widgetcanvas.h"

class KPanelTheme : public KConfigWidget
{
    Q_OBJECT
public:
    KPanelTheme(QWidget *parent=0, const char *name=0);
    ~KPanelTheme();
    virtual void loadSettings();
    virtual void applySettings();
    virtual void saveSettings();
protected slots:
    void slotWidgetClicked(int);
    void slotColor(const QColor &);
    void slotColorDrop(int, const QColor &);
    void slotPixmap(const QString &);
    void slotResetWidget();
    void slotResetAll();
protected:
    QString pixNames[4];
    KIconLoader ldr;
    QLabel *pixLbl;
    KIconLoaderButton *pixBtn;
    KColorButton *colorBtn;
    WidgetCanvas *canvas;
    QComboBox *wCombo;
};
    
#endif
