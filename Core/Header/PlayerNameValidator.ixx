export module Core.PlayerNameValidator;
import <string>;
import <regex>;

namespace Core {
    export class PlayerNameValidator {
    public:
        // Validate player name using regex - only letters (a-z, A-Z), no spaces, numbers, or special chars
        static bool isValid(const std::string& name);
        
        // Get validated name from user input (displays rules before and after invalid attempts)
        static std::string getValidatedName(const std::string& prompt);
        
    private:
        // Display validation rules
        static void displayRules();
    };
}
