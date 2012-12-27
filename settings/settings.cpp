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

#include "settings.h"
#include <QDir>
#include <QFile>
#include <QDebug>

Settings *Settings::m_instance = 0;

Settings::Settings(QObject *parent) :
    QObject(parent)
{
    if(m_instance)
            qFatal("Only one instance of Settings object is allowed");

    m_instance = this;

    m_sAppPath = QDir::homePath() + "/.config/pulsar";
    if(!QDir(m_sAppPath).exists()) {
        QDir().mkdir(m_sAppPath);
        QDir().mkdir(m_sAppPath+"/avatars");
        QDir().mkdir(m_sAppPath+"/tmp");
    }
    m_sAppPath += "/";

    setDefaults();

}

void Settings::setDefaults()
{
    m_MainSettings["general/account_use"] = QVariant(false);
    m_MainSettings["general/account_login"] = QVariant("");
    m_MainSettings["general/account_pass"] = QVariant("");
    m_MainSettings["general/vkid"] = QVariant("0");
    m_MainSettings["general/music_path"] = QVariant(QDir::homePath());
    m_MainSettings["general/language"] = QVariant("auto");

    m_MainSettings["playlist/tabs_by_content"] = QVariant(true);
    m_MainSettings["playlist/load_meta"] = QVariant(false);
    m_MainSettings["playlist/use_meta"] = QVariant(false);
    m_MainSettings["playlist/autoload_library"] = QVariant(false);

    m_MainSettings["network/buffering_off"] = QVariant(false);

    m_MainSettings["playing/play_on_start"] = QVariant(false);

    // Player
    m_MainSettings["player/shuffle"] = QVariant(1);
    m_MainSettings["player/repeat"] = QVariant(1);
    m_MainSettings["player/status"] = QVariant(false);
    m_MainSettings["player/volume"] = QVariant(0.7);

    m_MainSettings["mw/width"] = QVariant(700);
    m_MainSettings["mw/height"] = QVariant(500);

    m_MainSettings["song/title"] = QVariant("Pulsar");
    m_MainSettings["song/artist"] = QVariant("");
    m_MainSettings["song/album"] = QVariant("");
    m_MainSettings["song/art"] = QVariant("");

    m_MainSettings["window/hide_tray"] = QVariant(false);
    m_MainSettings["window/quit_onclose"] = QVariant(false);

    m_MainSettings["lastfm/sk"] = QVariant("");
    m_MainSettings["lastfm/username"] = QVariant("");
    m_MainSettings["lastfm/scrobbling"] = QVariant(false);

    // Shortcuts
    m_MainSettings["l_shortcut/playpause"] = QVariant("X");
    m_MainSettings["g_shortcut/playpause"] = QVariant("Media Play");
    m_MainSettings["l_shortcut/next"] = QVariant("C");
    m_MainSettings["g_shortcut/next"] = QVariant("Media Next");
    m_MainSettings["l_shortcut/prev"] = QVariant("Z");
    m_MainSettings["g_shortcut/prev"] = QVariant("Media Previous");
    m_MainSettings["l_shortcut/shuffle"] = QVariant("S");
    m_MainSettings["g_shortcut/shuffle"] = QVariant("");
    m_MainSettings["l_shortcut/repeat"] = QVariant("R");
    m_MainSettings["g_shortcut/repeat"] = QVariant("");
    m_MainSettings["g_shortcut/showwindow"] = QVariant("Meta+P");
    m_MainSettings["l_shortcut/current2library"] = QVariant("L");
    m_MainSettings["g_shortcut/current2library"] = QVariant("");
    m_MainSettings["l_shortcut/current_download"] = QVariant("D");
    m_MainSettings["g_shortcut/current_download"] = QVariant("");

}

Settings *Settings::instance() {
    return m_instance;
}

QString Settings::appPath() { return m_sAppPath; }

void Settings::load()
{
    QString fName = m_sAppPath;
    fName += "settings";
    QSettings cnFile(fName, QSettings::IniFormat);
    //cnFile.clear();

    // Main settings from settings dialog
    cnFile.beginGroup("Main");
    QMapIterator<QString, QVariant> i(m_MainSettings);
    while (i.hasNext()) {
        i.next();
        if(!cnFile.value(i.key()).isNull())
            m_MainSettings[i.key()] = cnFile.value(i.key());
    }
    cnFile.endGroup();

    if(!m_MainSettings["general/account_use"].toBool())
        m_MainSettings["player/status"] = QVariant(false);

    Q_EMIT changed();
}

void Settings::save()
{
    QString fName = m_sAppPath;
    fName += "settings";
    QSettings cnFile(fName, QSettings::IniFormat);
    //cnFile.clear();

    // Main settings from settings dialog
    cnFile.beginGroup("Main");
    QMapIterator<QString, QVariant> i(m_MainSettings);
    while (i.hasNext()) {
        i.next();
        cnFile.setValue(i.key(), i.value());
    }
    cnFile.endGroup();
}

void Settings::update()
{
    Q_EMIT changed();
}

void Settings::updateShortcuts()
{
    Q_EMIT shortcutsChanged();
}

QVariant Settings::getValue(QString value)
{
    return m_MainSettings[value].isValid() ? m_MainSettings[value] : false;
}

void Settings::setValue(QString key, QVariant value)
{
    if(m_MainSettings[key] != value) {
        m_MainSettings[key] = value;
        Q_EMIT changed();
    }
}

void Settings::onAccountChanged()
{
    QFile(m_sAppPath+"/cookies").remove();
}
