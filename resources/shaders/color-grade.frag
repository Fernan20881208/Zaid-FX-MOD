#ifdef GL_ES
precision mediump float;
#endif

varying vec2 v_texCoord;
varying vec4 v_fragmentColor;

uniform sampler2D CC_Texture0;
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
    vec3 color = source.rgb;

    if (u_sharpen > 0.001) {
        vec3 north = texture2D(CC_Texture0, v_texCoord + vec2(0.0, u_texelSize.y)).rgb;
        vec3 south = texture2D(CC_Texture0, v_texCoord - vec2(0.0, u_texelSize.y)).rgb;
        vec3 east = texture2D(CC_Texture0, v_texCoord + vec2(u_texelSize.x, 0.0)).rgb;
        vec3 west = texture2D(CC_Texture0, v_texCoord - vec2(u_texelSize.x, 0.0)).rgb;
        vec3 sharpened = color * 5.0 - north - south - east - west;
        color = mix(color, sharpened, clamp(u_sharpen, 0.0, 1.0));
    }

    color *= exp2(u_exposure);
    color = (color - 0.5) * u_contrast + 0.5;
    color = adjustSaturation(color, u_saturation);
    color = pow(max(color, vec3(0.0)), vec3(1.0 / max(u_gamma, 0.001)));

    vec2 centered = v_texCoord - vec2(0.5);
    float vignetteShape = smoothstep(0.8, 0.2, dot(centered, centered));
    color *= mix(1.0, vignetteShape, clamp(u_vignette, 0.0, 1.0));

    gl_FragColor = vec4(clamp(color, 0.0, 1.0), source.a) * v_fragmentColor;
}
