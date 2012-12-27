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

#ifndef MUSICDB_H
#define MUSICDB_H

#include <playlist/track.h>
#include <playlist/musicdbwidget.h>
#include <QObject>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include <QStandardItemModel>
#include <QStandardItem>
#include <QSortFilterProxyModel>

#include <QStringList>
#include <QQueue>

class MusicDB : public QObject
{
    Q_OBJECT
public:
    explicit MusicDB(QObject *parent = 0);

    MusicDbWidget *widget() { return m_Widget; }

    void getArtists(QString strArtist);

private:
    QNetworkAccessManager *m_nManager;
    QNetworkRequest m_nRequest;

    QStringList m_lReleasesStrings;
    QQueue<QString> m_lAllReleases;

    QStandardItemModel *m_ArtistsModel;
    QSortFilterProxyModel *m_ArtistsProxyModel;

    QStandardItemModel *m_ReleasesModel;
    QSortFilterProxyModel *m_ReleasesProxyModel;

    MusicDbWidget *m_Widget;

    bool m_bArtistView;
    bool m_bReleasesView;
    bool m_bAllTracks;

    QString m_sSelectedArtist;

    void getReleases(QString strArtistId);
    void getTracks(QString strReleaseId);
    void getAllTracks();

    void addArtist(QStringList entry);
    void addRelease(QStringList entry);

    QString msecs2duration(QString strLength);

Q_SIGNALS:
    // Sends hot parsed track to playlist
    void newTrack(Track *);

    void clearList();

public Q_SLOTS:

private Q_SLOTS:
    void search(QString);
    void listItemAcivated(QModelIndex);

    void artistsReply(QNetworkReply*);
    void releasesReply(QNetworkReply*);
    void tracksReply(QNetworkReply*);

};

#endif // MUSICDB_H
