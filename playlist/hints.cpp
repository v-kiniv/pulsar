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

#include "hints.h"
#include <QVBoxLayout>
#include <QTextCodec>
#include <QDebug>
#include <QScriptEngine>

Hints::Hints(QWidget *parent) :
    QWidget(parent)
{
    m_Auth = Auth::instance();

    m_nManager = new QNetworkAccessManager();
    m_nManager->setCookieJar(m_Auth->cookiejar());
    connect(m_nManager, SIGNAL(finished(QNetworkReply*)), SLOT(netReply(QNetworkReply*)));

    setObjectName("Hints");

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    QVBoxLayout *layout = new QVBoxLayout(this);

    setStyleSheet("QWidget#Hints {border-radius: 5px; border-top-left-radius: 0px; border-top-right-radius: 0px; border-bottom: none; background-color: rgba(55, 55, 55, 0); color: #fff; font-size: 15px; background-image: url(:/images/bkg_light);}");

    m_wList = new QListWidget(this);
    m_wList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_wList->setFrameStyle(0);
    m_wList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    connect(m_wList, SIGNAL(clicked(QModelIndex)), SLOT(itemActivated(QModelIndex)));
    connect(m_wList, SIGNAL(activated(QModelIndex)), SLOT(itemActivated(QModelIndex)));

    m_wList->setStyleSheet("QListWidget {padding-bottom: 8px; border-radius: 10px; border-top-left-radius: 0px; border-top-right-radius: 0px; border-bottom: none; border-right: none; background-color: rgba(55, 55, 55, 0); color: #333; font-size: 12px;} ::item {padding: 2px 0 2px 0;} \
                           QListWidget::item:selected {border: none; color: #fff; background-color: rgba(0, 0, 0, 100);} \
                           QListWidget::item:hover {border: none; background-color: rgba(0, 0, 0, 20);} \
                           QScrollBar:vertical {border: none; background-color: rgba(0, 0, 0, 20); width: 6px; } \
                           QScrollBar::handle:vertical {border: none; border-radius: 2px; background-color: #aaa;} \
                           QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {height: 0px; border: none;} \
                           QScrollBar::up-arrow:vertical, QScrollBar::down-arrow:vertical {border: none;} \
                           QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {background: none;} \
                         ");


    layout->addWidget(m_wList);

    m_wList->setMaximumHeight(110);
    hide();

}

void Hints::paintEvent(QPaintEvent *)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    p.setOpacity(0.8);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void Hints::parentResized(QRect pRect)
{
    QRect newRect;
    newRect.setY(pRect.y()+pRect.height()-2);
    newRect.setX(pRect.x()+10);
    newRect.setWidth(pRect.width()-20);
    newRect.setHeight(50);

    setMinimumWidth(pRect.width()-20);

    setGeometry(newRect);

    adjustSize();
}

void Hints::getFocus()
{
    m_wList->setFocus();
}

void Hints::textChanged(QString str)
{
    if(str != m_sLastHint) {
        QNetworkRequest request = QNetworkRequest(QUrl("http://vk.com/hints.php?act=a_gsearch_hints&section=audio&q="+str));
        if(m_Auth->isAuth()) {
            m_nManager->get(request);
        }
    }
}

void Hints::netReply(QNetworkReply *reply)
{
    QString data;
    QTextCodec * codec = QTextCodec::codecForName("utf-8");
    data = codec->toUnicode(reply->readAll());

    QStringList strList;

    QScriptEngine engine;
    QScriptValue values;
    values = engine.evaluate(data);

    QMap<int, QString> list;

    QString item;

    for(int i = 0; i < 20; i++) {
        item = values.property(QString::number(i)).property("3").toString();
        if(item != "") {
            list[i] = item;
            strList << item;
        }
    }

    m_wList->clear();

    if(!list.isEmpty()) {
        m_wList->addItems(strList);
        m_wList->scrollToTop();

        show();
    }
}

void Hints::itemActivated(QModelIndex index)
{
    m_sLastHint = m_wList->model()->data(index).toString();
    Q_EMIT selected(m_sLastHint);
    hide();
}
