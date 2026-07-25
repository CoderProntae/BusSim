#include "roadforge/renderer/DebugMeshAssets.hpp"

#include "roadforge/math/Mat4.hpp"
#include "roadforge/math/Transform.hpp"

#include <algorithm>
#include <array>
#include <cstddef>

namespace roadforge::renderer {
namespace {

void appendFace(std::vector<DebugVertex>& vertices,
                std::vector<uint16_t>& indices,
                const DebugVertex& a,
                const DebugVertex& b,
                const DebugVertex& c,
                const DebugVertex& d) {
    const uint16_t base = static_cast<uint16_t>(vertices.size());
    vertices.push_back(a);
    vertices.push_back(b);
    vertices.push_back(c);
    vertices.push_back(d);
    indices.push_back(base);
    indices.push_back(static_cast<uint16_t>(base + 1));
    indices.push_back(static_cast<uint16_t>(base + 2));
    indices.push_back(static_cast<uint16_t>(base + 2));
    indices.push_back(static_cast<uint16_t>(base + 3));
    indices.push_back(base);
}

void appendQuad(std::vector<DebugVertex>& vertices,
                std::vector<uint16_t>& indices,
                float minX,
                float minZ,
                float maxX,
                float maxZ,
                float y,
                const std::array<float, 3>& color) {
    appendFace(vertices,
               indices,
               DebugVertex{{ minX, y, minZ }, { color[0], color[1], color[2] }},
               DebugVertex{{ maxX, y, minZ }, { color[0], color[1], color[2] }},
               DebugVertex{{ maxX, y, maxZ }, { color[0], color[1], color[2] }},
               DebugVertex{{ minX, y, maxZ }, { color[0], color[1], color[2] }});
}

void appendBox(std::vector<DebugVertex>& vertices,
               std::vector<uint16_t>& indices,
               float minX,
               float minY,
               float minZ,
               float maxX,
               float maxY,
               float maxZ,
               const std::array<float, 3>& color) {
    const std::array<float, 3> top = { std::min(color[0] * 1.18F, 1.0F), std::min(color[1] * 1.18F, 1.0F), std::min(color[2] * 1.18F, 1.0F) };
    const std::array<float, 3> side = { color[0] * 0.92F, color[1] * 0.92F, color[2] * 0.92F };
    const std::array<float, 3> dark = { color[0] * 0.72F, color[1] * 0.72F, color[2] * 0.72F };

    appendFace(vertices, indices,
               DebugVertex{{ minX, maxY, minZ }, { top[0], top[1], top[2] }},
               DebugVertex{{ maxX, maxY, minZ }, { top[0], top[1], top[2] }},
               DebugVertex{{ maxX, maxY, maxZ }, { top[0], top[1], top[2] }},
               DebugVertex{{ minX, maxY, maxZ }, { top[0], top[1], top[2] }});
    appendFace(vertices, indices,
               DebugVertex{{ minX, minY, minZ }, { dark[0], dark[1], dark[2] }},
               DebugVertex{{ minX, minY, maxZ }, { dark[0], dark[1], dark[2] }},
               DebugVertex{{ maxX, minY, maxZ }, { dark[0], dark[1], dark[2] }},
               DebugVertex{{ maxX, minY, minZ }, { dark[0], dark[1], dark[2] }});
    appendFace(vertices, indices,
               DebugVertex{{ minX, minY, minZ }, { side[0], side[1], side[2] }},
               DebugVertex{{ maxX, minY, minZ }, { side[0], side[1], side[2] }},
               DebugVertex{{ maxX, maxY, minZ }, { side[0], side[1], side[2] }},
               DebugVertex{{ minX, maxY, minZ }, { side[0], side[1], side[2] }});
    appendFace(vertices, indices,
               DebugVertex{{ maxX, minY, minZ }, { color[0], color[1], color[2] }},
               DebugVertex{{ maxX, minY, maxZ }, { color[0], color[1], color[2] }},
               DebugVertex{{ maxX, maxY, maxZ }, { color[0], color[1], color[2] }},
               DebugVertex{{ maxX, maxY, minZ }, { color[0], color[1], color[2] }});
    appendFace(vertices, indices,
               DebugVertex{{ minX, minY, maxZ }, { side[0], side[1], side[2] }},
               DebugVertex{{ minX, minY, minZ }, { side[0], side[1], side[2] }},
               DebugVertex{{ minX, maxY, minZ }, { side[0], side[1], side[2] }},
               DebugVertex{{ minX, maxY, maxZ }, { side[0], side[1], side[2] }});
    appendFace(vertices, indices,
               DebugVertex{{ maxX, minY, maxZ }, { color[0], color[1], color[2] }},
               DebugVertex{{ minX, minY, maxZ }, { color[0], color[1], color[2] }},
               DebugVertex{{ minX, maxY, maxZ }, { color[0], color[1], color[2] }},
               DebugVertex{{ maxX, maxY, maxZ }, { color[0], color[1], color[2] }});
}

const std::array<uint8_t, 7>& glyphRows(char c) {
    static constexpr std::array<uint8_t, 7> kB = { 0b11110, 0b10001, 0b10001, 0b11110, 0b10001, 0b10001, 0b11110 };
    static constexpr std::array<uint8_t, 7> kS = { 0b01111, 0b10000, 0b10000, 0b01110, 0b00001, 0b00001, 0b11110 };
    static constexpr std::array<uint8_t, 7> kU = { 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110 };
    static constexpr std::array<uint8_t, 7> kBlank = { 0, 0, 0, 0, 0, 0, 0 };

    switch (c) {
        case 'B': return kB;
        case 'S': return kS;
        case 'U': return kU;
        default: return kBlank;
    }
}

void appendLabel(std::vector<DebugVertex>& vertices,
                 std::vector<uint16_t>& indices,
                 const char* text,
                 float centerX,
                 float centerZ,
                 float cellSize,
                 const std::array<float, 3>& color) {
    int length = 0;
    while (text[length] != '\0') {
        ++length;
    }

    const float glyphWidth = 5.0F * cellSize;
    const float glyphHeight = 7.0F * cellSize;
    const float glyphGap = cellSize;
    const float totalWidth = (static_cast<float>(length) * glyphWidth) + (static_cast<float>(std::max(0, length - 1)) * glyphGap);
    const float startX = centerX - (totalWidth * 0.5F);
    const float startZ = centerZ - (glyphHeight * 0.5F);
    constexpr float kTextY = 0.045F;

    for (int glyph = 0; glyph < length; ++glyph) {
        const std::array<uint8_t, 7>& rows = glyphRows(text[glyph]);
        const float glyphX = startX + (static_cast<float>(glyph) * (glyphWidth + glyphGap));
        for (int row = 0; row < 7; ++row) {
            for (int col = 0; col < 5; ++col) {
                const bool enabled = ((rows[row] >> (4 - col)) & 0x1U) != 0U;
                if (!enabled) {
                    continue;
                }
                const float x0 = glyphX + (static_cast<float>(col) * cellSize);
                const float z0 = startZ + (static_cast<float>(6 - row) * cellSize);
                appendQuad(vertices, indices, x0, z0, x0 + (cellSize * 0.82F), z0 + (cellSize * 0.82F), kTextY, color);
            }
        }
    }
}

void applyTransformToVertices(std::vector<DebugVertex>& vertices, std::size_t firstVertex, const math::Transform& transform) {
    const math::Mat4 matrix = math::transformToMat4(transform);
    for (std::size_t i = firstVertex; i < vertices.size(); ++i) {
        const float x = vertices[i].position[0];
        const float y = vertices[i].position[1];
        const float z = vertices[i].position[2];
        vertices[i].position[0] = (matrix.m[0] * x) + (matrix.m[4] * y) + (matrix.m[8] * z) + matrix.m[12];
        vertices[i].position[1] = (matrix.m[1] * x) + (matrix.m[5] * y) + (matrix.m[9] * z) + matrix.m[13];
        vertices[i].position[2] = (matrix.m[2] * x) + (matrix.m[6] * y) + (matrix.m[10] * z) + matrix.m[14];
    }
}

void appendRoadSurfaceMesh(std::vector<DebugVertex>& vertices, std::vector<uint16_t>& indices) {
    // Phase 2.5 test track: still simple procedural debug geometry, but now
    // long enough to drive, with a cross segment, terminal/service pad and edge
    // markers. Collision/navigation data will be introduced later; this is the
    // visual track scaffold for the kinematic driving slice.
    appendQuad(vertices, indices, -18.0F, -10.0F, 18.0F, 86.0F, -0.045F, { 0.016F, 0.060F, 0.026F }); // ground/grass

    // Main road and cross-road.
    appendQuad(vertices, indices, -2.75F, -8.0F, 2.75F, 82.0F, 0.0F, { 0.070F, 0.074F, 0.082F });
    appendQuad(vertices, indices, -14.0F, 25.0F, 14.0F, 30.4F, 0.002F, { 0.068F, 0.072F, 0.080F });
    appendQuad(vertices, indices, 6.0F, 36.0F, 15.0F, 49.0F, 0.003F, { 0.060F, 0.064F, 0.072F }); // terminal/service pad

    // Road edge lines.
    appendQuad(vertices, indices, -2.96F, -8.0F, -2.78F, 82.0F, 0.016F, { 0.78F, 0.82F, 0.88F });
    appendQuad(vertices, indices, 2.78F, -8.0F, 2.96F, 82.0F, 0.016F, { 0.78F, 0.82F, 0.88F });
    appendQuad(vertices, indices, -14.0F, 24.78F, 14.0F, 24.96F, 0.017F, { 0.78F, 0.82F, 0.88F });
    appendQuad(vertices, indices, -14.0F, 30.44F, 14.0F, 30.62F, 0.017F, { 0.78F, 0.82F, 0.88F });

    // Dashed lane markers along the main road and cross-road.
    for (int segment = 0; segment < 34; ++segment) {
        const float z0 = -6.8F + (static_cast<float>(segment) * 2.55F);
        appendQuad(vertices, indices, -0.075F, z0, 0.075F, z0 + 1.18F, 0.022F, { 0.96F, 0.92F, 0.72F });
    }
    for (int segment = 0; segment < 10; ++segment) {
        const float x0 = -12.5F + (static_cast<float>(segment) * 2.55F);
        appendQuad(vertices, indices, x0, 27.62F, x0 + 1.15F, 27.78F, 0.023F, { 0.96F, 0.92F, 0.72F });
    }

    // Start zone and terminal marker text.
    appendLabel(vertices, indices, "BUS", 0.0F, 6.35F, 0.135F, { 1.0F, 1.0F, 1.0F });
    appendLabel(vertices, indices, "BUS", 10.5F, 42.0F, 0.150F, { 1.0F, 0.92F, 0.35F });

    // Low curb/marker blocks at intervals so motion is easier to perceive.
    for (int marker = 0; marker < 16; ++marker) {
        const float z = -4.0F + (static_cast<float>(marker) * 5.0F);
        appendBox(vertices, indices, -3.55F, 0.0F, z, -3.18F, 0.22F, z + 0.55F, { 0.82F, 0.12F, 0.10F });
        appendBox(vertices, indices, 3.18F, 0.0F, z + 2.15F, 3.55F, 0.22F, z + 2.70F, { 0.82F, 0.82F, 0.16F });
    }

    // Terminal pad parking guide.
    appendQuad(vertices, indices, 7.0F, 38.0F, 14.0F, 38.18F, 0.025F, { 0.30F, 0.55F, 1.0F });
    appendQuad(vertices, indices, 7.0F, 46.8F, 14.0F, 46.98F, 0.025F, { 0.30F, 0.55F, 1.0F });
    appendQuad(vertices, indices, 7.0F, 38.0F, 7.18F, 47.0F, 0.025F, { 0.30F, 0.55F, 1.0F });
    appendQuad(vertices, indices, 13.82F, 38.0F, 14.0F, 47.0F, 0.025F, { 0.30F, 0.55F, 1.0F });
}

void appendBusPlaceholderMesh(std::vector<DebugVertex>& vertices, std::vector<uint16_t>& indices) {
    appendBox(vertices, indices, -0.82F, 0.12F, -1.05F, 0.82F, 0.82F, 1.05F, { 0.95F, 0.46F, 0.08F });
    appendBox(vertices, indices, -0.70F, 0.82F, -0.80F, 0.70F, 1.05F, 0.70F, { 0.88F, 0.38F, 0.06F });

    appendFace(vertices, indices,
               DebugVertex{{ -0.835F, 0.48F, -0.82F }, { 0.05F, 0.14F, 0.22F }},
               DebugVertex{{ -0.835F, 0.48F, 0.74F }, { 0.05F, 0.14F, 0.22F }},
               DebugVertex{{ -0.835F, 0.75F, 0.74F }, { 0.08F, 0.24F, 0.36F }},
               DebugVertex{{ -0.835F, 0.75F, -0.82F }, { 0.08F, 0.24F, 0.36F }});
    appendFace(vertices, indices,
               DebugVertex{{ 0.835F, 0.48F, 0.74F }, { 0.05F, 0.14F, 0.22F }},
               DebugVertex{{ 0.835F, 0.48F, -0.82F }, { 0.05F, 0.14F, 0.22F }},
               DebugVertex{{ 0.835F, 0.75F, -0.82F }, { 0.08F, 0.24F, 0.36F }},
               DebugVertex{{ 0.835F, 0.75F, 0.74F }, { 0.08F, 0.24F, 0.36F }});
    appendFace(vertices, indices,
               DebugVertex{{ -0.55F, 0.47F, -1.065F }, { 0.05F, 0.14F, 0.22F }},
               DebugVertex{{ 0.55F, 0.47F, -1.065F }, { 0.05F, 0.14F, 0.22F }},
               DebugVertex{{ 0.55F, 0.76F, -1.065F }, { 0.08F, 0.24F, 0.36F }},
               DebugVertex{{ -0.55F, 0.76F, -1.065F }, { 0.08F, 0.24F, 0.36F }});

    appendBox(vertices, indices, -0.95F, 0.02F, -0.75F, -0.72F, 0.32F, -0.35F, { 0.015F, 0.015F, 0.018F });
    appendBox(vertices, indices, -0.95F, 0.02F, 0.35F, -0.72F, 0.32F, 0.75F, { 0.015F, 0.015F, 0.018F });
    appendBox(vertices, indices, 0.72F, 0.02F, -0.75F, 0.95F, 0.32F, -0.35F, { 0.015F, 0.015F, 0.018F });
    appendBox(vertices, indices, 0.72F, 0.02F, 0.35F, 0.95F, 0.32F, 0.75F, { 0.015F, 0.015F, 0.018F });
    appendBox(vertices, indices, -0.48F, 0.24F, -1.12F, -0.22F, 0.36F, -1.06F, { 1.0F, 0.92F, 0.35F });
    appendBox(vertices, indices, 0.22F, 0.24F, -1.12F, 0.48F, 0.36F, -1.06F, { 1.0F, 0.92F, 0.35F });
}

} // namespace

void buildDebugSceneFromProxies(const std::vector<DebugRenderProxy>& proxies,
                                std::vector<DebugVertex>& vertices,
                                std::vector<uint16_t>& indices) {
    vertices.clear();
    indices.clear();
    vertices.reserve(1024);
    indices.reserve(1536);

    if (proxies.empty()) {
        math::Transform identityTransform{};
        std::size_t first = vertices.size();
        appendRoadSurfaceMesh(vertices, indices);
        applyTransformToVertices(vertices, first, identityTransform);
        first = vertices.size();
        appendBusPlaceholderMesh(vertices, indices);
        applyTransformToVertices(vertices, first, identityTransform);
        return;
    }

    for (const DebugRenderProxy& proxy : proxies) {
        if (!proxy.visible) {
            continue;
        }

        const std::size_t firstVertex = vertices.size();
        switch (proxy.meshKind) {
            case DebugMeshKind::RoadSurface:
                appendRoadSurfaceMesh(vertices, indices);
                break;
            case DebugMeshKind::BusPlaceholder:
                appendBusPlaceholderMesh(vertices, indices);
                break;
        }
        applyTransformToVertices(vertices, firstVertex, proxy.transform);
    }
}

} // namespace roadforge::renderer
