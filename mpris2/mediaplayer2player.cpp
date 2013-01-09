/***********************************************************************
 * Copyright 2012  Eike Hein <hein@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ***********************************************************************/

#include "mpris2/mediaplayer2player.h"
#include "playlist/track.h"
#include "settings/settings.h"
#include "network/artloader.h"

#include <QCryptographicHash>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QVariant>
#include <QFile>

static QByteArray idFromPlaylistItem(Track *item)
{
    return QByteArray("/org/yuberion/pulsar/tid_") +
           QByteArray::number(item->index(), 16).rightJustified(8, '0');
}

MediaPlayer2Player::MediaPlayer2Player(QObject* parent)
    : QDBusAbstractAdaptor(parent)
    , m_player(Player::instance())
{
    connect(m_player, SIGNAL(trackChanged()), this, SLOT(currentSourceChanged()));
    connect(m_player, SIGNAL(metaChanged()), this, SLOT(currentSourceChanged()));
    connect(ArtLoader::instance(), SIGNAL(artLoaded(bool)), this, SLOT(currentSourceChanged()));
    connect(m_player, SIGNAL(stateChanged(Player::States)), this, SLOT(stateUpdated()));
    connect(m_player, SIGNAL(totalTimeChanged(QTime)), this, SLOT(totalTimeChanged()));
    connect(m_player, SIGNAL(volumeChanged(double)), this, SLOT(volumeChanged(double)));
    //connect(m_player, SIGNAL(seek), this, SLOT(totalTimeChanged()));
    //connect(m_player, SIGNAL(updatePos()), this, SLOT(totalTimeChanged()));

    //connect(m_player, SIGNAL(signalItemChanged(FileHandle)), this, SLOT(currentSourceChanged()));
    //connect(m_player, SIGNAL(signalPlay()), this, SLOT(stateUpdated()));
    //connect(m_player, SIGNAL(signalPause()), this, SLOT(stateUpdated()));
    //connect(m_player, SIGNAL(signalStop()), this, SLOT(stateUpdated()));
    //connect(m_player, SIGNAL(totalTimeChanged(int)), this, SLOT(totalTimeChanged()));
    //connect(m_player, SIGNAL(seekableChanged(bool)), this, SLOT(seekableChanged(bool)));
    //connect(m_player, SIGNAL(volumeChanged(float)), this, SLOT(volumeChanged(float)));
    connect(m_player, SIGNAL(seeked(qint64)), this, SLOT(seeked(qint64)));
}

MediaPlayer2Player::~MediaPlayer2Player()
{
}

bool MediaPlayer2Player::CanGoNext() const
{
    return true;
}

void MediaPlayer2Player::Next() const
{
    m_player->next();
}

bool MediaPlayer2Player::CanGoPrevious() const
{
    return true;
}

void MediaPlayer2Player::Previous() const
{
    m_player->prev();
}

bool MediaPlayer2Player::CanPause() const
{
    return true;
}

void MediaPlayer2Player::Pause() const
{
    m_player->pause();
}

void MediaPlayer2Player::PlayPause() const
{
    m_player->playPause();
}

void MediaPlayer2Player::Stop() const
{
    m_player->pause();
}

bool MediaPlayer2Player::CanPlay() const
{
    return true;
}

void MediaPlayer2Player::Play() const
{
    m_player->play();
}

void MediaPlayer2Player::SetPosition(const QDBusObjectPath& TrackId, qlonglong Position) const
{
    Q_UNUSED(TrackId);
    m_player->seek(((Position * 100) / m_player->totalTime()) / 100);
}

void MediaPlayer2Player::OpenUri(QString Uri) const
{
    Q_UNUSED(Uri);
    //QUrl url(Uri);

    // JuK does not yet support KIO
    //if (url.isLocalFile()) {
        //m_player->play(url.toLocalFile());
    //}
}

QString MediaPlayer2Player::PlaybackStatus() const
{
    if (m_player->state() == Player::Playing) {
        return QLatin1String("Playing");
    }
    else if (m_player->state() == Player::Paused) {
        return QLatin1String("Paused");
    }

    return QLatin1String("Stopped");
}

QString MediaPlayer2Player::LoopStatus() const
{
    // TODO: Implement, although this is orthogonal to the PlayerManager
    return "None";
}

void MediaPlayer2Player::setLoopStatus(const QString& loopStatus) const
{
    Q_UNUSED(loopStatus)
}

double MediaPlayer2Player::Rate() const
{
    return 1.0;
}

void MediaPlayer2Player::setRate(double rate) const
{
    Q_UNUSED(rate)
}

bool MediaPlayer2Player::Shuffle() const
{
    // TODO: Implement
    return false;
}

void MediaPlayer2Player::setShuffle(bool shuffle) const
{
    Q_UNUSED(shuffle)
    // TODO: Implement
}

QVariantMap MediaPlayer2Player::Metadata() const
{
    QVariantMap metaData;

    // The track ID is annoying since it must result in a valid DBus object
    // path, and the regex for that is, and I quote: [a-zA-Z0-9_]*, along with
    // the normal / delimiters for paths.
    Track *track = m_player->currentTrack();
    if (!track)
        return metaData;

    //FileHandle playingFile = item->file();
    QByteArray playingTrackFileId = idFromPlaylistItem(track);

    metaData["mpris:trackid"] =
        QVariant::fromValue<QDBusObjectPath>(
                QDBusObjectPath(playingTrackFileId.constData()));

    QString path;
    if(track->metaLoaded()) {
        metaData["xesam:album"] = track->metaAlbum();
        metaData["xesam:title"] = track->metaTitle();
        metaData["xesam:artist"] = QStringList(track->metaArtist());
    } else {
        metaData["xesam:album"] = track->album();
        metaData["xesam:title"] = track->title();
        metaData["xesam:artist"] = QStringList(track->artist());
        metaData["xesam:genre"]  = QStringList();
    }

    //qDebug() << "Track lenght: " << track->lenght();
    metaData["mpris:length"] = qint64(track->lenght() * 1000000);
    metaData["xesam:url"] = track->url();

    path = ArtLoader::instance()->artLocalUri();
    metaData["mpris:artUrl"] = QString::fromLatin1(QUrl::fromLocalFile(
            path).toEncoded());

    //qDebug() << metaData["mpris:artUrl"];

    //if(playingFile.coverInfo()->hasCover()) {


//        QString path = fallbackFileName;
//        if(!QFile::exists(path)) {
//            path = playingFile.coverInfo()->localPathToCover(fallbackFileName);
//        }


    //}

    return metaData;
}

double MediaPlayer2Player::Volume() const
{
    return m_player->volume();
}

void MediaPlayer2Player::setVolume(double volume) const
{
    if (volume < 0.0)
        volume = 0.0;
    if (volume > 1.0)
        volume = 1.0;
    m_player->setVolume(volume);
}

qlonglong MediaPlayer2Player::Position() const
{
    return m_player->currentTime() * 1000;
}

double MediaPlayer2Player::MinimumRate() const
{
    return 1.0;
}

double MediaPlayer2Player::MaximumRate() const
{
    return 1.0;
}

bool MediaPlayer2Player::CanSeek() const
{
    return true;
}

void MediaPlayer2Player::Seek(qlonglong Offset) const
{
    Q_UNUSED(Offset);
    //qDebug() << "MPRIS_SEEK: " << ((m_player->currentTime()) + Offset) / 1000;
    //m_player->seek(((m_player->currentTime()) + Offset) / 1000);
}

bool MediaPlayer2Player::CanControl() const
{
    return true;
}

void MediaPlayer2Player::currentSourceChanged() const
{
    QVariantMap properties;
    properties["Metadata"] = Metadata();
    properties["CanSeek"] = CanSeek();
    signalPropertiesChange(properties);
}

void MediaPlayer2Player::stateUpdated() const
{
    QVariantMap properties;
    properties["PlaybackStatus"] = PlaybackStatus();
    signalPropertiesChange(properties);
    //Q_EMIT Seeked(m_lastPos);
}

void MediaPlayer2Player::totalTimeChanged() const
{
    QVariantMap properties;
    properties["Metadata"] = Metadata();
    signalPropertiesChange(properties);
}

void MediaPlayer2Player::seekableChanged(bool seekable) const
{
    QVariantMap properties;
    properties["CanSeek"] = seekable;
    signalPropertiesChange(properties);
}

void MediaPlayer2Player::volumeChanged(double newVol) const
{
    Q_UNUSED(newVol)

    QVariantMap properties;
    properties["Volume"] = Volume();
    signalPropertiesChange(properties);
}

void MediaPlayer2Player::seeked(qint64 newPos) const
{
    // casts int to uint64
    Q_EMIT Seeked(newPos);
}

void MediaPlayer2Player::signalPropertiesChange(const QVariantMap& properties) const
{
    QDBusMessage msg = QDBusMessage::createSignal("/org/mpris/MediaPlayer2",
        "org.freedesktop.DBus.Properties", "PropertiesChanged" );

    QVariantList args;
    args << "org.mpris.MediaPlayer2.Player";
    args << properties;
    args << QStringList();

    msg.setArguments(args);

    QDBusConnection::sessionBus().send(msg);
}
