#include <kcm_componentchooser.h>
#include <kcm_componentchooser.moc>
#include <qlayout.h>

KCMComponentChooser::KCMComponentChooser( QWidget *parent, const char *name ):
	KCModule(parent,name) {

	(new QVBoxLayout(this))->setAutoAdd(true);
	m_chooser=new ComponentChooser(this,"ComponentChooser");
	connect(m_chooser,SIGNAL(changed(bool)),this,SIGNAL(changed(bool)));
}
 
void KCMComponentChooser::load(){
	m_chooser->load();
}

void KCMComponentChooser::save(){
	m_chooser->save();
}

void KCMComponentChooser::defaults(){
	m_chooser->restoreDefault();
}

extern "C"
{
    KCModule *create_componentchooser( QWidget *parent, const char * )
    {
        return new KCMComponentChooser( parent, "kcmcomponentchooser" );
    }
}

