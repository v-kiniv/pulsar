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

#ifndef PULSAR_H
#define PULSAR_H

#include "network/auth.h"
#include "settings/settingsdialog.h"
#include "ui/pmessage.h"
#include "ui/playercontrols.h"
#include "ui/styledbutton.h"
#include "playlist/plistsgroup.h"
#include "playlist/player.h"
#include "network/artloader.h"
#include "ui/dmwidget.h"
#include "network/downloadmanager.h"
#include "ui/about.h"
#include "ui/authwindow.h"

#include <QMainWindow>
#include <QtGui>
#include <QLocalServer>

class Pulsar : public QMainWindow
{
    Q_OBJECT

public:
    explicit Pulsar(QWidget *parent = 0);
    ~Pulsar();

    // Return pointer to the Pulsar instance
    static Pulsar* instance();

protected:
     void closeEvent(QCloseEvent *event);

private:
    // Instance pointer
    static Pulsar *m_instance;

    QLocalServer *m_localServer;

    Auth *m_auth;
    Settings *m_Settings;
    SettingsDialog *m_settingsDialog;

    PlistsGroup *m_plistgroup;
    Player *m_player;

    int m_iShuffleMode;
    int m_iRepeatMode;

    /* GUI */

    // Middle layout: tabs, lists etc
    QVBoxLayout *m_mainLayout;
    QVBoxLayout *m_middleLayout;
    QVBoxLayout *m_bottomLayout;


    // Message widget
    PMessage *m_wMsg;

    // DownloadManager
    DownloadManager *m_DmManager;
    DmWidget *m_DmWidget;

    // Player controls widget
    PlayerControls *m_PlayerControls;

    // ArtCover loader
    ArtLoader *m_ArtLoader;

    // Menus
    QMenu *m_AppMenu;

    // Bottom controls group
    StyledButton *m_wbAppMenu;
    StyledButton *m_wbPrev;
    StyledButton *m_wbPlay;
    StyledButton *m_wbNext;
    StyledButton *m_wbShuffle;
    StyledButton *m_wbRepeat;
    StyledButton *m_wbStatus;

    // About
    About *m_wAbout;

    // Auth window for online services
    AuthWindow *m_wAuthWindow;

    // Setup menus
    void createMenus();

    // Start creating mainwindow gui
    void setupUi();

    // Setup bottom layout
    void setupBottomUi();

    // Register global & local shortcuts
    void registerShortcuts();

    // Init main objects
    void init();

    // Event filter
    bool eventFilter(QObject *target, QEvent *event);

    // Update widgets size&pos
    void updateWidgets();

Q_SIGNALS:
    void resized(QRect);
    void shuffleChanged(int);
    void repeatChanged(int);

public Q_SLOTS:
    void quit();
    void raiseWindow();

private Q_SLOTS:
    void newLocalSocketConnection();
    void authError(Auth::Errors);
    void setShuffle(int mode = 0);
    void setRepeat(int mode = 0);
    void setStatus(bool state);

    void setTitles(QString, QString);
    void showWindow();

    void current2library();
    void currentDownload();

    void test();

};

#endif // PULSAR_H
