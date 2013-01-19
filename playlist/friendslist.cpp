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

#include "friendslist.h"
#include "network/parser.h"

#include <QDebug>
#include <QFile>
#include <QTextCodec>
#include <QtScript/QtScript>
#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>


FriendsList::FriendsList(QObject *parent) :
    QObject(parent)
{
    m_Auth = Auth::instance();

    m_Settings = Settings::instance();
    m_AppPath = m_Settings->appPath();

    m_nManager = new QNetworkAccessManager();
    m_nManager->setCookieJar(m_Auth->cookiejar());

    m_Model = new QStandardItemModel(this);

    m_ProxyModel = new QSortFilterProxyModel(this);
    m_ProxyModel->setSourceModel(m_Model);
    m_ProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    m_Widget = new FriendsListWidget();
    m_Widget->setModel(m_ProxyModel);
    connect(m_Widget, SIGNAL(searchChanged(QString)), SLOT(search(QString)));
    connect(m_Widget, SIGNAL(itemActivated(QModelIndex)), SLOT(listItemAcivated(QModelIndex)));
    connect(m_Widget, SIGNAL(idChoosed(QString)), SLOT(setId(QString)));
    connect(m_Widget, SIGNAL(refresh()), SLOT(refreshList()));

    if(m_Auth->isAuth())
        load();
    else
        connect(m_Auth, SIGNAL(authComplete()), SLOT(load()));

}

void FriendsList::load()
{
    QStringList myId;
    myId << m_Auth->vkId() << tr("My Library");
    add(myId);

    QString fName = m_AppPath;
    fName += "friends";

    QSettings sfile(fName, QSettings::IniFormat);

    int size = sfile.beginReadArray("Friends");
    if(size > 0) {
        for(int i = 0; i < size; i++) {
            sfile.setArrayIndex(i);


            QString id = sfile.value("id").toString();
            QString name = sfile.value("name").toString();

            QStringList entry;
            entry << id << name;
            add(entry);
        }
        m_Widget->hideLoading();
    } else {
        getList();
    }
    sfile.endArray();
}

void FriendsList::save()
{
    QString fName = m_AppPath;
    fName += "friends";

    QSettings sfile(fName, QSettings::IniFormat);
    sfile.clear();

    sfile.beginWriteArray("Friends");
    for(int i = 0; i < m_List.size(); i++) {
        sfile.setArrayIndex(i);

        sfile.setValue("id", m_List.at(i).at(0));
        sfile.setValue("name", m_List.at(i).at(1));
    }
    sfile.endArray();
}

void FriendsList::getList()
{
    QNetworkRequest request = QNetworkRequest(QUrl("http://vk.com/al_friends.php?act=load_friends_silent&al=1&gid=0&id="+m_Auth->vkId()));

    m_nManager->disconnect();
    connect(m_nManager, SIGNAL(finished(QNetworkReply*)), SLOT(parseList(QNetworkReply*)));
    m_nManager->get(request);

    m_Widget->showLoading();
}

void FriendsList::getAvatars()
{
    //m_AvatarsUrls.at(m_AvatarI);
    if(m_AvatarI < m_AvatarsUrls.size()) {

        if(!m_AvatarsUrls.at(m_AvatarI).contains(".gif")) {
            QNetworkRequest request = QNetworkRequest(QUrl(m_AvatarsUrls.at(m_AvatarI)));

            m_nManager->disconnect();
            connect(m_nManager, SIGNAL(finished(QNetworkReply*)), SLOT(downloadAvatar(QNetworkReply*)));
            m_nManager->get(request);
            m_AvatarI++;
        } else {
            m_AvatarI++;
            getAvatars();
        }

   }
}

void FriendsList::getUserId(QString userLogin)
{
    QNetworkRequest request = QNetworkRequest(QUrl("http://vk.com/" + QString(userLogin.toInt() > 0 ? "id" + userLogin : userLogin)));
    request.setRawHeader("User-Agent", "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.11 (KHTML, like Gecko) Ubuntu/12.04 Chromium/23.0.1271.10 Chrome/23.0.1271.10 Safari/537.11");

    m_nManager->disconnect();
    connect(m_nManager, SIGNAL(finished(QNetworkReply*)), SLOT(parseId(QNetworkReply*)));
    m_nManager->get(request);
}

void FriendsList::parseList(QNetworkReply *reply)
{
    m_ItemsList.clear();
    m_List.clear();
    m_AvatarsUrls.clear();
    m_AvatarI = 0;

    QString content;
    QTextCodec * codec = QTextCodec::codecForName("windows-1251");
    content = codec->toUnicode(reply->readAll());

    QString data;
    content.replace("\\n", "");
    content.replace("\\", "");

    QRegExp rx("\\{\"all\":(.*)\\]\\]");
    rx.setMinimal(true);
    rx.indexIn(content);
    data = rx.capturedTexts()[1];

    data.append("]]");

    if(data != "")
    {
        QScriptEngine engine;
        QScriptValue values;
        values = engine.evaluate(data);
        QScriptValue item;


        QString id;
        QString name;
        QString avatarUrl;
        int i = 0;
        while(values.property(i).toString() != "") {
            item = values.property(i);

            id = item.property(0).toString();
            name = item.property(5).toString();
            avatarUrl = item.property(1).toString();

            m_AvatarsUrls << avatarUrl;

            QStringList entry;
            entry << id << name;

            add(entry);

            i++;
        }
        getAvatars();
        save();
    }
    m_Widget->hideLoading();
}

void FriendsList::parseId(QNetworkReply *reply)
{
    QString content;
    QTextCodec * codec = QTextCodec::codecForName("windows-1251");
    content = codec->toUnicode(reply->readAll());

    // Get ID
    QString id;
    QString gid;
    QRegExp rx("<a href=\"/audios(\\d+)\" onclick");
    rx.setMinimal(true);
    rx.indexIn(content);
    id = rx.capturedTexts()[1];

    rx.setPattern("<a href=\"/audios\\-(\\d+)\" onclick");
    rx.indexIn(content);
    gid = rx.capturedTexts()[1];

    // Get title of group/UserName
    QString title;
    rx.setPattern("<title>(.*)</title>");
    rx.indexIn(content);
    title = Parser::trimXlam(rx.capturedTexts()[1]);

    Q_EMIT friendSelected(id, gid, title);
}


void FriendsList::downloadAvatar(QNetworkReply *reply)
{
    QString fileName = m_List.at(m_AvatarI-1).at(0) + ".jpg";
    QFile file(fileName);
    QDir::setCurrent(m_AppPath + "/avatars");
    file.open(QIODevice::WriteOnly);
    file.write(reply->readAll());
    file.close();

    m_ItemsList.at(m_AvatarI-1)->setIcon(QIcon(QPixmap(fileName)));

    if(m_AvatarI < m_AvatarsUrls.size()) {
        getAvatars();
    }
}

void FriendsList::add(QStringList entry)
{
    QString avatarPath = m_AppPath + "avatars/" + entry.at(0) + ".jpg";
    QPixmap pix;
    if(QFile(avatarPath).exists()) {
        pix = QPixmap(avatarPath);
        //qDebug() << pix.isNull();
    } else {
        pix = QPixmap(":/icons/app_logo_light");
    }

    QStandardItem *item = new QStandardItem(QIcon(pix) ,entry.at(1));
    item->setData(QVariant(entry), Qt::UserRole + 1);

    m_ItemsList.append(item);

    if(entry.at(1) == m_Auth->vkId()) {
        m_Model->insertRow(0, item);
    } else {
        m_Model->appendRow(item);
    }

    m_List.append(entry);

}

void FriendsList::search(QString str)
{
    m_ProxyModel->setFilterFixedString(str);
//    QList<QStandardItem *> sList = m_Model->findItems(str, Qt::MatchContains);
//    //qDebug() << sList.at(0)->text();
//    //m_Model->clear();
//    m_SearchModel->clear();
//    for(int i = 0; i < sList.size(); i++) {
//        m_SearchModel->setItem(i, sList.at(i));
//        qDebug() << sList.at(i)->text();
//    }
//    m_SearchModel->appendColumn(sList);

    //m_Widget->setModel(m_SearchModel);
}

void FriendsList::listItemAcivated(QModelIndex index)
{
    QStringList data = m_ProxyModel->data(index, Qt::UserRole + 1).toStringList();
    QString vkid = data.at(0) == "0" ? m_Auth->vkId() : data.at(0);
    Q_EMIT friendSelected(vkid, "0", data.at(1));
    m_Widget->hide();
    m_Widget->clear();
}

void FriendsList::setId(QString id)
{
    getUserId(id);
    m_Widget->hide();
    m_Widget->clear();
}

void FriendsList::refreshList()
{
    m_Model->clear();

    QStringList myId;
    myId << m_Auth->vkId() << tr("My Library");
    add(myId);

    getList();
}
