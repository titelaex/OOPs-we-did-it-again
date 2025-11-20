// ResourceType.ixx

export module Models.ResourceType;

import <cstdint>;
import <string>;

export namespace Models {
    export enum class __declspec(dllexport) ResourceType : uint8_t {
        CLAY,
        WOOD,
        STONE,
        GLASS,
        PAPYRUS,
        CONDITION_RAW_GOODS,
        CONDITION_MANUFACTURED_GOODS
    };

    export __declspec(dllexport) std::string ResourceTypeToString(ResourceType resource); // implementation moved to ResourceType.cpp
}