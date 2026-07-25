#pragma once

#include <array>
#include <cstdint>
#include <string_view>

namespace roadforge::renderer {

enum class MaterialId : uint16_t {
    Asphalt = 0,
    LanePaint,
    RoadShoulder,
    BusBodyOrange,
    BusBodyDark,
    BusWindowTint,
    TireRubber,
    WheelRim,
    HeadlightGlass,
    BrakeLightRed,
    TerminalConcrete,
    GrassDry,
    DebugWhite,
    Count,
};

struct PbrMaterial final {
    MaterialId id = MaterialId::Asphalt;
    std::string_view name{};
    std::array<float, 4> baseColor{ 1.0F, 1.0F, 1.0F, 1.0F };
    float metallic = 0.0F;
    float roughness = 0.85F;
    float normalScale = 1.0F;
    std::array<float, 3> emissive{ 0.0F, 0.0F, 0.0F };
    float ambientOcclusion = 1.0F;
};

[[nodiscard]] const std::array<PbrMaterial, static_cast<size_t>(MaterialId::Count)>& builtInPbrMaterials();
[[nodiscard]] const PbrMaterial& materialById(MaterialId id);

} // namespace roadforge::renderer
