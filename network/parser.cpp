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

#include "parser.h"
#include <QDebug>
#include <QTextCodec>
#include <QRegExp>
#include <QtScript/QtScript>
#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>

Parser::Parser(QObject *parent) :
    QObject(parent)
{
    m_nManager = new QNetworkAccessManager();


    m_iOffset = 0;
    m_morePossible = false;
    m_ReqType = 0;

    m_bBusy = false;

    m_Auth = Auth::instance();
    m_nManager->setCookieJar(m_Auth->cookiejar());
    m_nRequest.setRawHeader("User-Agent", "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/31.0.1650.57 Safari/537.36");
    m_nRequest.setRawHeader("Host", "vk.com");
    m_nRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    connect(m_Auth, SIGNAL(authComplete()), SLOT(authComplete()));
}

QString Parser::trimXlam(QString str)
{
    str.replace(QRegExp("&#39;"), "'");
    str.replace(QRegExp("&#092;"), "\\");
    str.replace(QRegExp("&quot;"), "\"");
    str.replace(QRegExp("&#[\\d]{0,5};"), "");
    str.replace(QRegExp("&amp;"), "&");

    return str;
}

void Parser::setSearchStr(QString strSearch)
{
    m_lastSearch = strSearch;
}

void Parser::setOffset(int offset)
{
    m_iOffset = offset;
}

void Parser::setMore(bool s)
{
    m_morePossible = s;
}

void Parser::setReqType(int type)
{
    m_ReqType = type;
}


void Parser::search(QString str)
{
    m_iOffset = 0;

    m_lastSearch = str;

    m_ReqType = 0;

    // Clear track string, new search
    m_TrackStrings.clear();

    QUrl url("http://vk.com/al_search.php");
    url.addQueryItem("al", "1");
    url.addQueryItem("c[q]", str);
    url.addQueryItem("c[section]", "audio");

    m_nRequest.setUrl(url);

    m_nManager->disconnect();
    connect(m_nManager, SIGNAL(finished(QNetworkReply*)), SLOT(searchReply(QNetworkReply*)));
    m_nManager->get(m_nRequest);

    Q_EMIT busy();
}

void Parser::library(QString id, QString gid)
{
    m_ReqType = 1;

    m_nRequest.setUrl(QUrl(QString("http://vk.com/audio?act=load_audios_silent&al=1&gid=%1&id=%0&please_dont_ddos=2").arg(id).arg(gid)));

    m_nManager->disconnect();
    connect(m_nManager, SIGNAL(finished(QNetworkReply*)), SLOT(libraryReply(QNetworkReply*)));
    m_nManager->get(m_nRequest);

    Q_EMIT busy();
}

void Parser::suggestions()
{
    m_iOffset = 0;

    m_ReqType = 2;

    QNetworkRequest request = QNetworkRequest(QUrl("http://vk.com/audio?act=get_recommendations&al=1&id="+m_Auth->vkId()+"&offset="+QString::number(m_iOffset)));

    m_nManager->disconnect();
    connect(m_nManager, SIGNAL(finished(QNetworkReply*)), SLOT(suggestionsReply(QNetworkReply*)));
    m_nManager->get(request);

    Q_EMIT busy();
}

void Parser::updateTrackUrl()
{
    if(!m_UpTrackQueue.isEmpty()) {
        m_bBusy = true;
        Track *p = m_UpTrackQueue.dequeue();
        m_lastTrack = p;

        QString str = p->artist() + " " + p->title();

        QUrl url("http://vk.com/al_search.php");
        url.addQueryItem("al", "1");
        url.addQueryItem("c[q]", str);
        url.addQueryItem("c[section]", "audio");

        m_nRequest.setUrl(url);

        m_nManager->disconnect();
        connect(m_nManager, SIGNAL(finished(QNetworkReply*)), SLOT(trackReply(QNetworkReply*)));
        m_nManager->get(m_nRequest);
    }
}

void Parser::searchReply(QNetworkReply *reply)
{
    QString content;
    QTextCodec * codec = QTextCodec::codecForName("windows-1251");
    content = codec->toUnicode(reply->readAll());

    content = trimXlam(content);

    int nextOffset = m_iOffset + 100;

    if(nextOffset < 1000) {
        QRegExp rx3("\"section\":\"audio\",\"has_more\":(true)");
        rx3.indexIn(content);
        if(rx3.capturedTexts()[1] != "") {
            m_morePossible = true;
        } else {
            m_morePossible = false;
        }
    } else {
        m_morePossible = false;
    }

    QRegExp rx("<div class=\"audio\" id=\"audio[\\d_]+\"([\\s\\S]*)?<div class=\"audio_add\"></div>");
    rx.setMinimal(true);
    int pos = 0;

    while (pos >= 0) {
        pos = rx.indexIn(content, pos);
        if (pos >= 0) {
            Track *track = new Track(this);

            // Artist
            QRegExp rx2("<b onclick=\"event.cancelBubble = true;\"><a href=.*>(.*)</a></b>");
            rx2.setMinimal(true);
            rx2.indexIn(rx.capturedTexts()[0]);

            track->setArtist(QString(rx2.capturedTexts()[1]).replace("<span class=\"match\">", "").replace("</span>",""));

            rx2.setPattern("<span class=\"title\".*>(.*)<span class=\"user\"");
            if(rx2.indexIn(rx.capturedTexts()[0]) > 0)
                track->setTitle(QString(rx2.capturedTexts()[1]).remove(QRegExp("<span class=\"match\">")).remove(QRegExp("</span>")).remove(QRegExp("<a href.*;\">")).remove(QRegExp("</a>")));

            // Duration
            rx2.setPattern("<input type=\"hidden\" id=\"audio_info[\\d_]+\" value=\".*,([\\d]*)\" />");
            rx2.indexIn(rx.capturedTexts()[0]);
            {
                    int t = QString(rx2.capturedTexts()[1]).toInt();
                    track->setDuration((t/60<10?"":"") + QString::number(t/60)+":"+((t%60)<10?"0":"")+QString::number(t%60));
                    track->setLenght(t);
  	    }

            // Link etc.
            rx2.setPattern("<input type=\"hidden\" id=\"audio_info[\\d_]+\" value=\"(.*),[\\d]*\" />");
            rx2.indexIn(rx.capturedTexts()[0]);

            track->setUrl(rx2.capturedTexts()[1]);

            // Opts for 'Add to library'
            rx2.setPattern("<a class=\"addAudioLink\" href=\"\" onclick=\"addAudio\\(this, \\{act:'a_add',add:1,gid:0,aid:(.*),oid:(.*),album_id:0,hash:'(.*)'\\}\\);return false;\">");
            rx2.indexIn(rx.capturedTexts()[0]);

            track->setAid(rx2.capturedTexts()[1]);
            track->setOid(rx2.capturedTexts()[2]);
            track->setHash(rx2.capturedTexts()[3]);

            pos += rx.matchedLength();

            QString tStr = track->artist() + track->title() + track->lenght();
            tStr.remove(QRegExp("[ ?!:_+]+", Qt::CaseInsensitive));

            // Don't send duplicates to playlist
            if(!m_TrackStrings.contains(tStr, Qt::CaseInsensitive)) {
                Q_EMIT newTrack(track);
                m_TrackStrings.append(tStr);
            } else {
                track->deleteLater();
            }
        }
    }
    content.clear();
    reply->deleteLater();

    if(m_iOffset > 0)
        loadMoreResults();
    else
        Q_EMIT free();

}

void Parser::libraryReply(QNetworkReply *reply)
{
    QString content;
    QTextCodec * codec = QTextCodec::codecForName("windows-1251");
    content = codec->toUnicode(reply->readAll());

    // Parsing AddHash(need for adding songs to your library)
    QString addHash;
    QRegExp rx("\"add_hash\":\"(.*)\"");
    rx.setMinimal(true);
    rx.indexIn(content);
    addHash = rx.capturedTexts()[1];

    // DeleteHash
    QString delHash;
    rx.setPattern("\"delete_hash\":\"(.*)\"");
    rx.indexIn(content);
    delHash = rx.capturedTexts()[1];

    // Tracks json
    QString data;
    rx.setPattern("\\{\"all\":(.*)\\}<!>");
    rx.indexIn(content);
    data = rx.capturedTexts()[1];

    if(data != "")
    {
        QScriptEngine engine;
        QScriptValue values;
        values = engine.evaluate(data);
        QScriptValue item;

        int i = 0;
        while(values.property(i).toString() != "") {
            item = values.property(i);

            Track *track = new Track(this);

            track->setArtist(trimXlam(item.property(5).toString()));
            track->setTitle(trimXlam(item.property(6).toString()));
            track->setDuration(item.property(4).toString());
            track->setUrl(item.property(2).toString());

            track->setAid(item.property(1).toString());
            track->setOid(item.property(0).toString());
            track->setHash(addHash);
            track->setDelHash(delHash);

            Q_EMIT newTrack(track);

            i++;
        }
    }
    Q_EMIT free();

    content.clear();
}

void Parser::suggestionsReply(QNetworkReply *reply)
{
    QString content;
    QTextCodec * codec = QTextCodec::codecForName("windows-1251");
    content = codec->toUnicode(reply->readAll());


    // Parsing AddHash(need for adding songs to your library)
    QString addHash;
    QRegExp rx("onclick=\"Audio\\.addShareAudio\\(this, [\\d]+, [\\d]+, '(.*)', 0\\)\\;\"");
    rx.setMinimal(true);
    rx.indexIn(content);
    addHash = rx.capturedTexts()[1];

    QString data;

    //QRegExp rx("\\{\"all\":(.*)\\}<!>");
    rx.setPattern("\\{\"all\":(.*)\\}<!>");
    rx.indexIn(content);
    data = rx.capturedTexts()[1];

    if(data != "")
    {
        QScriptEngine engine;
        QScriptValue values;
        values = engine.evaluate(data);
        QScriptValue item;

        int i = 0;
        while(values.property(i).toString() != "") {
            item = values.property(i);

            Track *track = new Track(this);

            track->setArtist(trimXlam(item.property(5).toString()));
            track->setTitle(trimXlam(item.property(6).toString()));
            track->setDuration(item.property(4).toString());
            track->setUrl(item.property(2).toString());

            track->setAid(item.property(1).toString());
            track->setOid(item.property(0).toString());
            track->setHash(addHash);

            Q_EMIT newTrack(track);

            i++;
        }
    }
    if(m_iOffset > 0)
        loadMoreResults();
    else
        Q_EMIT free();
}

void Parser::trackReply(QNetworkReply *reply)
{

    QString content;
    QTextCodec * codec = QTextCodec::codecForName("windows-1251");
    content = codec->toUnicode(reply->readAll());

    content = trimXlam(content);
    //qDebug() << content;
    QRegExp rx("<div class=\"audio\" id=\"audio[\\d_]+\"([\\s\\S]*)?<div class=\"audio_add\"></div>");
    rx.setMinimal(true);
    int pos = 0;

    bool founded = false;
    while (pos >= 0 && !founded) {
        pos = rx.indexIn(content, pos);
        if (pos >= 0) {
            QString trackStr;
            QString newUrl;
            QString newAid;
            QString newOid;
            QString newHash;


            // Artist
            QRegExp rx2("<b onclick=\"event.cancelBubble = true;\"><a href=.*>(.*)</a></b>");
            rx2.setMinimal(true);
            rx2.indexIn(rx.capturedTexts()[0]);

            //track->setArtist(rx2.capturedTexts()[1]);
            trackStr.append(QString(rx2.capturedTexts()[1]).replace("<span class=\"match\">", "").replace("</span>",""));

            // Title
            rx2.setPattern("<span class=\"title\".*>(.*)</span><span class=\"user\"");
            if(rx2.indexIn(rx.capturedTexts()[0]) > 0)
                trackStr.append(QString(rx2.capturedTexts()[1]).remove(QRegExp("<a href.*;\">")).remove(QRegExp("</a>")).replace("<span class=\"match\">", "").replace("</span>",""));

            // Duration
            rx2.setPattern("<input type=\"hidden\" id=\"audio_info[\\d_]+\" value=\".*,([\\d]*)\" />");
            rx2.indexIn(rx.capturedTexts()[0]);

            int t = QString(rx2.capturedTexts()[1]).toInt();
            QString duration = (t/60<10?"":"") + QString::number(t/60)+":"+((t%60)<10?"0":"")+QString::number(t%60);
            QString duration_trimmed = duration;
            duration_trimmed.remove(duration.length()-1, 1);

            if(m_lastTrack->duration() != "0:00") {
                trackStr.append(duration_trimmed);
            } else {
                m_lastTrack->setDuration(duration);
            }
            m_lastTrack->setLenght(t);

            // Link etc.
            rx2.setPattern("<input type=\"hidden\" id=\"audio_info[\\d_]+\" value=\"(.*),[\\d]*\" />");
            rx2.indexIn(rx.capturedTexts()[0]);

            newUrl = rx2.capturedTexts()[1];

            // Opts for 'Add to library'
            rx2.setPattern("addAudio\\(this, \\{act:'add',add:1,gid:0,aid:(.*),oid:(.*),album_id:0,hash:'(.*)',top:0,search:1\\}\\); return false;\">");

            rx2.indexIn(rx.capturedTexts()[0]);

            newAid = rx2.capturedTexts()[1];
            newOid = rx2.capturedTexts()[2];
            newHash = rx2.capturedTexts()[3];


            pos += rx.matchedLength();

            trackStr.remove(QRegExp("[ ?!:_+/\\)\\(\\]\\[]+", Qt::CaseInsensitive));

            QString curTrackStr;
            curTrackStr.append(m_lastTrack->artist());
            curTrackStr.append(m_lastTrack->title());
            curTrackStr.append(m_lastTrack->duration().remove(m_lastTrack->duration().length()-1, 1));
            curTrackStr.remove(QRegExp("[ ?!:_+/\\)\\(\\]\\[]+", Qt::CaseInsensitive));

            if(m_lastTrack) {
                if(curTrackStr.contains(trackStr, Qt::CaseInsensitive)) {
                    if(m_lastTrack->aid().isEmpty()) {
                        m_lastTrack->setAid(newAid);
                        m_lastTrack->setOid(newOid);
                    }

                    if(m_lastTrack->hash().isEmpty()) {
                        m_lastTrack->setHash(newHash);
                    }

                    m_lastTrack->setUrl(newUrl);
                    m_lastTrack->urlUpdated(true);

                    founded = true;
                }
            }

            trackStr.clear();
            curTrackStr.clear();
            newUrl.clear();

        }
    }

    Q_EMIT free();

    content.clear();
    reply->deleteLater();

    if(m_lastTrack)
        if(!founded)
            m_lastTrack->urlUpdated(false);

    m_bBusy = false;

    //updateTrackUrl();
    QTimer::singleShot(1000, this, SLOT(updateTrackUrl()));
}

void Parser::authComplete()
{
    // unused
}

void Parser::loadMoreResults()
{
    if(m_ReqType == 0) {
        if(m_morePossible && m_iOffset < 1000) {
            Q_EMIT busy();

            m_iOffset += 100;

            QString sRequest = QString("al=1&c[q]="+m_lastSearch+"&c[section]=audio");
            m_nRequest.setUrl(QUrl(QString("http://vk.com/al_search.php?"+sRequest)));
	    //c[q]=%0&c[section]=audio&offset=%1").arg(m_lastSearch).arg(m_iOffset)));

    m_nRequest.setHeader(QNetworkRequest::ContentTypeHeader, 
        "application/x-www-form-urlencoded");
            m_nManager->disconnect();
            connect(m_nManager, SIGNAL(finished(QNetworkReply*)), SLOT(searchReply(QNetworkReply*)));
	    {
	    	QByteArray t;
		QString s=QString(sRequest)+"&offset="+QString::number(m_iOffset);
		t.insert(0, s.toUtf8());
	        m_nManager->post(m_nRequest, t);
	    }
        }  else {
            m_morePossible = false;
            Q_EMIT free();
        }
    } else if(m_ReqType == 2) {
        if(m_iOffset < 350) {
            Q_EMIT busy();
            m_morePossible = true;
            m_iOffset += 50;

            QNetworkRequest request = QNetworkRequest(QUrl("http://vk.com/audio?act=get_recommendations&al=1&id="+m_Auth->vkId()+"&offset="+QString::number(m_iOffset)));

            m_nManager->disconnect();
            connect(m_nManager, SIGNAL(finished(QNetworkReply*)), SLOT(suggestionsReply(QNetworkReply*)));
            m_nManager->get(request);
        } else {
            m_morePossible = false;
            Q_EMIT free();
        }
    }
}

void Parser::updateTrack()
{
    Track *p = (Track *)sender();

    connect(p, SIGNAL(destroyed()), SLOT(onTrackDestroyed()));

    m_UpTrackQueue.enqueue(p);

    if(m_UpTrackQueue.size() == 1 && m_Auth->isAuth() && !m_bBusy) {
        updateTrackUrl();
    }


}

void Parser::onTrackDestroyed()
{
    qDebug() << "Track in update queue destroyed.";
    Track *p = (Track *)sender();
    int index = m_UpTrackQueue.indexOf(p);

    if(index  > -1)
        m_UpTrackQueue.removeAt(index);

    if(m_lastTrack == p)
        m_lastTrack = 0;

}
