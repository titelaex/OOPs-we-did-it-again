module Models.ColorType;
import <optional>;
import <string>;

std::optional<Models::ColorType> StringToColorType(const std::string& str) {
    if (str == "BROWN") return Models::ColorType::BROWN;
    if (str == "GREY") return Models::ColorType::GREY;
    if (str == "RED") return Models::ColorType::RED;
    if (str == "YELLOW") return Models::ColorType::YELLOW;
    if (str == "GREEN") return Models::ColorType::GREEN;
    if (str == "BLUE") return Models::ColorType::BLUE;
    if (str == "PURPLE") return Models::ColorType::PURPLE;
    return std::nullopt;
}
