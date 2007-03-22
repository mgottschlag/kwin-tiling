#ifndef __EXTENSION_H__
#define __EXTENSION_H__

#include <stdio.h>
#include <X11/Xlib.h>


class XKBExtension
{
public:
	XKBExtension(Display *display=NULL);
	~XKBExtension();
	bool init();
	void reset();

	static bool setXkbOptions(const QString& options, bool resetOldOptions);
	bool setLayout(const QString& model,
					const QString& layout, const QString& variant,
					const QString& includeGroup, bool useCompiledLayouts=true);
	bool setLayout(const QString& layouts, const QString& variants);
	bool setGroup(unsigned int group);
	unsigned int getGroup() const;

private:
    Display *m_dpy;
/*	QString m_tempDir;
	static QMap<QString, FILE*> fileCache;*/
	
	bool setLayoutInternal(const QString& model,
				   const QString& layout, const QString& variant,
				   const QString& includeGroup);
// 	bool compileCurrentLayout(const QString& layoutKey);
// 	bool setCompiledLayout(const QString& layoutKey);
	
// 	QString getPrecompiledLayoutFilename(const QString& layoutKey);
//	void deletePrecompiledLayouts();
};

#endif
