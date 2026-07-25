#pragma once

#include "roadforge/renderer/RenderProxy.hpp"

#include <cstdint>
#include <vector>

namespace roadforge::renderer {

struct DebugVertex final {
    float position[3];
    float color[3];
};

void buildDebugSceneFromProxies(const std::vector<DebugRenderProxy>& proxies,
                                std::vector<DebugVertex>& vertices,
                                std::vector<uint16_t>& indices);

} // namespace roadforge::renderer
