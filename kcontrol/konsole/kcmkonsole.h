
#ifndef KCMKONSOLE_H
#define KCMKONSOLE_H

#include <kcmodule.h>
#include <kaboutdata.h>
#include "kcmkonsoledialog.h"
class QFont;

class KCMKonsole
	: public KCModule
{
	Q_OBJECT

public:
	KCMKonsole (QWidget *parent = 0, const char *name = 0);
	
	void load();
	void load(const QString &);
	void save();
	void defaults();
	QString quickHelp() const;
	virtual const KAboutData * aboutData() const;
public slots:
	void setupFont();
	void configChanged();
private:
	KCMKonsoleDialog *dialog;
	QFont currentFont;
};

#endif
