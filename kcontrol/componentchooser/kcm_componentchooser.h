#ifndef _KCM_KTEXTEDITORCHOOSER_H_
#define _KCM_KTEXTEDITORCHOOSER_H_

#include <kcmodule.h>
#include <componentchooser.h>
class KCMComponentChooser : public KCModule
{
    Q_OBJECT
public:
    KCMComponentChooser( QWidget *parent = 0, const char *name = 0 );
 
    void load();
    void save();
    void defaults();

private:
    ComponentChooser  *m_chooser;
};

#endif
