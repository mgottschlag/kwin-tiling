//
// KDE Shortcut config module
//
// Copyright (c)  Mark Donohoe 1998
// Copyright (c)  Matthias Ettrich 1998
// Converted to generic key configuration module, Duncan Haldane 1998.

#ifndef __KEYCONFIG_H__
#define __KEYCONFIG_H__

#include <QPushButton>
#include <qlistbox.h>

#include <kkeydialog.h>
//#include <kcmodule.h>
#include <qdict.h>

class QCheckBox;

class KeyChooserSpec;

class KKeyModule : public QWidget
{
	Q_OBJECT
public:
	KAccelActions actions;
        //KAccelActions dict;
        KeyChooserSpec *kc;

	KKeyModule( QWidget *parent, bool isGlobal, bool bSeriesOnly, bool bSeriesNone, const char *name = 0 );
	KKeyModule( QWidget *parent, bool isGlobal, const char *name = 0 );
	~KKeyModule ();

protected:
	void init( bool isGlobal, bool bSeriesOnly, bool bSeriesNone );

public:
        virtual void load();
        //virtual void save();
        virtual void defaults();
        static void init();

	bool writeSettings( const QString& sGroup, KConfig* pConfig );
	bool writeSettingsGlobal( const QString& sGroup );

public Q_SLOTS:
	//void slotPreviewScheme( int );
	//void slotAdd();
	//void slotSave();
	//void slotRemove();
	void slotKeyChange();
	void slotPreferMeta();
        //void updateKeys( const KAccelActions* map_P );
	//void readSchemeNames();

Q_SIGNALS:
	void keyChange();
        //void keysChanged( const KAccelActions* map_P );

protected:
	QListBox *sList;
	QStringList *sFileList;
	QPushButton *addBt;
	QPushButton *removeBt;
	QCheckBox *preferMetaBt;
	int nSysSchemes;
	bool bSeriesOnly;

	void readScheme( int index=0 );

	QString KeyType;
	QString KeyScheme;
	QString KeySet;

};

class KeyChooserSpec : public KKeyChooser
{
        Q_OBJECT
public:
        KeyChooserSpec( KAccelActions& actions, QWidget* parent,
                 bool bGlobal );
        //void updateKeys( const KAccelActions* map_P );
protected:
        bool m_bGlobal;
};

#endif

