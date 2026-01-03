#include "UserInterface.h"
#include <QtWidgets/QMainWindow>
#include <QString>

// Helper to convert std::string to QString
static QString QStringBuilder(const std::string& s)
{
    return QString::fromStdString(s);
}

UserInterface::UserInterface(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    connect(ui.pushButton, &QPushButton::clicked, this, &UserInterface::on_buttonClicked);
}

UserInterface::~UserInterface()
{
}

void UserInterface::on_buttonClicked()
{
    ui.pushButton->setStyleSheet("background-color: red");
    auto& gameState = Core::GameState::getInstance();
    std::shared_ptr<Core::Player> player1 = gameState.GetPlayer1();
    ui.pushButton->setText(QStringBuilder(player1->m_player->getPlayerUsername()));
}

// Ensure MOC is linked in non-Qt build systems
#include "moc_UserInterface.cpp"

