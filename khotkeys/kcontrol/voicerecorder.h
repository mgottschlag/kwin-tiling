/****************************************************************************

 KHotKeys
 
 Copyright (C) 2005 Olivier Goffgart <ogoffart @ kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#ifndef VOICE_RECORDER_H
#define VOICE_RECORDER_H

#include <qframe.h>
#include "voice_input_widget_ui.h"


#include "sound.h"
class QMouseEvent;
class  KAudioRecordStream ;
class KArtsServer;
class KTemporaryFile;
class KLibrary;

namespace KHotKeys
{

	class SoundRecorder;
	class Voice_trigger;

class VoiceRecorder : public Voice_input_widget_ui
    {
    Q_OBJECT

    public:
        VoiceRecorder(const Sound& sound_P, const QString &voiceId, QWidget *parent, const char *name);
        ~VoiceRecorder();

        Sound sound() const;
		
        enum State { sNotModified , sIncorrect, sModified  };

        inline State state() const
            {
            return _state;
            }
            
        static bool init( KLibrary* lib );
        typedef void (*arts_play_fun)( const QString& file );

    protected slots:
        void slotStopPressed();
        void slotRecordPressed();
        void slotPlayPressed();

    signals:
        void recorded(bool);
	private slots:
        void slotSoundRecorded(const Sound& sound);
        bool drawSound();

    private:
        SoundRecorder *_recorder;
        Sound _sound;
        State _state;
        KTemporaryFile *_tempFile;
        QString _voiceId;
        static arts_play_fun arts_play;
    };

} // namespace KHotKeys

#endif
