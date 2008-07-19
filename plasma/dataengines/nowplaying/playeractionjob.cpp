/*
 * Copyright 2008  Alex Merry <alex.merry@kdemail.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#include "playeractionjob.h"

#include <kdebug.h>

void PlayerActionJob::doAction()
{
    kDebug() << "Trying to perform the action" << operationName();
    if (!m_player) {
        setErrorText(i18n("The player '%1' cannot be found", destination()));
        setError(-1);
        emitResult();
        return;
    }

    const QString operation(operationName());
    if (operation == "play") {
        if (m_player->canPlay()) {
            m_player->play();
        } else {
            setErrorText(i18n("The player '%1' cannot perform the action 'play'", m_player->name()));
            setError(-1);
        }
    } else if (operation == "pause") {
        if (m_player->canPause()) {
            m_player->pause();
        } else {
            setErrorText(i18n("The player '%1' cannot perform the action 'pause'", m_player->name()));
            setError(-1);
        }
    } else if (operation == "stop") {
        if (m_player->canStop()) {
            m_player->stop();
        } else {
            setErrorText(i18n("The player '%1' cannot perform the action 'stop'", m_player->name()));
            setError(-1);
        }
    } else if (operation == "previous") {
        if (m_player->canGoPrevious()) {
            m_player->previous();
        } else {
            setErrorText(i18n("The player '%1' cannot perform the action 'previous'", m_player->name()));
            setError(-1);
        }
    } else if (operation == "next") {
        if (m_player->canGoNext()) {
            m_player->next();
        } else {
            setErrorText(i18n("The player '%1' cannot perform the action 'next'", m_player->name()));
            setError(-1);
        }
    } else if (operation == "volume") {
        if (m_player->canSetVolume()) {
            if (parameters().contains("level")) {
                qreal volume = parameters()["level"].toDouble();
                if (volume >= 0.0 && volume <= 1.0) {
                    m_player->setVolume(volume);
                } else {
                    setErrorText(i18n("The 'level' argument to the 'volume' command must be between 0 and 1"));
                    setError(-2);
                }
            } else {
                setErrorText(i18n("The 'volume' command requires a 'level' argument"));
                setError(-2);
            }
        } else {
            setErrorText(i18n("The player '%1' cannot perform the action 'volume'", m_player->name()));
            setError(-1);
        }
    } else if (operation == "seek") {
        if (m_player->canSeek()) {
            if (parameters().contains("seconds")) {
                qreal time = parameters()["seconds"].toInt();
                if (time >= 0 && time <= m_player->length()) {
                    m_player->seek(time);
                } else {
                    setErrorText(i18n("The 'seconds' argument to the 'seek' command must be "
                                      "between 0 and the length of the track"));
                    setError(-2);
                }
            } else {
                setErrorText(i18n("The 'seek' command requires a 'seconds' argument"));
                setError(-2);
            }
        } else {
            setErrorText(i18n("The player '%1' cannot perform the action 'seek'", m_player->name()));
            setError(-1);
        }
    }
    if (error()) {
        kDebug() << "Failed with error" << errorText();
    }
    emitResult();
}

#include "playeractionjob.moc"

// vim: sw=4 sts=4 et tw=100
