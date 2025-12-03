module Models.Card;
import <iostream>;
import <functional>;

using namespace Models;


Card::Card(Card&& other) = default;
Card& Card::operator=(Card&& other) = default;

const std::string& Card::GetName() const { return m_name; }
const std::unordered_map<ResourceType, uint8_t>& Card::GetResourceCost() const { return m_resourceCost; }
const std::string& Card::GetCaption() const { return m_caption; }
const ColorType& Card::GetColor() const { return m_color; }
const uint8_t& Card::GetVictoryPoints() const { return m_victoryPoints; }
const CoinWorthType& Card::GetCoinWorth() const { return m_coinWorth; }
const uint8_t& Card::GetCoinReward() const { return m_coinReward; }
const std::vector<std::function<void()>>& Card::GetOnPlayActions() const { return m_onPlayActions; }

void Card::onPlay()
{
    for (const auto& action : m_onPlayActions) {
        if (action) action();
    }
}

void Card::setName(const std::string& name) { m_name = name; }
void Card::setResourceCost(const std::unordered_map<ResourceType, uint8_t>& resourceCost) { m_resourceCost = resourceCost; }
void Card::setVictoryPoints(const uint8_t& victoryPoints) { m_victoryPoints = victoryPoints; }
void Card::setCoinWorth(const CoinWorthType& coinWorth) { m_coinWorth = coinWorth; }
void Card::setCoinReward(const uint8_t& coinReward) { m_coinReward = coinReward; }
void Card::setCaption(const std::string& caption) { m_caption = caption; }
void Card::setColor(const ColorType& color) { m_color = color; }
void Card::addOnPlayAction(const std::function<void()>& action) { m_onPlayActions.push_back(action); }

void Card::displayCardInfo() {
    std::cout << "Card Name: " << m_name << '\n';
    std::cout << "Caption: " << m_caption << '\n';
    std::cout << "Color: " << ColorTypeToString(m_color) << '\n';
    std::cout << "Victory Points: " << static_cast<int>(m_victoryPoints) << '\n';
    std::cout << "Coin Worth: " << CoinWorthTypeToString(m_coinWorth)
        << ", Coin Reward: " << static_cast<int>(m_coinReward) << '\n';
    std::cout << "Resource Cost:" << '\n';
    for (const auto& kv : m_resourceCost) {
        std::cout << " - " << ResourceTypeToString(kv.first) << ": " << static_cast<int>(kv.second) << '\n';
    }
    std::cout << "OnPlay actions: " << m_onPlayActions.size() << "\n";
}

CardBuilder& CardBuilder::setName(const std::string& name) {
    m_card.setName(name);
    return *this;
}

CardBuilder& CardBuilder::setResourceCost(const std::unordered_map<ResourceType, uint8_t>& resourceCost) {
    m_card.setResourceCost(resourceCost);
    return *this;
}

CardBuilder& CardBuilder::setVictoryPoints(const uint8_t& victoryPoints) {
    m_card.setVictoryPoints(victoryPoints);
    return *this;
}

CardBuilder& CardBuilder::setCoinWorth(const CoinWorthType& coinWorth) {
    m_card.setCoinWorth(coinWorth);
    return *this;
}

CardBuilder& CardBuilder::setCoinReward(const uint8_t& coinReward) {
    m_card.setCoinReward(coinReward);
    return *this;
}

CardBuilder& CardBuilder::setCaption(const std::string& caption) {
    m_card.setCaption(caption);
    return *this;
}

CardBuilder& CardBuilder::setColor(const ColorType& color) {
    m_card.setColor(color);
    return *this;
}

CardBuilder& CardBuilder::addOnPlayAction(const std::function<void()>& action) {
    m_card.addOnPlayAction(action);
    return *this;
}

Card CardBuilder::build() {
    return std::move(m_card);
}

