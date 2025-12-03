import <cstdint>;
module Models.Wonder;
import Models.Card;
import Models.ResourceType;
import Models.Token;
import <random>;
import <algorithm>;
import <iterator>;
import <ostream>;
import <iostream>;
import <unordered_map>;
import <bitset>;

using namespace Models;


const uint8_t& Wonder::getOpponentLosesMoney() const { return m_opponentLosesMoney; }
const uint8_t& Wonder::getShieldPoints() const { return m_shieldPoints; }
const ResourceType& Wonder::getResourceProduction() const { return m_resourceProduction; }
const std::bitset<5>& Wonder::getFlags() const { return m_flags; }

void Wonder::setOpponentLosesMoney(uint8_t amt) { m_opponentLosesMoney = amt; }
void Wonder::setShieldPoints(uint8_t pts) { m_shieldPoints = pts; }
void Wonder::setResourceProduction(ResourceType r) { m_resourceProduction = r; }
void Wonder::setFlags(const std::bitset<5>& flags) { m_flags = flags; }

std::ostream& Models::operator<<(std::ostream& os, const Wonder& card)
{
	os << "Card Name: " << card.GetName() << '\n';
	os << "Color: " << static_cast<int>(card.GetColor()) << '\n';
	os << "Caption: " << card.GetCaption() << '\n';
	os << "Resource Cost:" << '\n';
	for (const auto& [res, amt] : card.GetResourceCost())
		os << " - " << static_cast<int>(res) << ": " << static_cast<int>(amt) << '\n';
	os << "Victory Points: " << static_cast<int>(card.GetVictoryPoints()) << '\n';
	os << "Coin Worth Type: " << static_cast<int>(card.GetCoinWorth()) << '\n';
	os << "Coin Reward: " << static_cast<int>(card.GetCoinReward()) << '\n';
	os << "Opponent Loses Money: " << static_cast<int>(card.getOpponentLosesMoney()) << '\n';
	os << "Shield Points: " << static_cast<int>(card.getShieldPoints()) << '\n';
	os << "Resource Production: " << static_cast<int>(card.getResourceProduction()) << '\n';
	os << "Flags: " << card.getFlags() << '\n';
	return os;
}

void Wonder::displayCardInfo()
{
	Card::displayCardInfo();
	std::cout << " Resource Production: " << ResourceTypeToString(m_resourceProduction) << "\n";
	std::cout << " Shield Points: " << static_cast<int>(m_shieldPoints) << "\n";
	std::cout << " Opponent Loses Money: " << static_cast<int>(m_opponentLosesMoney) << "\n";
	std::cout << " Discarded Card Color: " << static_cast<int>(m_discardedCardColor) << "\n";
	std::cout << " Flags: " << m_flags << "\n";
}

WonderBuilder& WonderBuilder::setName(const std::string& name) { m_card.setName(name); return *this; }
WonderBuilder& WonderBuilder::setResourceCost(const std::unordered_map<ResourceType, uint8_t>& resourceCost) { m_card.setResourceCost(resourceCost); return *this; }
WonderBuilder& WonderBuilder::setVictoryPoints(const uint8_t& victoryPoints) { m_card.setVictoryPoints(victoryPoints); return *this; }
WonderBuilder& WonderBuilder::setCoinWorth(const CoinWorthType& coinWorth) { m_card.setCoinWorth(coinWorth); return *this; }
WonderBuilder& WonderBuilder::setCoinReward(const uint8_t& coinReward) { m_card.setCoinReward(coinReward); return *this; }
WonderBuilder& WonderBuilder::setCaption(const std::string& caption) { m_card.setCaption(caption); return *this; }
WonderBuilder& WonderBuilder::setColor(const ColorType& color) { m_card.setColor(color); return *this; }
WonderBuilder& WonderBuilder::setOpponentLosesMoney(const uint8_t& amt) { m_card.setOpponentLosesMoney(amt); return *this; }
WonderBuilder& WonderBuilder::setShieldPoints(const uint8_t& pts) { m_card.setShieldPoints(pts); return *this; }
WonderBuilder& WonderBuilder::setResourceProduction(const ResourceType& r) { m_card.setResourceProduction(r); return *this; }
WonderBuilder& WonderBuilder::setFlags(const std::bitset<5>& flags) { m_card.setFlags(flags); return *this; }
WonderBuilder& WonderBuilder::setDiscardedCardColor(const ColorType& color) { m_card.setResourceProduction(static_cast<ResourceType>(color)); /* reuse setter */ return *this; }
WonderBuilder& WonderBuilder::addOnPlayAction(const std::function<void()>& action) { m_card.addOnPlayAction(action); return *this; }
Wonder WonderBuilder::build() { return std::move(m_card); }