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

#ifndef TRACK_H
#define TRACK_H

#include <QObject>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QMap>
//#include <phonon/MediaObject>
#include <QMovie>

class Track : public QObject
{
    Q_OBJECT
public:
    enum MyRoles {
        MyClassRole = Qt::UserRole + 1
    };

    Track(QObject *parent = 0);

    Track* copy(QObject *parent = 0);

    /* Set track fields */
    void setArtist(QString artist);
    void setMetaArtist(QString artist);
    void setTitle(QString title);
    void setMetaTitle(QString title);
    void setAlbum(QString s);
    void setMetaAlbum(QString s);
    void setUrl(QString url);
    void setDuration(QString duration);
    void setLenght(int lenght);
    void setAid(QString aid);
    void setOid(QString oid);
    void setHash(QString hash);
    void setDelHash(QString hash);
    void setIndex(int index);
    void setBitrate(int bitrate) { m_iBitrate = bitrate; update(); }

    void setPlayState();
    void setDefaultState();
    void setToQueue(int queueIndex);

    // Update outdated url
    void updateUrl();
    void urlUpdated(bool s = false);

    // Track actions
    void download();
    void add2Library();

    // Return item for QStandartItemModel
    QList<QStandardItem *> mitem();

    /* Get track fields */
    QString artist();
    QString metaArtist() { return m_sMetaArtist; }
    QString title();
    QString metaTitle() { return m_sMetaTitle; }
    QString album() { return m_sAlbum; }
    QString metaAlbum() { return m_sMetaAlbum; }
    QString url();
    QString duration();
    int lenght();
    QString aid();
    QString oid();
    QString hash();
    QString delHash();
    QString bitrate();
    int rawBitrate() { return m_iBitrate; }
    bool metaLoaded() { return m_bMetaLoaded; }

    int index() { return m_nIndex; }

    void getMeta();

    void setAvailable(bool s);

    bool isLast() { return m_bLast; }
    void setLast(bool state) { m_bLast = state; }

    void showLoading();
    void hideLoading();
private:
    QList<QStandardItem *> m_iTrack;
    QStandardItem *m_Model;

    /* Track fields */
    QString m_sArtist;
    QString m_sMetaArtist;
    QString m_sTitle;
    QString m_sMetaTitle;
    QString m_sAlbum;
    QString m_sMetaAlbum;
    QString m_sUrl;
    QString m_sDuration;
    QString m_sAid;
    QString m_sOid;
    QString m_sHash;
    QString m_sDelHash;
    int m_iBitrate;
    int m_iLenght;

    // Track index in list
    int m_nIndex;

    // Meta extracting enabled/disabled
    bool m_bMetaEnabled;

    // Is meta data loaded currently
    bool m_bMetaLoaded;

    // If 'True' - using meta data for artist/title instead of parser data
    bool m_bUseMeta;

    // Phonon media object for extracting meta data
    //Phonon::MediaObject *m_MediaObject;

    // Animate gif loading
    QMovie *m_LoadingMov;

    // This is last played track
    bool m_bLast;

    void update();

Q_SIGNALS:
    void playState();
    void updated(bool);
    void updateMe();
    void updateMyMeta();
    void metaUpdated();
    void removeMe();

private Q_SLOTS:
    void updateMeta();
    void updateAnim(int);

public Q_SLOTS:
    void setMetaEnabled(bool);
    void setMetaLoaded(bool);
    void setUseMeta(bool);

    void metaTagsChanged(bool, bool);

};

#endif // TRACK_H
