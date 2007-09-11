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


#include <kcmodule.h>
#include <kdialog.h>

#include "widgetcanvas.h"

class QSlider;
class QPushButton;
class QCheckBox;
class QPalette;
class KListWidget;
class KColorTreeWidget;

class KColorSchemeEntry
{
public:
    KColorSchemeEntry(const QString &_path, const QString &_name, bool _local)
        : path(_path), name(_name), local(_local) {}

    const QString path;
    const QString name;
    const bool local;
};

class KColorSchemeList : public Q3PtrList<KColorSchemeEntry>
{
public:
    KColorSchemeList()
    {
        setAutoDelete(true);
    }

    int compareItems(Q3PtrCollection::Item item1, Q3PtrCollection::Item item2)
    {
        KColorSchemeEntry *i1 = (KColorSchemeEntry*)item1;
        KColorSchemeEntry *i2 = (KColorSchemeEntry*)item2;
        if (i1->local != i2->local)
            return i1->local ? -1 : 1;
        return i1->name.localeAwareCompare(i2->name);
    }
};

/**
 * The Desktop/Colors tab in kcontrol.
 */
class KCMColorScheme: public KCModule
{
    Q_OBJECT

public:
    KCMColorScheme(QWidget *parent, const QVariantList &);
    ~KCMColorScheme();

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
    KColorSchemeList mSchemeList;
    QString sCurrentScheme;

    WidgetCanvas *cs;
    
    QCheckBox *cbExportColors;
    QCheckBox *cbShadeList;
};

#endif
