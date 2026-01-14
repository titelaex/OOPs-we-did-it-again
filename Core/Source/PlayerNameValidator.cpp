module Core.PlayerNameValidator;
import <iostream>;
import <regex>;

namespace Core {
    bool PlayerNameValidator::isValid(const std::string& name) {
        // Regex pattern: ^[a-zA-Z]+$
        // ^ - start of string
        // [a-zA-Z]+ - one or more letters (uppercase or lowercase)
        // $ - end of string
        std::regex namePattern("^[a-zA-Z]+$");
        return std::regex_match(name, namePattern);
    }
    
    void PlayerNameValidator::displayRules() {
        std::cout << "\n=== NAME REQUIREMENTS ===\n";
        std::cout << "Player name rules:\n";
        std::cout << "  - Only letters (a-z, A-Z) allowed\n";
        std::cout << "  - No spaces, numbers, or special characters\n";
        std::cout << "  - Minimum 1 character\n";
        std::cout << "=======================\n\n";
    }
    
    std::string PlayerNameValidator::getValidatedName(const std::string& prompt) {
        std::string name;
        bool firstAttempt = true;
        
        while (true) {
            // Display rules before first attempt
            if (firstAttempt) {
                displayRules();
                firstAttempt = false;
            }
            
            std::cout << prompt;
            std::getline(std::cin, name);
            
            if (isValid(name)) {
                std::cout << "Name accepted: '" << name << "'\n\n";
                return name;
            } else {
                // Validation failed - show error and rules again
                std::cout << "\nERROR: Invalid name entered!\n";
                displayRules();
            }
        }
    }
}
