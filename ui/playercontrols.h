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

#ifndef PLAYERCONTROLS_H
#define PLAYERCONTROLS_H

#include "playlist/player.h"
#include "styledbutton.h"
#include "ui/songinfo.h"
#include "playlist/track.h"
#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QProgressBar>
#include <QGridLayout>

class PlayerControls : public QWidget
{
    Q_OBJECT
public:
    explicit PlayerControls(QMenu *appMenu, QWidget *parent = 0);

private:
    QGridLayout *m_mainLayout;

    QSlider *m_wSeekSlider;
    QProgressBar *m_wSeekProgress;
    QLabel *m_wlCurTime;
    QLabel *m_wlTotTime;

    QSlider *m_wVolumeSlider;
    QProgressBar *m_wVolumeProgress;
    QLabel *m_wlCurVolume;

    QString m_styleSlider;
    QString m_stylePrgs;
    QString m_stylePrgsLight;

    // Cover
    SongInfo *m_wSongInfo;

    // Track titles
    QLabel *m_wlTitle;
    QLabel *m_wlArtist;
    QLabel *m_wlAlbum;

    // Track actions
    StyledButton *m_wbDownload;
    StyledButton *m_wbAdd2List;
    StyledButton *m_wbAdd2Library;

    // Bottom controls group
    StyledButton *m_wbAppMenu;
    StyledButton *m_wbPrev;
    StyledButton *m_wbPlay;
    StyledButton *m_wbNext;
    StyledButton *m_wbShuffle;
    StyledButton *m_wbRepeat;
    StyledButton *m_wbStatus;
    StyledButton *m_wbLastfm;

    Track *m_currentTrack;

    QMenu *m_localsMenu;

    int m_oldSeekValue;

    bool m_bTickSlider;

    int m_iShuffleMode;
    int m_iRepeatMode;

    bool eventFilter(QObject *target, QEvent *event);

    void setupTrackbar();
    void setupSliders();
    void setupButtons(QMenu *appMenu);
    void setupCover();
    void saveTitles();

Q_SIGNALS:
    void volumeChanged(double);
    void seek(qint64);

    void shuffle(int);
    void repeat(int);
    void repeat();
    void status(bool);
    void prev();
    void next();
    void play();

public Q_SLOTS:
    void setVolume(double);

    void tick(QTime pos, QTime len);

    void setPlayingState(bool);
    void setStatusState(bool);
    void setLastfmState(bool);
    void setShuffle();
    void setShuffle(int);
    void setRepeat();
    void setRepeat(int);

    void onTrackChanged(Track*);

private Q_SLOTS:
    void sliderPressed();
    void sliderReleased();
    void onVolumeChanged(int);

    void downloadTrack();
    void addTrack2Library();

    void onMetaUpdated();
    void onCurrentDeleted();
    void settingsUpdated();

    void onLocalsChanged(const QStringList);

};

#endif // PLAYERCONTROLS_H
