#version 450

layout(location = 0) out vec3 vColor;

const vec2 kPositions[3] = vec2[3](
    vec2( 0.00, -0.55),
    vec2( 0.58,  0.42),
    vec2(-0.58,  0.42)
);

const vec3 kColors[3] = vec3[3](
    vec3(1.00, 0.60, 0.08),
    vec3(0.10, 0.68, 1.00),
    vec3(0.36, 1.00, 0.42)
);

void main() {
    gl_Position = vec4(kPositions[gl_VertexIndex], 0.0, 1.0);
    vColor = kColors[gl_VertexIndex];
}
