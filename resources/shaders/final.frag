#ifdef GL_ES
precision highp float;
#endif

varying vec2 v_texCoord;

uniform sampler2D CC_Texture0;
uniform sampler2D u_lightingTexture;
uniform vec2 u_texelSize;
uniform float u_time;

uniform vec4 u_master;
uniform vec4 u_hdr0;
uniform vec4 u_hdr1;
uniform vec4 u_color0;
uniform vec4 u_color1;
uniform vec4 u_color2;
uniform vec4 u_local0;
uniform vec4 u_specular0;
uniform vec4 u_specular1;
uniform vec4 u_depth0;
uniform vec4 u_depth1;
uniform vec4 u_sharpen0;
uniform vec4 u_finish0;
uniform vec4 u_reactive0;
uniform vec4 u_reactive1;

float luminance(vec3 color) {
    return dot(color, vec3(0.2126, 0.7152, 0.0722));
}

float maximum3(vec3 value) {
    return max(value.r, max(value.g, value.b));
}

float minimum3(vec3 value) {
    return min(value.r, min(value.g, value.b));
}

vec4 sampleOriginal(vec2 uv) {
    return texture2D(CC_Texture0, clamp(uv, vec2(0.001), vec2(0.999)));
}

vec4 sampleLighting(vec2 uv) {
    return texture2D(
        u_lightingTexture,
        clamp(uv, vec2(0.001), vec2(0.999))
    );
}

vec3 blurLighting(vec2 uv, vec2 offset, float quality) {
    vec3 sum = sampleLighting(uv).rgb * 2.0;
    float weight = 2.0;

    sum += sampleLighting(uv + vec2(offset.x, 0.0)).rgb;
    sum += sampleLighting(uv - vec2(offset.x, 0.0)).rgb;
    sum += sampleLighting(uv + vec2(0.0, offset.y)).rgb;
    sum += sampleLighting(uv - vec2(0.0, offset.y)).rgb;
    weight += 4.0;

    if (quality > 0.20) {
        sum += sampleLighting(uv + offset).rgb;
        sum += sampleLighting(uv - offset).rgb;
        sum += sampleLighting(uv + vec2(offset.x, -offset.y)).rgb;
        sum += sampleLighting(uv + vec2(-offset.x, offset.y)).rgb;
        weight += 4.0;
    }

    if (quality > 0.62) {
        vec2 farOffset = offset * 2.0;
        sum += sampleLighting(uv + vec2(farOffset.x, 0.0)).rgb;
        sum += sampleLighting(uv - vec2(farOffset.x, 0.0)).rgb;
        sum += sampleLighting(uv + vec2(0.0, farOffset.y)).rgb;
        sum += sampleLighting(uv - vec2(0.0, farOffset.y)).rgb;
        weight += 4.0;
    }

    return sum / weight;
}

vec3 huePreservingToneMap(vec3 color, float whitePoint) {
    color = max(color, vec3(0.0));
    float luma = max(luminance(color), 0.00001);
    float mappedLuma =
        luma * (1.0 + luma / max(whitePoint * whitePoint, 0.01)) /
        (1.0 + luma);
    return color * (mappedLuma / luma);
}

vec3 compressGamut(vec3 color) {
    color = max(color, vec3(0.0));
    float peak = maximum3(color);
    if (peak > 1.0) {
        float mappedPeak = 1.0 - exp(-peak);
        color *= mappedPeak / peak;
    }

    float floorValue = minimum3(color);
    if (floorValue < 0.0) color -= vec3(floorValue);
    return clamp(color, 0.0, 1.0);
}

vec3 adjustSaturation(vec3 color, float amount) {
    float luma = luminance(color);
    return vec3(luma) + (color - vec3(luma)) * amount;
}

vec3 applyHDR(vec3 color) {
    if (u_hdr0.x < 0.5) return color;

    float blackLift = u_hdr1.y * 0.12;
    color = max(color - vec3(blackLift), vec3(0.0));

    float luma = luminance(color);
    float shadowMask = 1.0 - smoothstep(0.04, 0.48, luma);
    color += (vec3(luma + 0.16) - color) *
        shadowMask * u_hdr0.w * 0.16;
    color += vec3(shadowMask * u_hdr0.w * 0.055);

    float rangeScale = mix(0.94, 1.42, u_hdr0.y);
    color *= rangeScale;

    float whitePoint = mix(1.25, 3.10, u_hdr1.x);
    vec3 mapped = huePreservingToneMap(color, whitePoint);
    float compression = mix(0.18, 0.92, u_hdr0.z);
    return mix(color, mapped, compression);
}

vec3 applyColorGrading(vec3 color) {
    float exposureStops = (u_color0.x - 0.5) * 2.0;
    color *= exp2(exposureStops);

    float contrastFactor = mix(0.78, 1.32, u_color0.y);
    float pivot = 0.46;
    color = (color - vec3(pivot)) * contrastFactor + vec3(pivot);

    float saturationFactor = mix(0.68, 1.24, u_color0.z);
    color = adjustSaturation(color, saturationFactor);

    float chroma = maximum3(color) - minimum3(color);
    float vibranceMask = 1.0 - smoothstep(0.05, 0.82, chroma);
    color = adjustSaturation(
        color,
        1.0 + u_color0.w * vibranceMask * 0.22
    );

    vec3 temperatureShift = vec3(1.0, 0.08, -1.0) *
        (u_color1.y - 0.5) * 0.10;
    vec3 tintShift = vec3(0.35, -0.70, 0.35) *
        (u_color1.z - 0.5) * 0.075;
    color += temperatureShift + tintShift;

    float luma = luminance(color);
    float highlightMask = smoothstep(0.52, 1.05, luma);
    float shadowMask = 1.0 - smoothstep(0.02, 0.50, luma);
    color *= 1.0 + (u_color1.w - 0.5) * highlightMask * 0.22;
    color += vec3((u_color2.x - 0.5) * shadowMask * 0.16);

    float gammaExponent = mix(1.18, 0.84, u_color1.x);
    color = pow(max(color, vec3(0.0)), vec3(gammaExponent));

    float blackPoint = u_color2.z * 0.12;
    float whitePoint = mix(0.72, 1.0, u_color2.y);
    color = max(color - vec3(blackPoint), vec3(0.0));
    color /= max(whitePoint - blackPoint, 0.08);

    return color;
}

vec3 applyLocalContrast(vec3 color, vec2 uv) {
    if (u_local0.x < 0.5 || u_local0.y <= 0.001) return color;

    vec2 radius = u_texelSize * mix(1.0, 8.0, u_local0.w);
    vec3 localAverage = blurLighting(uv, radius, u_master.y);
    vec3 detail = color - localAverage;
    float detailMask = smoothstep(0.018, 0.23, length(detail));
    float strength = u_local0.y * mix(0.35, 1.05, u_local0.z);

    return color + detail * strength * detailMask * 0.62;
}

vec3 applySpecular(vec3 color, vec2 uv) {
    if (u_specular0.x < 0.5 || u_specular0.y <= 0.001) return color;

    float radius = mix(1.0, 5.0, u_specular0.z);
    vec2 offset = u_texelSize * radius;

    float center = luminance(sampleOriginal(uv).rgb);
    float north = luminance(sampleOriginal(uv + vec2(0.0, offset.y)).rgb);
    float south = luminance(sampleOriginal(uv - vec2(0.0, offset.y)).rgb);
    float east = luminance(sampleOriginal(uv + vec2(offset.x, 0.0)).rgb);
    float west = luminance(sampleOriginal(uv - vec2(offset.x, 0.0)).rgb);

    float edge = abs(east - west) + abs(north - south);
    float threshold = mix(0.30, 1.02, u_specular0.w);
    float highlight = smoothstep(
        threshold,
        threshold + 0.16,
        center + edge * 1.25
    );

    vec3 neutralHighlight = vec3(1.0);
    vec3 metallicHighlight = mix(
        neutralHighlight,
        adjustSaturation(max(color, vec3(0.001)), 0.45),
        u_specular1.x * 0.48
    );
    vec3 glassHighlight = mix(
        metallicHighlight,
        vec3(0.78, 0.90, 1.0),
        u_specular1.y * 0.42
    );

    return color + glassHighlight * highlight * u_specular0.y *
        (0.08 + edge * 0.35);
}

vec3 applyDepthBlur(vec3 color, vec2 uv) {
    if (u_depth0.x < 0.5 || u_depth0.y <= 0.001) return color;

    float separation = mix(0.10, 0.80, u_depth0.w);
    float transition = mix(0.05, 0.36, u_depth1.x);
    float sceneLuma = luminance(sampleOriginal(uv).rgb);

    float verticalBackground = smoothstep(
        separation - transition,
        separation + transition,
        uv.y
    );
    float darkBackground = 1.0 - smoothstep(0.10, 0.55, sceneLuma);
    float playerProtection = exp(
        -dot(uv - vec2(0.24, 0.50), uv - vec2(0.24, 0.50)) * 48.0
    );

    float backgroundMask = clamp(
        max(verticalBackground * 0.48, darkBackground * 0.58) -
        playerProtection,
        0.0,
        1.0
    );

    vec2 blurRadius = u_texelSize * mix(1.0, 11.0, u_depth0.y);
    vec3 blurred = blurLighting(uv, blurRadius, u_master.y);
    color = mix(color, blurred, backgroundMask * u_depth0.y * 0.72);

    vec3 fineBlur = blurLighting(uv, u_texelSize * 1.5, u_master.y);
    vec3 foregroundDetail = color - fineBlur;
    color += foregroundDetail * u_depth0.z *
        (1.0 - backgroundMask) * 0.38;
    return color;
}

vec3 applySharpen(vec3 color, vec2 uv) {
    if (u_sharpen0.x < 0.5 || u_sharpen0.y <= 0.001) return color;

    vec2 radius = u_texelSize * mix(0.75, 3.8, u_sharpen0.z);
    vec3 blurred = blurLighting(uv, radius, u_master.y);
    vec3 detail = color - blurred;
    float amount = length(detail);
    float threshold = mix(0.004, 0.16, u_sharpen0.w);
    float mask = smoothstep(threshold, threshold + 0.065, amount);

    return color + detail * u_sharpen0.y * mask * 0.78;
}

float randomNoise(vec2 point) {
    return fract(sin(dot(point, vec2(12.9898, 78.233))) * 43758.5453);
}

void main() {
    vec2 uv = v_texCoord;
    vec4 sourceSample = sampleOriginal(uv);

    float chromaticAmount = u_finish0.y * 0.0045;
    vec2 radial = (uv - vec2(0.5)) * chromaticAmount;
    vec3 original = vec3(
        sampleOriginal(uv + radial).r,
        sourceSample.g,
        sampleOriginal(uv - radial).b
    );

    vec3 color = sampleLighting(uv).rgb;
    color = applyHDR(color);
    color = applyColorGrading(color);
    color = applyLocalContrast(color, uv);
    color = applySpecular(color, uv);
    color = applyDepthBlur(color, uv);
    color = applySharpen(color, uv);

    vec2 centered = uv - vec2(0.5);
    float radialDistance = length(centered) / 0.70710678;
    float vignetteMask = smoothstep(0.38, 1.0, radialDistance);
    color *= 1.0 - vignetteMask * u_finish0.x * 0.52;

    if (u_finish0.z > 0.001) {
        float noise = randomNoise(
            uv * vec2(1920.0, 1080.0) +
            vec2(u_time * 37.0, u_time * 19.0)
        ) - 0.5;
        color += noise * u_finish0.z * 0.035;
    }

    if (u_reactive0.x > 0.5) {
        float reactiveAmount = u_reactive0.y;
        float musicContribution = u_reactive1.z * u_reactive0.w;
        float speedContribution = u_reactive1.w * 0.22;
        float pulse = clamp(
            u_master.z + musicContribution + speedContribution,
            0.0,
            1.0
        );

        color *= 1.0 + pulse * reactiveAmount * 0.08;
        color += vec3(u_master.w * reactiveAmount * 0.15);
    }

    color = compressGamut(color);
    color = mix(original, color, clamp(u_master.x, 0.0, 1.0));
    gl_FragColor = vec4(clamp(color, 0.0, 1.0), sourceSample.a);
}
