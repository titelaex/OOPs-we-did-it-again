#include "PlayerPanelWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QScrollArea>
#include <QPalette>
#include <QMap>


QString PlayerPanelWidget::QStringBuilder(const std::string& s) const {
    return QString::fromStdString(s);
}

QString PlayerPanelWidget::ColorToCss(Models::ColorType c) const {
    switch (c) {
    case Models::ColorType::BROWN:  return "#A0522D";
    case Models::ColorType::GREY:   return "#808080";
    case Models::ColorType::RED:    return "#DC2626";
    case Models::ColorType::YELLOW: return "#F59E0B";
    case Models::ColorType::GREEN:  return "#10B981";
    case Models::ColorType::BLUE:   return "#3B82F6";
    case Models::ColorType::PURPLE: return "#8B5CF6";
    default: return "#6B7280"; // gray
    }
}

PlayerPanelWidget::PlayerPanelWidget(std::shared_ptr<Core::Player> player, QWidget* parent): QWidget(parent), m_player(player)
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(8, 8, 8, 8);
    m_layout->setSpacing(10);
    buildUi();
}

void PlayerPanelWidget::buildUi()
{

    QString username = m_player && m_player->m_player? QStringBuilder(m_player->m_player->getPlayerUsername()): QString("<unknown>");

    auto usernameLbl = new QLabel(username, this);
    usernameLbl->setStyleSheet("font-size:18px; font-weight: bold; margin-bottom:6px;");
    m_layout->addWidget(usernameLbl);

    addStatsRow();
    addCardSections();
	addWonderSection();

    m_layout->addStretch(1);
}
void PlayerPanelWidget::addWonderSection()
{
    QWidget* wondersSection = new QWidget(this);
    auto* wondersGrid = new QGridLayout(wondersSection);
    wondersGrid->setContentsMargins(4, 4, 4, 4);
    wondersGrid->setSpacing(8);

    auto title = new QLabel("Wonders:", wondersSection);
    title->setStyleSheet("font-weight: bold; margin-right:6px;");
   
    wondersGrid->addWidget(title, 0, 0, 1, 2);

    bool hasPlayer = (m_player && m_player->m_player);

    for (int i = 0; i < 4; ++i) {
        auto slot = new QLabel(wondersSection);
        slot->setAlignment(Qt::AlignCenter);
        slot->setFixedSize(150, 70);

        // Logic to place two per row:
        // row starts at 1 (because title is row 0). 
        // i=0,1 -> Row 1 | i=2,3 -> Row 2
        int row = (i / 2) + 1;
        int col = i % 2;

        QString emptyStyle = "background-color: rgba(255,255,255,0.04); border:1px solid #374151; border-radius:6px; color:#9CA3AF; font-style:italic;";

        if (hasPlayer) {
            auto& wonders = m_player->m_player->getOwnedWonders();
            if (static_cast<size_t>(i) < wonders.size() && wonders[i]) {
                const auto* w = wonders[i].get();
                QString name = QStringBuilder(w->getName());
                bool built = w->IsConstructed();
                slot->setText(name + (built ? "\n(constructed)" : "\n(unbuilt)"));

                QString bg = ColorToCss(w->getColor());
                QString style = QString(
                    "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 %1, stop:1 rgba(0,0,0,0.15));"
                    "border:1px solid #111; border-radius:6px; color:#fff; font-weight:600;")
                    .arg(bg);

                if (built) style += "opacity:0.8;";
                slot->setStyleSheet(style);
                slot->setToolTip(name);
            }
            else {
                slot->setText("<empty>");
                slot->setStyleSheet(emptyStyle);
            }
        }
        else {
            slot->setText("<empty>");
            slot->setStyleSheet(emptyStyle);
        }

        wondersGrid->addWidget(slot, row, col);
    }

    m_layout->addWidget(wondersSection);
}

void PlayerPanelWidget::addStatsRow()
{
    uint8_t coinsVal = 0;
    int vpVal = 0;
    if (m_player && m_player->m_player) {
        auto coinsTuple = m_player->m_player->getRemainingCoins();
        coinsVal = m_player->m_player->totalCoins(coinsTuple);
        vpVal = static_cast<int>(m_player->m_player->getTotalVictoryPoints());
    }

    auto statsRow = new QHBoxLayout();
    statsRow->setSpacing(12);

    auto coinsLbl = new QLabel(QString::number(coinsVal), this);
    coinsLbl->setAlignment(Qt::AlignCenter);
    coinsLbl->setFixedSize(40, 40);
    coinsLbl->setStyleSheet("background-color: #FFD700; color: #333; "
        "border:2px solid #B8860B; border-radius:20px; font-weight: bold;");

    auto vpLbl = new QLabel(QString::number(vpVal), this);
    vpLbl->setAlignment(Qt::AlignCenter);
    vpLbl->setFixedSize(40, 40);
    vpLbl->setStyleSheet("background-color: #6EE7B7; color: #1F2937; "
        "border:2px solid #10B981; border-radius:20px; font-weight: bold;");

    statsRow->addWidget(new QLabel("Coins:", this));
    statsRow->addWidget(coinsLbl);
    statsRow->addSpacing(8);
    statsRow->addWidget(new QLabel("VP:", this));
    statsRow->addWidget(vpLbl);
    statsRow->addStretch(1);

    m_layout->addLayout(statsRow);
}

void PlayerPanelWidget::addCardSections()
{
    QMap<int, QStringList> groups;
    if (m_player && m_player->m_player) {
        const auto& owned = m_player->m_player->getOwnedCards();
        for (const auto& uptr : owned) {
            if (!uptr) continue;
            Models::ColorType col = uptr->getColor();
            groups[static_cast<int>(col)].push_back(QStringBuilder(uptr->getName()));
        }
    }

    auto addSection = [&](Models::ColorType col) {
        QWidget* section = new QWidget(this);
        auto sectionLayout = new QVBoxLayout(section);
        sectionLayout->setContentsMargins(8, 8, 8, 8);
        sectionLayout->setSpacing(6);

        QString bg = this->palette().color(QPalette::Window).name();
        QString sectionStyle = QString(
            "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 %1,stop:0.05 %1, stop:0.2 %2);"
            "border:1px solid #374151; border-radius:6px;"
        ).arg(ColorToCss(col)).arg(bg);
        section->setStyleSheet(sectionStyle);

        auto list = new QListWidget(section);
        QString itemStyle = QString(
            "QListWidget::item { color: white; padding:4px; margin-bottom:2px; }"
            "QListWidget::item:selected { background-color: rgba(255,255,255,0.1); }"
        );
        list->setStyleSheet(itemStyle);

        for (const auto& name : groups.value(static_cast<int>(col)))
            list->addItem(name);

        sectionLayout->addWidget(list);
        m_layout->addWidget(section);
        };

    addSection(Models::ColorType::BROWN);
    addSection(Models::ColorType::GREY);
    addSection(Models::ColorType::RED);
    addSection(Models::ColorType::YELLOW);
    addSection(Models::ColorType::GREEN);
    addSection(Models::ColorType::BLUE);
    addSection(Models::ColorType::PURPLE);
}