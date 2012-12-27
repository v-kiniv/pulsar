/*
    Copyright (C) 2010 Marco Ballesio <gibrovacco@gmail.com>
    Copyright (C) 2011 Collabora Ltd.
      @author George Kiagiadakis <george.kiagiadakis@collabora.co.uk>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published
    by the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "gstplayer.h"
#include <QtCore/QDir>
#include <QtCore/QUrl>
#include <QGlib/Connect>
#include <QGlib/Error>
#include <QGst/Pipeline>
#include <QGst/ElementFactory>
#include <QGst/Bus>
#include <QGst/Message>
#include <QGst/Query>
#include <QGst/ClockTime>
#include <QGst/Event>
#include <QGst/StreamVolume>
#include <QGlib/Error>
#include <QGst/Buffer>
#include <QGst/TagList>

#include <QTextCodec>


GstPlayer::GstPlayer(QObject *parent)
    : QObject(parent)
{
    //this timer is used to tell the ui to change its position slider & label
    //every 100 ms, but only when the pipeline is playing
    connect(&m_positionTimer, SIGNAL(timeout()), this, SIGNAL(positionChanged()));


    if (!m_pipeline) {
        m_pipeline = QGst::ElementFactory::make("playbin2").dynamicCast<QGst::Pipeline>();
        if (m_pipeline) {
            //let the video widget watch the pipeline for new video sinks
            //watchPipeline(m_pipeline);

            //watch the bus for messages
            QGst::BusPtr bus = m_pipeline->bus();
            bus->addSignalWatch();
            QGlib::connect(bus, "message", this, &GstPlayer::onBusMessage);
        } else {
            qCritical() << "Failed to create the pipeline";
        }
    }

    m_volume = 0.7;
}

GstPlayer::~GstPlayer()
{
    if (m_pipeline) {
        m_pipeline->setState(QGst::StateNull);
    }
}

QString strToUnicode(QString str) {
    QString trimmedStr = str;
    trimmedStr.replace(" ", "");
    bool isUnicode = trimmedStr.toAscii().contains("??");

    if(!isUnicode) {
        QTextCodec * codec = QTextCodec::codecForName("windows-1251");
        return codec->toUnicode(str.toAscii());
    } else {
        return str;
    }
}

void GstPlayer::setUri(const QString & uri)
{
    QString realUri = uri;

    //if uri is not a real uri, assume it is a file path
    if (realUri.indexOf("://") < 0) {
        realUri = QUrl::fromLocalFile(realUri).toEncoded();
    }

    if (m_pipeline) {
        m_pipeline->setProperty("uri", realUri);
    }

    m_Meta.clear();

    //m_pipeline->
}

QTime GstPlayer::position() const
{
    if (m_pipeline) {
        //here we query the pipeline about its position
        //and we request that the result is returned in time format
        QGst::PositionQueryPtr query = QGst::PositionQuery::create(QGst::FormatTime);
        m_pipeline->query(query);
        return QGst::ClockTime(query->position()).toTime();
    } else {
        return QTime();
    }
}

void GstPlayer::setPosition(const QTime & pos)
{
    QGst::SeekEventPtr evt = QGst::SeekEvent::create(
        1.0, QGst::FormatTime, QGst::SeekFlagFlush,
        QGst::SeekTypeSet, QGst::ClockTime::fromTime(pos),
        QGst::SeekTypeNone, QGst::ClockTime::None
    );
    m_pipeline->sendEvent(evt);
}

double GstPlayer::volume() const
{
    if (m_pipeline) {
        QGst::StreamVolumePtr svp =
            m_pipeline.dynamicCast<QGst::StreamVolume>();

        if (svp) {
            return svp->volume(QGst::StreamVolumeFormatCubic);
        }
    }

    return 0;
}


void GstPlayer::setVolume(double volume)
{
    if (m_pipeline) {
        QGst::StreamVolumePtr svp =
            m_pipeline.dynamicCast<QGst::StreamVolume>();

        if(svp) {
            svp->setVolume(volume, QGst::StreamVolumeFormatCubic);
        }
    }
    m_volume = volume;
}

QTime GstPlayer::length() const
{
    if (m_pipeline) {

        //here we query the pipeline about the content's duration
        //and we request that the result is returned in time format
        QGst::DurationQueryPtr query = QGst::DurationQuery::create(QGst::FormatTime);
        m_pipeline->query(query);
        return QGst::ClockTime(query->duration()).toTime();
    } else {
        return QTime();
    }
}

QGst::State GstPlayer::state() const
{
    return m_pipeline ? m_pipeline->currentState() : QGst::StateNull;
}

void GstPlayer::setBuffer(QBuffer *streamBuffer)
{
    QGst::BufferPtr gstBuffer = QGst::Buffer::create(streamBuffer->size());

    quint8 *bdata = gstBuffer->data();

//    qDebug() << streamBuffer->size();
    Q_UNUSED(bdata);
}

qint64 GstPlayer::duration()
{
    QGst::DurationQueryPtr query1 = QGst::DurationQuery::create(QGst::FormatTime);
    m_pipeline->query(query1);
    return query1->duration();
}

qint64 GstPlayer::pos()
{
    QGst::PositionQueryPtr query = QGst::PositionQuery::create(QGst::FormatTime);
    m_pipeline->query(query);
    return query->position();
}

void GstPlayer::seek(int value)
{
    Q_UNUSED(value);
}

QMap<QString, QString> GstPlayer::meta()
{
    return m_Meta;
}

void GstPlayer::play()
{
    setVolume(m_volume);
    if (m_pipeline) {
        m_pipeline->setState(QGst::StatePlaying);
    }
}

void GstPlayer::pause()
{
    if (m_pipeline) {
        m_pipeline->setState(QGst::StatePaused);
    }
}

void GstPlayer::stop()
{
    if (m_pipeline) {
        m_pipeline->setState(QGst::StateNull);

        //once the pipeline stops, the bus is flushed so we will
        //not receive any StateChangedMessage about this.
        //so, to inform the ui, we have to Q_EMIT this signal manually.
        Q_EMIT stateChanged();
    }
}

void GstPlayer::onBusMessage(const QGst::MessagePtr & message)
{
    switch (message->type()) {
    case QGst::MessageEos: //End of stream. We reached the end of the file.
        stop();
        Q_EMIT finished();
        break;
    case QGst::MessageError: //Some error occurred.
        qCritical() << message.staticCast<QGst::ErrorMessage>()->error();
        if(message.staticCast<QGst::ErrorMessage>()->error().code() == 5)
            Q_EMIT linkExpired();
        stop();
        break;
    case QGst::MessageStateChanged: //The element in message->source() has changed state
        if (message->source() == m_pipeline) {
            handlePipelineStateChange(message.staticCast<QGst::StateChangedMessage>());
        }
        break;
    case QGst::MessageTag:
            handleTag(message.staticCast<QGst::TagMessage>());
        break;
    default:
        break;
    }
}

void GstPlayer::handleTag(const QGst::TagMessagePtr &scm)
{
    QGst::TagList list = scm->taglist();

    quint32 brate = list.bitrate();
    if(brate > 0)
        Q_EMIT gotBitrate(brate);

    if(!list.artist().isEmpty())
        m_Meta["artist"] = strToUnicode(list.artist());

    if(!list.title().isEmpty())
        m_Meta["title"] = strToUnicode(list.title());

    if(!list.genre().isEmpty())
        m_Meta["genre"] = strToUnicode(list.genre());

    if(list.maximumBitrate() > 0)
        m_Meta["brate"] = QString::number(list.maximumBitrate() / 1000);

    if(list.tagValueCount("album") > 0)
        m_Meta["album"] = strToUnicode(list.tagValue("album").toString());

    Q_EMIT metaChanged();


}

void GstPlayer::handlePipelineStateChange(const QGst::StateChangedMessagePtr & scm)
{
    switch (scm->newState()) {
    case QGst::StatePlaying:
        //start the timer when the pipeline starts playing
        m_positionTimer.start(100);
        break;
    case QGst::StatePaused:
        //stop the timer when the pipeline pauses
        if(scm->oldState() == QGst::StatePlaying) {
            m_positionTimer.stop();
        }
        break;
    default:
        break;
    }

    Q_EMIT stateChanged();
}

#ifdef Q_OS_LINUX
#include "moc_gstplayer.cpp"
#endif
