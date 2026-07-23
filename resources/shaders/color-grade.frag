#ifdef GL_ES
precision highp float;
#endif

varying vec2 v_texCoord;

uniform sampler2D CC_Texture0;
uniform float u_debugRed;
uniform float u_intensity;
uniform float u_brightness;
uniform float u_exposure;
uniform float u_contrast;
uniform float u_saturation;
uniform float u_gamma;
uniform float u_bloom;
uniform float u_vignette;
uniform float u_sharpen;
uniform float u_chromaticAberration;
uniform float u_tonemapping;
uniform vec2 u_texelSize;

vec3 adjustSaturation(vec3 color, float saturation) {
    float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
    return mix(vec3(luminance), color, saturation);
}

vec3 acesToneMap(vec3 color) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
}

vec3 chromaticSample(vec2 uv, float amount) {
    vec2 radial = (uv - vec2(0.5)) * amount * 0.02;
    return vec3(
        texture2D(CC_Texture0, uv + radial).r,
        texture2D(CC_Texture0, uv).g,
        texture2D(CC_Texture0, uv - radial).b
    );
}

void main() {
    if (u_debugRed > 0.5) {
        gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
        return;
    }

    vec4 source = texture2D(CC_Texture0, v_texCoord);
    vec3 original = source.rgb;
    vec3 color = mix(original, chromaticSample(v_texCoord, clamp(u_chromaticAberration, 0.0, 1.0)), clamp(u_chromaticAberration, 0.0, 1.0));

    float sharpenAmount = clamp(u_sharpen, 0.0, 1.0);
    vec3 north = texture2D(CC_Texture0, v_texCoord + vec2(0.0, u_texelSize.y)).rgb;
    vec3 south = texture2D(CC_Texture0, v_texCoord - vec2(0.0, u_texelSize.y)).rgb;
    vec3 east = texture2D(CC_Texture0, v_texCoord + vec2(u_texelSize.x, 0.0)).rgb;
    vec3 west = texture2D(CC_Texture0, v_texCoord - vec2(u_texelSize.x, 0.0)).rgb;

    if (sharpenAmount > 0.001) {
        vec3 sharpened = color * 5.0 - north - south - east - west;
        color = mix(color, sharpened, sharpenAmount);
    }

    float bloomAmount = clamp(u_bloom, 0.0, 1.0);
    if (bloomAmount > 0.001) {
        vec3 blur = (north + south + east + west) * 0.25;
        vec3 highlights = max(blur - vec3(0.65), vec3(0.0));
        color += highlights * bloomAmount * 2.5;
    }

    color *= exp2(clamp(u_exposure, -2.0, 2.0));
    color += vec3(clamp(u_brightness, -1.0, 1.0));
    color = (color - vec3(0.5)) * clamp(u_contrast, 0.25, 3.0) + vec3(0.5);
    color = adjustSaturation(color, clamp(u_saturation, 0.0, 3.0));
    color = mix(color, acesToneMap(color), clamp(u_tonemapping, 0.0, 1.0));
    color = pow(max(color, vec3(0.0)), vec3(1.0 / clamp(u_gamma, 0.25, 3.0)));

    vec2 centered = v_texCoord - vec2(0.5);
    float radialDistance = length(centered) / 0.70710678;
    float vignetteMask = smoothstep(0.35, 1.0, radialDistance);
    color *= 1.0 - vignetteMask * clamp(u_vignette, 0.0, 1.0);

    color = mix(original, color, clamp(u_intensity, 0.0, 1.0));
    gl_FragColor = vec4(clamp(color, 0.0, 1.0), 1.0);
}
