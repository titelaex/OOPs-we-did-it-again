module Core.Player;

import <iostream>;
import <vector>;
import <tuple>;

import Models.Wonder;
import Models.Card;
import Models.Player;
import Models.Token;
import Models.LinkingSymbolType;

static thread_local Core::Player* g_current_player = nullptr;

void SetCurrentPlayer(Core::Player* p) { g_current_player = p; }
Core::Player* GetCurrentPlayer() { return g_current_player; }

Core::ScopedPlayerContext::ScopedPlayerContext(Player* p) : m_prev(g_current_player) { SetCurrentPlayer(p); }
Core::ScopedPlayerContext::~ScopedPlayerContext() { SetCurrentPlayer(m_prev); }

void Core::Player::chooseWonder(std::vector<std::unique_ptr<Models::Wonder>>& availableWonders, uint8_t chosenIndex)
{
    if (availableWonders.empty())
    {
        std::cout << "No available wonders to choose from->\n";
        return;
    }

    if(availableWonders.size() == 1)
    {
        m_player.addWonder(std::move(availableWonders[0]));
        availableWonders.erase(availableWonders.begin());
        std::cout << "Only one wonder available. Automatically chosen: " << m_player.getOwnedWonders().back()->GetName() << "->\n";
        return;
	}

    if (chosenIndex >= availableWonders.size())
    {
        std::cout << "Invalid wonder index->\n";
        return;
    }
    auto& chosenWonder = availableWonders[chosenIndex];
    m_player.addWonder(std::move(chosenWonder));
    availableWonders.erase(availableWonders.begin() + chosenIndex);
    std::cout << "Chosen wonder: " << m_player.getOwnedWonders().back()->GetName() << "->\n";

}

void Core::Player::sellCard()
{
    uint8_t yellowCards = countYellowCards();
    uint8_t coinsEarned = 2 + yellowCards;
	//addCoins(coinsEarned);

	/*
        auto it = std::find_if(m_player.getOwnedCards().begin(), m_player.getOwnedCards().end(), [&](const std::unique_ptr<Models::Card>& p) { return p && p.get() == &card; });
	if (it != m_player.getOwnedCards().end()) {
		(*it)->SetIsVisible(false);
		std::cout << "Card \"" << (*it)->GetName() << "\" discarded. Player \"" << m_player.getPlayerUsername() << "\" gains " << static_cast<int>(coinsEarned) << " coins.\n";
	//	m_player.getOwnedCards().erase(it);
	} else {
		card.SetIsVisible(false);
		std::cout << "Card \"" << card.GetName() << "\" discarded. Player \"" << m_player.getPlayerUsername() << "\" gains " << static_cast<int>(coinsEarned) << " coins.\n";
	}
    */
}

uint8_t Core::Player::countYellowCards()
{
    uint8_t count = 0;
    for (const auto& p : m_player.getOwnedCards()) {
        if (!p) continue;
        if (p->GetColor() == Models::ColorType::YELLOW) ++count;
    }
    return count;
}

//de refacut functia asta
void Core::Player::playCardWonder(std::unique_ptr<Models::Wonder>& wonder, std::unique_ptr<Models::Card>& ageCard, std::unique_ptr<Models::Player>& opponent,
    std::vector<Models::Token>& discardedTokens, std::vector<std::unique_ptr<Models::Card>>& discardedCards,
    uint8_t& totalWondersBuilt)
{
    // check if WondersBuilt < 7

    // 1-> Check if Wonder is already built
    if (wonder->GetIsVisible())
    {
        std::cout << "Wonder \"" << wonder->GetName() << "\" is already constructed->\n";
        return;
    }

    // 2-> Check affordability 
    if (!canAffordWonder(wonder, opponent))
    {
        std::cout << "Cannot afford Wonder \"" << wonder->GetName() << "\"->\n";
        return;
    }

    // 3-> Pay cost (resources + coins)
    payForWonder(wonder, opponent);

    // 4-> Use Age card to build Wonder
    ageCard->toggleVisibility(); // mark as discarded

    // 5-> Trigger Wonder effects
    if (wonder->GetCoinReward() > 0)
    {
        // placeholder: give coins to player
    }

    if (wonder->getOpponentLosesMoney() > 0)
    {
        // placeholder: opponent loses money
    }

    if (wonder->getPlaySecondTurn())
    {
        // placeholder: schedule second turn
    }

    if (wonder->getDrawProgressTokens())
    {
        // placeholder: draw progress tokens
    }

    if (wonder->getChooseAndConstructBuilding())
    {
        // placeholder: allow choose and construct building
    }

    if (wonder->getDiscardCardFromOpponent())
    {
        // placeholder: discard an opponent's card
    }

    // 6-> Add Wonder to player's built list
    //m_player->addWonder(wonder);
    wonder->SetIsVisible(true); // mark as constructed

    // 7-> Check if 7 Wonders have been built
    ++totalWondersBuilt;
    if (totalWondersBuilt == 7)
    {
        discardRemainingWonder();
    }

    std::cout << "Wonder \"" << wonder->GetName() << "\" constructed successfully->\n";
}

void Core::Player::playCardBuilding(std::unique_ptr<Models::Card>& card, std::unique_ptr<Models::Player>& opponent)
{
    if (!card->GetIsVisible())
    {
        std::cout << "Card \"" << card->GetName() << "\" is not accessible->\n";
        return;
    }

    // Chain construction logic requires specific card APIs; commented out for now
    /*
    if (card->GetRequiresLinkingSymbol() != Models::LinkingSymbolType::NO_SYMBOL)
    {
        for (const auto& ownedCard : m_player->getOwnedCards())
        {
            if (ownedCard->GetHasLinkingSymbol() == card->GetRequiresLinkingSymbol())
            {
                m_player->addCard(card);
                card->SetIsVisible(false);
                std::cout << "Card \"" << card->GetName() << "\" constructed for free via chain->\n";
                applyCardEffects(card);
                return;
            }
        }
    }
    */

    // Check if card is free (resource cost only checked; coin-cost logic omitted)
    //if (card.GetResourceCost()->empty() /* && coin-cost logic omitted */)
    //{
    //    m_player->addCard(card);
    //    card->SetIsVisible(false);
    //    std::cout << "Card \"" << card->GetName() << "\" constructed for free->\n";
    //    applyCardEffects(card);
    //    return;
    //}

    // Check affordability
    if (!canAffordCard(card, opponent))
    {
        std::cout << "Cannot afford to construct \"" << card->GetName() << "\"->\n";
        return;
    }

    // Pay cost
    payForCard(card, opponent);

    // Construct card
    /*m_player->addCard(card);
    card->SetIsVisible(false);
    std::cout << "Card \"" << card->GetName() << "\" constructed->\n";
    applyCardEffects(card);*/
}

bool Core::Player::canAffordWonder(std::unique_ptr<Models::Wonder>& wonder, std::unique_ptr<Models::Player>& opponent)
{
    const auto& cost = wonder->GetResourceCost();
    /*const auto& ownPermanent = m_player->getOwnedPermanentResources();
    const auto& ownTrading = m_player->getOwnedTradingResources();
    const auto& opponentPermanent = opponent->getOwnedPermanentResources();

    uint8_t totalAvailableCoins = m_player->totalCoins(m_player->getRemainingCoins());

    for (const auto& kv : cost)
    {
        auto resource = kv->first;
        auto requiredAmount = kv->second;

        uint8_t produced = 0;
        if (ownPermanent->find(resource) != ownPermanent->end()) produced += ownPermanent->at(resource);
        if (ownTrading->find(resource) != ownTrading->end()) produced += ownTrading->at(resource);

        if (produced >= requiredAmount)
            continue;

        uint8_t missing = requiredAmount - produced;
        uint8_t opponentProduction = (opponentPermanent->find(resource) != opponentPermanent->end()) ? opponentPermanent->at(resource) : 0;
        uint8_t costPerUnit = 2 + opponentProduction;
        uint8_t totalCost = costPerUnit * missing;

        if (totalAvailableCoins < totalCost)
            return false;

        totalAvailableCoins -= totalCost;
    }*/
    return true;
}

void Core::Player::payForWonder(std::unique_ptr<Models::Wonder>& wonder, std::unique_ptr<Models::Player>& opponent)
{
    const auto& cost = wonder->GetResourceCost();
    /*const auto& ownPermanent = m_player->getOwnedPermanentResources();
    const auto& ownTrading = m_player->getOwnedTradingResources();
    const auto& opponentPermanent = opponent->getOwnedPermanentResources();

    uint8_t totalCoinsToPay = 0;

    for (const auto& kv : cost)
    {
        auto resource = kv->first;
        auto requiredAmount = kv->second;

        uint8_t produced = 0;
        if (ownPermanent->find(resource) != ownPermanent->end()) produced += ownPermanent->at(resource);
        if (ownTrading->find(resource) != ownTrading->end()) produced += ownTrading->at(resource);

        if (produced >= requiredAmount)
            continue;

        uint8_t missing = requiredAmount - produced;
        uint8_t opponentProduction = (opponentPermanent->find(resource) != opponentPermanent->end()) ? opponentPermanent->at(resource) : 0;
        uint8_t costPerUnit = 2 + opponentProduction;

        totalCoinsToPay += costPerUnit * missing;
    }

    m_player->subtractCoins(totalCoinsToPay);*/
}

void Core::Player::discardRemainingWonder()
{
    uint8_t builtCount = 0;
    // placeholder - actual list of all wonders should be provided by caller or game state
    // this implementation keeps method for API compatibility
}

bool Core::Player::canAffordCard(std::unique_ptr<Models::Card>& card, std::unique_ptr<Models::Player>& opponent)
{
    // 1-> Free card (coin-cost omitted)
    //if (card->GetResourceCost()->empty() /* && coin-cost omitted */)
    //    return true;

    // 2-> Chain construction (commented out; requires card linking APIs)
    /*
    if (card->GetRequiresLinkingSymbol() != Models::LinkingSymbolType::NO_SYMBOL)
    {
        for (const auto& ownedCard : m_player->getOwnedCards())
        {
            if (ownedCard->GetHasLinkingSymbol() == card->GetRequiresLinkingSymbol())
                return true;
        }
    }
    */

    // 3-> Resource cost
    const auto& cost = card->GetResourceCost();
    /*const auto& ownPermanent = m_player->getOwnedPermanentResources();
    const auto& ownTrading = m_player->getOwnedTradingResources();
    const auto& opponentProduction = opponent->getOwnedPermanentResources();

    uint8_t availableCoins = m_player->totalCoins(m_player->getRemainingCoins());

    for (const auto& kv : cost)
    {
        auto resource = kv->first;
        auto requiredAmount = kv->second;

        uint8_t produced = 0;
        if (ownPermanent->find(resource) != ownPermanent->end()) produced += ownPermanent->at(resource);
        if (ownTrading->find(resource) != ownTrading->end()) produced += ownTrading->at(resource);

        if (produced >= requiredAmount)
            continue;

        uint8_t missing = requiredAmount - produced;
        uint8_t opponentAmount = (opponentProduction->find(resource) != opponentProduction->end()) ? opponentProduction->at(resource) : 0;
        uint8_t costPerUnit = 2 + opponentAmount;

        uint8_t totalCost = costPerUnit * missing;
        if (availableCoins < totalCost)
            return false;
    }*/
    return true;
}

// Need to include payForCard, applyCardEffects, etc
void Core::Player::payForCard(std::unique_ptr<Models::Card>& card, std::unique_ptr<Models::Player>& opponent)
{
    const auto& cost = card->GetResourceCost();
    /*const auto& ownPermanent = m_player->getOwnedPermanentResources();
    const auto& ownTrading = m_player->getOwnedTradingResources();
    const auto& opponentPermanent = opponent->getOwnedPermanentResources();

    uint8_t totalCoinsToPay = 0;

    for (const auto& kv : cost)
    {
        auto resource = kv->first;
        auto requiredAmount = kv->second;

        uint8_t produced = 0;
        if (ownPermanent->find(resource) != ownPermanent->end()) produced += ownPermanent->at(resource);
        if (ownTrading->find(resource) != ownTrading->end()) produced += ownTrading->at(resource);

        if (produced >= requiredAmount)
            continue;

        uint8_t missing = requiredAmount - produced;
        uint8_t opponentProduction = (opponentPermanent->find(resource) != opponentPermanent->end()) ? opponentPermanent->at(resource) : 0;
        uint8_t costPerUnit = 2 + opponentProduction;

        totalCoinsToPay += costPerUnit * missing;
    }

    m_player->subtractCoins(totalCoinsToPay);*/
}

void Core::Player::applyCardEffects(std::unique_ptr<Models::Card>& card)
{
    std::cout << "Applying effects of card \"" << card->GetName() << "\"->\n";
}

void Core::Player::takeCard(std::unique_ptr<Models::Card> card)
{
    // Rebuild the card to bind its onPlay actions to this Core::Player
    const auto& oldActions = card->GetOnPlayActions();
    Models::CardBuilder builder;
    builder.setName(card->GetName())
        .setResourceCost(card->GetResourceCost())
        .setVictoryPoints(card->GetVictoryPoints())
        .setCoinWorth(card->GetCoinWorth())
        .setCoinReward(card->GetCoinReward())
        .setCaption(card->GetCaption())
        .setColor(card->GetColor());

    for (const auto& act : oldActions) {
        // wrap original action so it always runs with this player as current context
        builder.addOnPlayAction([this, act]() {
            Core::ScopedPlayerContext ctx(this);
            if (act) act();
        });
    }

    auto newCard = std::make_unique<Models::Card>(builder.build());

    std::cout << "Player takes card: " << newCard->GetName() << "\n";
    // play the card immediately in the context of this player
    {
        Core::ScopedPlayerContext ctx(this);
        newCard->onPlay();
    }
    m_player.addCard(std::move(newCard));
}

void Core::Player::addCoins(uint8_t amt)
{
    auto coins = m_player.getRemainingCoins();
    uint32_t total = m_player.totalCoins(coins) + amt;
    uint8_t sixes = static_cast<uint8_t>(total / 6u);
    uint8_t rem = static_cast<uint8_t>(total % 6u);
    uint8_t threes = static_cast<uint8_t>(rem / 3u);
    uint8_t ones = static_cast<uint8_t>(rem % 3u);
    m_player.setRemainingCoins({ones, threes, sixes});
}

void Core::Player::subtractCoins(uint8_t amt)
{
    auto coins = m_player.getRemainingCoins();
    uint32_t total = m_player.totalCoins(coins);
    if (amt >= total) { m_player.setRemainingCoins({0,0,0}); return; }
    uint32_t newTotal = total - amt;
    uint8_t sixes = static_cast<uint8_t>(newTotal / 6u);
    uint8_t rem = static_cast<uint8_t>(newTotal % 6u);
    uint8_t threes = static_cast<uint8_t>(rem / 3u);
    uint8_t ones = static_cast<uint8_t>(rem % 3u);
    m_player.setRemainingCoins({ones, threes, sixes});
}

