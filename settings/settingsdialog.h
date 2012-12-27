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

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include "settings.h"

#include <QDialog>
#include <QListWidget>
#include <QStackedWidget>
#include <QLabel>
#include <QGroupBox>
#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QRadioButton>
#include <QComboBox>
#include <QMap>
#include <QTreeView>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QEvent>

class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget *parent = 0);

private:
    Settings *m_settings;

    QListWidget *m_wPagesList;
    QStackedWidget *m_wPages;
    QList<QWidget*> m_pageWidgets;

    QStandardItemModel *m_ShortcusModel;
    QList <QMap <QString, QVariant> > m_shortcutList;


    bool m_bNeedRestart;
    bool m_bShortcutsChanged;

    // Controls
    QPushButton *m_wbOk;
    QPushButton *m_wbCancel;

    QCheckBox *m_wcAccount;
    QLineEdit *m_weLogin;
    QLineEdit *m_wePass;
    QLineEdit *m_wePath;
    QPushButton *m_wbChoosePath;
    QComboBox *m_wbLanguage;

    QRadioButton *m_wrTabsByType;
    QRadioButton *m_wrTabsByContent;
    QCheckBox *m_wcAutoloadLibrary;

    QTreeView *m_wtShortcuts;

    QCheckBox *m_wcPlayOnStart;

    QCheckBox *m_wcQuitOnClose;
    QCheckBox *m_wcHideTray;

    // Get settings from Settings object
    void load();

    void setupUi();
    void setupPages();
    void setupShortcuts();


private Q_SLOTS:
    void showDialog();
    void setCachePath();
    void saveAction();
    void onShortcutsChanged(QModelIndex, QModelIndex);

};

class ShortcutDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    ShortcutDelegate(QObject *parent = 0);

protected:
     bool eventFilter(QObject *object, QEvent *event);

};

#endif // SETTINGSDIALOG_H
