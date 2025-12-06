export module Core.CardCsvParser;

import Models.AgeCard;
import Models.GuildCard;
import Models.Wonder;
import Models.Token;
import Core.Player;
import <string>;
import <vector>;
import <memory>;

export
{
    Models::AgeCard ageCardFactory(const std::vector<std::string>& columns);
    Models::GuildCard guildCardFactory(const std::vector<std::string>& columns);
    Models::Wonder wonderFactory(const std::vector<std::string>& columns);
    std::vector<std::unique_ptr<Models::Token>> parseTokensFromCSV(const std::string& path);
}
