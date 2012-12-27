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

#ifndef PLISTSGROUP_H
#define PLISTSGROUP_H

#include <playlist/playlist.h>
#include <playlist/plistsgroupwidget.h>
#include <playlist/playlistwidget.h>
#include "network/auth.h"
#include "settings/settings.h"

#include <QObject>
#include <QAction>
#include <QQueue>
#include <QStack>

class PlistsGroup : public QObject
{
    Q_OBJECT
public:
    explicit PlistsGroup(QObject *parent = 0);

    static PlistsGroup *instance();

    // Actions on app exit
    void onExit();

    // Returns pointer to playlists group widget
    PlistsGroupWidget *widget() { return m_widget; }

    // Create new playlist obj
    void createList(PlaylistWidget::ListTypes listType, QString title = "", quint64 hash = 0, bool bManually = false, bool bLast = false);

    // Return shuffle mode
    int shuffleMode();

    // Return repeat mode
    int repeatMode();

    void updateLocalLists();


private:
    static PlistsGroup *m_instance;

    Auth *m_Auth;

    Settings *m_Settings;

    // Contents playlist's objects
    QList<Playlist *> m_playLists;

    // Widget that contents playlists tabs and controls
    PlistsGroupWidget *m_widget;

    // Store number of dupicate names of lists
    QList<QString> m_ListsNames;

    // Local lists menu
    QList<Playlist *> m_localLists;

    // Last played list
    Playlist *m_LastList;

    // Last track
    Track *m_LastTrack;

    // Queue
    QQueue<Track *> m_TracksQueue;

    // Prev tracks history
    QStack<Track *> m_TracksHistory;

    // Shuffle & repeat mode
    int m_shuffleMode;
    int m_repeatMode;

    // Meta
    bool m_bLoadMeta;
    bool m_bUseMeta;


    void loadLists();
    void savePlaylists();

Q_SIGNALS:
    void message(QString, QString);

    void playTrack(Track *);

    void shuffleChanged(bool);
    void repeatChanged(bool);

    void shuffleModeChanged(int);
    void repeatModeChanged(int);

    void metaTagsChanged(bool, bool);
    void localsChanged(const QStringList);

    void appExit();

    void playLast();

public Q_SLOTS:
    void nextTrack();
    void prevTrack();

    void setShuffle(int);
    void setRepeat(int);

    void toggleShuffle();
    void toggleRepeat();

    void settingsChanged();

    void addCurrentToLocal();

private Q_SLOTS:
    void createSearchList();
    void createLibraryList();
    void createLocalList();
    void createSuggestionsList();
    void createDbSearch();

    void listSwap(int, int);
    void deleteList(int);
    void renameList(int, QString);

    void trackActivate(Track *);

    void addToQueue(Track *);

    void setActiveList();

    void setPlaylistTitle(QString);
};

#endif // PLISTSGROUP_H
