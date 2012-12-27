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

#ifndef PLAYLISTWIDGET_H
#define PLAYLISTWIDGET_H

#include "playlist/friendslist.h"
#include "playlist/hints.h"
#include "playlist/track.h"
#include "playlist/musicdb.h"

#include <QWidget>
#include <QEvent>
#include <QStandardItemModel>
#include <QTreeView>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

#include <QMovie>
#include <QMenu>

class PlaylistWidget : public QWidget
{
    Q_OBJECT
public:
    enum ListType {
             Search = 0x0,
             AudioLib = 0x1,
             LocalList = 0x2,
             Suggestions = 0x4,
             DbSearch = 0x8
         };
         Q_DECLARE_FLAGS(ListTypes, ListType)

    #ifdef Q_OS_LINUX
    enum KeyCodes {
             QKey = 24,
             DKey = 40,
             DelKey = 119,
             EnterKey = 36
         };
    #endif

    #ifdef Q_OS_WIN
    enum KeyCodes {
             QKey = 16,
             DKey = 32,
             DelKey = 339,
             EnterKey = 28
         };
    #endif

    explicit PlaylistWidget(PlaylistWidget::ListTypes listType, QWidget *parent = 0);
    //explicit PlaylistWidget(QWidget *parent = 0);

    void setModel(QSortFilterProxyModel *model);
    QTreeView *treeList() { return m_treeView; }
    FriendsList *friendsList() { return m_FriensList; }
    Hints *hints() { return m_Hints; }
    void setLibraryName(QString name);
    void setSearchStr(QString str);
    void setMenu(QMenu *menu) { m_Menu = menu; }
    void setActiveIndex(const QModelIndex &index);
    void focusOnSearch();
private:
    QImage *m_bkgImage;
    // List
    QTreeView *m_treeView;
    int m_iScrollMax;
    int m_iScrollPrevMax;

    // Popup menu
    QMenu *m_Menu;

    // Instructions label
    QLabel *m_instLbl;

    // Label show animated loading gif
    QLabel *m_wlLoading;

    // Animate gif loading
    QMovie *m_LoadingMov;

    //Search field
    QLineEdit *m_weSearch;

    // Friend library widget
    QPushButton *m_wbFriends;
    FriendsList *m_FriensList;

    // MusicDB widget
    MusicDB *m_MusicDb;

    // Hints widget
    Hints *m_Hints;

    // Refresh buton
    QPushButton *m_wbRefresh;

    // In List search lineedit
    QLineEdit *m_weListSearch;

    PlaylistWidget::ListTypes m_type;

    // Event filter
    bool eventFilter(QObject *target, QEvent *event);

    // Called when list widged resized
    void listResize(QSize);
    void setWidgetPos();

Q_SIGNALS:
    // List
    void searchChanged(QString);
    void loadMore();
    void refresh();
    void trackActivate(Track *);
    void newTrack(Track *);
    void clearList();

    // Hints
    void searchResized(QRect);
    void focus2Hints();
    void hideHints();

    // Key Shortcuts
    void skQueue();
    void skDownload();
    void skRemove();

    // List search
    void listSearchChanged(QString);


private Q_SLOTS:
    // Init search when enter key pressed on search field
    void searchActivated();

    // Update columns width, when model initialized
    void updateColumns();

    // List item activated
    void itemActivated(QModelIndex);

    void listMenu(const QPoint);

    // Connections for initialize loading more results, when scrolled to end of list
    void scrollRange(int, int);
    void scrollChanged(int);

    void hintSelected(QString);

public Q_SLOTS:
    // Track added to playlist
    void trackAdded();

    void showLoading();
    void hideLoading();


};

#endif // PLAYLISTWIDGET_H
