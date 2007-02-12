//-----------------------------------------------------------------------------
//
// KDE Display color scheme setup module
//
// Copyright (c)  Mark Donohoe 1997
//

#ifndef __COLORSCM_H__
#define __COLORSCM_H__

#include <QColor>
#include <QObject>
#include <QString>
#include <QStringList>

#include <kcmodule.h>
#include <kdialog.h>

#include "widgetcanvas.h"

class QSlider;
class QComboBox;
class QPushButton;
class QCheckBox;
class QResizeEvent;
class KLineEdit;
class QPalette;
class KListWidget;
class KColorButton;
class KConfig;
class KStdDirs;
class KColorSchemeList;
class KColorTreeWidget;

/**
 * The Desktop/Colors tab in kcontrol.
 */
class KColorScheme: public KCModule
{
    Q_OBJECT

public:
    KColorScheme(QWidget *parent, const QStringList &);
    ~KColorScheme();

    virtual void load();
    virtual void save();
    virtual void defaults();

private Q_SLOTS:
    void sliderValueChanged(int val);
    void slotSave();
    void slotAdd();
    void slotRemove();
    void slotImport();
    void slotColorForWidget(int, const QColor &);
    void slotPreviewScheme(int);
    void slotShadeSortColumnChanged(bool);
    void slotColorChanged(int, const QColor &);

private:
    void setColorName( const QString &name, int id , int id2 );
    void readScheme(int index=0);
    void readSchemeNames();
	void insertEntry(const QString &sFile, const QString &sName);
    int findSchemeByName(const QString &scheme);
    QPalette createPalette();
    
    QColor &color(int index);

    int nSysSchemes;
    bool useRM;

    QColor colorPushColor;
    QSlider *sb;
    KColorTreeWidget *mColorTreeWidget;
    QPushButton *addBt, *removeBt, *importBt;
    KListWidget *sList;
    KColorSchemeList *mSchemeList;
    QString sCurrentScheme;

    WidgetCanvas *cs;
    
    QCheckBox *cbExportColors;
    QCheckBox *cbShadeList;
};

#endif
