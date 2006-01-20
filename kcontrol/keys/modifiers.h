#ifndef __MODIFIERS_MODULE_H
#define __MODIFIERS_MODULE_H

#include <qwidget.h>

class QCheckBox;
class QLabel;
class KComboBox;
class KListView;

class ModifiersModule : public QWidget
{
	Q_OBJECT
 public:
	ModifiersModule( QWidget *parent = 0, const char *name = 0 );

	void load();
	void save();
	void defaults();

	static void setupMacModifierKeys();

 Q_SIGNALS:
	void changed( bool );

 protected:
	bool m_bMacKeyboardOrig, m_bMacSwapOrig;
	QString m_sLabelCtrlOrig, m_sLabelAltOrig, m_sLabelWinOrig;

	QLabel* m_plblCtrl, * m_plblAlt, * m_plblWin;
	QLabel* m_plblWinModX;
	QCheckBox* m_pchkMacKeyboard;
	KListView* m_plstXMods;
	QCheckBox* m_pchkMacSwap;

	void readConfig();
	void initGUI();
	// Places the values in the *Orig variables into their
	//  respective widgets.
	void updateWidgetData();
	// Updates the labels according to the check-box settings
	//  and also reads in the X modifier map.
	void updateWidgets();

 protected Q_SLOTS:
	void slotMacKeyboardClicked();
	void slotMacSwapClicked();
};

#endif
