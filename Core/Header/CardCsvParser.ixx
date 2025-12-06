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
    Models::AgeCard AgeCardFactory(const std::vector<std::string>& columns);
    Models::GuildCard GuildCardFactory(const std::vector<std::string>& columns);
    Models::Wonder WonderFactory(const std::vector<std::string>& columns);
    std::vector<std::unique_ptr<Models::Token>> ParseTokensFromCSV(const std::string& path);
}
