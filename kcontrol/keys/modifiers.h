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

 signals:
	void changed( bool );

 protected:
	bool m_bMacKeyboardOrig, m_bMacSwapOrig;
	QString m_sLabelCtrlOrig, m_sLabelAltOrig, m_sLabelWinOrig;

	QLabel* m_plblCtrl, * m_plblAlt, * m_plblWin;
	KComboBox* m_pcbWinX;
	QCheckBox* m_pchkMacKeyboard;
	KListView* m_plstXMods;
	QCheckBox* m_pchkMacSwap;

	void initGUI();
	KComboBox* newModXComboBox( QWidget* parent );
	void updateWidgets();

 protected slots:
	void slotMacKeyboardClicked();
	void slotMacSwapClicked();
};

#endif
