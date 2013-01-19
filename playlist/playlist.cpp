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

#include "playlist.h"
#include "network/downloadmanager.h"

#include <QDebug>
#include <QStandardItem>
#include <QDateTime>
#include <QSettings>
#include <QFile>
#include <QMessageBox>

Q_DECLARE_METATYPE(Track*)

Playlist::Playlist(PlaylistWidget::ListTypes listType, QString title, quint64 hash, QObject *parent) :
    QObject(parent)
{
    m_listType = listType;
    m_sTitle = title;
    m_bCustomTitle = false;

    m_Settings = Settings::instance();
    connect(m_Settings, SIGNAL(changed()), SLOT(onSettingsChanged()));

    m_bLoadingState = false;
    m_bShuffleList = m_Settings->getValue("player/shuffle").toInt() > 1 ? true : false;

    m_LastTrack = 0;

    m_bLoadMeta = false;
    m_bUseMeta = false;

    m_bTitleByContent = m_Settings->getValue("playlist/tabs_by_content").toBool();

    m_Auth = Auth::instance();
    connect(m_Auth, SIGNAL(authComplete()), SLOT(update()));


    // Set hash
    if(hash == 0) {
        QDateTime *time = new QDateTime(QDateTime::currentDateTime());
        m_Hash = time->toMSecsSinceEpoch();
        m_bNewly = true;
    } else {
        m_Hash = hash;
        m_bNewly = false;
    }

    // Create playlist widget
    m_listWidget = new PlaylistWidget(m_listType);
    connect(this, SIGNAL(trackAdded()), m_listWidget, SLOT(trackAdded()));
    connect(m_listWidget, SIGNAL(trackActivate(Track*)), SLOT(trackActivate(Track*)));

    // Connect key events
    connect(m_listWidget, SIGNAL(skQueue()), SLOT(addToQueue()));
    connect(m_listWidget, SIGNAL(skRemove()), SLOT(removeTrack()));
    connect(m_listWidget, SIGNAL(skDownload()), SLOT(downloadTrack()));

    // Create object of parser
    m_Parser = new Parser(this);
    connect(m_Parser, SIGNAL(newTrack(Track*)), SLOT(addTrack(Track*)));
    connect(m_Parser, SIGNAL(busy()), SLOT(parserBusy()));
    connect(m_Parser, SIGNAL(free()), SLOT(parserFree()));
    connect(m_Parser, SIGNAL(savePlaylist()), SLOT(save()));
    connect(m_Parser, SIGNAL(busy()), m_listWidget, SLOT(showLoading()));
    connect(m_Parser, SIGNAL(free()), m_listWidget, SLOT(hideLoading()));

    // Create actions parser
    m_vkActions = VkActions::instance();
    connect(m_vkActions, SIGNAL(message(QString,QString)), SIGNAL(message(QString,QString)));

    switch(m_listType) {
        case PlaylistWidget::Search:
            connect(m_listWidget, SIGNAL(searchChanged(QString)), SLOT(searchChanged(QString)));
            connect(m_listWidget, SIGNAL(loadMore()), m_Parser, SLOT(loadMoreResults()));
        break;
        case PlaylistWidget::AudioLib:
            connect(m_listWidget->friendsList(), SIGNAL(friendSelected(QString,QString,QString)), SLOT(librarySelected(QString,QString,QString)));
            connect(m_listWidget, SIGNAL(refresh()), SLOT(refresh()));
        break;
        case PlaylistWidget::Suggestions:
            connect(m_listWidget, SIGNAL(loadMore()), m_Parser, SLOT(loadMoreResults()));
            connect(m_listWidget, SIGNAL(refresh()), SLOT(refresh()));
        break;
        case PlaylistWidget::DbSearch:
            connect(m_listWidget, SIGNAL(searchChanged(QString)), SLOT(searchChanged(QString)));
            connect(m_listWidget, SIGNAL(newTrack(Track*)), SLOT(addTrack(Track*)));
            connect(m_listWidget, SIGNAL(clearList()), SLOT(clearList()));
        break;
    }

    m_Model = new QStandardItemModel(this);

    m_ProxyModel = new QSortFilterProxyModel(this);
    m_ProxyModel->setSourceModel(m_Model);
    m_ProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_ProxyModel->setFilterKeyColumn(2);

    m_listWidget->setModel(m_ProxyModel);

    connect(m_listWidget, SIGNAL(listSearchChanged(QString)), SLOT(setListSearch(QString)));

    createMenus();

    load();

    if(type() == PlaylistWidget::AudioLib &&
       m_bNewly &&
       m_Settings->getValue("playlist/autoload_library").toBool()
       )
        librarySelected(m_Auth->vkId(), "0", tr("My Library"));

    // Actions on newly created plalylists
    if(m_bNewly) {
        switch(m_listType) {
            case PlaylistWidget::Search:
                m_listWidget->focusOnSearch();
            break;
            case PlaylistWidget::AudioLib:
            if(m_Settings->getValue("playlist/autoload_library").toBool())
                librarySelected(m_Auth->vkId(), "0", tr("My Library"));
            break;
            case PlaylistWidget::Suggestions:
                refresh();
            break;
            case PlaylistWidget::DbSearch:
                // Unused
            break;
        }
    }
}

Playlist::~Playlist()
{

}

FriendsList *Playlist::friendsList()
{
    if(m_listType == PlaylistWidget::AudioLib)
            return m_listWidget->friendsList();
    return 0;
}

bool Playlist::isEmpty()
{
    return m_tList.isEmpty();
}

int Playlist::size()
{
    return m_tList.size();
}

void Playlist::addTrack(Track *track)
{
    track->setParent(this);
    connect(track, SIGNAL(playState()), SLOT(setLastTrack()));

    m_tList.insert(m_tList.end(), track);
    int trackIndex = m_tList.size()-1;
    track->setIndex(trackIndex);

    for(int i=0; i<5; i++) {
         m_Model->setItem(trackIndex, i, m_tList.at(trackIndex)->mitem().at(i));
    }

    connect(track, SIGNAL(playState()), SLOT(trackSelfActivate()));
    connect(track, SIGNAL(updated(bool)), SLOT(save()));
    connect(track, SIGNAL(updateMe()), m_Parser, SLOT(updateTrack()));

    Q_EMIT trackAdded();

    if(m_listType == PlaylistWidget::LocalList || m_listType == PlaylistWidget::DbSearch)
        if(!m_bLoadingState)
            save();
}

void Playlist::remove()
{
    // Remove playlist file
    QString fName = m_Settings->appPath();
    fName += QString::number(m_Hash);
    QFile(fName).remove();

    deleteLater();
}

void Playlist::setLocalLists(QList<Playlist *> lists)
{
    m_localLists.clear();
    m_localLists = lists;

    if(!m_localLists.isEmpty()) {
        m_localsMenu->clear();

        QList<QAction *> actions;

        for(int i = 0; i < m_localLists.size(); i++) {
            actions.append(new QAction(m_localLists.at(i)->listTitle(), this));
            actions.last()->setData(actions.size()-1);
            connect(actions.last(), SIGNAL(triggered()), SLOT(addToLocal()));
        }

        m_localsMenu->addActions(actions);

        m_localsMenu->setEnabled(true);
    } else {
        m_localsMenu->setDisabled(true);
    }

}

void Playlist::setCustomTitle(QString str)
{
    m_sTitle = str;
    m_bCustomTitle = true;

    // Update playlist titles
    Q_EMIT titleChanged(str);
}

void Playlist::playLast()
{
    if(m_LastTrack)
        Q_EMIT playTrack(m_LastTrack);
}

void Playlist::createMenus()
{
    m_localsMenu = new QMenu(tr("Add to list"));
    m_listMenu = new QMenu(m_listWidget);

    /* Popup Track actions */

    // Add to Queue
    QAction *toQueueA = new QAction(tr("Add to queue"), this);
    toQueueA->setShortcut(QKeySequence("Q"));
    m_listMenu->addAction(toQueueA);
    connect(toQueueA, SIGNAL(triggered()), this, SLOT(addToQueue()));


    // Add to library
    m_toLibraryA = new QAction(tr("Add to Library"), this);
    m_listMenu->addAction(m_toLibraryA);
    connect(m_toLibraryA, SIGNAL(triggered()), this, SLOT(addToLibrary()));

    // Download track
    QAction *downloadA = new QAction(tr("Download"), this);;
//    downloadA->setShortcut(QKeySequence("D"));
    m_listMenu->addAction(downloadA);
    connect(downloadA, SIGNAL(triggered()), this, SLOT(downloadTrack()));

    // Remove track
    QAction *removeA = new QAction(tr("Remove"), this);;
    removeA->setShortcut(QKeySequence::Delete);
    m_listMenu->addAction(removeA);
    connect(removeA, SIGNAL(triggered()), this, SLOT(removeTrack()));


    // Set 'add to local list' menu for all lists except local list
    if(m_listType != PlaylistWidget::LocalList)
        m_listMenu->addMenu(m_localsMenu);

    m_listWidget->setMenu(m_listMenu);
}

void Playlist::setModelHeaders()
{
    QStringList headers;
    headers << " " << "#" << tr("Artist") + " - " + tr("Track") << "B/s" << "m:s";
    m_Model->setHorizontalHeaderLabels(headers);
}

void Playlist::load()
{
    m_bLoadingState = true;
    int lastIndex = 0;

    setModelHeaders();

    QString fName = m_Settings->appPath();
    fName += QString::number(m_Hash);

    if(QFile::exists(fName)) {
        QSettings sfile(fName, QSettings::IniFormat);

        // Playlist data: library id/name, etc.
        sfile.beginGroup("Playlist");

        switch(m_listType) {
            case PlaylistWidget::Search:
                m_sSearch = sfile.value("strSearch").toString();

                m_Parser->setSearchStr(m_sSearch);
                m_Parser->setOffset(sfile.value("offset").toInt());
                m_Parser->setMore(sfile.value("more").toBool());
                m_Parser->setReqType(0);

                m_listWidget->setSearchStr(m_sSearch);
            break;
            case PlaylistWidget::AudioLib:
            setLibId(sfile.value("libraryId").toString(), "0");
                m_LibraryName = sfile.value("libraryName").toString();
                m_listWidget->setLibraryName(m_LibraryName);

                m_Parser->setReqType(1);
            break;
            case PlaylistWidget::Suggestions:
                m_Parser->setOffset(sfile.value("offset").toInt());
                m_Parser->setMore(sfile.value("more").toBool());
                m_Parser->setReqType(2);
            break;
            case PlaylistWidget::DbSearch:
                m_sSearch = sfile.value("strSearch").toString();

                m_listWidget->setSearchStr(m_sSearch);
            break;
        }
        lastIndex = sfile.value("last_track").toInt();

        sfile.endGroup();

        // Tracks data
        int size = sfile.beginReadArray("Tracks");
        for(int i = 0; i < size; i++) {
            sfile.setArrayIndex(i);

            Track *t = new Track(this);

            t->setArtist(sfile.value("artist").toString());
            t->setTitle(sfile.value("title").toString());
            t->setAlbum(sfile.value("album").toString());
            t->setUrl(sfile.value("url").toString());
            t->setDuration(sfile.value("duration").toString());
            t->setLenght(sfile.value("lenght").toInt());
            t->setAid(sfile.value("aid").toString());
            t->setOid(sfile.value("oid").toString());
            t->setHash(sfile.value("hash").toString());
            t->setDelHash(sfile.value("delHash").toString());
            t->setBitrate(sfile.value("bitrate").toInt());
            t->setMetaArtist(sfile.value("metaArtist").toString());
            t->setMetaTitle(sfile.value("metaTitle").toString());
            t->setMetaAlbum(sfile.value("metaAlbum").toString());
            t->setMetaLoaded(sfile.value("meta").toBool());

            addTrack(t);
            connect(t, SIGNAL(removeMe()), SLOT(onTrackSelfRemove()));

        }

        //fixme
        if(lastIndex < 0)
            lastIndex = 0;

        if(!m_tList.isEmpty()) {
            if(m_tList.size() > 0 && m_tList.size() >= lastIndex)
                m_LastTrack = m_tList.at(lastIndex);
            else
                m_LastTrack = m_tList.first();
        } else {
            m_LastTrack = 0;
        }

            sfile.endArray();
    }

    m_bLoadingState = false;

    m_listWidget->setActiveIndex(m_ProxyModel->index(lastIndex, 0));

}

void Playlist::save()
{
    QString fName = m_Settings->appPath();
    fName += QString::number(m_Hash);

    QSettings sfile(fName, QSettings::IniFormat);

    // Playlist data: library id/name, etc.
    sfile.beginGroup("Playlist");

    switch(m_listType) {
        case PlaylistWidget::Search:
            sfile.setValue("strSearch", m_sSearch);
            sfile.setValue("offset", m_Parser->offset());
            sfile.setValue("more", m_Parser->more());
        break;
        case PlaylistWidget::AudioLib:
            sfile.setValue("libraryId", m_LibraryId);
            sfile.setValue("libraryName", m_LibraryName);
        break;
        case PlaylistWidget::Suggestions:
            sfile.setValue("offset", m_Parser->offset());
            sfile.setValue("more", m_Parser->more());
        break;
        case PlaylistWidget::DbSearch:
            sfile.setValue("strSearch", m_sSearch);
        break;
    }

    sfile.setValue("last_track", m_tList.indexOf(m_LastTrack));
    sfile.endGroup();

    // Tracks data
    sfile.beginWriteArray("Tracks");
    for(int i = 0; i < m_tList.size(); i++) {
        sfile.setArrayIndex(i);

        Track *t = m_tList.at(i);

        sfile.setValue("artist", t->artist());
        sfile.setValue("title", t->title());
        sfile.setValue("album", t->album());
        sfile.setValue("url", t->url());
        sfile.setValue("duration", t->duration());
        sfile.setValue("lenght", t->lenght());
        sfile.setValue("aid", t->aid());
        sfile.setValue("oid", t->oid());
        sfile.setValue("hash", t->hash());
        sfile.setValue("delHash", t->delHash());
        sfile.setValue("bitrate", t->rawBitrate());
        sfile.setValue("metaArtist", t->metaArtist());
        sfile.setValue("metaTitle", t->metaTitle());
        sfile.setValue("metaAlbum", t->metaAlbum());
        sfile.setValue("meta", t->metaLoaded());
    }
    sfile.endArray();
}

void Playlist::setLibId(QString lbId, QString lbGid)
{
    m_LibraryId = lbId;
    m_LibraryGid = lbGid;

    if(m_listType == PlaylistWidget::AudioLib && lbId == m_Auth->vkId()) {
        m_toLibraryA->setDisabled(true);
    } else {
        m_toLibraryA->setDisabled(false);
    }
}

void Playlist::setContentTitle(QString strTitle)
{
    if(m_bTitleByContent && !m_bCustomTitle) {
        m_sTitle = strTitle;

        // Update playlist titles
        Q_EMIT titleChanged(strTitle);
    }
}

void Playlist::removeTrack(QList<Track *> plist)
{
    // Remove track from model and list of track pointers
    foreach(Track *p, plist) {
        m_Model->removeRow(m_tList.indexOf(p));
        p->deleteLater();
        m_tList.removeAt(m_tList.indexOf(p));
    }

    // Update indexes
    for(int i = 0; i < m_tList.size(); i++) {
        m_tList.at(i)->setIndex(i);
    }

    // Save playlist after removing track
    save();

    // If it is a 'mylibrary' show message 'delete from library too?'
    if(m_listType == PlaylistWidget::AudioLib && m_LibraryId == m_Auth->vkId() && plist.size() == 1) {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setWindowTitle(tr("Deleting track"));
        msgBox.setText(tr("Delete track from your online library too?"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        int result = msgBox.exec();

        if(result == QMessageBox::Yes)
            m_vkActions->removeFromLibrary(plist.at(0));
    }

}

void Playlist::searchChanged(QString str)
{
    m_sSearch = str;

    if(m_listType == PlaylistWidget::Search) {
        clearList();
        m_Parser->search(str);
    }

    // Update playlist titles
    setContentTitle(str);
}

void Playlist::refresh()
{
    clearList();

    if(m_listType == PlaylistWidget::AudioLib) {
        if(m_LibraryId > 0 || m_LibraryGid > 0) {
            m_Parser->library(m_LibraryId, m_LibraryGid);
        }
    }

    if(m_listType == PlaylistWidget::Suggestions) {
        m_Parser->suggestions();
    }

    setModelHeaders();
}

void Playlist::parserBusy()
{

}

void Playlist::parserFree()
{
    save();
}

void Playlist::librarySelected(QString id, QString gid, QString name)
{
    setLibId(id, gid);
    m_LibraryName = name;

    m_listWidget->setLibraryName(name);

    refresh();

    // Update playlist titles
    setContentTitle(name);
}

void Playlist::update()
{
    setLibId(m_LibraryId, m_LibraryGid);
}

void Playlist::removeTrack()
{
    //QModelIndex index = m_listWidget->treeList()->currentIndex();
    QModelIndexList indexes = m_listWidget->treeList()->selectionModel()->selectedRows();
    QList<Track*> plist;
    foreach(QModelIndex index, indexes) {
        if(index.isValid()) {

            // Get track pointer from ModelIndex
            plist << index.data(Qt::UserRole + 1).value<Track*>();
        } else {
            qDebug() << "bad index";
        }
    }
    removeTrack(plist);
}

void Playlist::addToLocal()
{
    QModelIndexList indexes = m_listWidget->treeList()->selectionModel()->selectedRows();
    foreach(QModelIndex index, indexes) {
        if(index.isValid()) {

            // Selected Track pointer
            Track *pT = index.data(Qt::UserRole + 1).value<Track*>();

            // Action(sender) pinter
            QAction *pA = (QAction *)sender();

            // Playlist pointer
            Playlist *pL = m_localLists.at(pA->data().toInt());


            // Copy track
            Track *newTrack = pT->copy(pL);

            // Add track to list
            m_localLists.at(pA->data().toInt())->addTrack(newTrack);
        }
    }
}

void Playlist::addToLibrary()
{
    //QModelIndex index = m_listWidget->treeList()->currentIndex();
    QModelIndexList indexes = m_listWidget->treeList()->selectionModel()->selectedRows();
    foreach(QModelIndex index, indexes) {
        if(index.isValid()) {

            // Selected Track pointer
            Track *pT = index.data(Qt::UserRole + 1).value<Track*>();

            m_vkActions->addToLibrary(pT);
        }
    }
}

void Playlist::addToQueue()
{
    QModelIndexList indexes = m_listWidget->treeList()->selectionModel()->selectedRows();
    foreach(QModelIndex index, indexes) {
        if(index.isValid()) {

            // Selected Track pointer
            Track *pT = index.data(Qt::UserRole + 1).value<Track*>();

            Q_EMIT addQueue(pT);
        }
    }
}

void Playlist::downloadTrack()
{
    QModelIndexList indexes = m_listWidget->treeList()->selectionModel()->selectedRows();
    foreach(QModelIndex index, indexes) {
        if(index.isValid()) {

            // Selected Track pointer
            Track *pT = index.data(Qt::UserRole + 1).value<Track*>();
            DownloadManager::instance()->add(pT);
        }
    }
}

void Playlist::trackActivate(Track *p)
{
    if(sender()) {
        m_TracksHistory.clear();
    }

    m_LastTrack = p;
    // add to history
    m_TracksHistory.push(p);

    Q_EMIT playTrack(p);
}

void Playlist::trackSelfActivate()
{
    Track *p = (Track *)sender();
    m_listWidget->setActiveIndex(m_ProxyModel->index(m_Model->indexFromItem(p->mitem().at(0)).row(), m_Model->indexFromItem(p->mitem().at(0)).column(), m_Model->indexFromItem(p->mitem().at(0))));

    Q_EMIT active();
}

void Playlist::setLastTrack()
{
    m_LastTrack = (Track *)sender();
}

void Playlist::clearList()
{
    // Delete tracks objects
    if(!m_tList.isEmpty()) {
        for(int i = 0; i < m_tList.size(); i++) {
            m_tList.at(i)->setIndex(-1);
            delete m_tList.at(i);
        }

        // Clear pointers list
        m_tList.clear();

        // Clear model
        m_Model->clear();
    }

    setModelHeaders();
}

void Playlist::setListSearch(QString str) {
    m_ProxyModel->setFilterFixedString(str);
}

void Playlist::onTrackSelfRemove()
{
    //Track *p = (Track *)sender();
    //removeTrack(p);
}

void Playlist::onSettingsChanged()
{
    m_bTitleByContent = m_Settings->getValue("playlist/tabs_by_content").toBool();
}

void Playlist::nextTrack()
{
    // Shuffle mode
    if(m_bShuffleList) {
        int trackIndex = qrand() % m_tList.size();

        if(m_TracksHistory.size() < m_tList.size()) {
            if((!m_TracksHistory.contains(m_tList.at(trackIndex))) || m_tList.size() < 2)
                Q_EMIT trackActivate(m_tList.at(trackIndex));
            else
                nextTrack();
        } else {
            if(m_bRepeatList) {
                m_TracksHistory.clear();
                nextTrack();
            }
        }

    // Normal mode
    } else {
        if(m_LastTrack) {
            if(m_LastTrack->index()+1 < m_tList.size())
                Q_EMIT trackActivate(m_tList.at(m_LastTrack->index()+1));
            else
                if(m_bRepeatList)
                    Q_EMIT trackActivate(m_tList.first());
        } else {
            if(!m_tList.isEmpty())
                Q_EMIT trackActivate(m_tList.first());
        }
    }

}

void Playlist::setShuffle(bool s)
{
    m_bShuffleList = s;
}

void Playlist::setRepeat(bool s)
{
    m_bRepeatList = s;
}

void Playlist::setMetaTagsChanged(bool load, bool use)
{
    m_bLoadMeta = load;
    m_bUseMeta = use;
}

void Playlist::onExit()
{
    save();
}
