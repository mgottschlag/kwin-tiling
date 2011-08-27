/****************************************************************************

 KHotKeys

 Copyright (C) 2005 Olivier Goffart  <ogoffart @ kde.org>

 Distributed under the terms of the GNU General Public License version 2.

****************************************************************************/


#include <kaction.h>

#include "voices.h"
#include "voicesignature.h"
#include "triggers.h"
#include "soundrecorder.h"

#include <stdlib.h>
#include <math.h>
#include <assert.h>


#include <kapplication.h>
#include <kdebug.h>
#include <kxerrorhandler.h>
#include <QtCore/QTimer>


#include <X11/Xlib.h>
#include <fixx11h.h>



namespace KHotKeys
{

Voice* voice_handler;

Voice::Voice( bool enabled_P, QObject* parent_P )
	: QObject( parent_P) ,_enabled( enabled_P ), _recording( false ), _recorder(0)
    {
    assert( voice_handler == NULL );
    voice_handler = this;

	_kga=0L;
	_timer=0L;

	kDebug() ;

    }

Voice::~Voice()
    {
    kDebug() ;
    enable( false );
    voice_handler = NULL;
    }


void Voice::enable( bool enabled_P )
    {
#ifndef HAVE_ARTS
    enabled_P = false; // never enabled when there's no support
#endif
    if( _enabled == enabled_P )
        return;
    _enabled = enabled_P;
    _kga->setEnabled(enabled_P);
    }

void Voice::register_handler( Voice_trigger *trigger_P )
    {
    if( !_references.contains( trigger_P ))
        _references.append(trigger_P);
    }

void Voice::unregister_handler( Voice_trigger *trigger_P )
    {
		_references.remove(trigger_P);
    }


void Voice::record_start()
{
	kDebug() ;
	if(!_recorder)
	{
		_recorder= SoundRecorder::create(this);
		connect(_recorder, SIGNAL(recorded(Sound)), this, SLOT(slot_sound_recorded(Sound)));
	}

	_recorder->start();
	_recording=true;
}

void Voice::record_stop()
{
	if(!_recording)
		return;

	kDebug() ;
	delete _timer;
	_timer=0L;
	_recording=false;
	if(_recorder)
		_recorder->stop();
}


void Voice::slot_sound_recorded(const Sound &sound_P)
{
	VoiceSignature signature(sound_P);

	Voice_trigger *trig=0L;
	Voice_trigger *sec_trig=0L;
	double minimum=800000;
	double second_minimum=80000;
	int got_count=0;
    foreach(Voice_trigger *t, _references)
	{
		for(int ech=1; ech<=2 ; ech++)
		{
			double diff=VoiceSignature::diff(signature, t->voicesignature(ech));
			if(minimum>=diff)
			{
				second_minimum=minimum;
				minimum=diff;
				sec_trig=trig;
				trig=t;
			}
			else if(second_minimum>=diff)
			{
				second_minimum=diff;
				sec_trig=t;
			}
			if( diff < REJECT_FACTOR_DIFF )
				got_count++;
			kDebug() << ( (diff < REJECT_FACTOR_DIFF) ? "+++" : "---" )  <<t->voicecode() << ech << " : " << diff;
		}
	}
//	double ecart_relatif=(second_minimum-minimum)/minimum;

//	kDebug() <<  ecart_relatif;

	if(trig)
		kDebug() << "**** " << trig->voicecode() << " : " << minimum;


//	if(trig && ecart_relatif > REJECT_FACTOR_ECART_REL)
//	if(trig && got_count==1)
	bool selected=trig &&  (got_count==1 || ( minimum < 1.5*REJECT_FACTOR_DIFF   &&  trig==sec_trig  ) );

	if(selected)
	{
		trig->handle_Voice();
	}

}


/*bool Voice::x11Event( XEvent* pEvent )
{
	if( pEvent->type != XKeyPress && pEvent->type != XKeyRelease )
		return false;

	KKeyNative keyNative( pEvent );

	//kDebug() << keyNative.key().toString();

	if(_shortcut.contains(keyNative))
	{
		if(pEvent->type == XKeyPress  && !_recording )
		{
			record_start();
			return true;
		}
		if(pEvent->type == XKeyRelease  && _recording )
		{
			record_stop();
			return true;
		}
	}
	return false;
}


*/


void Voice::set_shortcut( const KShortcut &shortcut)
{
    _shortcut = shortcut;
    if( !_enabled )
        return;
    if(!_kga)
    {
        _kga = new KAction(this);
        _kga->setObjectName("khotkeys_voice");
        connect(_kga,SIGNAL(triggered(bool)) , this , SLOT(slot_key_pressed()));
    }
    _kga->setGlobalShortcut(shortcut);
}

void Voice::slot_key_pressed()
{
	if( _recording )
		record_stop();
	else
	{
		record_start();
		if(!_timer)
		{
			_timer=new QTimer(this);
			connect(_timer, SIGNAL(timeout()) , this, SLOT(slot_timeout()));
		}
		_timer->setSingleShot(true);
		_timer->start(1000*20);
	}
}


void Voice::slot_timeout()
{
	if(_recording && _recorder)
	{
		_recorder->abort();
		_recording=false;
	}
	_timer->deleteLater();
	_timer=0L;
}


QString Voice::isNewSoundFarEnough(const VoiceSignature& signature, const QString &currentTrigger)
{
	Voice_trigger *trig=0L;
	Voice_trigger *sec_trig=0L;
	double minimum=800000;
	double second_minimum=80000;
	int got_count=0;
    foreach (Voice_trigger *t ,  _references)
	{
		if(t->voicecode()==currentTrigger)
			continue;

		for(int ech=1; ech<=2 ; ech++)
		{
			double diff=VoiceSignature::diff(signature, t->voicesignature(ech));
			if(minimum>=diff)
			{
				second_minimum=minimum;
				minimum=diff;
				sec_trig=trig;
				trig=t;
			}
			else if(second_minimum>=diff)
			{
				second_minimum=diff;
				sec_trig=t;
			}
			if( diff < REJECT_FACTOR_DIFF )
				got_count++;
			kDebug() << ( (diff < REJECT_FACTOR_DIFF) ? "+++" : "---" )  <<t->voicecode() << ech << " : " << diff;
		}
	}

	if(trig)
		kDebug() << "**** " << trig->voicecode() << " : " << minimum;

	bool selected=trig &&  ((got_count==1 && minimum < REJECT_FACTOR_DIFF*0.7 ) || ( minimum < REJECT_FACTOR_DIFF   &&  trig==sec_trig  ) );
	return selected ? trig->voicecode() : QString();
}

bool Voice::doesVoiceCodeExists(const QString &vc)
{
    foreach (Voice_trigger *t ,  _references)
    {
		if(t->voicecode()==vc)
			return true;
	}
	return false;
}

} // namespace KHotKeys

#include "voices.moc"
