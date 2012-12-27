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

#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <playlist/track.h>
#include <playlist/playlistwidget.h>
#include "network/parser.h"
#include "settings/settings.h"
#include "network/vkactions.h"
#include "settings/settings.h"

#include <QObject>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QMenu>
#include <QAction>
#include <QStack>

class Playlist : public QObject
{
    Q_OBJECT
public:
    explicit Playlist(PlaylistWidget::ListTypes listType, QString title, quint64 hash = 0,  QObject *parent = 0);

    ~Playlist();

    Parser *parser() { return m_Parser; }
    VkActions *vkActions() { return m_vkActions; }
    FriendsList *friendsList();

    // Returns playlist title
    QString listTitle() { return m_sTitle; }

    PlaylistWidget::ListTypes type() { return m_listType; }

    bool isEmpty();
    int size();

    // Return playlistwidget
    PlaylistWidget *listWidget() { return m_listWidget; }

    // Remove playlist
    void remove();

    // Rerurn hash
    quint64 hash() { return m_Hash; }

    // Set local lists menu
    void setLocalLists(QList<Playlist *> lists);

    // Title by content
    void setTitleByContent(bool state) { m_bTitleByContent = state; }

    // Set custom title
    void setCustomTitle(QString);

    bool isTitleCustom() { return m_bCustomTitle; }
    void setTitleCustom(bool b) { m_bCustomTitle = b; }


private:
    Settings *m_Settings;

    Auth *m_Auth;

    // Type of playlist(Search, My Audio, My List)
    PlaylistWidget::ListTypes m_listType;

    // If playlist created in current session
    bool m_bNewly;

    // Playlist title
    QString m_sTitle;
    bool m_bCustomTitle;

    // Library ID
    QString m_LibraryId;
    QString m_LibraryName;

    /* List hash
      Uses as a filename for saving/loading playlist */
    qint64 m_Hash;

    // List of track objects
    QList <Track *> m_tList;

    // Prev tracks history
    QStack<Track *> m_TracksHistory;

    // Model for playlistwidget
    QStandardItemModel *m_Model;

    // Search proxy model
    QSortFilterProxyModel *m_ProxyModel;

    // List widget
    PlaylistWidget *m_listWidget;

    QMenu *m_listMenu;
    QMenu *m_localsMenu;
    QAction *m_toLibraryA;
    QList<Playlist *> m_localLists;

    // Parser
    Parser *m_Parser;

    // Actions parser
    VkActions *m_vkActions;

    // Search text
    QString m_sSearch;

    // Last played track
    Track *m_LastTrack;

    // Loading state
    // True if playlist loading from file at this time
    bool m_bLoadingState;

    // Repeat mode
    bool m_bRepeatList;

    // Shuffle mode
    bool m_bShuffleList;

    // Meta
    bool m_bLoadMeta;
    bool m_bUseMeta;

    // Set title of playlist by content
    bool m_bTitleByContent;

    // Create popup menu for track
    void createMenus();

    // Set models headers
    void setModelHeaders();

    void load();//Q_EMIT addQueue(pT);
    // IMPLEMENT DTACK

    void setLibId(QString lbId);

    // Change title by content
    void setContentTitle(QString);

    // Remove track
    void removeTrack(QList<Track*> plist);

Q_SIGNALS:
    void trackAdded();
    void message(QString, QString);

    void playTrack(Track *);

    void addQueue(Track *);

    void active();

    void metaTagsChanged(bool, bool);

    void titleChanged(QString);

private Q_SLOTS:
    void save();

    void searchChanged(QString);
    void refresh();

    void parserBusy();
    void parserFree();

    void librarySelected(QString, QString);
    void update();

    void removeTrack();
    void addToLocal();
    void addToLibrary();
    void addToQueue();
    void downloadTrack();

    // Activate track, from playlist widget
    void trackActivate(Track *);

    // Track emmitted that he was active
    void trackSelfActivate();

    void setLastTrack();

    void clearList();

    void setListSearch(QString);

    void onTrackSelfRemove();

    void onSettingsChanged();

public Q_SLOTS:
    // Add track obj to playlist
    void addTrack(Track *track);

    void nextTrack();

    void setShuffle(bool);
    void setRepeat(bool);

    void setMetaTagsChanged(bool, bool);

    // Play last track
    void playLast();

    void onExit();

};

#endif // PLAYLIST_H
