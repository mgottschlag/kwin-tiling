#ifndef _COMPONENTCHOOSER_H_
#define _COMPONENTCHOOSER_H_

#include "componentchooser_ui.h"
#include <qdict.h>
#include <qstring.h>

class QListBoxItem;
class KEMailSettings;

class ComponentChooser : public ComponentChooser_UI
{

Q_OBJECT

public:
	ComponentChooser(QWidget *parent=0, const char *name=0);
	virtual ~ComponentChooser();
	void load();
	void save();
	void restoreDefault();

private:
	QDict<QString>  m_lookupDict,m_revLookupDict;
	QString latestEditedService;
	bool somethingChanged;
	void emitChanged(bool);
	KEMailSettings *pSettings;

protected slots:
	void configChanged();
	void slotServiceSelected(QListBoxItem *);
	void slotComponentChanged(const QString&);
	void selectEmailClient();

signals:
	void changed(bool);

};


#endif
