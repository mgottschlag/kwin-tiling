#ifndef __PIXMAP_H__
#define __PIXMAP_H__


#include <qpixmap.h>
#include <qhash.h>
#include <qstring.h>


class LayoutIcon {

private:
	static LayoutIcon* instance;
	static const QString flagTemplate;
	
	QHash<QString, QPixmap*> m_pixmapCache;
	QFont m_labelFont;

	LayoutIcon();
	QPixmap* createErrorPixmap();
	void dimPixmap(QPixmap& pixmap);
	QString getCountryFromLayoutName(const QString& layoutName);
	
  public:
	static const QString& ERROR_CODE;
	
	static LayoutIcon& getInstance();
	const QPixmap& findPixmap(const QString& code, bool showFlag, const QString& displayName="");
};

#endif
