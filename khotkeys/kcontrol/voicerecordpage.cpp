/****************************************************************************

 KHotKeys
 
 Copyright (C) 2005 Olivier Goffart  < ogoffart @ kde.org >

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#include <qwidget.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <klineedit.h>
#include <kstandarddirs.h>

#include <klocale.h>
#include <kmessagebox.h>

#include "voicerecordpage.h"
#include "voicerecorder.h"
#include "voicesignature.h"
#include "voices.h"

namespace KHotKeys
{

VoiceRecordPage::VoiceRecordPage( const QString &voiceid_P, QWidget *parent)
	: KVBox(parent) , _original_voiceId(voiceid_P)
   {
	  _message = i18n("Enter a code for the sound (e.g. the word you are saying) and record the same word twice.");

    _label = new QLabel(_message, this);
    _label->setObjectName("label");
/*    _label->setAlignment(QLabel::AlignLeft | QLabel::WordBreak |
    QLabel::AlignVCenter);*/
	
	_lineEdit = new KLineEdit( this );
	_lineEdit->setText(voiceid_P);

	
	Sound s;
	if(voiceid_P!=QString::null)
	{
        QString fileName = KStandardDirs::locateLocal( "data", "khotkeys/" + voiceid_P +  "1.wav"  );
		s.load(fileName);
	}
	_recorder1 = new VoiceRecorder(s, voiceid_P, this, "recorder");
	if(voiceid_P!=QString::null)
	{
        QString fileName = KStandardDirs::locateLocal( "data", "khotkeys/" + voiceid_P +  "2.wav"  );
		s.load(fileName);
	}
	_recorder2 = new VoiceRecorder(s, voiceid_P, this, "recorder");

    //_recorder->setMinimumHeight(150);
    //setStretchFactor(_recorder, 1);

    QWidget *spacer = new QWidget(this);
    setStretchFactor(spacer, 1);


	connect(_recorder1, SIGNAL(recorded(bool)) , this, SLOT(slotChanged()));
	connect(_recorder2, SIGNAL(recorded(bool)) , this, SLOT(slotChanged()));
	connect(_lineEdit , SIGNAL( textChanged (const QString&)) , this , SLOT(slotChanged()));


    }

VoiceRecordPage::~VoiceRecordPage()
    {
    }

void VoiceRecordPage::slotChanged()
    {
		bool voiceCodeOK=!_lineEdit->text().isEmpty();
		if( voiceCodeOK && _lineEdit->text() != _original_voiceId)
		{
			voiceCodeOK=!voice_handler->doesVoiceCodeExists(_lineEdit->text());
			if(!voiceCodeOK)
			{
				_label->setText(i18n("<qt>%1<br><font color='red'>The sound code already exists</font></qt>").arg(_message));
			}
		}
		if( voiceCodeOK )
		{
			voiceCodeOK=_recorder1->state()!=VoiceRecorder::sIncorrect && _recorder2->state()!=VoiceRecorder::sIncorrect;
			if(!voiceCodeOK)
			{
				_label->setText(i18n("<qt>%1<br><font color='red'>One of the sound references is not correct</font></qt>").arg(_message));
			}
		}
		if( voiceCodeOK )
			_label->setText(_message);
		
		emit voiceRecorded( voiceCodeOK &&
				( (  (_recorder1->state()==VoiceRecorder::sModified || _recorder2->state()==VoiceRecorder::sModified || _lineEdit->text() != _original_voiceId) 
					&& !_original_voiceId.isEmpty()) 
				||  (_recorder1->state()==VoiceRecorder::sModified && _recorder2->state()==VoiceRecorder::sModified )   )  );
    }

QString VoiceRecordPage::getVoiceId() const
   {
	   return _lineEdit->text();
   }

VoiceSignature VoiceRecordPage::getVoiceSignature(int ech) const
   {
	   VoiceRecorder *recorder= (ech==1) ? _recorder1 : _recorder2 ;
       QString fileName = KStandardDirs::locateLocal( "data", "khotkeys/" + getVoiceId() + QString::number(ech) +  ".wav"  );
	   Sound s=recorder->sound();
	   s.save(fileName);
	   return VoiceSignature(s);
   }
   
bool VoiceRecordPage::isModifiedSignature(int ech) const
   {
	   VoiceRecorder *recorder= (ech==1) ? _recorder1 : _recorder2 ;
	   return recorder->state()==VoiceRecorder::sModified;
   }


} // namespace KHotKeys

#include "voicerecordpage.moc"
