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

#include "plistsgroupwidget.h"

#include <QVBoxLayout>
#include <QDebug>
#include <QtCore>
#include <QPushButton>
#include <QResizeEvent>
#include <QMenu>
#include <QInputDialog>

TabWidget::TabWidget(QWidget *parent) :
    QTabWidget(parent)
{

}

QTabBar* TabWidget::tabBar() const
{
    return QTabWidget::tabBar();
}

PlistsGroupWidget::PlistsGroupWidget(QWidget *parent) :
    QWidget(parent)
{
    m_bResizeTabs = false;



    m_tabWidget = new TabWidget(this);
    m_tabWidget->setTabsClosable(true);
    m_tabWidget->tabBar()->setMovable(true);
    m_tabWidget->installEventFilter(this);
    m_tabWidget->tabBar()->installEventFilter(this);
    //m_tabWidget->setStyleSheet(" background-color: #474747; ");

    addTabBtn = new StyledButton(m_tabWidget);
    addTabBtn->setTransparent(true);
    addTabBtn->setIcon(QIcon(QPixmap(":/icons/new_tab")));
    addTabBtn->setIconSize(QSize(16,16));

    m_listsBtn = new StyledButton(m_tabWidget);
    m_listsBtn->setTransparent(true);
    m_listsBtn->setIcon(QIcon(QPixmap(":/icons/dropdown_arrow")));
    m_listsBtn->setIconSize(QSize(16,16));

    m_listsMenu = new QMenu(m_listsBtn);
    m_listsBtn->setMenu(m_listsMenu);

    createMenus();

    QVBoxLayout *wLayout = new QVBoxLayout(this);
    wLayout->addWidget(m_tabWidget);

    connect(m_tabWidget, SIGNAL(tabCloseRequested(int)), SLOT(tabClosed(int)));
    connect(m_tabWidget->tabBar(), SIGNAL(tabMoved(int,int)), SIGNAL(tabMoved(int,int)));
    connect(m_tabWidget->tabBar(), SIGNAL(tabMoved(int,int)), SLOT(onTabMoved(int,int)));
}

void PlistsGroupWidget::addList(Playlist *plist, bool bManually)
{
    QString tabTitle = plist->listTitle();
    if(tabTitle.size() > 15)
        tabTitle = tabTitle.left(15)+"...";

    m_tabWidget->addTab(plist->listWidget(), tabTitle);
    tabsResized();

    if(bManually)
        QTimer::singleShot(10, this, SLOT(setCurrentTab()));

    QAction *action = new QAction(plist->listTitle(), this);
    action->setData(m_tabWidget->currentIndex());
    m_listsMenu->addAction(action);

    connect(action, SIGNAL(triggered()), SLOT(onTabsListSelected()));

}

void PlistsGroupWidget::setTabIcon(int index, QIcon icon)
{
    m_tabWidget->tabBar()->setTabIcon(index, icon);

    tabsResized();
}

void PlistsGroupWidget::setActiveTab(int index)
{
    m_tabWidget->setCurrentIndex(index);
}

void PlistsGroupWidget::setTabTitle(int tabIndex, QString tabTitle)
{
    m_tabWidget->tabBar()->setTabToolTip(tabIndex, tabTitle);

    if(tabTitle.size() > 15)
        tabTitle = tabTitle.left(15)+"...";

    m_tabWidget->tabBar()->setTabText(tabIndex, tabTitle);
}

bool PlistsGroupWidget::eventFilter(QObject *target, QEvent *event)
{
    if(target == m_tabWidget) {
        if(event->type() == QEvent::Resize || event->type() == QEvent::Show || event->type() == QEvent::LayoutRequest) {
            if(event->type() == QEvent::Show) {
                m_bResizeTabs = true;
            }
            tabsResized();
        }
    }

    if(target == m_tabWidget->tabBar()) {
        if(event->type() == QEvent::MouseButtonDblClick) {
            bool ok;
            QString newName = QInputDialog::getText(this, tr("Rename playlist"),
                                                      tr("New title:"), QLineEdit::Normal,
                                                    m_tabWidget->tabBar()->tabText(m_tabWidget->tabBar()->currentIndex()), &ok);
            m_listsMenu->actions().at(m_tabWidget->tabBar()->currentIndex())->setText(newName);
            if(ok)
                Q_EMIT renameTab(m_tabWidget->tabBar()->currentIndex(), newName);
        }
    }
    return false;
}

void PlistsGroupWidget::tabsResized()
{
    int index = m_tabWidget->count()-1;

    int tabXs = 0;
    if(m_tabWidget->currentIndex() == index)
        tabXs += 50;

    int tabX = m_tabWidget->tabBar()->tabRect(index).x();
    tabX += m_tabWidget->tabBar()->tabRect(index).width();
    tabXs += tabX;


    int tabH = m_tabWidget->tabBar()->childrenRect().height();
    if(tabH > addTabBtn->height())
        tabH = addTabBtn->height();

    int tabY = m_tabWidget->tabBar()->childrenRect().y();
    if(tabY < addTabBtn->pos().y())
        tabY = addTabBtn->pos().y();

    if(tabXs < width() - 90) {
        addTabBtn->setGeometry(tabX+8, tabY, tabH, tabH);

        m_listsBtn->setGeometry(addTabBtn->x()+20, tabY, tabH, tabH);
        addTabBtn->setGeometry(addTabBtn->x(), tabY, tabH, tabH);
    } else {
        addTabBtn->setGeometry(m_tabWidget->width() - 30, tabY, tabH, tabH);

        m_listsBtn->setGeometry(addTabBtn->x(), tabY, tabH, tabH);
        addTabBtn->setGeometry(addTabBtn->x() - 20, tabY, tabH, tabH);
    }


    if(m_bResizeTabs)
        m_tabWidget->tabBar()->setMaximumWidth(m_tabWidget->width() - 35);
}

void PlistsGroupWidget::createMenus()
{
    QMenu *menu = new QMenu(addTabBtn);

    QAction *search = new QAction(tr("Search"), menu);
    //search->set
    connect(search, SIGNAL(triggered()), SIGNAL(createSearchList()));

    QAction *library = new QAction(tr("Library"), menu);
    connect(library, SIGNAL(triggered()), SIGNAL(createLibraryList()));

    QAction *local = new QAction(tr("Local"), menu);
    connect(local, SIGNAL(triggered()), SIGNAL(createLocalList()));

    QAction *suggestions = new QAction(tr("Suggestions"), menu);
    connect(suggestions, SIGNAL(triggered()), SIGNAL(createSuggestionsList()));

    QAction *musicdb = new QAction(tr("Discography"), menu);
    connect(musicdb, SIGNAL(triggered()), SIGNAL(createDbSearch()));

    menu->addAction(search);
    menu->addAction(library);
    menu->addAction(local);
    menu->addAction(suggestions);
    menu->addAction(musicdb);

    bool isAcc = Settings::instance()->getValue("general/account_use").toBool();
    if(!isAcc) {
        suggestions->setVisible(false);
        library->setVisible(false);
    }

    addTabBtn->setMenu(menu);
}

void PlistsGroupWidget::tabClosed(int index)
{
    if(m_tabWidget->count() > 1) {
        m_tabWidget->removeTab(index);
        Q_EMIT removeTab(index);
        m_listsMenu->removeAction(m_listsMenu->actions().at(index));
//        m_listsMenu->move();
    }

    tabsResized();
}

void PlistsGroupWidget::setCurrentTab()
{
    m_tabWidget->setCurrentIndex(m_tabWidget->count()-1);
}

void PlistsGroupWidget::onTabMoved(int index1, int index2)
{
    QString tab1 = m_listsMenu->actions().at(index1)->text();
    QString tab2 = m_listsMenu->actions().at(index2)->text();

    m_listsMenu->actions().at(index1)->setText(tab2);
    m_listsMenu->actions().at(index2)->setText(tab1);
}

void PlistsGroupWidget::onTabsListSelected()
{
    QAction *p = static_cast<QAction*>(sender());
    if(p) {
        setActiveTab(m_listsMenu->actions().indexOf(p));
    }
}
