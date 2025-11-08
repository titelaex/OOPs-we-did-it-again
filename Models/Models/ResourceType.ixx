// ResourceType.ixx

export module ResourceType;

#ifdef BUILDING_DLL
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllimport)
#endif

import <cstdint>;
import <string>;

export namespace Models {
    export enum class DLL_API ResourceType : uint8_t {
        CLAY,
        WOOD,
        STONE,
        GLASS,
        PAPYRUS,
        CONDITION_RAW_GOODS,
        CONDITION_MANUFACTURED_GOODS
    };

    export DLL_API std::string ResourceTypeToString(ResourceType resource);
    
    export std::string ResourceTypeToString(ResourceType resource) {
        switch (resource) {
        case ResourceType::CLAY: return "Clay";
        case ResourceType::WOOD: return "Wood";
        case ResourceType::STONE: return "Stone";
        case ResourceType::GLASS: return "Glass";
        case ResourceType::PAPYRUS: return "Papyrus";
        case ResourceType::CONDITION_RAW_GOODS: return "Raw Good Condition Placeholder";
        case ResourceType::CONDITION_MANUFACTURED_GOODS: return "Manufactured Good Condition Placeholder";
        default: return {};
        }
    }
}