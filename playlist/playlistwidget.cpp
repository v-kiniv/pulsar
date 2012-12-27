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

#include "playlistwidget.h"
#include <QResizeEvent>
#include "QVBoxLayout"
#include "QHBoxLayout"
#include "QLabel"
#include <QDebug>
#include <QScrollBar>
#include <QTimer>

Q_DECLARE_METATYPE(Track*)

PlaylistWidget::PlaylistWidget(PlaylistWidget::ListTypes listType, QWidget *parent) :
    QWidget(parent)
{
    m_type = listType;

    QHBoxLayout *topLayout = new QHBoxLayout();
    QVBoxLayout *middleLayout = new QVBoxLayout(this);

    // Setup list
    m_treeView = new QTreeView(this);
    m_treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_treeView->setRootIsDecorated(false);
    m_treeView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_treeView->setSortingEnabled(true);
    m_treeView->setStyleSheet("::item {min-height: 16px; padding: 4px 0 4px 0;}"); //QTreeView{backgrou1nd-image: url(:/images/bkg_light);}
    m_treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);


    QPalette pal = m_treeView->viewport()->palette();
    pal.setColor(QPalette::Base, Qt::transparent);
    m_treeView->viewport()->setPalette(pal);


    // Install event filter to list
    m_treeView->installEventFilter(this);

    m_treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_treeView->setSortingEnabled(false);
    m_treeView->setAlternatingRowColors(true);

    // Set label with instructions, if list is empty
    m_instLbl = new QLabel(m_treeView);
    m_instLbl->setAlignment(Qt::AlignCenter);

    // Setup loading label
    m_wlLoading = new QLabel("LOADING", m_treeView);
    m_wlLoading->setMinimumWidth(160);

    // Set loading qmovie
    m_LoadingMov = new QMovie(":/icons/loading_dark");
    m_wlLoading->setMovie(m_LoadingMov);
    m_wlLoading->hide();

    // In list search lineedit
    m_weListSearch = new QLineEdit(this);
    m_weListSearch->setMaximumWidth(50);
    m_weListSearch->installEventFilter(this);

    connect(m_weListSearch, SIGNAL(textChanged(QString)), SIGNAL(listSearchChanged(QString)));



    QString instLblText;

    /* Top list controls */
    switch(m_type) {
        case Search:
            m_weSearch = new QLineEdit(this);
            m_weSearch->installEventFilter(this);
            connect(m_weSearch, SIGNAL(returnPressed()), SLOT(searchActivated()));

            topLayout->addWidget(m_weSearch);

            m_Hints = new Hints(this);
            connect(this, SIGNAL(searchResized(QRect)), m_Hints, SLOT(parentResized(QRect)));
            connect(this, SIGNAL(focus2Hints()), m_Hints, SLOT(getFocus()));
            connect(this, SIGNAL(hideHints()), m_Hints, SLOT(hide()));
            connect(m_weSearch, SIGNAL(textChanged(QString)), m_Hints, SLOT(textChanged(QString)));
            connect(m_Hints, SIGNAL(selected(QString)), SLOT(hintSelected(QString)));

            instLblText = tr("Search playlist is empty<br><b>Use the search bar above to find music</b>");
        break;
        case AudioLib:
            // Setup button for choose friend library
            m_wbFriends = new QPushButton(QIcon(QPixmap(":/icons/user_small")), tr("Choose library"), this);
            m_wbFriends->setStyleSheet("QPushButton {padding: 3px 0px 3px 10px; text-align: left; }");
            topLayout->addWidget(m_wbFriends);

            // Rehresh list button
            m_wbRefresh = new QPushButton(this);
            m_wbRefresh->setIcon(QIcon(QPixmap(":/icons/refresh")));
            m_wbRefresh->setFlat(true);
            m_wbRefresh->setMaximumWidth(35);
            m_wbRefresh->setMaximumHeight(m_wbFriends->height()-2);
            connect(m_wbRefresh, SIGNAL(clicked()), SIGNAL(refresh()));
            topLayout->addWidget(m_wbRefresh);
            // Stretch button to full width
            topLayout->setStretch(0,1);

            instLblText = tr("Audio library playlist is empty<br><b>Above, you can choose your friend's library or ID of group or user.</b>");

            m_FriensList = new FriendsList(this);
            m_FriensList->widget()->setParent(m_treeView);
            connect(m_wbFriends, SIGNAL(clicked()), m_FriensList->widget(), SLOT(show()));
            connect(m_wbFriends, SIGNAL(clicked()), m_instLbl, SLOT(hide()));
        break;
        case LocalList:
            instLblText = tr("Local playlist is empty<br><b>Add tracks here by selecting 'Add to Local List' in other lists.</b>");
        break;
        case Suggestions:
            // Rehresh list button
            m_wbRefresh = new QPushButton(tr("Refresh"), this);
            m_wbRefresh->setIcon(QIcon(QPixmap(":/icons/refresh")));
            m_wbRefresh->setFlat(true);

            topLayout->addWidget(m_wbRefresh);

            // Stretch button to full width
            topLayout->setStretch(0,1);

            connect(m_wbRefresh, SIGNAL(clicked()), SIGNAL(refresh()));

            instLblText = tr("Suggestions playlist is empty<br><b>Click 'Refresh' button to update suggestions list for you.</b>");
        break;
        case DbSearch:
            m_weSearch = new QLineEdit(this);
            connect(m_weSearch, SIGNAL(returnPressed()), SLOT(searchActivated()));


            topLayout->addWidget(m_weSearch);

            instLblText = tr("Discography playlist is empty<br>\
                             <b>Enter the name of the artist, then select an album from the list.</b><br>\
                             Note: not all tracks from discography available for listening.<br><br>\
                             Music database provided by MusicBrainz.\
                             ");

            // MusicDB
            m_MusicDb = new MusicDB(this);
            m_MusicDb->widget()->setParent(m_treeView);
            connect(m_MusicDb, SIGNAL(newTrack(Track*)), SIGNAL(newTrack(Track*)));
            connect(m_MusicDb, SIGNAL(clearList()), SIGNAL(clearList()));

        break;
    }
    // List Search
    topLayout->addWidget(m_weListSearch);
    topLayout->setAlignment(m_weListSearch, Qt::AlignRight);

    middleLayout->addLayout(topLayout);

    // Set instruction text
    m_instLbl->setText(instLblText);


    // Add list to layout
    middleLayout->addWidget(m_treeView);

    // Item activated
    connect(m_treeView, SIGNAL(doubleClicked(QModelIndex)), SLOT(itemActivated(QModelIndex)));
    //connect(m_treeView, SIGNAL(pressed(QModelIndex)), SLOT(itemActivated(QModelIndex)));

    // Popup menu
    connect(m_treeView, SIGNAL(customContextMenuRequested(const QPoint)), SLOT(listMenu(const QPoint)));

    // Connections for initialize loading more results, when scrolled to end of list
    connect(m_treeView->verticalScrollBar(), SIGNAL(rangeChanged(int,int)), SLOT(scrollRange(int,int)));
    connect(m_treeView->verticalScrollBar(), SIGNAL(valueChanged(int)), SLOT(scrollChanged(int)));

    //m_instLbl->hide();
}

void PlaylistWidget::setLibraryName(QString name)
{
    m_wbFriends->setText(name);
}

void PlaylistWidget::setSearchStr(QString str)
{
    m_weSearch->blockSignals(true);
    m_weSearch->setText(str);
    m_weSearch->blockSignals(false);
}

void PlaylistWidget::setActiveIndex(const QModelIndex &index)
{
    m_treeView->scrollTo(index);
}

void PlaylistWidget::focusOnSearch()
{
    QTimer::singleShot(20, m_weSearch, SLOT(setFocus()));
}

void PlaylistWidget::scrollRange(int, int max) { m_iScrollMax = max; }

void PlaylistWidget::scrollChanged(int value)
{
    //qDebug() << value;
//    if(value >= m_iScrollMax - 20 && m_iScrollPrevMax != m_iScrollMax - 20) {
//        m_iScrollPrevMax = m_iScrollMax - 20;
//        Q_EMIT loadMore();
//    }
    if(value >= m_iScrollMax - 1 ) {
        Q_EMIT loadMore();
    }
}

void PlaylistWidget::setModel(QSortFilterProxyModel *model)
{
    m_treeView->setModel(model);
    connect(model, SIGNAL(headerDataChanged(Qt::Orientation,int,int)), SLOT(updateColumns()));
}

bool PlaylistWidget::eventFilter(QObject *target, QEvent *event)
{
    // List Widget filter
    if(target == m_treeView) {
        if(event->type() == QEvent::Resize) {
            QResizeEvent *resizeEvent = (QResizeEvent *)event;
            listResize(resizeEvent->size());
        }
        if(event->type() == QEvent::Show) {
            setWidgetPos();
        }

        if(event->type() == QEvent::FocusIn) {
            Q_EMIT hideHints();
        }

        // Key events
        if(event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = (QKeyEvent *)event;
            quint32 kCode = keyEvent->nativeScanCode();

            switch(kCode) {
            case QKey: // Q
                Q_EMIT skQueue();
                break;

            case DelKey: // Del
                Q_EMIT skRemove();
                break;

            case DKey: // D
                Q_EMIT skDownload();
                break;

            case EnterKey: // Enter
                Q_EMIT itemActivated(m_treeView->currentIndex());
                break;
            }
        }

    }

    // Search widgets filter
    if(m_type == PlaylistWidget::Search) {
        if(target == m_weSearch) {
            if(event->type() == QEvent::Resize || event->type() == QEvent::Show) {
                Q_EMIT searchResized(m_weSearch->geometry());
            }

            if(event->type() == QEvent::KeyPress) {
                QKeyEvent *keyEvent = (QKeyEvent *)event;
                if(keyEvent->key() == Qt::Key_Down) {
                    Q_EMIT focus2Hints();
                }
            }
        }
    }

    // ListSearch widget filter
    if(target == m_weListSearch) {
        if(event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = (QKeyEvent *)event;
            quint32 kCode = keyEvent->nativeScanCode();
            m_treeView->setCurrentIndex(m_treeView->model()->index(0,0));
            if(keyEvent->key() == Qt::Key_Down || keyEvent->key() == Qt::Key_Up) {
                m_treeView->setFocus();
                m_treeView->setCurrentIndex(m_treeView->model()->index(1,0));
            }

            if(kCode == EnterKey) {
                Q_EMIT itemActivated(m_treeView->model()->index(0,0));
            }
        }
    }
    return false;
}

void PlaylistWidget::listResize(QSize size)
{
    m_treeView->setColumnWidth(2, size.width() - 178);

    m_instLbl->move(m_treeView->rect().center() - m_instLbl->rect().center());
    m_wlLoading->move(m_treeView->rect().center() - m_wlLoading->rect().center());

    if(m_type == PlaylistWidget::AudioLib) {
        m_FriensList->widget()->setGeometry(0, 0, m_treeView->rect().width() - 50, m_treeView->height() - 100);
        m_FriensList->widget()->move(m_treeView->rect().center() - m_FriensList->widget()->rect().center());
    }

    if(m_type == PlaylistWidget::DbSearch) {
        m_MusicDb->widget()->setGeometry(0, 0, m_treeView->rect().width() - 50, m_treeView->height() - 100);
        m_MusicDb->widget()->move(m_treeView->rect().center() - m_MusicDb->widget()->rect().center());
    }
}

void PlaylistWidget::setWidgetPos()
{
    m_instLbl->move(m_treeView->rect().center() - m_instLbl->rect().center());

    if(m_type == PlaylistWidget::AudioLib)
        m_FriensList->widget()->move(m_treeView->rect().center() - m_FriensList->widget()->rect().center());
}

void PlaylistWidget::searchActivated()
{
    if(m_type == Search) {
        m_Hints->hide();
        Q_EMIT searchChanged(m_weSearch->text());
    }

    if(m_type == DbSearch) {
        m_MusicDb->getArtists(m_weSearch->text());
        Q_EMIT searchChanged(m_weSearch->text());
    }
}

void PlaylistWidget::updateColumns()
{
    m_treeView->setColumnWidth(0, 20);
    m_treeView->setColumnWidth(1, 40);
    m_treeView->setColumnWidth(2, m_treeView->width() - 170);
    m_treeView->setColumnWidth(3, 50);
//    m_treeView->setColumnWidth(4, 30);
}

void PlaylistWidget::itemActivated(QModelIndex index)
{
    if(index.isValid()) {
        Track* p = index.data(Qt::UserRole + 1).value<Track*>();

        m_weListSearch->clear();
        Q_EMIT trackActivate(p);
    }

}

void PlaylistWidget::listMenu(const QPoint point)
{
    m_Menu->exec(m_treeView->mapToGlobal(point));
}

void PlaylistWidget::hintSelected(QString str)
{
    setSearchStr(str);
    Q_EMIT searchChanged(str);
}

void PlaylistWidget::trackAdded()
{
    if(!m_instLbl->isHidden())
        m_instLbl->hide();
}

void PlaylistWidget::showLoading()
{
    m_LoadingMov->start();
    m_wlLoading->show();

    if(!m_instLbl->isHidden())
        m_instLbl->hide();
}

void PlaylistWidget::hideLoading()
{
    m_LoadingMov->stop();
    m_wlLoading->hide();
}
