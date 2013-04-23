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

#include "track.h"
#include "network/parser.h"
#include "settings/settings.h"
#include "network/downloadmanager.h"
#include "network/vkactions.h"

#include "QDebug"
#include <QPainter>
#include <QStyle>


Q_DECLARE_METATYPE(Track*)

Track::Track(QObject *parent) :
    QObject(parent), m_nIndex(-1)
{
    m_bMetaEnabled = false;
    m_bMetaLoaded = false;
    m_bUseMeta = false;
    m_iBitrate = 0;
    m_nIndex = -1;
    m_bLast = false;

    m_sAlbum = "";

    for(int i=0; i<5; i++) {
        m_iTrack.insert(i, new QStandardItem());

        // Store pointer "this" pointer to model item
        m_iTrack.at(i)->setData(QVariant::fromValue(this), MyClassRole);
    }

    m_LoadingMov = new QMovie(":/icons/loading_small_dark");
    connect(m_LoadingMov, SIGNAL(frameChanged(int)), SLOT(updateAnim(int)));

}

Track* Track::copy(QObject *parent)
{
    Track *newTrack = new Track(parent);
    newTrack->setAid(this->aid());
    newTrack->setOid(this->oid());
    newTrack->setUrl(this->url());
    newTrack->setBitrate(this->rawBitrate());
    newTrack->setDuration(this->duration());
    newTrack->setHash(this->hash());
    newTrack->setLenght(this->lenght());
    newTrack->setArtist(this->artist());
    newTrack->setTitle(this->title());
    newTrack->setMetaArtist(this->metaArtist());
    newTrack->setMetaTitle(this->metaTitle());
    newTrack->setMetaLoaded(this->metaLoaded());

    return newTrack;
}

void Track::setArtist(QString artist)  { m_sArtist = artist; update();}
void Track::setMetaArtist(QString artist)  { m_sMetaArtist = artist; update();}
void Track::setTitle(QString title) { m_sTitle = title; update(); }
void Track::setMetaTitle(QString title) { m_sMetaTitle = title; update(); }
void Track::setAlbum(QString s) { m_sAlbum = s; update(); }
void Track::setMetaAlbum(QString s) { m_sMetaAlbum = s; update(); }
void Track::setUrl(QString url) { m_sUrl = url; }

void Track::setDuration(QString duration) { m_sDuration = duration; update(); }
void Track::setLenght(int lenght) { m_iLenght = lenght; }
void Track::setAid(QString aid) { m_sAid = aid; }
void Track::setOid(QString oid) { m_sOid = oid; }
void Track::setHash(QString hash) { m_sHash = hash; }
void Track::setDelHash(QString hash) { m_sDelHash = hash; }

void Track::setPlayState()
{
    m_iTrack.first()->setIcon(QIcon(QPixmap(":/icons/play_small")));
    Q_EMIT playState();
}

void Track::setDefaultState()
{
    m_iTrack.first()->setIcon(QIcon(QPixmap()));
}

void Track::setToQueue(int queueIndex)
{
    QPainter p;

    QPixmap *pm = new QPixmap(":/icons/queue");

    p.begin(pm);
    p.setPen(Qt::white);
    p.setFont(QFont("Mono", 20, 100));
    p.drawText(pm->width()/2 - 11,pm->height()/2+8, QString::number(queueIndex));
    p.end();

    m_iTrack.first()->setIcon(QIcon(QPixmap(pm->pixmapData())));
    delete pm;

}

void Track::updateUrl()
{
    showLoading();

    setAvailable(false);

    Q_EMIT updateMe();
}

void Track::urlUpdated(bool s)
{
    hideLoading();
    if(s) {
        setAvailable(true);
        Q_EMIT updated(true);
    } else {
        Q_EMIT updated(false);
    }
}

void Track::download()
{
    DownloadManager::instance()->add(this);
}

void Track::add2Library()
{
    VkActions::instance()->addToLibrary(this);
}

void Track::setIndex(int index) { m_nIndex = index; update(); }

QList<QStandardItem *> Track::mitem() { return m_iTrack; }

QString Track::artist()
{
    if(m_bUseMeta && !m_sMetaArtist.isEmpty())
        return m_sMetaArtist;
    else
        return m_sArtist;
}

QString Track::title()
{
    if(m_bUseMeta && !m_sMetaTitle.isEmpty())
        return m_sMetaTitle;
    else
        return m_sTitle;
}

QString Track::url() { return m_sUrl; }
QString Track::duration() { return m_sDuration; }
int Track::lenght() { return m_iLenght; }
QString Track::aid() { return m_sAid; }
QString Track::oid() { return m_sOid; }
QString Track::hash() { return m_sHash; }
QString Track::delHash() { return m_sDelHash; }

QString Track::bitrate()
{
    int brate = m_iBitrate / 1000;
    if(brate > 0 && brate < 1000)
        return QString("%0 B/s").arg(brate);
    else
        return "0 B/s";
}

void Track::getMeta()
{
    if(m_bMetaEnabled && !m_bMetaLoaded) {
        Q_EMIT updateMyMeta();
    }
}

void Track::update()
{
    m_iTrack.at(1)->setText(QString::number(m_nIndex+1));
    m_iTrack.at(2)->setText(QString("%0 - %1").arg(artist()).arg(title()));
    m_iTrack.at(3)->setText(bitrate());
    m_iTrack.at(4)->setText(m_sDuration);
}

void Track::showLoading()
{
    m_LoadingMov->start();
}

void Track::hideLoading()
{
    m_LoadingMov->stop();
    m_iTrack.first()->setIcon(QIcon(QPixmap()));
}

void Track::setAvailable(bool s)
{
    if(s) {
        for(int i=0; i<5; i++) {
            m_iTrack.at(i)->setEnabled(true);
        }
    } else {
        for(int i=0; i<5; i++) {
            m_iTrack.at(i)->setEnabled(false);
        }
    }
}

void Track::updateMeta()
{
    // Now, update view
    update();
    m_bMetaLoaded = true;

    hideLoading();
}

void Track::updateAnim(int index)
{
    Q_UNUSED(index);
    m_iTrack.at(0)->setIcon(QIcon(m_LoadingMov->currentPixmap()));

}

void Track::setMetaEnabled(bool s) { Q_UNUSED(s); /*m_bMetaEnabled = s;*/ }

void Track::setMetaLoaded(bool s)
{
    m_bMetaLoaded = s;

    // Now, update view
    if(s)
        update();

    //hideLoading();

    Q_EMIT metaUpdated();
}
void Track::setUseMeta(bool s) { m_bUseMeta = s; update(); }

void Track::metaTagsChanged(bool load, bool use)
{
    m_bMetaEnabled = load;
    m_bUseMeta = use;

    if(m_bMetaEnabled)
        getMeta();

    if(m_bUseMeta)
        update();

}
