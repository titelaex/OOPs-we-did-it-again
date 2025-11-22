module Models.ColorType;
import <optional>;
import <string>;

export __declspec(dllexport) std::optional<ColorType> StringToColorType(const std::string& str) {
    if (str == "BROWN") return ColorType::BROWN;
    if (str == "GREY") return ColorType::GREY;
    if (str == "RED") return ColorType::RED;
    if (str == "YELLOW") return ColorType::YELLOW;
    if (str == "GREEN") return ColorType::GREEN;
    if (str == "BLUE") return ColorType::BLUE;
    if (str == "PURPLE") return ColorType::PURPLE;
    return std::nullopt;
}
