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

#include "settingsdialog.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardItem>
#include <QKeyEvent>
#include <QApplication>

#include <QDebug>


SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent)
{
    m_ShortcusModel = new QStandardItemModel(this);

    m_settings = Settings::instance();
    setupUi();
    setupShortcuts();
}

void SettingsDialog::load()
{
    m_bNeedRestart = false;
    m_bShortcutsChanged = false;

    // General
    m_wePath->setText(m_settings->getValue("general/music_path").toString());
    m_wbLanguage->setCurrentIndex(m_wbLanguage->findData(m_settings->getValue("general/language").toString()));

    // Account
    m_wcAccount->setChecked(m_settings->getValue("general/account_use").toBool());
    m_weLogin->setText(m_settings->getValue("general/account_login").toString());
    m_wePass->setText(m_settings->getValue("general/account_pass").toString());

    // Playlist
    m_wrTabsByContent->setChecked(m_settings->getValue("playlist/tabs_by_content").toBool());
    m_wcAutoloadLibrary->setChecked(m_settings->getValue("playlist/autoload_library").toBool());

    // Playing
    m_wcPlayOnStart->setChecked(m_settings->getValue("playing/play_on_start").toBool());

    // Window
    m_wcHideTray->setChecked(m_settings->getValue("window/hide_tray").toBool());
    m_wcQuitOnClose->setChecked(m_settings->getValue("window/quit_onclose").toBool());

    // Shortcuts
    for (int i = 0; m_shortcutList.size() > i; i++) {
        //qDebug() << m_shortcutList.at(i)["key"] << " : " << m_ShortcusModel->item(i, 1)->text();
        if(m_shortcutList.at(i)["local"].toBool())
            m_ShortcusModel->item(i, 1)->setText(m_settings->getValue("l_"+m_shortcutList.at(i)["key"].toString()).toString());

        if(m_shortcutList.at(i)["global"].toBool())
            m_ShortcusModel->item(i, 2)->setText(m_settings->getValue("g_"+m_shortcutList.at(i)["key"].toString()).toString());
    }


}

void SettingsDialog::setupUi()
{
    setWindowTitle(tr("Settings"));

    setFixedSize(600,500);
    setWindowModality(Qt::WindowModal);

    // Pages list

    m_wPagesList = new QListWidget(this);
    m_wPagesList->setMinimumWidth(150);
    m_wPagesList->setIconSize(QSize(48, 48));

    m_wPagesList->setStyleSheet("QListView { font-size: 14px; } QListWidget::item { height: 50px; }");

    QListWidgetItem *item1 = new QListWidgetItem(QIcon(QPixmap(":/icons/settings_general")), tr("General"));
    QListWidgetItem *item2 = new QListWidgetItem(QIcon(QPixmap(":/icons/settings_list")), tr("Playlist"));
    QListWidgetItem *item3 = new QListWidgetItem(QIcon(QPixmap(":/icons/settings_shortcuts")), tr("Shortcuts"));
    QListWidgetItem *item4 = new QListWidgetItem(QIcon(QPixmap(":/icons/settings_play")), tr("Playing"));
    QListWidgetItem *item5 = new QListWidgetItem(QIcon(QPixmap(":/icons/settings_window")), tr("Window"));
    m_wPagesList->addItem(item1);
    m_wPagesList->addItem(item2);
    m_wPagesList->addItem(item3);
    m_wPagesList->addItem(item4);
    m_wPagesList->addItem(item5);
//    item3->setHidden(true);

    // Setup page widgets
    setupPages();

    m_wPages = new QStackedWidget(this);
    for(int i = 0; i < m_pageWidgets.size(); i++) {
        m_wPages->addWidget(m_pageWidgets[i]);
    }
    m_wPages->setCurrentIndex(0);

    connect(m_wPagesList, SIGNAL(currentRowChanged(int)), m_wPages, SLOT(setCurrentIndex(int)));

    // OK CANCEL
    m_wbOk = new QPushButton(tr("&OK"));
    m_wbOk->setIcon(QIcon::fromTheme("dialog-ok"));
    m_wbOk->setMaximumWidth(70);

    connect(m_wbOk, SIGNAL(clicked()), SLOT(saveAction()));

    m_wbCancel = new QPushButton(tr("Ca&ncel"));
    m_wbCancel->setIcon(QIcon::fromTheme("dialog-cancel"));
    m_wbCancel->setMaximumWidth(100);

    connect(m_wbCancel, SIGNAL(clicked()), SLOT(close()));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QHBoxLayout *topLayout = new QHBoxLayout();
    QHBoxLayout *bottomLayout = new QHBoxLayout();

    bottomLayout->addStretch();
    bottomLayout->addWidget(m_wbOk);
    bottomLayout->addWidget(m_wbCancel);


    topLayout->addWidget(m_wPagesList);
    topLayout->addWidget(m_wPages);
    m_wPagesList->setFixedWidth(200);

    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(bottomLayout);
}

void SettingsDialog::setupPages()
{
    QString notesStyle = "QLabel {font-size: 8pt;}";
    QString notesStyleCbox = "QLabel {font-size: 8pt; padding-left: 25px;}";

    // Setup pages

    // GENERAL
    QWidget *wpGeneral = new QWidget(this);
    QVBoxLayout *lpGeneral = new QVBoxLayout(wpGeneral);
    wpGeneral->setLayout(lpGeneral);

    // Account group
    QGroupBox *wgAccount = new QGroupBox(tr("VK Account"));
    QVBoxLayout *lAccount = new QVBoxLayout(wgAccount);

    m_wcAccount = new QCheckBox(tr("Use own account"));
    m_weLogin = new QLineEdit();
    m_wePass = new QLineEdit();
    m_wePass->setMaximumWidth(200);
    m_weLogin->setMaximumWidth(200);
    m_wePass->setEchoMode(QLineEdit::Password);

    m_weLogin->setEnabled(false);
    m_wePass->setEnabled(false);


    connect(m_wcAccount, SIGNAL(toggled(bool)), m_weLogin, SLOT(setEnabled(bool)));
    connect(m_wcAccount, SIGNAL(toggled(bool)), m_wePass, SLOT(setEnabled(bool)));

    QLabel *wlLogin = new QLabel(tr("&Login"));
    wlLogin->setBuddy(m_weLogin);

    QLabel *wlPass = new QLabel(tr("&Password"));
    wlPass->setBuddy(m_wePass);

    wlLogin->setEnabled(false);
    wlPass->setEnabled(false);

    connect(m_wcAccount, SIGNAL(toggled(bool)), wlLogin, SLOT(setEnabled(bool)));
    connect(m_wcAccount, SIGNAL(toggled(bool)), wlPass, SLOT(setEnabled(bool)));

    QLabel *wlAccountNotes = new QLabel(tr("<b>Note:</b> To use all the features and types of playlists, select your own account."));
    wlAccountNotes->setWordWrap(true);
    wlAccountNotes->setStyleSheet(notesStyle);

    lAccount->addWidget(m_wcAccount);
    lAccount->addWidget(wlLogin);
    lAccount->addWidget(m_weLogin);
    lAccount->addWidget(wlPass);
    lAccount->addWidget(m_wePass);
    lAccount->addSpacing(10);
    lAccount->addWidget(wlAccountNotes);
    lpGeneral->addWidget(wgAccount);

    // Cache path group
    QGroupBox *wgPath = new QGroupBox(tr("Music folder"));
    QGridLayout *lPath = new QGridLayout(wgPath);
    m_wePath = new QLineEdit();
    m_wbChoosePath = new QPushButton(tr("Choose"));
    connect(m_wbChoosePath, SIGNAL(clicked()), SLOT(setCachePath()));
    QLabel *wlPathNotes = new QLabel(tr("Path to directory where application saves the music"));
    wlPathNotes->setWordWrap(true);
    wlPathNotes->setStyleSheet(notesStyle);

    lPath->addWidget(m_wePath, 0, 0);
    lPath->addWidget(m_wbChoosePath, 0, 1);
    //lPath->addSpacing(10,);
    lPath->addWidget(wlPathNotes, 1, 0);
    lpGeneral->addWidget(wgPath);

    // Language
    QGroupBox *wgOthers = new QGroupBox(tr("Others"));
    QVBoxLayout *lOthers = new QVBoxLayout(wgOthers);

    m_wbLanguage = new QComboBox();
    m_wbLanguage->addItem(tr("Auto"), "auto");
    m_wbLanguage->addItem(tr("English"), "en");
    m_wbLanguage->addItem(tr("Ukrainian"), "uk");
    m_wbLanguage->addItem(tr("Russian"), "ru");

    QLabel *wlLanguage = new QLabel(tr("&Language"));
    wlLanguage->setBuddy(m_wbLanguage);

    lOthers->addWidget(wlLanguage);
    lOthers->addWidget(m_wbLanguage);
    lpGeneral->addWidget(wgOthers);

    // Set page
    lpGeneral->addStretch();
    m_pageWidgets << wpGeneral;

    // PLAYLISTS
    QWidget *wpPlaylist = new QWidget(this);
    QVBoxLayout *lpPlaylist = new QVBoxLayout(wpPlaylist);

    // Tabs
    QGroupBox *wgTabs = new QGroupBox(tr("Playlist tabs"));
    QVBoxLayout *lTabs = new QVBoxLayout(wgTabs);

    m_wrTabsByType = new QRadioButton(tr("Title by type"));
    m_wrTabsByContent = new QRadioButton(tr("Title by content"));
    m_wrTabsByContent->setChecked(true);

    lTabs->addWidget(m_wrTabsByContent);
    lTabs->addWidget(m_wrTabsByType);

    // Tracks
    QGroupBox *wgLibrary = new QGroupBox(tr("Library playlist"));
    //wgTracks->setVisible(false);
    QVBoxLayout *lLibrary = new QVBoxLayout(wgLibrary);

    m_wcAutoloadLibrary = new QCheckBox(tr("Load your library"));
    QLabel *wlAutoloadLibrary = new QLabel(tr("Load your library on create Library playlist"));
    wlAutoloadLibrary->setStyleSheet(notesStyleCbox);
    wlAutoloadLibrary->setWordWrap(true);

    lLibrary->addWidget(m_wcAutoloadLibrary);
    lLibrary->addWidget(wlAutoloadLibrary);


    wpPlaylist->setLayout(lpPlaylist);
    lpPlaylist->addWidget(wgTabs);
    lpPlaylist->addWidget(wgLibrary);
    lpPlaylist->addStretch();

    m_pageWidgets << wpPlaylist;


    // SHORTCUTS
    QWidget *wpScuts = new QWidget(this);
    QVBoxLayout *lpScuts = new QVBoxLayout(wpScuts);

    ShortcutDelegate *shortcutDelegate = new ShortcutDelegate(this);

    m_wtShortcuts = new QTreeView();
    m_wtShortcuts->setRootIsDecorated(false);
    m_wtShortcuts->setAlternatingRowColors(true);
    m_wtShortcuts->setModel(m_ShortcusModel);
    m_wtShortcuts->setItemDelegateForColumn(1, shortcutDelegate);
    m_wtShortcuts->setItemDelegateForColumn(2, shortcutDelegate);


    lpScuts->addWidget(m_wtShortcuts);

    m_pageWidgets << wpScuts;

    // PLAYING
    QWidget *wpPlay = new QWidget(this);
    QVBoxLayout *lpPlay = new QVBoxLayout(wpPlay);

    QGroupBox *wgBehavior = new QGroupBox(tr("Behavior"));
    QVBoxLayout *lBehavior = new QVBoxLayout(wgBehavior);

    m_wcPlayOnStart = new QCheckBox(tr("Resume playing on start"));
    QLabel *wlPlayOnStart = new QLabel(tr("Play last track from previous session, when application is starting"));
    wlPlayOnStart->setStyleSheet(notesStyleCbox);
    wlPlayOnStart->setWordWrap(true);

    lBehavior->addWidget(m_wcPlayOnStart);
    lBehavior->addWidget(wlPlayOnStart);

    lpPlay->addWidget(wgBehavior);
    lpPlay->addStretch();
    m_pageWidgets << wpPlay;

    // WINDOW
    QWidget *wpWindow = new QWidget(this);
    QVBoxLayout *lpWindow = new QVBoxLayout(wpWindow);

    // Main window
    QGroupBox *wgMainWidow = new QGroupBox(tr("Main Window"));
    QVBoxLayout *lMainWidow = new QVBoxLayout(wgMainWidow);

    m_wcQuitOnClose = new QCheckBox(tr("Quit when closed"));
    QLabel *wlQuitNote = new QLabel(tr("Quit from application when main window closes"));
    wlQuitNote->setStyleSheet(notesStyleCbox);
    wlQuitNote->setWordWrap(true);

    lMainWidow->addWidget(m_wcQuitOnClose);
    lMainWidow->addWidget(wlQuitNote);

    // Tray
    QGroupBox *wgTray = new QGroupBox(tr("Tray"));
    QVBoxLayout *lpTray = new QVBoxLayout(wgTray);

    m_wcHideTray = new QCheckBox(tr("Hide tray icon"));

    lpTray->addWidget(m_wcHideTray);


    lpWindow->addWidget(wgMainWidow);
    lpWindow->addWidget(wgTray);
    lpWindow->addStretch();

    m_pageWidgets << wpWindow;



}

void SettingsDialog::setupShortcuts()
{
    QStringList headers;
    headers << tr("Action") << tr("Local") << tr("Global");
    m_ShortcusModel->setHorizontalHeaderLabels(headers);

//    QList <QMap <QString, QVariant> > shortcutList;
    QMap <QString, QVariant> shortcut;

    shortcut["label"] = tr("Play/Pause");
    shortcut["key"] = "shortcut/playpause";
    shortcut["local"] = true;
    shortcut["global"] = true;
    m_shortcutList << shortcut;

    shortcut["label"] = tr("Next track");
    shortcut["key"] = "shortcut/next";
    shortcut["local"] = true;
    shortcut["global"] = true;
    m_shortcutList << shortcut;

    shortcut["label"] = tr("Previous track");
    shortcut["key"] = "shortcut/prev";
    shortcut["local"] = true;
    shortcut["global"] = true;
    m_shortcutList << shortcut;

    shortcut["label"] = tr("Shuffle");
    shortcut["key"] = "shortcut/shuffle";
    shortcut["local"] = true;
    shortcut["global"] = true;
    m_shortcutList << shortcut;

    shortcut["label"] = tr("Repeat");
    shortcut["key"] = "shortcut/repeat";
    shortcut["local"] = true;
    shortcut["global"] = true;
    m_shortcutList << shortcut;

    shortcut["label"] = tr("Show window");
    shortcut["key"] = "shortcut/showwindow";
    shortcut["local"] = false;
    shortcut["global"] = true;
    m_shortcutList << shortcut;

    shortcut["label"] = tr("Add current track to Library");
    shortcut["key"] = "shortcut/current2library";
    shortcut["local"] = true;
    shortcut["global"] = true;
    m_shortcutList << shortcut;

    shortcut["label"] = tr("Download current track");
    shortcut["key"] = "shortcut/current_download";
    shortcut["local"] = true;
    shortcut["global"] = true;
    m_shortcutList << shortcut;


    Q_FOREACH (shortcut, m_shortcutList) {
        QList<QStandardItem *> items;

        QStandardItem *label = new QStandardItem(shortcut["label"].toString());
        label->setToolTip(shortcut["label"].toString());
        QStandardItem *local = new QStandardItem();
        QStandardItem *global = new QStandardItem();
        local->setEnabled(shortcut["local"].toBool());
        global->setEnabled(shortcut["global"].toBool());

        items << label << local << global;
        m_ShortcusModel->appendRow(items);
    }

    m_wtShortcuts->resizeColumnToContents(0);
}

void SettingsDialog::showDialog()
{
    load();
    show();
    connect(m_ShortcusModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(onShortcutsChanged(QModelIndex,QModelIndex)));
}

void SettingsDialog::setCachePath()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Choose Directory"), m_wePath->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if(dir != "")
        m_wePath->setText(dir);
}

void SettingsDialog::saveAction()
{
    for (int i = 0; m_shortcutList.size() > i; i++) {
        m_settings->setValue("l_"+m_shortcutList.at(i)["key"].toString(), m_ShortcusModel->item(i, 1)->text());
        m_settings->setValue("g_"+m_shortcutList.at(i)["key"].toString(), m_ShortcusModel->item(i, 2)->text());
    }

    if(m_settings->getValue("general/account_use").toBool() != m_wcAccount->isChecked() ||
       m_settings->getValue("general/account_login").toString() != m_weLogin->text() ||
       m_settings->getValue("general/account_pass").toString() != m_wePass->text()) {
        m_bNeedRestart = true;
    }

    m_settings->setValue("general/account_use", m_wcAccount->isChecked());
    m_settings->setValue("general/account_login", m_weLogin->text());
    m_settings->setValue("general/account_pass", m_wePass->text());
    m_settings->setValue("general/language", m_wbLanguage->itemData(m_wbLanguage->currentIndex()).toString());

    m_settings->setValue("general/music_path", m_wePath->text());

    m_settings->setValue("playlist/tabs_by_content", m_wrTabsByContent->isChecked());
    m_settings->setValue("playlist/autoload_library", m_wcAutoloadLibrary->isChecked());

    m_settings->setValue("playing/play_on_start", m_wcPlayOnStart->isChecked());

    m_settings->setValue("window/hide_tray", m_wcHideTray->isChecked());
    m_settings->setValue("window/quit_onclose", m_wcQuitOnClose->isChecked());

    m_settings->save();

    if(m_bShortcutsChanged)
        m_settings->updateShortcuts();

    hide();

    if(m_bNeedRestart) {
        QMessageBox::information(this, tr("Pulsar"), tr("Restart application for apply changes."));
        m_settings->onAccountChanged();
    }
}

void SettingsDialog::onShortcutsChanged(QModelIndex, QModelIndex)
{
    m_bShortcutsChanged = true;
}

ShortcutDelegate::ShortcutDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{

}

bool ShortcutDelegate::eventFilter(QObject *object, QEvent *event)
{
    QLineEdit *lEdit = static_cast<QLineEdit*>(object);
    static QString oldKs;

    if(event->type() == QEvent::KeyPress) {
        QKeySequence ks;
        QList<int> modifiers_list;
        modifiers_list << Qt::Key_Shift << Qt::Key_Control << Qt::Key_Meta
                         << Qt::Key_Alt << Qt::Key_AltGr;


        QKeyEvent* ke = static_cast<QKeyEvent*>(event);

        if(ke->key() == Qt::Key_Return) {
            oldKs = lEdit->text();
            this->commitData(lEdit);
            this->closeEditor(lEdit);
            return true;
        }

        if(ke->key() == Qt::Key_Backspace) {
            lEdit->clear();
            return true;
        }

        if(ke->key() == Qt::Key_Escape) {
            this->closeEditor(lEdit);
            return true;
        }

        if (modifiers_list.contains(ke->key()))
          ks = QKeySequence(ke->modifiers());
        else
            if (ke->modifiers() & Qt::KeypadModifier)
                ks = QKeySequence(ke->key());
            else
                ks = QKeySequence(ke->modifiers() | ke->key());


        lEdit->setText(ks.toString());
        return true;
    }

    if(event->type() == QEvent::FocusIn) {
        oldKs = lEdit->text();
    }
    if(event->type() == QEvent::FocusOut) {
        lEdit->setText(oldKs);
        this->closeEditor(lEdit);
    }
    return false;
}
