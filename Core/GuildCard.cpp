module Core.GuildCard;

const bool& Core::GuildCard::getCardVisible() const { return isVisible; }
void Core::GuildCard::setCardVisibility(const bool& visible) { isVisible = visible; }

const bool& Core::GuildCard::getCardAccessible() const { return isAccessible; }
void Core::GuildCard::setCardAccessibility(const bool& accessible) { isAccessible = accessible; }