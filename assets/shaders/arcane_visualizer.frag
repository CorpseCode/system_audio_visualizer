precision highp float;

uniform float uBinsCount;
uniform float uBins[256];

void main() {
    vec2 uv = gl_FragCoord.xy / vec2(1920.0, 1080.0);
    vec2 p = uv * 2.0 - 1.0;

    float radius = length(p);
    float angle = atan(p.y, p.x);
    float normAngle = (angle + 3.14159265) / (6.28318530);

    float indexF = normAngle * uBinsCount;
    int index = clamp(int(floor(indexF)), 0, int(uBinsCount)-1);

    float amp = uBins[index];

    float ring = smoothstep(0.25 + amp * 0.4, 0.26 + amp * 0.6, radius);

    float glow = pow(amp * 4.0, 1.3);

    vec3 color = mix(
        vec3(0.1, 0.7, 1.0),
        vec3(0.6, 0.2, 1.0),
        glow
    );

    gl_FragColor = vec4(color * ring * 1.8, 1.0);
}
