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

#include "player.h"
#include <QDebug>
#include <QTimer>

#include <QGst/Object>

Player *Player::m_instance = 0;

Player::Player(QObject *parent) :
    QObject(parent)
{
    if(m_instance)
            qFatal("Only one instance of Player object is allowed");
    m_instance = this;

    m_plistsgroup = PlistsGroup::instance();

    m_VkActions = VkActions::instance();

    m_Prev = 0;
    m_Current = 0;

    m_bArtRequested = false;

    m_bBroadcastStatus = Settings::instance()->getValue("player/status").toBool();

    m_State = Stoped;

    // QGstreamer init
    m_GstPlayer = new GstPlayer(this);
    connect(m_GstPlayer, SIGNAL(positionChanged()), SLOT(positionChanged()));
    connect(m_GstPlayer, SIGNAL(linkExpired()), SLOT(onLinkExpired()));
    connect(m_GstPlayer, SIGNAL(stateChanged()), SLOT(onStateChanged()));
    connect(m_GstPlayer, SIGNAL(finished()), SIGNAL(needTrack()));
    connect(m_GstPlayer, SIGNAL(gotBitrate(quint32)), SLOT(setBitrate(quint32)));
    connect(m_GstPlayer, SIGNAL(metaChanged()), SLOT(onMetaChanged()));

    QTimer::singleShot(10, this, SLOT(setInitVolume()));

    m_GstPlayer->setVolume(Settings::instance()->getValue("player/volume").toDouble());

}

Player *Player::instance()
{
    return m_instance;
}

Player::States Player::state()
{
    return m_State;
}

Track *Player::currentTrack()
{
    return m_Current ? m_Current : 0;
}

double Player::volume()
{
    return m_GstPlayer->volume();
}

bool Player::shuffle()
{
    return m_plistsgroup->shuffleMode() > 1 ? true : false;
}

bool Player::repeat()
{
    return m_plistsgroup->repeatMode() > 1 ? true : false;
}

void Player::setShuffle(bool state)
{
    m_plistsgroup->setShuffle(state ? 2 : 1);
}

void Player::setRepeat(bool state)
{
    m_plistsgroup->setRepeat(state ? 2 : 1);
}

int Player::totalTime()
{
    if(m_GstPlayer->state() == QGst::StateNull) {
        //qDebug() << "skipped total time";
        return 0;
    }

    qint64 value = m_GstPlayer->duration();
    if(value > 0)
        return value / 1000000;
    else {
        QTimer::singleShot(1000, this, SIGNAL(changed()));
        return 0;
    }
}

int Player::currentTime()
{
    if(m_GstPlayer->state() != QGst::StatePlaying) {
        return 0;
    }

    qint64 value = m_GstPlayer->pos();
    if(value > 0)
        return value / 1000000;
    else {
        return 0;
    }

}

void Player::setState(Player::States state)
{
    m_State = state;

    Q_EMIT stateChanged(m_State);

    if(m_State == Playing)
        Q_EMIT playintState(true);
    else
        Q_EMIT playintState(false);

    if(m_State == Playing)
        Q_EMIT playing();

    if(m_State == Paused)
        Q_EMIT paused();


    Q_EMIT changed();

}

void Player::playTrack(Track *p)
{
    Q_EMIT trackChanged();
    Q_EMIT trackChanged(p);

    // Set PlayState for track
    p->setPlayState();

    // Stop playing prev tack
    m_GstPlayer->stop();
    setState(Player::Stoped);

    Q_EMIT newTitles(p->artist(), p->title());

    m_GstPlayer->stop();
    m_GstPlayer->setUri(p->url());
    m_GstPlayer->play();

    setState(Playing);
    m_bArtRequested = false;

    Q_EMIT changed();

    if(p->metaLoaded())
        Q_EMIT metaChanged();
}

void Player::prevDeleted()
{
    m_Prev = 0;
    m_Current = 0;
}

void Player::trackUpdated(bool s)
{
    if(s) {
        if(m_Current) {
            playTrack(m_Current);
            Q_EMIT changed();
            if(m_bBroadcastStatus)
                m_VkActions->setStatus(m_Current);
        }
    } else {
        Q_EMIT needTrack();
    }
}

void Player::setInitVolume()
{
    Q_EMIT initVolume(Settings::instance()->getValue("player/volume").toDouble());
}

void Player::positionChanged()
{
    QTime position = m_GstPlayer->position();
    QTime length = m_GstPlayer->length();

    int s_pos = (position.minute() * 60) + position.second();

    if(position.toString("mss") != "3433") {
        Q_EMIT tick(position, length);
        Q_EMIT tick(s_pos);
    }

    Q_EMIT updatePos();
}

void Player::onLinkExpired()
{
    if(m_Current) {
        m_Current->updateUrl();
        connect(m_Current, SIGNAL(updated(bool)), SLOT(trackUpdated(bool)));
    }
}

void Player::onStateChanged()
{
    //qDebug() << m_GstPlayer->state();
}

void Player::setBitrate(quint32 value)
{
    if(m_Current)
        m_Current->setBitrate(value);
}

void Player::onMetaChanged()
{
    if(m_Current)
        if(!m_Current->metaLoaded()) {
            QMap<QString, QString> tags = m_GstPlayer->meta();

            if(!tags["brate"].isEmpty())
                Q_EMIT metaChanged(tags);

            QString artStr;
            if(!m_bArtRequested && !tags["album"].isEmpty()) {
                artStr = tags["artist"] + " " + tags["album"];
                Q_EMIT loadArt(artStr);
                m_bArtRequested = true;
            }

            if(!tags["artist"].isEmpty())
                m_Current->setMetaArtist(tags["artist"].trimmed());

            if(!tags["title"].isEmpty())// {
                m_Current->setMetaTitle(tags["title"]);

            if(!tags["album"].isEmpty()) {
                m_Current->setMetaAlbum(tags["album"]);
                m_Current->setMetaLoaded(true);
            }


            Q_EMIT metaChanged();
         }
}

void Player::loopPos()
{
    if(m_GstPlayer->pos() < 0) {
        QTimer::singleShot(100, this, SLOT(loopPos()));
    } else {
        Q_EMIT seeked(m_GstPlayer->pos() / 1000);
    }
}

void Player::onPrevDeleted()
{
    m_Prev = 0;
}

void Player::setTrack(Track *p)
{
    if(m_Current) {
        m_Prev = m_Current;
        connect(m_Prev, SIGNAL(destroyed()), SLOT(onPrevDeleted()));
    }

    if(m_Prev) {
        m_Prev->setDefaultState();
        disconnect(m_Prev, SIGNAL(updated(bool)), this, SLOT(trackUpdated(bool)));
        m_Prev->setLast(false);
    }

    m_Current = p;
    connect(m_Current, SIGNAL(destroyed()), SLOT(prevDeleted()));

    m_Current->setLast(true);

    if(m_Current->url().isEmpty()) {
        m_Current->updateUrl();
        connect(m_Current, SIGNAL(updated(bool)), SLOT(trackUpdated(bool)));
    } else {
        playTrack(m_Current);
    }

    if(m_bBroadcastStatus)
        m_VkActions->setStatus(m_Current);

    // Meta
    if(m_Current->metaLoaded()) {
        Q_EMIT metaChanged();

        QString artStr;
        if(!m_bArtRequested && !m_Current->metaAlbum().isEmpty()) {
            artStr = m_Current->metaArtist() + " " + m_Current->metaAlbum();
            Q_EMIT loadArt(artStr);
            m_bArtRequested = true;
        }
    }

}

void Player::play()
{
    if(m_State == Stoped) {
        Q_EMIT needLast();
        return;
    }

    m_GstPlayer->play();
    setState(Playing);

    Q_EMIT changed();
    loopPos();
}

void Player::pause()
{
    m_GstPlayer->pause();
    setState(Paused);

    Q_EMIT changed();
    loopPos();
}

void Player::playPause()
{
    loopPos();
    if(m_State == Paused) {
        m_GstPlayer->play();
        setState(Playing);

        return;
    }

    if(m_State == Playing) {
        m_GstPlayer->pause();
        setState(Paused);

        return;
    }

    if(m_State == Stoped) {
        Q_EMIT needLast();
        return;
    }

    Q_EMIT changed();

}

void Player::next()
{
    m_plistsgroup->nextTrack();
}

void Player::prev()
{
    m_plistsgroup->prevTrack();
}

void Player::seek(qint64 value)
{
    uint length = -m_GstPlayer->length().msecsTo(QTime());
    if (length != 0 && value > -1) {
        QTime pos;
        pos = pos.addMSecs(length * (value / 1000.0));
        m_GstPlayer->setPosition(pos);

        Q_EMIT seeked(length * (value / 1000.0) * 1000);
    }
}

void Player::setVolume(double value)
{
    m_GstPlayer->setVolume(value);
    Q_EMIT volumeChanged(value);

    Settings::instance()->setValue("player/volume", value);

    if(!sender())
        setInitVolume();
}

void Player::setBroadcastStatus(bool state)
{
    if(state && !m_bBroadcastStatus && m_Current)
        m_VkActions->setStatus(m_Current);

    m_bBroadcastStatus = state;

    // Save state
    Settings::instance()->setValue("player/status", state);
}

void Player::settingsChanged()
{

}
