/****************************************************************************

 KHotKeys
 
 Copyright (C) 2005        Olivier Goffart <ogoffart @ kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#ifndef VOICE_SETTINGS_TAB_H_
#define VOICE_SETTINGS_TAB_H_

#include <voice_settings_tab_ui.h>

class KShortcut;

namespace KHotKeys
{

class Voice_settings_tab
    : public Voice_settings_tab_ui
    {
    Q_OBJECT
    public:
        Voice_settings_tab( QWidget* parent = NULL, const char* name = NULL );
        void read_data();
        void write_data() const;
    public slots:
        void clear_data();
    private slots:
		void slotCapturedKey( const KShortcut& );
    };

//***************************************************************************
// Inline
//***************************************************************************

} // namespace KHotKeys

#endif
