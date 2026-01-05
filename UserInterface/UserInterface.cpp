import Models.ColorType;
import Models.Player;
#include "UserInterface.h"
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtCore/QMap>
#include <QString>

// Helper to convert std::string to QString
static QString QStringBuilder(const std::string& s)
{
 return QString::fromStdString(s);
}

static QString ColorHeader(Models::ColorType c)
{
 switch (c) {
 case Models::ColorType::BROWN: return "BROWN";
 case Models::ColorType::GREY: return "GREY";
 case Models::ColorType::RED: return "RED";
 case Models::ColorType::YELLOW: return "YELLOW";
 case Models::ColorType::GREEN: return "GREEN";
 case Models::ColorType::BLUE: return "BLUE";
 case Models::ColorType::PURPLE: return "PURPLE";
 default: return "UNKNOWN";
 }
}

static QString ColorToCss(Models::ColorType c)
{
 switch (c) {
 case Models::ColorType::BROWN: return "#A0522D"; // sienna
 case Models::ColorType::GREY: return "#808080";
 case Models::ColorType::RED: return "#DC2626";
 case Models::ColorType::YELLOW: return "#F59E0B";
 case Models::ColorType::GREEN: return "#10B981";
 case Models::ColorType::BLUE: return "#3B82F6";
 case Models::ColorType::PURPLE: return "#8B5CF6";
 default: return "#6B7280"; // gray
 }
}

UserInterface::UserInterface(QWidget* parent)
 : QMainWindow(parent)
{
 ui.setupUi(this);

 // Build right-side panel
 auto splitter = new QSplitter(this);
 QWidget* central = this->centralWidget();
 if (!central) {
 central = new QWidget(this);
 setCentralWidget(central);
 }

 splitter->addWidget(central);

 auto rightPanel = new QWidget(splitter);
 auto rightLayout = new QVBoxLayout(rightPanel);

 auto& gameState = Core::GameState::getInstance();
 std::shared_ptr<Core::Player> player1 = gameState.GetPlayer1();

 // Username header
 QString username = player1 && player1->m_player ? QStringBuilder(player1->m_player->getPlayerUsername()) : QString("<unknown>");
 auto usernameLbl = new QLabel(username, rightPanel);
 usernameLbl->setStyleSheet("font-size:18px; font-weight: bold; margin-bottom:6px;");
 rightLayout->addWidget(usernameLbl);

 // Coins and Victory Points row with circular indicators
 uint8_t coinsVal =0;
 int vpVal =0;
 if (player1 && player1->m_player) {
 auto coinsTuple = player1->m_player->getRemainingCoins();
 coinsVal = player1->m_player->totalCoins(coinsTuple);
 vpVal = static_cast<int>(player1->m_player->getTotalVictoryPoints());
 }

 auto statsRow = new QHBoxLayout();
 statsRow->setSpacing(12);

 auto coinsLbl = new QLabel(QString::number(coinsVal), rightPanel);
 coinsLbl->setAlignment(Qt::AlignCenter);
 coinsLbl->setFixedSize(40,40);
 coinsLbl->setStyleSheet("background-color: #FFD700; color: #333; border:2px solid #B8860B; border-radius:20px; font-weight: bold;");

 auto vpLbl = new QLabel(QString::number(vpVal), rightPanel);
 vpLbl->setAlignment(Qt::AlignCenter);
 vpLbl->setFixedSize(40,40);
 vpLbl->setStyleSheet("background-color: #6EE7B7; color: #1F2937; border:2px solid #10B981; border-radius:20px; font-weight: bold;");

 statsRow->addWidget(new QLabel("Coins:", rightPanel));
 statsRow->addWidget(coinsLbl);
 statsRow->addSpacing(8);
 statsRow->addWidget(new QLabel("VP:", rightPanel));
 statsRow->addWidget(vpLbl);
 statsRow->addStretch(1);
 rightLayout->addLayout(statsRow);

 // Group cards by color
 QMap<int, QStringList> groups; // key by int(ColorType)
 if (player1 && player1->m_player) {
 const auto& owned = player1->m_player->getOwnedCards();
 for (const auto& uptr : owned) {
 if (!uptr) continue;
 Models::ColorType col = uptr->getColor();
 groups[static_cast<int>(col)].push_back(QStringBuilder(uptr->getName()));
 }
 }

 auto addSection = [&](Models::ColorType col) {
 // colored square instead of text header
 auto headerRow = new QHBoxLayout();
 headerRow->setAlignment(Qt::AlignLeft);
 auto colorBox = new QWidget(rightPanel);
 colorBox->setFixedSize(16,16);
 colorBox->setStyleSheet(QString("background-color: %1; border:1px solid #374151; border-radius:2px;").arg(ColorToCss(col)));
 headerRow->addWidget(colorBox);
 headerRow->addStretch(1);
 rightLayout->addLayout(headerRow);

 auto list = new QListWidget(rightPanel);
 for (const auto& name : groups.value(static_cast<int>(col))) list->addItem(name);
 rightLayout->addWidget(list);
 };

 addSection(Models::ColorType::BROWN);
 addSection(Models::ColorType::GREY);
 addSection(Models::ColorType::RED);
 addSection(Models::ColorType::YELLOW);
 addSection(Models::ColorType::GREEN);
 addSection(Models::ColorType::BLUE);
 addSection(Models::ColorType::PURPLE);

 rightLayout->addStretch(1);

 auto scroll = new QScrollArea(splitter);
 scroll->setWidgetResizable(true);
 scroll->setWidget(rightPanel);

 splitter->addWidget(scroll);
 splitter->setStretchFactor(0,3);
 splitter->setStretchFactor(1,2);

 setCentralWidget(splitter);
}

UserInterface::~UserInterface()
{
}




