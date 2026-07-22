#ifdef GL_ES
precision mediump float;
#endif

varying vec2 v_texCoord;
varying vec4 v_fragmentColor;

uniform sampler2D CC_Texture0;
uniform float u_intensity;
uniform float u_brightness;
uniform float u_exposure;
uniform float u_contrast;
uniform float u_saturation;
uniform float u_gamma;
uniform float u_vignette;
uniform float u_sharpen;
uniform vec2 u_texelSize;

vec3 adjustSaturation(vec3 color, float saturation) {
    float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
    return mix(vec3(luminance), color, saturation);
}

void main() {
    vec4 source = texture2D(CC_Texture0, v_texCoord);
    vec3 original = source.rgb;
    vec3 color = original;

    float sharpenAmount = clamp(u_sharpen, 0.0, 1.0);
    if (sharpenAmount > 0.001) {
        vec3 north = texture2D(CC_Texture0, v_texCoord + vec2(0.0, u_texelSize.y)).rgb;
        vec3 south = texture2D(CC_Texture0, v_texCoord - vec2(0.0, u_texelSize.y)).rgb;
        vec3 east = texture2D(CC_Texture0, v_texCoord + vec2(u_texelSize.x, 0.0)).rgb;
        vec3 west = texture2D(CC_Texture0, v_texCoord - vec2(u_texelSize.x, 0.0)).rgb;
        vec3 sharpened = color * 5.0 - north - south - east - west;
        color = mix(color, sharpened, sharpenAmount);
    }

    color *= exp2(clamp(u_exposure, -2.0, 2.0));
    color += clamp(u_brightness, -1.0, 1.0);
    color = (color - 0.5) * clamp(u_contrast, 0.25, 3.0) + 0.5;
    color = adjustSaturation(color, clamp(u_saturation, 0.0, 3.0));
    color = pow(max(color, vec3(0.0)), vec3(1.0 / clamp(u_gamma, 0.25, 3.0)));

    vec2 centered = v_texCoord - vec2(0.5);
    float radialDistance = length(centered) / 0.70710678;
    float vignetteMask = smoothstep(0.35, 1.0, radialDistance);
    color *= 1.0 - vignetteMask * clamp(u_vignette, 0.0, 1.0);

    float intensity = clamp(u_intensity, 0.0, 1.0);
    color = mix(original, color, intensity);

    gl_FragColor = vec4(clamp(color, 0.0, 1.0), source.a) * v_fragmentColor;
}
