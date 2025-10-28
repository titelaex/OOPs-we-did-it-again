#include "Card.h"
using namespace Models;
class Card
{
private:
	std::map<ResourceType, uint8_t> m_resourceCost;
	std::map<ResourceType, uint8_t> m_resourceProduction;
	uint8_t m_victoryPoints : 1;
	uint8_t m_shieldPoints : 1;
	std::array<uint8_t, 7> m_scientificSymbols = { 0, 0, 0, 0, 0, 0, 0 };
public:
	const std::map<ResourceType, uint8_t>& GetResourceCost() const { return m_resourceCost; }
	const std::map<ResourceType, uint8_t>& GetResourceProduction() const { return m_resourceProduction; }
	uint8_t GetVictoryPoints() const { return m_victoryPoints; }
	uint8_t GetShieldPoints() const { return m_shieldPoints; }
	const std::array<uint8_t, 7>& GetScientificSymbols() const { return m_scientificSymbols; }

	void SetResourceCost(const std::map<ResourceType, uint8_t>& resourceCost) { m_resourceCost = resourceCost; }
	void SetResourceProduction(const std::map<ResourceType, uint8_t>& resourceProduction) { m_resourceProduction = resourceProduction; }
	void SetVictoryPoints(uint8_t victoryPoints) { m_victoryPoints = victoryPoints; }
	void SetShieldPoints(uint8_t shieldPoints) { m_shieldPoints = shieldPoints; }
	void SetScientificSymbols(const std::array<uint8_t, 7>& scientificSymbols) { m_scientificSymbols = scientificSymbols; }
};
