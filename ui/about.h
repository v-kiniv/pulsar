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

#ifndef ABOUT_H
#define ABOUT_H

#include "ui/styledwidget.h"

class About : public StyledWidget
{
    Q_OBJECT
public:
    explicit About(QWidget *parent = 0);
private:
    bool eventFilter(QObject *target, QEvent *event);
    void setupUi();

Q_SIGNALS:

public Q_SLOTS:
    void showDialog();

};

#endif // ABOUT_H
