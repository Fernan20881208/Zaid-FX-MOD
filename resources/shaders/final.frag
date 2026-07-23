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

vec3 acesToneMap(vec3 color) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp(
        (color * (a * color + b)) /
        (color * (c * color + d) + e),
        0.0,
        1.0
    );
}

vec3 normalizeLevels(vec3 color, float blackPoint, float whitePoint) {
    float safeWhite = max(whitePoint, blackPoint + 0.01);
    return max(
        (color - vec3(blackPoint)) /
        max(safeWhite - blackPoint, 0.01),
        vec3(0.0)
    );
}

vec3 applyHDR(vec3 color) {
    if (u_hdr0.x < 0.5) {
        return color;
    }

    color = normalizeLevels(color, u_hdr1.y, u_hdr1.x);

    float luma = luminance(color);
    float shadowMask = 1.0 - smoothstep(0.05, 0.55, luma);
    color += (vec3(1.0) - color) *
        shadowMask *
        u_hdr0.w *
        0.38;

    float rangeScale = mix(1.0, 2.25, u_hdr0.y);
    color *= rangeScale;

    float compression = mix(0.05, 1.40, u_hdr0.z);
    vec3 compressed = color / (vec3(1.0) + color * compression);
    color = mix(color, compressed, u_hdr0.z);

    return mix(color, acesToneMap(color), 0.45 + u_hdr0.y * 0.45);
}

vec3 applyColorGrading(vec3 color) {
    float exposureStops = (u_color0.x - 0.5) * 4.0;
    color *= exp2(exposureStops);

    float contrastFactor = exp2((u_color0.y - 0.5) * 1.8);
    color = (color - vec3(0.5)) * contrastFactor + vec3(0.5);

    float luma = luminance(color);
    float saturationFactor = exp2((u_color0.z - 0.5) * 1.8);
    color = mix(vec3(luma), color, saturationFactor);

    float chroma = maximum3(color) - minimum3(color);
    float vibranceMask = 1.0 - clamp(chroma, 0.0, 1.0);
    color = mix(
        vec3(luminance(color)),
        color,
        1.0 + u_color0.w * vibranceMask * 0.9
    );

    vec3 temperatureShift = vec3(1.0, 0.0, -1.0) *
        (u_color1.y - 0.5) *
        0.22;
    vec3 tintShift = vec3(0.25, -0.45, 0.25) *
        (u_color1.z - 0.5) *
        0.22;
    color += temperatureShift + tintShift;

    luma = luminance(color);
    float highlightMask = smoothstep(0.48, 1.0, luma);
    float shadowMask = 1.0 - smoothstep(0.0, 0.52, luma);
    color *= 1.0 + (u_color1.w - 0.5) * highlightMask * 0.85;
    color += (u_color2.x - 0.5) * shadowMask * 0.28;

    float gammaExponent = exp2((0.5 - u_color1.x) * 1.5);
    color = pow(max(color, vec3(0.0)), vec3(gammaExponent));

    return normalizeLevels(color, u_color2.z, u_color2.y);
}

vec3 applyLocalContrast(vec3 color, vec2 uv) {
    if (u_local0.x < 0.5 || u_local0.y <= 0.001) {
        return color;
    }

    vec2 radius = u_texelSize * mix(1.0, 9.0, u_local0.w);
    vec3 localAverage = blurLighting(uv, radius, u_master.y);
    vec3 detail = color - localAverage;

    float detailMask = smoothstep(
        0.015,
        0.24,
        length(detail)
    );
    color += detail *
        u_local0.y *
        mix(0.55, 1.7, u_local0.z) *
        detailMask;

    return color;
}

vec3 applySpecular(vec3 color, vec2 uv) {
    if (u_specular0.x < 0.5 || u_specular0.y <= 0.001) {
        return color;
    }

    float radius = mix(1.0, 6.0, u_specular0.z);
    vec2 offset = u_texelSize * radius;

    float center = luminance(sampleOriginal(uv).rgb);
    float north = luminance(sampleOriginal(uv + vec2(0.0, offset.y)).rgb);
    float south = luminance(sampleOriginal(uv - vec2(0.0, offset.y)).rgb);
    float east = luminance(sampleOriginal(uv + vec2(offset.x, 0.0)).rgb);
    float west = luminance(sampleOriginal(uv - vec2(offset.x, 0.0)).rgb);

    float edge = abs(east - west) + abs(north - south);
    float threshold = mix(0.18, 1.0, u_specular0.w);
    float highlight = smoothstep(
        threshold,
        threshold + 0.18,
        center + edge * 1.6
    );

    vec3 metallicColor = mix(
        vec3(1.0),
        normalize(max(color, vec3(0.001))),
        u_specular1.x
    );
    vec3 glassColor = mix(
        metallicColor,
        vec3(0.72, 0.90, 1.0),
        u_specular1.y
    );

    return color + glassColor *
        highlight *
        u_specular0.y *
        (0.25 + edge * 1.2);
}

vec3 applyDepthBlur(vec3 color, vec2 uv) {
    if (u_depth0.x < 0.5 || u_depth0.y <= 0.001) {
        return color;
    }

    float separation = mix(0.08, 0.82, u_depth0.w);
    float transition = mix(0.04, 0.40, u_depth1.x);
    float sceneLuma = luminance(sampleOriginal(uv).rgb);

    float verticalBackground = smoothstep(
        separation - transition,
        separation + transition,
        uv.y
    );
    float darkBackground = 1.0 - smoothstep(
        0.10,
        0.60,
        sceneLuma
    );

    float playerProtection = exp(
        -dot(
            uv - vec2(0.24, 0.50),
            uv - vec2(0.24, 0.50)
        ) * 45.0
    );

    float backgroundMask = clamp(
        max(verticalBackground * 0.55, darkBackground * 0.75) -
        playerProtection,
        0.0,
        1.0
    );

    vec2 blurRadius = u_texelSize * mix(1.0, 13.0, u_depth0.y);
    vec3 blurred = blurLighting(uv, blurRadius, u_master.y);
    color = mix(color, blurred, backgroundMask * u_depth0.y);

    vec3 fineBlur = blurLighting(uv, u_texelSize * 1.5, u_master.y);
    vec3 foregroundDetail = color - fineBlur;
    color += foregroundDetail *
        u_depth0.z *
        (1.0 - backgroundMask) *
        0.7;

    return color;
}

vec3 applySharpen(vec3 color, vec2 uv) {
    if (u_sharpen0.x < 0.5 || u_sharpen0.y <= 0.001) {
        return color;
    }

    vec2 radius = u_texelSize * mix(0.75, 4.5, u_sharpen0.z);
    vec3 blurred = blurLighting(uv, radius, u_master.y);
    vec3 detail = color - blurred;
    float amount = length(detail);
    float threshold = mix(0.0, 0.18, u_sharpen0.w);
    float mask = smoothstep(threshold, threshold + 0.08, amount);

    return color + detail * u_sharpen0.y * mask * 1.35;
}

float randomNoise(vec2 point) {
    return fract(
        sin(dot(point, vec2(12.9898, 78.233))) *
        43758.5453
    );
}

void main() {
    vec2 uv = v_texCoord;
    vec4 sourceSample = sampleOriginal(uv);

    float chromaticAmount = u_finish0.y * 0.012;
    vec2 radial = (uv - vec2(0.5)) * chromaticAmount;
    vec3 original = vec3(
        sampleOriginal(uv + radial).r,
        sourceSample.g,
        sampleOriginal(uv - radial).b
    );

    vec3 lighting = sampleLighting(uv).rgb;
    vec3 color = lighting;

    color = applyHDR(color);
    color = applyColorGrading(color);
    color = applyLocalContrast(color, uv);
    color = applySpecular(color, uv);
    color = applyDepthBlur(color, uv);
    color = applySharpen(color, uv);

    vec2 centered = uv - vec2(0.5);
    float radialDistance = length(centered) / 0.70710678;
    float vignetteMask = smoothstep(0.34, 1.0, radialDistance);
    color *= 1.0 - vignetteMask * u_finish0.x * 0.72;

    if (u_finish0.z > 0.001) {
        float noise = randomNoise(
            uv * vec2(1920.0, 1080.0) +
            vec2(u_time * 37.0, u_time * 19.0)
        ) - 0.5;
        color += noise * u_finish0.z * 0.085;
    }

    if (u_reactive0.x > 0.5) {
        float reactiveAmount = u_reactive0.y;
        float musicContribution = u_reactive1.z * u_reactive0.w;
        float speedContribution = u_reactive1.w * 0.30;
        float pulse = clamp(
            u_master.z + musicContribution + speedContribution,
            0.0,
            1.0
        );

        color *= 1.0 + pulse * reactiveAmount * 0.20;
        color += vec3(
            u_master.w * reactiveAmount * 0.55
        );
    }

    color = mix(original, color, clamp(u_master.x, 0.0, 1.0));
    gl_FragColor = vec4(clamp(color, 0.0, 1.0), sourceSample.a);
}
