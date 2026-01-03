#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_UserInterface.h"
import GameState;

class UserInterface : public QMainWindow
{
    Q_OBJECT

public:
    UserInterface(QWidget *parent = nullptr);
    ~UserInterface();

private slots:
	void on_buttonClicked();

private:
    Ui::UserInterfaceClass ui;
};

