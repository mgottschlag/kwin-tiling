#ifndef __EXTENSION_H__
#define __EXTENSION_H__

#include <X11/Xlib.h>
#define explicit int_explicit        // avoid compiler name clash in XKBlib.h
#include <X11/XKBlib.h> 
#undef explicit

class XKBExtension
{
public:

    XKBExtension(Display *display=0);
    ~XKBExtension();
    bool init();

    static bool setXkbOptions(const QString& options, bool resetOldOptions);
    bool setLayout(const QString& model, const QString& layout, const char* variant, unsigned int group,
				const char* baseGr);
    bool setGroup(unsigned int group);
    unsigned int getGroup();

    bool getCompiledLayout(const QString &fileName);
    bool setCompiledLayout(const QString &fileName);

private:

    Display *m_dpy;
};

#endif
