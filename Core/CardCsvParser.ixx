export module Core.CardCsvParser;

import Models.AgeCard;
import Models.GuildCard;
import Models.Wonder;
import Models.Token;
import <string>;
import <vector>;

export
{
    Models::AgeCard AgeCardFactory(const std::vector<std::string>& columns);
    Models::GuildCard GuildCardFactory(const std::vector<std::string>& columns);
    Models::Wonder WonderFactory(const std::vector<std::string>& columns);
    std::vector<Models::Token> ParseTokensFromCSV(const std::string& path);
}
