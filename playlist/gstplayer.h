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
#ifndef GstPlayer_H
#define GstPlayer_H

#include <QtCore/QTimer>
#include <QtCore/QTime>
#include <QtCore/QBuffer>
#include <QGst/Pipeline>
#include <QGst/Ui/VideoWidget>
#include <QMap>

class GstPlayer : public QObject
{
    Q_OBJECT
public:
    GstPlayer(QObject *parent = 0);
    ~GstPlayer();

    void setUri(const QString & uri);

    QTime position() const;
    void setPosition(const QTime & pos);
    double volume() const;

    QTime length() const;
    QGst::State state() const;


    void setBuffer(QBuffer *streamBuffer);

    qint64 duration();
    qint64 pos();
    void seek(int);

    QMap<QString, QString> meta();

private:
    QMap<QString, QString> m_Meta;
    double m_volume;

public Q_SLOTS:
    void play();
    void pause();
    void stop();
    void setVolume(double volume);

Q_SIGNALS:
    void positionChanged();
    void stateChanged();
    void linkExpired();
    void finished();
    void gotBitrate(quint32);
    void metaChanged();

private:
    //QGst::Buffer m_GstBuffer;

    void onBusMessage(const QGst::MessagePtr & message);
    void handlePipelineStateChange(const QGst::StateChangedMessagePtr & scm);
    void handleTag(const QGst::TagMessagePtr &scm);

    QGst::PipelinePtr m_pipeline;
    QTimer m_positionTimer;
};

#endif
