//
// KDE Shortcut config module
//
// Copyright (c)  Mark Donohoe 1998
// Copyright (c)  Matthias Ettrich 1998
// Converted to generic key configuration module, Duncan Haldane 1998.

#ifndef __KEYCONFIG_H__
#define __KEYCONFIG_H__

#include <qpushbutton.h>
#include <qlistbox.h>

#include <kaccel.h>
#include <kkeydialog.h>
#include <kcmodule.h>
#include <qdict.h>
#include "savescm.h"

class KKeyModule : public KCModule
{
	Q_OBJECT
public:
	KAccel *keys;
        KKeyEntryMap dict;
        KKeyChooser *kc;

	KKeyModule( QWidget *parent, bool isGlobal, const char *name = 0 );
	~KKeyModule ();

        void load();
        void save();
        void defaults();
        void init();

public slots:
	void slotPreviewScheme( int );
	void slotAdd();
	void slotSave();
	void slotRemove();
	void slotChanged();


protected:
	QListBox *sList;
	QStringList *sFileList;
	QDict<int> *globalDict;
	QPushButton *saveBt;
	QPushButton *addBt;
	QPushButton *removeBt;
	int nSysSchemes;

	void readSchemeNames();
	void readScheme( int index=0 );

	bool changed;

        QString KeyType ;
        QString KeyScheme ;
        QString KeySet ;

};
#endif










