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

#ifndef PLISTSGROUPWIDGET_H
#define PLISTSGROUPWIDGET_H

#include <QWidget>
#include <QTabWidget>
#include <QMenu>
#include "playlist/playlist.h"
#include "ui/styledbutton.h"


class TabWidget : public QTabWidget
{
    Q_OBJECT
public:
    explicit TabWidget(QWidget *parent = 0);

    QTabBar *tabBar() const;
};

class PlistsGroupWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PlistsGroupWidget(QWidget *parent = 0);

    // Add list to groupwidget
    void addList(Playlist *plist, bool bManually);

    void setTabIcon(int index, QIcon icon);

    void setActiveTab(int);

    void setTabTitle(int tabIndex, QString tabTitle);

private:
    TabWidget *m_tabWidget;
    StyledButton *addTabBtn;
    StyledButton *m_listsBtn;
    QMenu *m_listsMenu;
    bool m_bResizeTabs;

    // Event filter
    bool eventFilter(QObject *target, QEvent *event);

    // Update widgets with dynamic position
    void tabsResized();

    void createMenus();

Q_SIGNALS:
    // Send signal for remove closed list
    void removeTab(int);
    void renameTab(int, QString);
    void tabMoved(int, int);

    void createSearchList();
    void createLibraryList();
    void createLocalList();
    void createSuggestionsList();
    void createDbSearch();

private Q_SLOTS:
    void tabClosed(int index);
    void setCurrentTab();
    void onTabMoved(int, int);
    void onTabsListSelected();

public Q_SLOTS:

};

#endif // PLISTSGROUPWIDGET_H
