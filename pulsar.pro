#-------------------------------------------------
#
# Project created by QtCreator 2011-10-05T19:39:58
#
#-------------------------------------------------

QT        += core gui network script xml dbus

TARGET    = pulsar
TEMPLATE  = app

CONFIG    += link_pkgconfig qxt
PKGCONFIG += QtGStreamer-0.10 QtGStreamerUi-0.10
QXT       += core gui

include(translations/translations.pri)

SOURCES += main.cpp\
    playlist/playlist.cpp \
    playlist/playlistwidget.cpp \
    network/auth.cpp \
    network/parser.cpp \
    network/cookiejar.cpp \
    playlist/player.cpp \
    ui/about.cpp \
    settings/settings.cpp \
    settings/settingsdialog.cpp \
    playlist/track.cpp \
    playlist/plistsgroup.cpp \
    playlist/plistsgroupwidget.cpp \
    playlist/friendslist.cpp \
    playlist/friendslistwidget.cpp \
    network/vkactions.cpp \
    ui/pmessage.cpp \
    playlist/hints.cpp \
    ui/playercontrols.cpp \
    ui/styledbutton.cpp \
    playlist/musicdb.cpp \
    playlist/musicdbwidget.cpp \
    playlist/gstplayer.cpp \
    ui/songinfo.cpp \
    network/artloader.cpp \
    ui/subwindow.cpp \
    ui/dmwidget.cpp \
    network/downloadmanager.cpp \
    mpris2/mpris2.cpp \
    mpris2/mediaplayer2player.cpp \
    mpris2/mediaplayer2.cpp \
    ui/tray.cpp \
    ui/pulsar.cpp \
    network/lastfm.cpp \
    ui/styledwidget.cpp \
    ui/authwindow.cpp \
    ui/shortcutsmanager.cpp

HEADERS  += \
    playlist/playlist.h \
    playlist/playlistwidget.h \
    network/auth.h \
    network/parser.h \
    network/cookiejar.h \
    playlist/player.h \
    ui/about.h \
    settings/settings.h \
    settings/settingsdialog.h \
    playlist/track.h \
    playlist/plistsgroup.h \
    playlist/plistsgroupwidget.h \
    playlist/friendslist.h \
    playlist/friendslistwidget.h \
    network/vkactions.h \
    ui/pmessage.h \
    playlist/hints.h \
    ui/playercontrols.h \
    ui/styledbutton.h \
    playlist/musicdb.h \
    playlist/musicdbwidget.h \
    playlist/gstplayer.h \
    ui/songinfo.h \
    network/artloader.h \
    ui/subwindow.h \
    ui/dmwidget.h \
    network/downloadmanager.h \
    mpris2/mpris2.h \
    mpris2/mediaplayer2player.h \
    mpris2/mediaplayer2.h \
    ui/tray.h \
    ui/pulsar.h \
    network/lastfm.h \
    ui/styledwidget.h \
    ui/authwindow.h \
    ui/shortcutsmanager.h

RESOURCES += \
    data/data.qrc

WIN {
  RC_FILE = pulsar.rc
}














































