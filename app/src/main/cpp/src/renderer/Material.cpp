#include "roadforge/renderer/Material.hpp"

#include <algorithm>

namespace roadforge::renderer {

const std::array<PbrMaterial, static_cast<size_t>(MaterialId::Count)>& builtInPbrMaterials() {
    static constexpr std::array<PbrMaterial, static_cast<size_t>(MaterialId::Count)> kMaterials = {
        PbrMaterial{ MaterialId::Asphalt, "asphalt_worn_dark", { 0.055F, 0.058F, 0.062F, 1.0F }, 0.0F, 0.92F, 0.75F, { 0.0F, 0.0F, 0.0F }, 0.95F },
        PbrMaterial{ MaterialId::LanePaint, "lane_paint_aged_ivory", { 0.86F, 0.83F, 0.68F, 1.0F }, 0.0F, 0.72F, 0.35F, { 0.0F, 0.0F, 0.0F }, 0.88F },
        PbrMaterial{ MaterialId::RoadShoulder, "road_shoulder_gravel", { 0.18F, 0.15F, 0.12F, 1.0F }, 0.0F, 0.96F, 0.90F, { 0.0F, 0.0F, 0.0F }, 0.82F },
        PbrMaterial{ MaterialId::BusBodyOrange, "bus_body_orange_clearcoat", { 0.95F, 0.38F, 0.06F, 1.0F }, 0.0F, 0.38F, 0.45F, { 0.0F, 0.0F, 0.0F }, 1.0F },
        PbrMaterial{ MaterialId::BusBodyDark, "bus_body_graphite_trim", { 0.035F, 0.038F, 0.042F, 1.0F }, 0.0F, 0.62F, 0.55F, { 0.0F, 0.0F, 0.0F }, 0.9F },
        PbrMaterial{ MaterialId::BusWindowTint, "bus_window_blue_tint", { 0.045F, 0.13F, 0.20F, 0.72F }, 0.0F, 0.18F, 0.20F, { 0.0F, 0.0F, 0.0F }, 1.0F },
        PbrMaterial{ MaterialId::TireRubber, "tire_rubber_matte", { 0.008F, 0.008F, 0.009F, 1.0F }, 0.0F, 0.94F, 0.80F, { 0.0F, 0.0F, 0.0F }, 0.72F },
        PbrMaterial{ MaterialId::WheelRim, "wheel_rim_brushed_metal", { 0.46F, 0.46F, 0.44F, 1.0F }, 0.85F, 0.36F, 0.45F, { 0.0F, 0.0F, 0.0F }, 1.0F },
        PbrMaterial{ MaterialId::HeadlightGlass, "headlight_warm_glass", { 1.0F, 0.88F, 0.42F, 1.0F }, 0.0F, 0.16F, 0.25F, { 0.55F, 0.45F, 0.18F }, 1.0F },
        PbrMaterial{ MaterialId::BrakeLightRed, "brake_light_red_emissive", { 0.88F, 0.02F, 0.015F, 1.0F }, 0.0F, 0.22F, 0.20F, { 0.65F, 0.02F, 0.01F }, 1.0F },
        PbrMaterial{ MaterialId::TerminalConcrete, "terminal_concrete_worn", { 0.48F, 0.47F, 0.43F, 1.0F }, 0.0F, 0.88F, 0.65F, { 0.0F, 0.0F, 0.0F }, 0.86F },
        PbrMaterial{ MaterialId::GrassDry, "dry_grass_mixed", { 0.12F, 0.24F, 0.08F, 1.0F }, 0.0F, 0.96F, 0.85F, { 0.0F, 0.0F, 0.0F }, 0.8F },
        PbrMaterial{ MaterialId::DebugWhite, "debug_white", { 1.0F, 1.0F, 1.0F, 1.0F }, 0.0F, 0.5F, 1.0F, { 0.0F, 0.0F, 0.0F }, 1.0F },
    };
    return kMaterials;
}

const PbrMaterial& materialById(MaterialId id) {
    const auto& materials = builtInPbrMaterials();
    const size_t index = std::clamp(static_cast<size_t>(id), size_t{0}, materials.size() - 1U);
    return materials[index];
}

} // namespace roadforge::renderer
