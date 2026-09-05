#pragma once

#include "RED4ext/Api/v1/FileVer.hpp"
#include "RED4ext/Api/v1/SemVer.hpp"

struct Image
{
    void Initialize();

    static std::tuple<uint32_t, uint16_t> GetSupportedVersion() noexcept { return std::make_tuple(2, 31); }

    uintptr_t base_address;
    mem::region TextRegion;
    RED4ext::v1::FileVer FileVersion{};
    RED4ext::v1::SemVer SemVersion{};
};
