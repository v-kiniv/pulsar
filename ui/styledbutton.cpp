#include "styledbutton.h"

StyledButton::StyledButton (QWidget *parent) :
    QPushButton(parent)
{
    setupUi();
}

StyledButton::StyledButton(const QString &text, QWidget *parent) :
    QPushButton(parent)
{
    QPushButton::setText(text);

    setupUi();
}

StyledButton::StyledButton(const QIcon& icon, const QString &text, QWidget *parent) :
    QPushButton(parent)
{
    QPushButton::setIcon(icon);
    QPushButton::setText(text);

    setupUi();
}

void StyledButton::setTransparent(const bool &s)
{
    if(s) {
        QString buttonStyle = "QPushButton { background: none; opacity: 0.1; border: none; padding: 3px;} \
                QPushButton:hover { /*opacity: 1;  padding-bottom: 4px;*/} \
                QPushButton:checked { padding-bottom: 2px; } \
                QPushButton:pressed { padding-bottom: 1px;} \
                QPushButton::menu-indicator { width: 0px; height: 0px; background-color: rgba(55, 55, 55, 1);} \
               ";

        setStyleSheet(buttonStyle);
    } else {
        setupUi();
    }
}

void StyledButton::setupUi()
{
    QString buttonStyle = "QPushButton { border-radius: 2px; background-color: #777777; color: #fff; padding: 5px; font-size: 11px; border-bottom: 2px solid #45636D; } \
            QPushButton:hover { background-color: #8A8A8A; } \
            QPushButton:checked { color: #A3E7FF; border-bottom: 1px solid #45636D; padding-bottom: 3px; } \
            QPushButton:pressed { border-bottom: 1px solid #45636D; padding-bottom: 3px;} \
            QPushButton::menu-indicator { width: 0px; height: 0px; background-color: rgba(55, 55, 55, 1);} \
           ";

    setStyleSheet(buttonStyle);
}
