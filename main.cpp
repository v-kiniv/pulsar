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

#include <QtGui/QApplication>
#include <QLocalSocket>
#include <QGst/Init>

#include "settings/settings.h"
#include "ui/pulsar.h"

int main(int argc, char *argv[])
{
//    QApplication::setStyle(new QWindowsStyle);
    QApplication a(argc, argv);
    a.setApplicationName("Pulsar");
    a.setApplicationVersion("0.9.2 beta");

    // Check if instance of app is exists
    QLocalSocket socket;
    socket.connectToServer("945479168ffc177a8e968dd26eff61ce");
    if (socket.waitForConnected(500)) {
        qDebug() << "Already running instance of Pulsar. Exit." ;
        return 0; // Exit already a process running
    }
    ////

    QGst::init(&argc, &argv);

    Settings settings;
    settings.load();

    QTranslator translator;
    QString language;
    if(settings.getValue("general/language").toString() == "auto")
        language = QLocale::system().name();
    else
        language = settings.getValue("general/language").toString();

    QString lang_path = "/usr/share/pulsar/translations";
    #ifdef Q_OS_WIN
    lang_path = QApplication::applicationDirPath() + "/translations";
    #endif

    translator.load(lang_path+"/pulsar_" + language);
    a.installTranslator(&translator);

    Pulsar pulsar;
    pulsar.show();



    return a.exec();
}
