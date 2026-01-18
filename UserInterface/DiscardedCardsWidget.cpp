#include "DiscardedCardsWidget.h"
#include "CardSpineDelegate.h" 
#include <QListWidgetItem>

import Core.IGameListener;

namespace {
    QString getHexColorForCard(const QString& colorName) {
        if (colorName.compare("BROWN", Qt::CaseInsensitive) == 0) return "#7C4A1C";
        if (colorName.compare("GREY", Qt::CaseInsensitive) == 0) return "#374151";
        if (colorName.compare("RED", Qt::CaseInsensitive) == 0) return "#7F1D1D";
        if (colorName.compare("YELLOW", Qt::CaseInsensitive) == 0) return "#FCD34D";
        if (colorName.compare("GREEN", Qt::CaseInsensitive) == 0) return "#065F46";
        if (colorName.compare("BLUE", Qt::CaseInsensitive) == 0) return "#0369A1";
        if (colorName.compare("PURPLE", Qt::CaseInsensitive) == 0) return "#4C1D95";
        return "#374151"; 
    }
}

class DiscardObserver : public Core::IGameListener {
public:
    DiscardObserver(DiscardedCardsWidget* widget) : m_widget(widget) {}

 
    void onCardSold(const Core::CardEvent& event) override {
        QMetaObject::invokeMethod(m_widget, [this, event]() {
            m_widget->addDiscardedCard(
                QString::fromStdString(event.cardName),
                QString::fromStdString(event.cardColor)
            );
            });
    }

    void onCardDiscarded(const Core::CardEvent& event) override {
        QMetaObject::invokeMethod(m_widget, [this, event]() {
            m_widget->addDiscardedCard(
                QString::fromStdString(event.cardName),
                QString::fromStdString(event.cardColor)
            );
            });
    }

    void onCardBuilt(const Core::CardEvent&) override {}
    void onWonderBuilt(const Core::WonderEvent&) override {}
    void onTreeNodeChanged(const Core::TreeNodeEvent&) override {}
    void onTreeNodeEmptied(const Core::TreeNodeEvent&) override {}
    void onResourceGained(const Core::ResourceEvent&) override {}
    void onResourceLost(const Core::ResourceEvent&) override {}
    void onCoinsChanged(const Core::CoinEvent&) override {}
    void onTokenAcquired(const Core::TokenEvent&) override {}
    void onPawnMoved(const Core::PawnEvent&) override {}
    void onTurnStarted(const Core::TurnEvent&) override {}
    void onTurnEnded(const Core::TurnEvent&) override {}
    void onPhaseChanged(const Core::PhaseEvent&) override {}
    void onRoundChanged(int, int) override {}
    void onVictoryAchieved(const Core::VictoryEvent&) override {}
    void onGameStarted(int, Core::Playstyle, Core::Playstyle) override {}
    void onGameEnded() override {}
    void onPointsChanged(const Core::PointsEvent&) override {}
    void onPlayerStateChanged(int) override {}

    void onDisplayRequested(const Core::DisplayRequestEvent&) override {}

    void displayGameModeMenu() override {}
    void displayPlaystyleMenu(const std::string&) override {}
    void displayAvailableSaves(const std::vector<int>&) override {}
    void displayAvailableCards(const std::deque<std::reference_wrapper<Models::Card>>&) override {}
    void displayWonderList(const std::vector<std::reference_wrapper<Models::Wonder>>&) override {}
    void displayPlayerHands(const Core::Player&, const Core::Player&) override {}
    void displayTurnStatus(const Core::Player&, const Core::Player&) override {}
    void displayBoard() override {}
    void displayMessage(const std::string&) override {}
    void displayError(const std::string&) override {}
    void displayWarning(const std::string&) override {}

    virtual void displayCardInfo(const Models::Card& card) override {}
    virtual void displayWonderInfo(const Models::Wonder& wonder) override {}
    virtual void displayAgeCardInfo(const Models::AgeCard& ageCard) override {}

private:
    DiscardedCardsWidget* m_widget;
};

DiscardedCardsWidget::DiscardedCardsWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
    m_listener = std::make_shared<DiscardObserver>(this);
}

DiscardedCardsWidget::~DiscardedCardsWidget() {}

std::shared_ptr<Core::IGameListener> DiscardedCardsWidget::getListener()
{
    return m_listener;
}

void DiscardedCardsWidget::setupUi()
{
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 5, 0, 0);
    layout->setSpacing(2);

    m_titleLabel = new QLabel("Discarded Cards", this);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setStyleSheet("font-weight: bold; color: #ccc; font-size: 11px; text-transform: uppercase;");
    layout->addWidget(m_titleLabel);

    m_cardsList = new QListWidget(this);
    m_cardsList->setViewMode(QListWidget::IconMode);
    m_cardsList->setFlow(QListWidget::LeftToRight);
    m_cardsList->setWrapping(false); 
    m_cardsList->setSpacing(8);
    m_cardsList->setStyleSheet("QListWidget { background-color: rgba(20, 20, 20, 100); border: 1px solid #444; border-radius: 4px; }");
    m_cardsList->setFixedHeight(120);

    m_cardsList->setItemDelegate(new CardSpineDelegate(m_cardsList));

    layout->addWidget(m_cardsList);
}

void DiscardedCardsWidget::addDiscardedCard(const QString& cardName, const QString& cardColor)
{
    QListWidgetItem* item = new QListWidgetItem(m_cardsList);
    item->setData(Qt::DisplayRole, cardName);
    item->setData(Qt::BackgroundRole, QColor(getHexColorForCard(cardColor)));

    item->setSizeHint(QSize(75, 110));

    m_cardsList->addItem(item);
    m_cardsList->scrollToBottom();
}