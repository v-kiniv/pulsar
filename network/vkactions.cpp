/* This file is part of Pulsar.
   Copyright 2011, Vasily Kiniv <yuberion@gmail.com>

   Pulsar is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   Pulsar is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with Pulsar.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "vkactions.h"
#include <QDebug>
#include <QTextCodec>

VkActions *VkActions::m_instance = 0;

VkActions::VkActions(QObject *parent) :
    QObject(parent)
{
    if(m_instance)
            qFatal("Only one instance of VkActions object is allowed");
    m_instance = this;

    m_nManager = new QNetworkAccessManager();

    m_Auth = Auth::instance();
    m_nManager->setCookieJar(m_Auth->cookiejar());

    m_nRequest.setRawHeader("User-Agent", "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/31.0.1650.57 Safari/537.36");
    m_nRequest.setRawHeader("Host", "vk.com");
    m_nRequest.setRawHeader("Referer", "http://vk.com/audio");
    m_nRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
}

VkActions* VkActions::instance()
{
    return m_instance;
}

void VkActions::addToLibrary(Track *track)
{
    m_lastTrack = track;

    // Track has no all needed data for add to library : making update
    if(track->hash().isEmpty() || track->aid().isEmpty()) {
        track->updateUrl();
        connect(track, SIGNAL(updated(bool)), SLOT(trackUpdated(bool)));

        // Break action
        return;
    }

    m_nRequest.setUrl(QUrl("http://vk.com/audio.php?act=add&al=0&gid=0&aid=" + track->aid() + "&oid=" + track->oid() + "&album_id=0&hash="+track->hash()));

    m_nManager->disconnect();
    connect(m_nManager, SIGNAL(finished(QNetworkReply*)), SLOT(toLibReply(QNetworkReply*)));
    m_nManager->get(m_nRequest);
}

void VkActions::removeFromLibrary(Track *track)
{
    m_lastTrack = track;
    m_sLastTitle = track->title();

    m_nRequest.setUrl(QUrl("http://vk.com/audio.php?act=delete_audio&al=1&restore=0&gid=0&aid=" + track->aid() + "&oid=" + track->oid() + "&hash="+track->delHash()));

    m_nManager->disconnect();
    connect(m_nManager, SIGNAL(finished(QNetworkReply*)), SLOT(fromLibReply(QNetworkReply*)));
    m_nManager->get(m_nRequest);
}

void VkActions::setStatus(Track *track)
{
    m_lastTrack = track;

    if(!m_lastTrack->aid().isEmpty()) {
        QNetworkRequest request = QNetworkRequest(QUrl("http://vk.com/audio?act=add&oid=" + track->oid() + "&aid=" + track->aid() + "&hash=" + track->hash()));

        m_nManager->disconnect();
        m_nManager->get(request);
    }


}


void VkActions::toLibReply(QNetworkReply *reply)
{
    QString data;
    data = (QString)reply->readAll();

    if(data.contains("\"ok\":0"))
        Q_EMIT message(tr("Added to Library"), tr("Track '<b>%0</b>' added to library. Refresh your library list to see the new tracks.").arg(m_lastTrack->title()));
    else
        Q_EMIT message(tr("Unknown Error"), tr("Can't add Track to library."));
}

void VkActions::fromLibReply(QNetworkReply *reply)
{
    QString data;
    data = (QString)reply->readAll();

    if(data.contains("success"))
        Q_EMIT message(tr("Removed from Library"), tr("Track '<b>%0</b>' removed from library.").arg(m_sLastTitle));
    else
        Q_EMIT message(tr("Unknown Error"), tr("Can't remove Track from library."));
}

void VkActions::trackUpdated(bool s)
{
    Track *p = (Track*)sender();
    if(s) {
        addToLibrary(p);
    } else {
        Q_EMIT message(tr("Error"), tr("Can't add Track to library. Track can not be founded on server."));
    }
    disconnect(p, SIGNAL(updated(bool)), this, SLOT(trackUpdated(bool)));
}
