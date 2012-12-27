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

#ifndef PLAYER_H
#define PLAYER_H

#include "playlist/track.h"
#include "network/auth.h"
#include "playlist/gstplayer.h"
#include "playlist/plistsgroup.h"
#include "network/vkactions.h"

#include <QObject>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QBuffer>
#include <QMap>


class Player : public QObject
{
    Q_OBJECT
public:
    enum State {
             Stoped = 0x0,
             Paused = 0x1,
             Playing = 0x2,
             UpdatingTrack = 0x4,
             Buffering = 0x8
    };
    Q_DECLARE_FLAGS(States, State)

    enum PlayMethod {
        Buffer = 0x0,
        Direct = 0x1,
        File = 0x2
    };
    Q_DECLARE_FLAGS(PlayMethods, PlayMethod)

    explicit Player(QObject *parent = 0);

    // Return pointer to the Player instance
    static Player* instance();

    /* player controls and states */

    // Return state
    Player::States state();

    // Return current track pointer
    Track *currentTrack();

    // Return volume
    double volume();

    // Return shuffle
    bool shuffle();

    // Return repeat
    bool repeat();

    // Set shuffle
    void setShuffle(bool state);

    // Set repeat
    void setRepeat(bool state);

    // Return total time of current track
    int totalTime();

    // Return current time of current track
    int currentTime();

    // Long pos
    qint64 longPos();


private:
    // State
    Player::States m_State;

    // Play method
    Player::PlayMethods m_PlayMethod;

    // Instance pointer
    static Player *m_instance;

    //PlistsGroup instance
    PlistsGroup *m_plistsgroup;

    // Track pointers
    Track *m_Prev;
    Track *m_Current;

    // Gst
    GstPlayer *m_GstPlayer;

    // vkaction(track status)
    VkActions *m_VkActions;

    // Network
    QNetworkAccessManager m_nManager;
    QNetworkReply *m_nReply;

    // Buffer for cashing mp3
    QBuffer *m_Buffer;
    QByteArray *m_BaBuffer;
    qint64 m_BufferLenght;
    int m_BufferBlock;
    int m_BufferPercent;

    // Options
    bool m_bBuffering;

    bool m_bArtRequested;

    bool m_bBroadcastStatus;

    void setState(Player::States);
    void setPlayMethod(Player::PlayMethods);

    void playTrack(Track *p);

Q_SIGNALS:
    void needTrack();
    void needLast();

    void tick(QTime, QTime);
    void tick(int);
    void totalTimeChanged(QTime);

    void stateChanged(Player::States);

    void playintState(bool);

    void initVolume(double);
    void volumeChanged(double);

    void playMethodChanged(Player::PlayMethods);

    void bufferChanged(int);

    void newTitles(QString, QString);

    void changed();

    void updatePos();

    void metaChanged(QMap<QString, QString>);
    void metaChanged();

    void loadArt(QString);

    void trackChanged();
    void trackChanged(Track*);
    void seeked(qint64);

    void playing();
    void paused();

    //void seekTo(int);

//    void

private Q_SLOTS:
    void prevDeleted();
    void trackUpdated(bool);
    void setInitVolume();
    void networkError(QNetworkReply::NetworkError);

    // For buffering
    void readData();
    void bufferProgress(qint64,qint64);

    // gst
    void positionChanged();
    void onLinkExpired();
    void onStateChanged();
    void setBitrate(quint32);
    void onMetaChanged();
    void loopPos();
    void onPrevDeleted();

public Q_SLOTS:
    void setTrack(Track *);
    void play();
    void pause();
    void playPause();
    void next();
    void prev();
    void seek(qint64);
    void setVolume(double);

    // Set status translate
    void setBroadcastStatus(bool state);

    void settingsChanged();

};

#endif // PLAYER_H
