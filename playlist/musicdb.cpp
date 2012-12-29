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

#include "musicdb.h"
#include <QDebug>
#include <QtXml>

MusicDB::MusicDB(QObject *parent) :
    QObject(parent)
{
    m_Widget = new MusicDbWidget();
    connect(m_Widget, SIGNAL(searchChanged(QString)), SLOT(search(QString)));
    connect(m_Widget, SIGNAL(itemActivated(QModelIndex)), SLOT(listItemAcivated(QModelIndex)));

    m_nManager = new QNetworkAccessManager();
    m_nRequest.setRawHeader("User-Agent", "LinuxPulsar/0.9 +http://forum.ubuntu.ru/index.php?topic=168217");

    m_ArtistsModel = new QStandardItemModel(this);
    m_ReleasesModel = new QStandardItemModel(this);

    m_ArtistsProxyModel = new QSortFilterProxyModel(this);
    m_ArtistsProxyModel->setSourceModel(m_ArtistsModel);
    m_ArtistsProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    m_ReleasesProxyModel = new QSortFilterProxyModel(this);
    m_ReleasesProxyModel->setSourceModel(m_ReleasesModel);
    m_ReleasesProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

}

void MusicDB::getArtists(QString strArtist)
{
    m_bArtistView = true;
    m_bReleasesView = false;

    m_ArtistsModel->clear();
    m_Widget->showLoading();

    m_nRequest.setUrl(QUrl(QString("http://musicbrainz.org/ws/2/artist/?query=%0").arg(strArtist)));

    m_nManager->disconnect();
    connect(m_nManager, SIGNAL(finished(QNetworkReply*)), SLOT(artistsReply(QNetworkReply*)));
    m_nManager->get(m_nRequest);

    m_Widget->setModel(m_ArtistsProxyModel);

    m_Widget->show();

}

void MusicDB::getReleases(QString strArtistId)
{
    m_lReleasesStrings.clear();

    m_bArtistView = false;
    m_bReleasesView = true;

    m_ReleasesModel->clear();
    QStringList entry;
    entry << "GETALL" << tr("Get all albums");
    QStandardItem *item = new QStandardItem(entry.at(1));
    item->setData(QVariant(entry), Qt::UserRole + 1);
    m_ReleasesModel->appendRow(item);


    m_Widget->showLoading();

    m_nRequest.setUrl(QUrl(QString("http://musicbrainz.org/ws/2/release/?artist=%0").arg(strArtistId)));

    m_nManager->disconnect();
    connect(m_nManager, SIGNAL(finished(QNetworkReply*)), SLOT(releasesReply(QNetworkReply*)));
    m_nManager->get(m_nRequest);

    m_Widget->setModel(m_ReleasesProxyModel);

    //m_Widget->show();

}

void MusicDB::getTracks(QString strReleaseId)
{
    if(!m_bAllTracks)
        Q_EMIT clearList();

    m_nRequest.setUrl(QUrl(QString("http://musicbrainz.org/ws/2/recording/?release=%0").arg(strReleaseId)));

    m_nManager->disconnect();
    connect(m_nManager, SIGNAL(finished(QNetworkReply*)), SLOT(tracksReply(QNetworkReply*)));
    m_nManager->get(m_nRequest);
}

void MusicDB::getAllTracks()
{
    if(!m_lAllReleases.isEmpty())
        getTracks(m_lAllReleases.dequeue());

    //qDebug() << m_lAllReleases.dequeue();
}

void MusicDB::addArtist(QStringList entry)
{
    QStandardItem *item = new QStandardItem(entry.at(1));
    item->setData(QVariant(entry), Qt::UserRole + 1);

    m_ArtistsModel->appendRow(item);
}

void MusicDB::addRelease(QStringList entry)
{
    QStandardItem *item = new QStandardItem(entry.at(1));
    item->setData(QVariant(entry), Qt::UserRole + 1);

    m_lAllReleases.enqueue(entry.at(0));

    m_ReleasesModel->appendRow(item);
}

QString MusicDB::msecs2duration(QString strLength)
{
    QString strOutput;

    int num = strLength.toInt() / 1000;

    strOutput = QString::number(num / 60);
    strOutput.append(":");

    int secs =  num % 60;
    if(secs < 10)
        strOutput.append("0");

    strOutput.append(QString::number(secs));

    return strOutput;
}

void MusicDB::search(QString str)
{
    if(m_bArtistView)
        m_ArtistsProxyModel->setFilterFixedString(str);

    if(m_bReleasesView)
        m_ReleasesProxyModel->setFilterFixedString(str);
}

void MusicDB::listItemAcivated(QModelIndex index)
{
    if(m_bReleasesView) {
        m_Widget->hide();

        QStringList data = m_ReleasesProxyModel->data(index, Qt::UserRole + 1).toStringList();

        if(data.at(0) == "GETALL")
            m_bAllTracks = true;
        else
            m_bAllTracks = false;


        if(m_bAllTracks) {
            Q_EMIT clearList();
            getAllTracks();
        } else
            getTracks(data.at(0));
    }

    if(m_bArtistView) {
        QStringList data = m_ArtistsProxyModel->data(index, Qt::UserRole + 1).toStringList();

        m_sSelectedArtist = data.at(1);

        getReleases(data.at(0));
    }

    m_Widget->clear();
}


void MusicDB::artistsReply(QNetworkReply *reply)
{
    QString content;
    QTextCodec * codec = QTextCodec::codecForName("utf-8");
    content = codec->toUnicode(reply->readAll());

    QDomDocument domDocument;
    domDocument.setContent(content);

    QDomElement root = domDocument.documentElement();

    QDomNodeList children = root.childNodes().at(0).childNodes();

        for (int i = 0; i < children.size(); ++i)
        {
            if (children.at(i).isElement())
            {
                QDomElement childElement = children.at(i).toElement();

                QStringList entry;
                entry << childElement.attribute("id");
                entry << childElement.childNodes().at(0).toElement().text();

                addArtist(entry);

                if(children.size() == 1) {
                    m_sSelectedArtist = entry.at(1);
                    getReleases(entry.at(0));
                }
            }
        }

    m_Widget->hideLoading();

    content.clear();
    domDocument.clear();
    root.clear();
}

void MusicDB::releasesReply(QNetworkReply *reply)
{
    QString content;
    QTextCodec * codec = QTextCodec::codecForName("utf-8");
    content = codec->toUnicode(reply->readAll());


    QDomDocument domDocument;
    domDocument.setContent(content);

    QDomElement root = domDocument.documentElement();

    QDomNodeList children = root.childNodes().at(0).childNodes();

    for (int i = 0; i < children.size(); ++i)
    {
        if (children.at(i).isElement())
        {
            QDomElement childElement = children.at(i).toElement();

            QString sId = childElement.attribute("id");
            QString sName = childElement.childNodes().at(0).toElement().text();

            if(!m_lReleasesStrings.contains(sName, Qt::CaseInsensitive)) {
                m_lReleasesStrings << sName;

                QStringList entry;
                entry << sId;
                entry << sName;

                addRelease(entry);
            }
        }
    }

    content.clear();
    domDocument.clear();
    root.clear();

    m_Widget->hideLoading();
}

void MusicDB::tracksReply(QNetworkReply *reply)
{
    QString content;
    QTextCodec * codec = QTextCodec::codecForName("utf-8");
    content = codec->toUnicode(reply->readAll());


    QDomDocument domDocument;
    domDocument.setContent(content);

    QDomElement root = domDocument.documentElement();

    QDomNodeList children = root.childNodes().at(0).childNodes();

    for (int i = 0; i < children.size(); ++i)
    {
        if (children.at(i).isElement())
        {
            QDomElement childElement = children.at(i).toElement();

            QString sName = childElement.childNodes().at(0).toElement().text();
            QString sLenght = childElement.childNodes().at(1).toElement().text();

            Track *track = new Track(this);

            track->setArtist(m_sSelectedArtist);
            track->setTitle(sName);
            track->setDuration(msecs2duration(sLenght));

            Q_EMIT newTrack(track);

            // Update track url after added to playlist
            //DISABELD
            //track->updateUrl();

        }
    }

    content.clear();
    domDocument.clear();
    root.clear();

    if(m_bAllTracks)
        getAllTracks();
}
