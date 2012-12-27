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

#include "about.h"
#include <QApplication>
#include <QGridLayout>
#include <QLabel>
#include <QDebug>

About::About(QWidget *parent) :
    StyledWidget(parent)
{
    installEventFilter(this);
    setupUi();
    hide();
}

bool About::eventFilter(QObject *target, QEvent *event)
{
    if(target == this) {
        if(event->type() == QEvent::MouseButtonPress) {
            hide();
            return true;
        }
    }
    return false;
}

void About::setupUi()
{
    setFixedSize(400, 300);

    QGridLayout *mLayout = new QGridLayout();
    setLayout(mLayout);
    QGridLayout *sLayout = new QGridLayout();
    mLayout->setMargin(10);

    QLabel *lLogo = new QLabel();
    lLogo->setPixmap(QPixmap(":/images/about_bg"));
    lLogo->setAlignment(Qt::AlignCenter);
    mLayout->addWidget(lLogo, 0,0);
    mLayout->addLayout(sLayout, 0, 0);
    mLayout->setAlignment(sLayout, Qt::AlignTop);

    QLabel *lAppName = new QLabel(QApplication::applicationName());
    lAppName->setStyleSheet("font-size: 20px;");
    lAppName->setAlignment(Qt::AlignTop|Qt::AlignHCenter);
    sLayout->addWidget(lAppName, 0,0);

    QLabel *lVersion = new QLabel(QString(tr("Version: %0").arg(QApplication::applicationVersion())));
    lVersion->setAlignment(Qt::AlignTop|Qt::AlignHCenter);
    sLayout->addWidget(lVersion, 1,0);

    //QSpacerItem *spacer = new QSpacerItem(0,30, QSizePolicy::Minimum, QSizePolicy::Fixed);
    sLayout->addItem(new QSpacerItem( 0, 30, QSizePolicy::Expanding, QSizePolicy::Fixed), 2, 0);

    QLabel *lAuthor = new QLabel(QString(tr("Author")));
    lAuthor->setAlignment(Qt::AlignTop|Qt::AlignHCenter);
    sLayout->addWidget(lAuthor, 3,0);

    QLabel *lAuthorR1 = new QLabel("Vasily Kiniv");
    lAuthorR1->setStyleSheet("font-weight: bold;");
    lAuthorR1->setAlignment(Qt::AlignTop|Qt::AlignHCenter);
    sLayout->addWidget(lAuthorR1, 4,0);
    QLabel *lAuthorR2 = new QLabel("<a href=\"mailto: yuberion@gmail.com\" style=\"color: #6FE7FF;\">yuberion@gmail.com</a>");
    lAuthorR2->setOpenExternalLinks(true);
    lAuthorR2->setAlignment(Qt::AlignTop|Qt::AlignHCenter);
    sLayout->addWidget(lAuthorR2, 5,0);
    QLabel *lAuthorR3 = new QLabel("<a href=\"http://twitter.com/yuberion\" style=\"color: #6FE7FF;\">@yuberion on Twitter</a>");
    lAuthorR3->setOpenExternalLinks(true);
    lAuthorR3->setAlignment(Qt::AlignTop|Qt::AlignHCenter);
    sLayout->addWidget(lAuthorR3, 6,0);

    sLayout->addItem(new QSpacerItem( 0, 10, QSizePolicy::Fixed, QSizePolicy::Fixed), 7, 0);

    QLabel *lDonate = new QLabel(tr("If you like this app, you can donate some money for its development")+"<br><a href=\"https://www.paypal.com/cgi-bin/webscr/?cmd=_s-xclick&hosted_button_id=EGUW4R7ZSMQAW&submit\" style=\"color: #6FE7FF;\">"+tr("Donate through PayPal")+"</a>");
    lDonate->setOpenExternalLinks(true);
    lDonate->setAlignment(Qt::AlignTop|Qt::AlignHCenter);
    lDonate->setWordWrap(true);
    lDonate->setStyleSheet("font-size: 8pt;");
    sLayout->addWidget(lDonate, 8,0);

    sLayout->addItem(new QSpacerItem( 0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding), 9, 0);

    QLabel *lLicense = new QLabel(QString(tr("The program is provided as is with no warranty of any kind, including the warranty of design, merchantability and fitness for a particular purpose.")));
    lLicense->setWordWrap(true);
    lLicense->setAlignment(Qt::AlignBottom|Qt::AlignHCenter);
    lLicense->setStyleSheet("font-size: 7pt;");
    sLayout->addWidget(lLicense, 10,0);
}

void About::showDialog()
{
    show();
}
