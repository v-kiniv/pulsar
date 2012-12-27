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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QMap>
#include <QSettings>

class Settings : public QObject
{
    Q_OBJECT
public:
    explicit Settings(QObject *parent = 0);

    static Settings *instance();

    QString appPath();
    void load();
    void save();

    void update();
    void updateShortcuts();

    QVariant getValue(QString value);
    void setValue(QString key, QVariant value);

private:
    static Settings *m_instance;

    QString m_sAppPath;

    QMap<QString, QVariant>  m_MainSettings;

    void setDefaults();

Q_SIGNALS:
    void changed();
    void shortcutsChanged();

public Q_SLOTS:
    void onAccountChanged();

private Q_SLOTS:


};

#endif // SETTINGS_H
