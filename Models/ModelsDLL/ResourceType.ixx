// ResourceType.ixx

export module Models.ResourceType;

import <cstdint>;
import <string>;
import <optional>;

export namespace Models {
    export enum class __declspec(dllexport) ResourceType : uint8_t {
        NO_RESOURCE,
        CLAY,
        WOOD,
        STONE,
        GLASS,
        PAPYRUS,
        CONDITION_RAW_GOODS,
        CONDITION_MANUFACTURED_GOODS
    };

    export __declspec(dllexport) std::string ResourceTypeToString(ResourceType resource); // implementation moved to ResourceType.cpp
    export __declspec(dllexport) std::optional<ResourceType> StringToResourceType(const std::string& str);
}