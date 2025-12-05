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


const uint8_t& Wonder::getShieldPoints() const { return m_shieldPoints; }
const ResourceType& Wonder::getResourceProduction() const { return m_resourceProduction; }
bool Wonder::IsConstructed() const { return isConstructed; }

void Wonder::setShieldPoints(uint8_t pts) { m_shieldPoints = pts; }
void Wonder::setResourceProduction(ResourceType r) { m_resourceProduction = r; }
void Wonder::setConstructed(bool constructed) { isConstructed = constructed; }

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
	os << "Shield Points: " << static_cast<int>(card.getShieldPoints()) << '\n';
	os << "Resource Production: " << static_cast<int>(card.getResourceProduction()) << '\n';
	os << "Constructed: " << (card.IsConstructed() ? "Yes" : "No") << '\n';
	return os;
}

void Wonder::displayCardInfo()
{
	Card::displayCardInfo();
	std::cout << " Resource Production: " << ResourceTypeToString(m_resourceProduction) << "\n";
	std::cout << " Shield Points: " << static_cast<int>(m_shieldPoints) << "\n";
	std::cout << " Constructed: " << (isConstructed ? "Yes" : "No") << "\n";
}

WonderBuilder& WonderBuilder::setName(const std::string& name) { m_card.setName(name); return *this; }
WonderBuilder& WonderBuilder::setResourceCost(const std::unordered_map<ResourceType, uint8_t>& resourceCost) { m_card.setResourceCost(resourceCost); return *this; }
WonderBuilder& WonderBuilder::setVictoryPoints(const uint8_t& victoryPoints) { m_card.setVictoryPoints(victoryPoints); return *this; }
WonderBuilder& WonderBuilder::setCoinWorth(const CoinWorthType& coinWorth) { m_card.setCoinWorth(coinWorth); return *this; }
WonderBuilder& WonderBuilder::setCoinReward(const uint8_t& coinReward) { m_card.setCoinReward(coinReward); return *this; }
WonderBuilder& WonderBuilder::setCaption(const std::string& caption) { m_card.setCaption(caption); return *this; }
WonderBuilder& WonderBuilder::setColor(const ColorType& color) { m_card.setColor(color); return *this; }
WonderBuilder& WonderBuilder::setShieldPoints(const uint8_t& pts) { m_card.setShieldPoints(pts); return *this; }
WonderBuilder& WonderBuilder::setResourceProduction(const ResourceType& r) { m_card.setResourceProduction(r); return *this; }
WonderBuilder& WonderBuilder::setConstructed(bool constructed) { m_card.setConstructed(constructed); return *this; }
WonderBuilder& WonderBuilder::addOnPlayAction(const std::function<void()>& action) { m_card.addOnPlayAction(action); return *this; }
WonderBuilder& WonderBuilder::addOnDiscardAction(const std::function<void()>& action) { m_card.addOnDiscardAction(action); return *this; }
Wonder WonderBuilder::build() { return std::move(m_card); }

void Wonder::onDiscard()
{
    Card::onDiscard();
}