#ifndef RANDOM_H
#define RANDOM_H

class QWidget;
class QCheckBox;

class KRandomSetup : public KDialogBase
{
	Q_OBJECT
	public:
		KRandomSetup( QWidget *parent = NULL, const char *name = NULL );

	private:

		QWidget *preview;
		QCheckBox *openGL;
		QCheckBox *manipulateScreen;

		private slots:

		void slotOkClicked();

};

#endif
