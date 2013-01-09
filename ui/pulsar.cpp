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

#include "pulsar.h"
#include "settings/settings.h"
#include "playlist/track.h"
#include "ui/tray.h"
#include "network/lastfm.h"
#ifdef Q_OS_LINUX
#include "mpris2/mpris2.h"
#endif

#include "ui/styledwidget.h"
#include "ui/shortcutsmanager.h"

#include <QDebug>
#include <QxtGlobalShortcut>

Pulsar *Pulsar::m_instance = 0;

Pulsar::Pulsar(QWidget *parent) :
    QMainWindow(parent)
{
    if(m_instance)
            qFatal("Only one instance of Pulsar object is allowed");
    m_instance = this;

    // Start local socket server to prevent more instances
    m_localServer = new QLocalServer(this);
    connect(m_localServer, SIGNAL(newConnection()), this, SLOT(newLocalSocketConnection()));
    m_localServer->listen("945479168ffc177a8e968dd26eff61ce");
    ////

    installEventFilter(this);

    // Settings
    m_Settings = Settings::instance();
    m_settingsDialog = new SettingsDialog();

    ShortcutsManager *shortcutsManager = new ShortcutsManager(this);

    // Auth
    m_auth = new Auth(this);
    connect(m_auth, SIGNAL(nError(Auth::Errors)), SLOT(authError(Auth::Errors)));

    // Download manager
    m_DmManager = new DownloadManager(this);

    // Vkactions
    new VkActions(this);

    // About
    m_wAbout = new About();

    createMenus();
    init();
    setupUi();

    #ifdef Q_OS_LINUX
    new Mpris2(this);
    #endif

    QApplication::setQuitOnLastWindowClosed(false);


    /////////////
    Lastfm *lastfm = new Lastfm(this);
    connect(lastfm, SIGNAL(needAuth(QString,QString)), m_wAuthWindow, SLOT(requireAuth(QString,QString)));
    connect(lastfm, SIGNAL(authSuccess(bool)), m_wAuthWindow, SLOT(onSuccess(bool)));
    connect(m_wAuthWindow, SIGNAL(authCompleted()), lastfm, SLOT(onAuthCompleted()));
    connect(m_wAuthWindow, SIGNAL(authCanceled()), lastfm, SLOT(onAuthCanceled()));
    connect(m_player, SIGNAL(trackChanged(Track*)), lastfm, SLOT(onTrackUpdated(Track*)));
    connect(m_player, SIGNAL(tick(int)), lastfm, SLOT(onTick(int)));

    // Shortcuts
    connect(shortcutsManager, SIGNAL(playPause()), m_player, SLOT(playPause()));
    connect(shortcutsManager, SIGNAL(next()), m_player, SLOT(next()));
    connect(shortcutsManager, SIGNAL(prev()), m_player, SLOT(prev()));
    connect(shortcutsManager, SIGNAL(shuffle()), m_plistgroup, SLOT(toggleShuffle()));
    connect(shortcutsManager, SIGNAL(repeat()), m_plistgroup, SLOT(toggleRepeat()));
    connect(shortcutsManager, SIGNAL(showWindow()), SLOT(raiseWindow()));
    connect(shortcutsManager, SIGNAL(current2Library()), SLOT(current2library()));
    connect(shortcutsManager, SIGNAL(currentDownload()), SLOT(currentDownload()));
}

Pulsar::~Pulsar()
{

}

Pulsar *Pulsar::instance()
{
    return m_instance;
}

void Pulsar::closeEvent(QCloseEvent *event)
{   
    event->accept();

    if(m_Settings->getValue("window/quit_onclose").toBool())
        quit();
}

void Pulsar::createMenus()
{
    m_AppMenu = new QMenu(this);

    QAction *settingsA = new QAction(tr("Settings"), this);
    QAction *aboutA = new QAction(tr("About"), this);
    QAction *quitA = new QAction(tr("Quit"), this);


    connect(settingsA, SIGNAL(triggered()), m_settingsDialog, SLOT(showDialog()));
    connect(aboutA, SIGNAL(triggered()), m_wAbout, SLOT(showDialog()));
    connect(quitA, SIGNAL(triggered()), SLOT(quit()));

    m_AppMenu->addAction(settingsA);
    m_AppMenu->addAction(aboutA);
    m_AppMenu->addSeparator();
    m_AppMenu->addAction(quitA);
}


void Pulsar::setupUi()
{
    setWindowTitle(QApplication::applicationName() + " " + QApplication::applicationVersion());
    setWindowIcon(QIcon(":/icons/pulsar"));

    QWidget *cWidget = new QWidget(this);

    m_mainLayout = new QVBoxLayout(cWidget);
    m_middleLayout = new QVBoxLayout();
    m_bottomLayout = new QVBoxLayout();

    m_middleLayout->addWidget(m_plistgroup->widget());

    setupBottomUi();

    m_mainLayout->addLayout(m_middleLayout);
    m_mainLayout->addLayout(m_bottomLayout);
    setCentralWidget(cWidget);


    // Popup messages widget
    m_wMsg = new PMessage(this);
    connect(this, SIGNAL(resized(QRect)), m_wMsg, SLOT(parentResized(QRect)));

    connect(m_plistgroup, SIGNAL(message(QString,QString)), m_wMsg, SLOT(message(QString,QString)));

    // dmwidget
    m_DmWidget = new DmWidget(this);
    connect(this, SIGNAL(resized(QRect)), m_DmWidget, SLOT(parentResized(QRect)));

    connect(m_DmManager, SIGNAL(currentChanged(Track*)), m_DmWidget, SLOT(onCurrentChanged(Track*)));
    connect(m_DmManager, SIGNAL(progressChanged(QString,QString,int)), m_DmWidget, SLOT(onProgressChanged(QString,QString,int)));
    connect(m_DmManager, SIGNAL(queueChanged(int)), m_DmWidget, SLOT(onQueueChanged(int)));
    connect(m_DmManager, SIGNAL(allComplete()), m_DmWidget, SLOT(onAllComplete()));
    connect(m_DmManager, SIGNAL(started()), m_DmWidget, SLOT(onStarted()));
    connect(m_DmWidget, SIGNAL(cancelAll()), m_DmManager, SLOT(onCancelAll()));


    /* Size and position of main window */

    adjustSize();

    // Set static size of widow
    setGeometry(QRect(0, 0, m_Settings->getValue("mw/width").toInt(), m_Settings->getValue("mw/height").toInt()));

    // Move window to center of the screen
    move(QApplication::desktop()->availableGeometry(this).center()-rect().center());

    // Auth widget
    m_wAuthWindow = new AuthWindow();
}

void Pulsar::setupBottomUi()
{
    m_PlayerControls = new PlayerControls(m_AppMenu, this);

    connect(m_player, SIGNAL(tick(QTime, QTime)), m_PlayerControls, SLOT(tick(QTime, QTime)));
    connect(m_player, SIGNAL(initVolume(double)), m_PlayerControls, SLOT(setVolume(double)));

    connect(m_PlayerControls, SIGNAL(shuffle(int)), m_plistgroup, SLOT(setShuffle(int)));
    connect(m_plistgroup, SIGNAL(shuffleModeChanged(int)), m_PlayerControls, SLOT(setShuffle(int)));

    connect(m_PlayerControls, SIGNAL(repeat(int)), m_plistgroup,  SLOT(setRepeat(int)));
    connect(m_plistgroup, SIGNAL(repeatModeChanged(int)), m_PlayerControls, SLOT(setRepeat(int)));

    connect(m_PlayerControls, SIGNAL(status(bool)), m_player, SLOT(setBroadcastStatus(bool)));

    m_bottomLayout->addWidget(m_PlayerControls);

    connect(m_PlayerControls, SIGNAL(prev()), m_plistgroup, SLOT(prevTrack()));
    connect(m_PlayerControls, SIGNAL(next()), m_plistgroup, SLOT(nextTrack()));
    connect(m_PlayerControls, SIGNAL(play()), m_player, SLOT(playPause()));
    connect(m_player, SIGNAL(playintState(bool)), m_PlayerControls, SLOT(setPlayingState(bool)));
    connect(m_PlayerControls, SIGNAL(seek(qint64)), m_player, SLOT(seek(qint64)));
    connect(m_PlayerControls, SIGNAL(volumeChanged(double)), m_player, SLOT(setVolume(double)));

    connect(m_player, SIGNAL(trackChanged(Track*)), m_PlayerControls, SLOT(onTrackChanged(Track*)));
}


void Pulsar::init()
{
    m_plistgroup = new PlistsGroup(this);
    connect(m_Settings, SIGNAL(changed()), m_plistgroup, SLOT(settingsChanged()));
    connect(this, SIGNAL(shuffleChanged(int)), m_plistgroup, SLOT(setShuffle(int)));
    connect(this, SIGNAL(repeatChanged(int)), m_plistgroup, SLOT(setRepeat(int)));



    m_player = new Player(this);
    connect(m_Settings, SIGNAL(changed()), m_player, SLOT(settingsChanged()));
    connect(m_player, SIGNAL(needTrack()), m_plistgroup, SLOT(nextTrack()));
    connect(m_player, SIGNAL(needLast()), m_plistgroup, SIGNAL(playLast()));
    connect(m_player, SIGNAL(newTitles(QString,QString)), this, SLOT(setTitles(QString,QString)));
    connect(m_plistgroup, SIGNAL(playTrack(Track*)), m_player, SLOT(setTrack(Track*)));

    m_ArtLoader = new ArtLoader();
    connect(m_player, SIGNAL(loadArt(QString)), m_ArtLoader, SLOT(loadArt(QString)));
    connect(m_player, SIGNAL(trackChanged()), m_ArtLoader, SLOT(clearArt()));

    m_Settings->update();

    // Tray icon
    Tray *tray = new Tray(this);
    connect(m_player, SIGNAL(playing()), tray, SLOT(setPlayIcon()));
    connect(m_player, SIGNAL(paused()), tray, SLOT(setPauseIcon()));
    connect(m_player, SIGNAL(newTitles(QString,QString)), tray, SLOT(setTitles(QString,QString)));

    connect(tray, SIGNAL(pause()), m_player, SLOT(playPause()));
    connect(tray, SIGNAL(next()), m_player, SLOT(next()));
    connect(tray, SIGNAL(prev()), m_player, SLOT(prev()));
    connect(tray, SIGNAL(showW()), SLOT(showWindow()));
    connect(tray, SIGNAL(quit()), SLOT(quit()));
}

bool Pulsar::eventFilter(QObject *target, QEvent *event)
{
    if(target == this) {
        //FIXME
        if(event->type() == QEvent::Show) {
            //QTimer::singleShot(200, m_Settings, SLOT(showDialog()));
        }//END
        if(event->type() == QEvent::Resize || event->type() == QEvent::Show) {
            updateWidgets();
        }
    }
    return false;
}

void Pulsar::updateWidgets()
{
    Q_EMIT resized(rect());
    m_Settings->setValue("mw/width", rect().width());
    m_Settings->setValue("mw/height", rect().height());
}

void Pulsar::quit()
{
    m_Settings->save();
    m_plistgroup->onExit();

    m_auth->cancelAuth();

    QApplication::exit(0);
}

void Pulsar::newLocalSocketConnection()
{
    // Someone want run app again... hmmm...
    raiseWindow();
}

void Pulsar::authError(Auth::Errors e)
{
    switch(e) {
    case Auth::BADLOGIN:
        QMessageBox::critical(this, QApplication::applicationName(), tr("Can't authenticate: bad login or password."));
        break;
    case Auth::NETWORK:
        QMessageBox::information(this, QApplication::applicationName(), tr("Can't authenticate: network error."));
        break;

    }
}

void Pulsar::setShuffle(int mode)
{
    Q_UNUSED(mode);
}

void Pulsar::setRepeat(int mode)
{
    if(mode > 0) {
        m_iRepeatMode = mode;
    } else {
        if(m_iRepeatMode < 3)
            m_iRepeatMode++;
        else
            m_iRepeatMode = 1;

    }

    switch(m_iRepeatMode) {
    case 1:
        m_wbRepeat->setIcon(QIcon(QPixmap(":icons/circle_empty")));
        m_wbRepeat->setChecked(false);

        break;
    case 2:
        m_wbRepeat->setIcon(QIcon(QPixmap(":icons/circle_one")));
        m_wbRepeat->setChecked(true);
        break;
    case 3:
        m_wbRepeat->setIcon(QIcon(QPixmap(":icons/circle_list")));
        m_wbRepeat->setChecked(true);
        break;
    }

    Q_EMIT repeatChanged(m_iRepeatMode);

}

void Pulsar::setTitles(QString artist, QString title)
{
    setWindowTitle(title + " - " + artist);
}

void Pulsar::showWindow()
{
    if(!isVisible()) {
        show();
    } else {
        if(isMinimized())
            setWindowState(Qt::WindowActive);
        else
            hide();
    }
}

void Pulsar::current2library()
{
    if(m_player->currentTrack())
        VkActions::instance()->addToLibrary(m_player->currentTrack());
}

void Pulsar::currentDownload()
{
    if(m_player->currentTrack())
        DownloadManager::instance()->add(m_player->currentTrack());
}

void Pulsar::raiseWindow()
{
    if(!isVisible())
        show();

    setWindowState(Qt::WindowActive);
    activateWindow();
}
