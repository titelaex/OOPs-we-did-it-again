module Core.AgeCard;

const bool& Core::AgeCard::getCardVisible() const { return isVisible; }
void Core::AgeCard::setCardVisibility(const bool& visible) { isVisible = visible; }

const bool& Core::AgeCard::getCardAccessible() const { return isAccessible; }
void Core::AgeCard::setCardAccessibility(const bool& accessible) { isAccessible = accessible; }