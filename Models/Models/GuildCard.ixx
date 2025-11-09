export module GuildCard;

#ifdef BUILDING_DLL
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllimport)
#endif

// GuildCard module: a concrete Card subtype representing Guild cards (Age III)
// Design notes (why derive from Card and not ICard):
// - Guild cards are a specialized kind of Card with additional fields (guild name, scoring rules).
// - Deriving from Card preserves all existing Card behaviour (cost, production, display, etc.)
// and allows reuse of Card's members and helpers. Deriving directly from ICard would
// force GuildCard to reimplement a lot of Card behaviour or to duplicate data.

import Card; // imports the Card module (provides Models::Card)
import Models.Age; // used to set card age to AGE_III

import <string>;
import <vector>;
import <utility>;
import <iostream>; // only for displayCardInfo override

export namespace Models
{
 export class DLL_API GuildCard : public Card
 {
 public:
 
 private:
 // Guild-specific data: human-readable name and a set of scoring rule descriptions.
 // You may later replace scoring rules with a structured rule object or callback
 // that applies points to a Player; strings are a simple first step.
 std::string m_guildName;
 std::vector<std::string> m_scoringRules;
 };
}
