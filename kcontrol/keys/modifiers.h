#ifndef __MODIFIERS_MODULE_H
#define __MODIFIERS_MODULE_H

#include <qwidget.h>

class KComboBox;

class ModifiersModule : public QWidget
{
	Q_OBJECT
 public:
	ModifiersModule( QWidget *parent = 0, const char *name = 0 );

 protected:
	KComboBox *m_pcbShift, *m_pcbCtrl,
	          *m_pcbAltX, *m_pcbAlt,
	          *m_pcbWinX, *m_pcbWin;

	void initGUI();
	KComboBox* newModXComboBox();
};

#endif
