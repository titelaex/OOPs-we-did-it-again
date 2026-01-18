export module Core.PlayerNameValidator;
import <string>;
import <regex>;

namespace Core {
    export class PlayerNameValidator {
    public:
        static bool isValid(const std::string& name);
        
        static std::string getValidatedName(const std::string& prompt);
        
    private:
        static void displayRules();
    };
}
