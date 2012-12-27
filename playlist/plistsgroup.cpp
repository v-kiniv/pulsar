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

#include "plistsgroup.h"
#include <QDebug>
#include <QSettings>
#include <QTimer>
#include <QInputDialog>

#include <playlist/player.h>

PlistsGroup *PlistsGroup::m_instance = 0;

PlistsGroup::PlistsGroup(QObject *parent) :
    QObject(parent)
{
    if(m_instance)
            qFatal("Only one instance of PlistsGroup object is allowed");

    m_instance = this;

    m_Settings = Settings::instance();

    m_Auth = Auth::instance();

    // Create new widget for playlist
    m_widget = new PlistsGroupWidget();

    // init vars
    m_LastList = 0;
    m_LastTrack = 0;

    m_bLoadMeta = false;
    m_bUseMeta = false;

    m_shuffleMode = m_Settings->getValue("player/shuffle").toInt();
    m_repeatMode  = m_Settings->getValue("player/repeat").toInt();

    connect(m_widget, SIGNAL(createSearchList()), SLOT(createSearchList()));
    connect(m_widget, SIGNAL(createLibraryList()), SLOT(createLibraryList()));
    connect(m_widget, SIGNAL(createLocalList()), SLOT(createLocalList()));
    connect(m_widget, SIGNAL(createSuggestionsList()), SLOT(createSuggestionsList()));
    connect(m_widget, SIGNAL(createDbSearch()), SLOT(createDbSearch()));

    connect(m_widget, SIGNAL(tabMoved(int,int)), SLOT(listSwap(int,int)));
    connect(m_widget, SIGNAL(removeTab(int)), SLOT(deleteList(int)));
    connect(m_widget, SIGNAL(renameTab(int, QString)), SLOT(renameList(int, QString)));

    loadLists();

    connect(this, SIGNAL(playLast()), m_LastList, SLOT(playLast()));

    if(m_Settings->getValue("playing/play_on_start").toBool() && m_LastList)
        QTimer::singleShot(200, m_LastList, SLOT(playLast()));
}

PlistsGroup *PlistsGroup::instance()
{
    return m_instance;
}

void PlistsGroup::onExit()
{
    Q_EMIT appExit();
    savePlaylists();
}

void PlistsGroup::createList(PlaylistWidget::ListTypes listType, QString title, quint64 hash, bool bManually, bool bLast)
{
    QString tabIconFile;
    QString type;
    switch(listType) {
        case PlaylistWidget::Search:
            type = tr("Search");
            tabIconFile = ":/icons/tab_search";
        break;
        case PlaylistWidget::AudioLib:
            type = tr("Library");
            tabIconFile = ":/icons/tab_library";
        break;
        case PlaylistWidget::LocalList:
            type = tr("Local");
            tabIconFile = ":/icons/tab_local";
        break;
        case PlaylistWidget::Suggestions:
            type = tr("Suggestions");
            tabIconFile = ":/icons/tab_suggestions";
        break;
        case PlaylistWidget::DbSearch:
            type = tr("Discography");
            tabIconFile = ":/icons/tab_db";
        break;
    }

    if(title == "")
        m_ListsNames << type;
    else
        m_ListsNames << title;

    if(title == "") {
        if(m_ListsNames.count(type) > 1)
            title = QString("%0 (%1)").arg(type).arg(m_ListsNames.count(type));
        else
            title = type;
    } else {
        if(m_ListsNames.count(title) > 1)
            title = QString("%0 (%1)").arg(title).arg(m_ListsNames.count(title));
    }

    m_playLists.insert(m_playLists.end(), new Playlist(listType, title, hash));

    m_widget->addList(m_playLists.last(), bManually);

    m_widget->setTabIcon(m_playLists.size()-1, QIcon(QPixmap(tabIconFile)));

    connect(m_playLists.last(), SIGNAL(message(QString,QString)), SIGNAL(message(QString,QString)));
    connect(m_playLists.last(), SIGNAL(playTrack(Track*)), SIGNAL(playTrack(Track*)));
    connect(m_playLists.last(), SIGNAL(playTrack(Track*)), SLOT(trackActivate(Track*)));
    connect(m_playLists.last(), SIGNAL(addQueue(Track*)), SLOT(addToQueue(Track*)));
    connect(m_playLists.last(), SIGNAL(active()), SLOT(setActiveList()));
    connect(m_playLists.last(), SIGNAL(titleChanged(QString)), SLOT(setPlaylistTitle(QString)));
    connect(this, SIGNAL(shuffleChanged(bool)), m_playLists.last(), SLOT(setShuffle(bool)));
    connect(this, SIGNAL(repeatChanged(bool)), m_playLists.last(), SLOT(setRepeat(bool)));
    connect(this, SIGNAL(metaTagsChanged(bool,bool)), m_playLists.last(), SIGNAL(metaTagsChanged(bool,bool)));
    connect(this, SIGNAL(metaTagsChanged(bool,bool)), m_playLists.last(), SLOT(setMetaTagsChanged(bool,bool)));
    connect(this, SIGNAL(appExit()), m_playLists.last(), SLOT(onExit()));

    m_playLists.last()->setMetaTagsChanged(m_bLoadMeta, m_bUseMeta);

    if(m_repeatMode > 2)
        m_playLists.last()->setRepeat(true);
    else
        m_playLists.last()->setRepeat(false);

    if(bLast)
        m_LastList = m_playLists.last();


    updateLocalLists();
}

int PlistsGroup::shuffleMode()
{
    return m_shuffleMode;
}

int PlistsGroup::repeatMode()
{
    return m_repeatMode;
}

void PlistsGroup::loadLists()
{
    QString fName = m_Settings->appPath();
    fName += "playlists";

    QSettings sfile(fName, QSettings::IniFormat);

    sfile.beginGroup("Options");
    int lastIndex = sfile.value("last_list").toInt();
    sfile.endGroup();

    int size = sfile.beginReadArray("Lists");
    if(size > 0) {
        for(int i = 0; i < size; i++) {
            sfile.setArrayIndex(i);

            int type = sfile.value("type").toInt();

            PlaylistWidget::ListTypes eType;
            switch(type) {
            case 1:
                eType = PlaylistWidget::Search;
            break;
            case 2:
                eType = PlaylistWidget::AudioLib;
            break;
            case 3:
                eType = PlaylistWidget::LocalList;
            break;
            case 4:
                eType = PlaylistWidget::Suggestions;
            break;
            case 5:
                eType = PlaylistWidget::DbSearch;
            break;
            }

            QString title = sfile.value("title").toString();
            bool isTitleCustom = sfile.value("title_custom").toBool();
            quint64 hash = sfile.value("hash").toULongLong();
            bool isLast = i == lastIndex ? true : false;

            createList(eType, title, hash, false, isLast);
            m_playLists.last()->setTitleCustom(isTitleCustom);
        }
    } else {
        createList(PlaylistWidget::Search, tr("Search"));
    }
    sfile.endArray();

    // Activate last played list tab
    m_widget->setActiveTab(lastIndex);
}

void PlistsGroup::savePlaylists()
{
    QString fName = m_Settings->appPath();
    fName += "playlists";

    QSettings sfile(fName, QSettings::IniFormat);
    sfile.clear();

    sfile.beginGroup("Options");
    sfile.setValue("last_list", m_playLists.indexOf(m_LastList));
    sfile.endGroup();

    sfile.beginWriteArray("Lists");
    for(int i = 0; i < m_playLists.size(); i++) {
        sfile.setArrayIndex(i);

        //PlaylistWidget::ListTypes type = m_playLists.at(i)->type();
        int iType = 0;
        switch(m_playLists.at(i)->type()) {
        case PlaylistWidget::Search:
            iType = 1;
        break;
        case PlaylistWidget::AudioLib:
            iType = 2;
        break;
        case PlaylistWidget::LocalList:
            iType = 3;
        break;
        case PlaylistWidget::Suggestions:
            iType = 4;
        break;
        case PlaylistWidget::DbSearch:
            iType = 5;
        break;
        }

        sfile.setValue("type", iType);
        sfile.setValue("title", m_playLists.at(i)->listTitle());
        sfile.setValue("title_custom", m_playLists.at(i)->isTitleCustom());
        sfile.setValue("hash", QString::number(m_playLists.at(i)->hash()));
    }
    sfile.endArray();
}

void PlistsGroup::updateLocalLists()
{
    m_localLists.clear();
    QStringList lList;

    for(int i = 0; i < m_playLists.size(); i++) {
        // Create action, if list is local
        if(m_playLists.at(i)->type() == PlaylistWidget::LocalList) {
            m_localLists.append(m_playLists.at(i));
            lList << m_playLists.at(i)->listTitle();
        }
    }

    for(int i = 0; i < m_playLists.size(); i++) {
        if(m_playLists.at(i)->type() != PlaylistWidget::LocalList)
            m_playLists.at(i)->setLocalLists(m_localLists);
    }

    Q_EMIT localsChanged(lList);
}

void PlistsGroup::createSearchList() { createList(PlaylistWidget::Search, "", 0, true); savePlaylists(); }

void PlistsGroup::createLibraryList() { createList(PlaylistWidget::AudioLib, "", 0, true); savePlaylists(); }

void PlistsGroup::createLocalList() { createList(PlaylistWidget::LocalList, "", 0, true); savePlaylists(); }

void PlistsGroup::createSuggestionsList() { createList(PlaylistWidget::Suggestions, "", 0, true); savePlaylists(); }

void PlistsGroup::createDbSearch() { createList(PlaylistWidget::DbSearch, "", 0, true); savePlaylists(); }

void PlistsGroup::listSwap(int newIndex, int oldIndex) {
    m_playLists.swap(oldIndex, newIndex);

    savePlaylists();
}

void PlistsGroup::deleteList(int index)
{
    m_playLists.takeAt(index)->remove();
    savePlaylists();
    updateLocalLists();
}

void PlistsGroup::renameList(int index, QString newName)
{
    //m_widget->setTabTitle(index, newName);

    m_playLists.at(index)->setCustomTitle(newName);

    savePlaylists();
    updateLocalLists();
}

void PlistsGroup::trackActivate(Track *p)
{
    p->setLast(true);

    if(m_LastTrack)
        m_LastTrack->setLast(false);

    m_LastList = (Playlist *)p->parent();

    // Add track to prev history
    if(m_LastTrack) {
        m_TracksHistory.push(m_LastTrack);

        // Keep fixed size of tracks history
        if(m_TracksHistory.size() > 20)
            m_TracksHistory.pop_front();
    }

    if(m_LastTrack != p)
        m_LastTrack = p;
}

void PlistsGroup::nextTrack()
{
    // If queue list is not empty, take track from queue
    if(m_TracksQueue.isEmpty()) {
        if(m_LastList) {
            if(m_repeatMode == 2 && m_LastTrack) {
                // Emit play action to player object
                Q_EMIT playTrack(m_LastTrack);

                // Simulate track activate for set last list pointer
                trackActivate(m_LastTrack);
            } else {
                if(m_shuffleMode == 3)
                    m_playLists.at(qrand() % m_playLists.size())->nextTrack();
                else
                    m_LastList->nextTrack();
            }
        } else {
            m_playLists.first()->nextTrack();
        }

    } else {
        Track *p = m_TracksQueue.dequeue();

        // Emit play action to player object
        Q_EMIT playTrack(p);

        // Simulate track activate for set last list pointer
        trackActivate(p);

        // Update numbers on queued tracks
        for(int i = 0; i < m_TracksQueue.size(); i++) {
            m_TracksQueue.at(i)->setToQueue(i+1);
        }
    }
}

void PlistsGroup::prevTrack()
{
    if(!m_TracksHistory.isEmpty()) {
        Track *p = m_TracksHistory.pop();
        m_LastList = (Playlist *)p->parent();
        Q_EMIT playTrack(p);
    }
}

void PlistsGroup::setShuffle(int mode)
{
    m_shuffleMode = mode;

    if(m_shuffleMode > 1)
        Q_EMIT shuffleChanged(true);
    else
        Q_EMIT shuffleChanged(false);

    if(!sender()) {
        Q_EMIT shuffleModeChanged(mode);
    }

    // Save value
    m_Settings->setValue("player/shuffle", mode);
}

void PlistsGroup::setRepeat(int mode)
{
    m_repeatMode = mode;

    if(m_repeatMode == 3)
        Q_EMIT repeatChanged(true);
    else
        Q_EMIT repeatChanged(false);

    if(!sender())
        Q_EMIT repeatModeChanged(mode);

    // Save value
    m_Settings->setValue("player/repeat", mode);
}

void PlistsGroup::toggleShuffle()
{
    setShuffle(shuffleMode() > 1 ? 1 : 2);
}

void PlistsGroup::toggleRepeat()
{
    setRepeat(repeatMode() > 1 ? 1 : 2);
}

void PlistsGroup::settingsChanged()
{
    // Meta
    m_bLoadMeta = m_Settings->getValue("playlist/load_meta").toBool();
    m_bUseMeta = m_Settings->getValue("playlist/use_meta").toBool();

    Q_EMIT metaTagsChanged(m_bLoadMeta, m_bUseMeta);
}

void PlistsGroup::addCurrentToLocal()
{
    if(Player::instance()->currentTrack()) {
        // Action(sender) pinter
        QAction *pA = (QAction *)sender();

        // Copy track
        Track *newTrack = Player::instance()->currentTrack()->copy();

        // Add track to list
        m_localLists.at(pA->data().toInt())->addTrack(newTrack);
    }
}

void PlistsGroup::addToQueue(Track *p)
{
    m_TracksQueue.enqueue(p);
    p->setToQueue(m_TracksQueue.size());
}

void PlistsGroup::setActiveList()
{
    Playlist *p = (Playlist*)sender();

    m_widget->setActiveTab(m_playLists.indexOf(p));
}

void PlistsGroup::setPlaylistTitle(QString str)
{
    Playlist *p = (Playlist*)sender();

    m_widget->setTabTitle(m_playLists.indexOf(p), str);

    savePlaylists();
}
