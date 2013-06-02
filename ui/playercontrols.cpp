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

#include "playercontrols.h"
#include "settings/settings.h"
#include "network/artloader.h"
#include "playlist/player.h"
#include "playlist/plistsgroup.h"

#include <QHBoxLayout>
#include <QGridLayout>
#include <QTime>
#include <QDebug>
#include <QMouseEvent>
#include <QAbstractSlider>

PlayerControls::PlayerControls(QMenu *appMenu, QWidget *parent) :
    QWidget(parent)
{
    m_iShuffleMode = 1;
    m_iRepeatMode = 1;

    m_currentTrack = 0;

    m_mainLayout = new QGridLayout(this);

    m_localsMenu = new QMenu(tr("Add to list"));
    connect(PlistsGroup::instance(), SIGNAL(localsChanged(QStringList)), SLOT(onLocalsChanged(QStringList)));
    PlistsGroup::instance()->updateLocalLists();

    setupCover();
    setupTrackbar();
    setupSliders();
    setupButtons(appMenu);



    layout()->setMargin(0);

    connect(Settings::instance(), SIGNAL(changed()), SLOT(settingsUpdated()));
}


bool PlayerControls::eventFilter(QObject *target, QEvent *event)
{
    if(target == m_wSeekSlider) {
        if(event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = (QMouseEvent *)event;
            if(mouseEvent->button() == Qt::LeftButton) {
                int value = m_wSeekSlider->minimum() + ((m_wSeekSlider->maximum()-m_wSeekSlider->minimum()) * mouseEvent->x()) / m_wSeekSlider->width();
                Q_EMIT seek(value);
                m_wSeekSlider->blockSignals(true);
                m_wSeekSlider->setValue(value);
                 m_wSeekSlider->blockSignals(false);
            }
        }
    }
    return false;
}

void PlayerControls::setupTrackbar()
{
    QHBoxLayout *hLayout = new QHBoxLayout();
    QHBoxLayout *hLayoutTop = new QHBoxLayout();
    QHBoxLayout *hLayoutActions = new QHBoxLayout();
    QVBoxLayout *vLayout = new QVBoxLayout();

    Settings *sett = Settings::instance();

    m_wlTitle = new QLabel(sett->getValue("song/title").toString());
    m_wlTitle->setStyleSheet("font-size: 17pt;");
    m_wlTitle->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_wlTitle->setAlignment(Qt::AlignLeft|Qt::AlignBottom);
    m_wlArtist = new QLabel(sett->getValue("song/artist").toString());
    m_wlArtist->setStyleSheet("font-size: 11pt;");

    m_wlAlbum = new QLabel(sett->getValue("song/album").toString());
    m_wlAlbum->setStyleSheet("font-size: 10pt; color: grey;");
    m_wlAlbum->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);

    // Track actions
    m_wbDownload = new StyledButton(QIcon(QPixmap(":icons/panel_download")), "");
    m_wbDownload->setTransparent(true);
    m_wbDownload->setToolTip(tr("Download track"));
    connect(m_wbDownload, SIGNAL(clicked()), SLOT(downloadTrack()));

    m_wbAdd2List = new StyledButton(QIcon(QPixmap(":icons/panel_listadd")), "");
    m_wbAdd2List->setTransparent(true);
    m_wbAdd2List->setToolTip(tr("Add to list"));
    m_wbAdd2List->setMenu(m_localsMenu);

    m_wbAdd2Library = new StyledButton(QIcon(QPixmap(":icons/panel_libraryadd")), "");
    m_wbAdd2Library->setTransparent(true);
    m_wbAdd2Library->setToolTip(tr("Add to library"));
    connect(m_wbAdd2Library, SIGNAL(clicked()), SLOT(addTrack2Library()));

    hLayoutTop->addWidget(m_wlTitle);
    hLayoutTop->addLayout(hLayoutActions);
    hLayoutTop->setAlignment(hLayoutActions, Qt::AlignRight);

    hLayoutActions->addWidget(m_wbAdd2Library);
    hLayoutActions->addWidget(m_wbAdd2List);
    hLayoutActions->addWidget(m_wbDownload);



    hLayout->addWidget(m_wlArtist);
    hLayout->addWidget(m_wlAlbum);
    hLayout->setStretch(0, 0);
    hLayout->setStretch(1, 1);

    vLayout->addLayout(hLayoutTop);
    vLayout->addLayout(hLayout);

    m_mainLayout->addLayout(vLayout, 0, 1);

}

void PlayerControls::setupSliders()
{
    QString lblStyle = "QLabel { color: #555; font-size: 9px; }";
    m_styleSlider = "::sub-page:horizontal { background-image: url(:/images/prog_sub); } ::add-page:horizontal { background-image: url(:/images/prog_add); } ::groove:horizontal {background:  transparent; } ::handle:horizontal {image: url(:/images/handle);} ::handle:horizontal:hover {image: url(:/images/handle_hover);}";

    QHBoxLayout *layout = new QHBoxLayout();

    // Seek
    m_wSeekSlider = new QSlider(this);
    m_wSeekSlider->setOrientation(Qt::Horizontal);
    m_wSeekSlider->setFixedHeight(20);
    m_wSeekSlider->setStyleSheet(m_styleSlider);
    m_wSeekSlider->setMaximum(1000);
    m_wSeekSlider->setTickInterval(10);

    m_wSeekSlider->installEventFilter(this);

    m_bTickSlider = true;

    m_wlCurTime = new QLabel("0:00");
    m_wlCurTime->setStyleSheet(lblStyle);
    m_wlTotTime = new QLabel("0:00");
    m_wlTotTime->setStyleSheet(lblStyle);

    // Volume
    m_wVolumeSlider = new QSlider(this);
    m_wVolumeSlider->setOrientation(Qt::Horizontal);
    m_wVolumeSlider->setFixedHeight(20);
    m_wVolumeSlider->setMaximumWidth(100);
    m_wVolumeSlider->setObjectName("volumeSlider");
    m_wVolumeSlider->setStyleSheet(m_styleSlider);
    m_wVolumeSlider->setValue(75);
    m_wVolumeSlider->setMaximum(100);

    m_wlCurVolume = new QLabel("75");
    m_wlCurVolume->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    m_wlCurVolume->setMinimumWidth(18);
    m_wlCurVolume->setStyleSheet(lblStyle);

    // Layouts
    layout->addWidget(m_wlCurTime);
    layout->addWidget(m_wSeekSlider);
    layout->addWidget(m_wlTotTime);
    layout->addSpacing(30);
    layout->addWidget(m_wlCurVolume);
    layout->addWidget(m_wVolumeSlider);;

    // Connection
    //connect(m_wVolumeSlider, SIGNAL(valueChanged(int)), SLOT(setVolume(int)));
    connect(m_wVolumeSlider, SIGNAL(valueChanged(int)), SLOT(onVolumeChanged(int)));

    connect(m_wSeekSlider, SIGNAL(sliderPressed()), SLOT(sliderPressed()));
    connect(m_wSeekSlider, SIGNAL(sliderReleased()), SLOT(sliderReleased()));


    m_mainLayout->addLayout(layout, 1, 1);
}

void PlayerControls::setupButtons(QMenu *appMenu)
{
    QHBoxLayout *layout = new QHBoxLayout();

    QHBoxLayout *leftLayout = new QHBoxLayout();
    leftLayout->setAlignment(Qt::AlignLeft);

    QHBoxLayout *centerLayout = new QHBoxLayout();
    centerLayout->setAlignment(Qt::AlignCenter);

    QHBoxLayout *rightLayout = new QHBoxLayout();
    rightLayout->setAlignment(Qt::AlignRight);

    // Menu button
    m_wbAppMenu = new StyledButton(QIcon(QPixmap(":icons/menu")), tr("Menu"));
    m_wbAppMenu->setFixedHeight(28);
    m_wbAppMenu->setIconSize(QSize(16, 16));
    m_wbAppMenu->setMenu(appMenu);

    // Prev
    m_wbPrev = new StyledButton(QIcon(QPixmap(":icons/prev")), "Prev");
    m_wbPrev->setFixedHeight(28);
    m_wbPrev->setIconSize(QSize(16, 16));
    connect(m_wbPrev, SIGNAL(clicked()), SIGNAL(prev()));

    // Play
    QIcon playIcon;
    playIcon.addFile(QString::fromUtf8(":/icons/play"), QSize(), QIcon::Active, QIcon::Off);
    playIcon.addFile(QString::fromUtf8(":/icons/pause"), QSize(), QIcon::Active, QIcon::On);
    m_wbPlay = new StyledButton(playIcon, "Play");
    m_wbPlay->setFixedHeight(28);
    m_wbPlay->setIconSize(QSize(16, 16));
    m_wbPlay->setCheckable(true);
    connect(m_wbPlay, SIGNAL(clicked()), SIGNAL(play()));

    // Next
    m_wbNext = new StyledButton(QIcon(QPixmap(":icons/next")), "Next");
    m_wbNext->setFixedHeight(28);
    m_wbNext->setIconSize(QSize(16, 16));
    m_wbNext->setLayoutDirection(Qt::RightToLeft);
    connect(m_wbNext, SIGNAL(clicked()), SIGNAL(next()));

    // Shuffle
    m_wbShuffle = new StyledButton(QIcon(QPixmap(":icons/shuffle")), "Shuffle");
    m_wbShuffle->setCheckable(true);
    m_wbShuffle->setFixedSize(70, 28);
    m_wbShuffle->setIconSize(QSize(16, 16));
    m_wbShuffle->setLayoutDirection(Qt::LeftToRight);
    setShuffle(Settings::instance()->getValue("player/shuffle").toInt());
    connect(m_wbShuffle, SIGNAL(clicked()), SLOT(setShuffle()));

    // Repeat
    m_wbRepeat = new StyledButton(QIcon(QPixmap(":icons/repeat")), "Repeat");
    m_wbRepeat->setCheckable(true);
    m_wbRepeat->setFixedSize(70, 28);
    m_wbRepeat->setIconSize(QSize(16, 16));
    setRepeat(Settings::instance()->getValue("player/repeat").toInt());
    connect(m_wbRepeat, SIGNAL(clicked()), SLOT(setRepeat()));

    // Status
    m_wbStatus = new StyledButton(QIcon(QPixmap(":icons/vk")), "");
    m_wbStatus->setCheckable(true);
    m_wbStatus->setFixedSize(28,28);
    m_wbStatus->setIconSize(QSize(20, 20));
    m_wbStatus->setTransparent(true);
    connect(m_wbStatus, SIGNAL(clicked(bool)), SLOT(setStatusState(bool)));
    setStatusState(Settings::instance()->getValue("player/status").toBool());

    m_wbLastfm = new StyledButton(QIcon(QPixmap(":icons/lf")), "");
    m_wbLastfm->setCheckable(true);
    m_wbLastfm->setFixedSize(28,28);
    m_wbLastfm->setIconSize(QSize(20, 20));
    m_wbLastfm->setTransparent(true);
    connect(m_wbLastfm, SIGNAL(clicked(bool)), SLOT(setLastfmState(bool)));
    setLastfmState(Settings::instance()->getValue("lastfm/scrobbling").toBool());

    layout->addLayout(leftLayout);
    layout->addLayout(centerLayout);
    layout->addLayout(rightLayout);

    leftLayout->addWidget(m_wbAppMenu);

    centerLayout->addWidget(m_wbShuffle);
    centerLayout->addSpacing(20);
    centerLayout->addWidget(m_wbPrev);
    centerLayout->addWidget(m_wbPlay);
    centerLayout->addWidget(m_wbNext);
    centerLayout->addSpacing(20);
    centerLayout->addWidget(m_wbRepeat);

    rightLayout->addWidget(m_wbStatus);
    rightLayout->addWidget(m_wbLastfm);

    m_mainLayout->addLayout(layout, 2, 1);
    m_mainLayout->setAlignment(layout, Qt::AlignTop);

    bool isAcc = Settings::instance()->getValue("general/account_use").toBool();

    if(!isAcc) {
        m_wbStatus->setDisabled(true);
    }
}

void PlayerControls::setupCover()
{
    m_wSongInfo = new SongInfo(this);
    connect(ArtLoader::instance(), SIGNAL(artLoaded(bool)), m_wSongInfo, SLOT(onArtLoaded(bool)));
    connect(Player::instance(), SIGNAL(trackChanged()), m_wSongInfo, SLOT(clearCover()));

    m_mainLayout->addWidget(m_wSongInfo, 0, 0, 3, 1);
    m_mainLayout->setAlignment(m_wSongInfo, Qt::AlignBottom);
    m_mainLayout->setRowMinimumHeight(2, 43);
}

void PlayerControls::saveTitles()
{
    Settings *sett = Settings::instance();

    sett->setValue("song/title", m_wlTitle->text());
    sett->setValue("song/artist", m_wlArtist->text());
    sett->setValue("song/album", m_wlAlbum->text());
}

void PlayerControls::setVolume(double volume)
{
    int val = volume * 100;

    if(sender() != m_wVolumeSlider)
        m_wVolumeSlider->setValue(val);

    m_wlCurVolume->setText(QString::number(val));

    Q_EMIT volumeChanged(volume);
}

void PlayerControls::tick(QTime pos, QTime len)
{
    QTime length(0,0);
    QTime curpos(0,0);

    length = len;
    curpos = pos;

    m_wlCurTime->setText(curpos.toString("m:ss"));
    m_wlTotTime->setText(length.toString("mm:ss"));

    if(m_bTickSlider) {
        if (length != QTime(0,0)) {
            int curVal = curpos.msecsTo(QTime()) * 1000 / length.msecsTo(QTime());
            m_wSeekSlider->setValue(curVal);
            if(curVal < 999)
                curVal+=1;
        } else {
            m_wSeekSlider->setValue(0);
        }
    }

//    if (curpos != QTime(0,0)) {
//        m_positionLabel->setEnabled(true);
//        m_positionSlider->setEnabled(true);
//    }
}

void PlayerControls::setPlayingState(bool s)
{
    m_wbPlay->setChecked(s);
}

void PlayerControls::setStatusState(bool s)
{
    m_wbStatus->setChecked(s);
    Q_EMIT status(s);

    // Change icon
    if(s) {
        m_wbStatus->setIcon(QIcon(QPixmap(":icons/vk_on")));
    } else {
        m_wbStatus->setIcon(QIcon(QPixmap(":icons/vk")));
    }
}

void PlayerControls::setLastfmState(bool s)
{
    Settings::instance()->setValue("lastfm/scrobbling", s);
    m_wbLastfm->setChecked(s);
    //Q_EMIT lastfm(s);


    // Change icon
    if(s) {
        m_wbLastfm->setIcon(QIcon(QPixmap(":icons/lf_on")));
    } else {
        m_wbLastfm->setIcon(QIcon(QPixmap(":icons/lf")));
    }
}

void PlayerControls::setShuffle()
{
    setShuffle(0);
}

void PlayerControls::setShuffle(int mode)
{
    if(mode > 0) {
        m_iShuffleMode = mode;
    } else {
        if(m_iShuffleMode < 3)
            m_iShuffleMode++;
        else
            m_iShuffleMode = 1;

    }

    Q_EMIT shuffle(m_iShuffleMode);

    switch(m_iShuffleMode) {
    case 1:
        m_wbShuffle->setText("OFF");
        m_wbShuffle->setChecked(false);

        break;
    case 2:
        m_wbShuffle->setText("Tracks");
        m_wbShuffle->setChecked(true);
        break;
    case 3:
        m_wbShuffle->setText("Lists");
        m_wbShuffle->setChecked(true);
        break;
    }
}

void PlayerControls::setRepeat()
{
    setRepeat(0);
}

void PlayerControls::setRepeat(int mode)
{
    if(mode > 0) {
        m_iRepeatMode = mode;
    } else {
        if(m_iRepeatMode < 3)
            m_iRepeatMode++;
        else
            m_iRepeatMode = 1;

    }

    Q_EMIT repeat(m_iRepeatMode);

    switch(m_iRepeatMode) {
    case 1:
        m_wbRepeat->setText("OFF");
        m_wbRepeat->setChecked(false);

        break;
    case 2:
        m_wbRepeat->setText("Track");
        m_wbRepeat->setChecked(true);
        break;
    case 3:
        m_wbRepeat->setText("List");
        m_wbRepeat->setChecked(true);
        break;
    }
}

QString crop(QString str) {
    int aSize = 35;
    if(str.size() > aSize)
        return str.left(aSize)+"...";
    else
        return str;
}

void PlayerControls::onTrackChanged(Track *p)
{

    if(m_currentTrack) {
        disconnect(m_currentTrack, SIGNAL(metaUpdated()), this, SLOT(onMetaUpdated()));
    }

    m_wlTitle->setText(crop(p->title()));
    m_wlArtist->setText(crop(p->artist()));
    m_wlAlbum->setText("");

    m_currentTrack = p;
    connect(p, SIGNAL(destroyed()), SLOT(onCurrentDeleted()));

    if(!m_currentTrack->metaLoaded())
        connect(p, SIGNAL(metaUpdated()), SLOT(onMetaUpdated()));
    else
        onMetaUpdated();

    saveTitles();
}

void PlayerControls::sliderPressed()
{
    m_oldSeekValue = m_wSeekSlider->value();

    m_bTickSlider = false;
}

void PlayerControls::sliderReleased()
{
    Q_EMIT seek(m_wSeekSlider->value());
    m_bTickSlider = true;
}


void PlayerControls::onVolumeChanged(int volume)
{
    double val = volume / 100.0;
    setVolume(val);

    Q_EMIT volumeChanged(val);
}

void PlayerControls::downloadTrack()
{
    if(m_currentTrack)
        m_currentTrack->download();
}

void PlayerControls::addTrack2Library()
{
    if(m_currentTrack)
        m_currentTrack->add2Library();
}

void PlayerControls::onMetaUpdated()
{
    if(m_currentTrack->metaTitle() != "")
        m_wlTitle->setText(crop(m_currentTrack->metaTitle()));
    if(m_currentTrack->metaArtist() != "")
        m_wlArtist->setText(crop(m_currentTrack->metaArtist()));
    if(m_currentTrack->metaAlbum() != "")
        m_wlAlbum->setText(crop(m_currentTrack->metaAlbum()));

    saveTitles();
}

void PlayerControls::onCurrentDeleted()
{
    m_currentTrack = 0;
}

void PlayerControls::settingsUpdated()
{
    setLastfmState(Settings::instance()->getValue("lastfm/scrobbling").toBool());
    setShuffle(Settings::instance()->getValue("player/shuffle").toInt());
    setRepeat(Settings::instance()->getValue("player/repeat").toInt());
}

void PlayerControls::onLocalsChanged(const QStringList list)
{
    if(!list.isEmpty()) {
        m_localsMenu->clear();

        QList<QAction *> actions;

        for(int i = 0; i < list.size(); i++) {
            actions.append(new QAction(list.at(i), this));
            actions.last()->setData(actions.size()-1);
            connect(actions.last(), SIGNAL(triggered()), PlistsGroup::instance(), SLOT(addCurrentToLocal()));
        }

        m_localsMenu->addActions(actions);

        m_localsMenu->setEnabled(true);
    } else {
        m_localsMenu->setDisabled(true);
    }
}
