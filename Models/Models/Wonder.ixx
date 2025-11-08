
export module Wonder;

#ifdef BUILDING_DLL
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllimport)
#endif

import ResourceType;
import <string>;
import <vector>;

export namespace Models
{
	export class DLL_API Wonder
	{
	private:
		const std::string m_name;
		const uint8_t m_VictoryPointsWorth;
		const uint8_t m_playerReceivesMoney;
		const int8_t m_opponentLosesMoney;
		const uint8_t ShieldsWorth;
		const std::vector<ResourceType> m_resourcesProduced; //produced each turn
		const bool m_playSecondTurn;
		const bool m_drawProgressTokens;
		const bool m_chooseAndConstructBuilding;
		const bool m_discardBrownCardFromOpponent;

	};
}