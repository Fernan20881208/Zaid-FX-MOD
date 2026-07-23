#ifdef GL_ES
precision highp float;
#endif

varying vec2 v_texCoord;

uniform sampler2D CC_Texture0;
uniform sampler2D u_lightingTexture;
uniform vec2 u_texelSize;
uniform float u_time;

uniform vec4 u_pipelineFlags;
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
    return dot(max(color, vec3(0.0)), vec3(0.2126, 0.7152, 0.0722));
}

float maximum3(vec3 value) {
    return max(value.r, max(value.g, value.b));
}

float minimum3(vec3 value) {
    return min(value.r, min(value.g, value.b));
}

vec4 sampleOriginal(vec2 uv) {
    return texture2D(CC_Texture0, clamp(uv, vec2(0.0), vec2(1.0)));
}

vec4 sampleLighting(vec2 uv) {
    return texture2D(u_lightingTexture, clamp(uv, vec2(0.0), vec2(1.0)));
}

vec3 blurOriginal(vec2 uv, vec2 offset, float quality) {
    vec3 sum = sampleOriginal(uv).rgb * 4.0;
    float weight = 4.0;

    sum += sampleOriginal(uv + vec2(offset.x, 0.0)).rgb;
    sum += sampleOriginal(uv - vec2(offset.x, 0.0)).rgb;
    sum += sampleOriginal(uv + vec2(0.0, offset.y)).rgb;
    sum += sampleOriginal(uv - vec2(0.0, offset.y)).rgb;
    weight += 4.0;

    if (quality > 0.18) {
        sum += sampleOriginal(uv + offset).rgb;
        sum += sampleOriginal(uv - offset).rgb;
        sum += sampleOriginal(uv + vec2(offset.x, -offset.y)).rgb;
        sum += sampleOriginal(uv + vec2(-offset.x, offset.y)).rgb;
        weight += 4.0;
    }

    if (quality > 0.66) {
        vec2 farOffset = offset * 2.0;
        sum += sampleOriginal(uv + vec2(farOffset.x, 0.0)).rgb;
        sum += sampleOriginal(uv - vec2(farOffset.x, 0.0)).rgb;
        sum += sampleOriginal(uv + vec2(0.0, farOffset.y)).rgb;
        sum += sampleOriginal(uv - vec2(0.0, farOffset.y)).rgb;
        weight += 4.0;
    }

    return sum / max(weight, 0.0001);
}

vec3 adjustSaturation(vec3 color, float amount) {
    float luma = luminance(color);
    return vec3(luma) + (color - vec3(luma)) * amount;
}

vec3 safeColor(vec3 color) {
    color = max(color, vec3(0.0));
    float peak = maximum3(color);
    if (peak > 1.0) {
        float compressedPeak = peak / (1.0 + (peak - 1.0) * 0.72);
        color *= compressedPeak / peak;
    }
    return clamp(color, 0.0, 1.0);
}

vec3 applyChromaticAberration(vec2 uv, vec3 centerColor) {
    float amount = clamp(u_finish0.y, 0.0, 1.0) * 0.0022;
    if (amount <= 0.000001) return centerColor;

    vec2 radial = (uv - vec2(0.5)) * amount;
    return vec3(
        sampleOriginal(uv + radial).r,
        centerColor.g,
        sampleOriginal(uv - radial).b
    );
}

vec3 applyLighting(vec3 source, vec2 uv) {
    if (u_pipelineFlags.x < 0.5) return source;

    vec4 lighting = sampleLighting(uv);
    float ao = clamp(lighting.a, 0.0, 0.25);
    vec3 base = source * (1.0 - ao);
    vec3 additive = clamp(lighting.rgb, 0.0, 1.0);

    // Screen-style composition adds light without replacing the source hue.
    return base + additive * (vec3(1.0) - clamp(base, 0.0, 1.0));
}

vec3 applyHDR(vec3 color) {
    if (u_hdr0.x < 0.5) return color;

    float range = clamp(u_hdr0.y, 0.0, 1.0);
    float compression = clamp(u_hdr0.z, 0.0, 1.0);
    float shadowRecovery = clamp(u_hdr0.w, 0.0, 1.0);
    float blackPoint = clamp(u_hdr1.y, 0.0, 0.95) * 0.10;
    float whitePoint = mix(1.05, 2.80, clamp(u_hdr1.x, 0.0, 1.0));

    color = max(color - vec3(blackPoint), vec3(0.0));
    float luma = luminance(color);
    float shadowMask = 1.0 - smoothstep(0.03, 0.48, luma);
    color += vec3(shadowMask * shadowRecovery * 0.075);
    color *= mix(1.0, 1.30, range);

    float mappedLuma = luma * (1.0 + luma / max(whitePoint * whitePoint, 0.01)) /
        max(1.0 + luma, 0.0001);
    vec3 mapped = color * (mappedLuma / max(luminance(color), 0.0001));
    return mix(color, mapped, compression * 0.78);
}

vec3 applyColorGrading(vec3 color) {
    // Every midpoint maps exactly to the neutral operation.
    float exposureStops = clamp((u_color0.x - 0.5) * 1.6, -0.8, 0.8);
    color *= exp2(exposureStops);

    float contrastFactor = exp2((u_color0.y - 0.5) * 1.20);
    color = (color - vec3(0.50)) * contrastFactor + vec3(0.50);

    float saturationFactor = exp2((u_color0.z - 0.5) * 1.05);
    color = adjustSaturation(color, saturationFactor);

    float chroma = maximum3(color) - minimum3(color);
    float vibranceMask = 1.0 - smoothstep(0.08, 0.75, chroma);
    color = adjustSaturation(
        color,
        1.0 + clamp(u_color0.w, 0.0, 1.0) * vibranceMask * 0.16
    );

    float temperature = clamp((u_color1.y - 0.5) * 0.12, -0.06, 0.06);
    float tint = clamp((u_color1.z - 0.5) * 0.10, -0.05, 0.05);
    vec3 balance = vec3(
        1.0 + temperature + tint * 0.45,
        1.0 - tint,
        1.0 - temperature + tint * 0.45
    );
    color *= clamp(balance, vec3(0.88), vec3(1.12));

    float luma = luminance(color);
    float highlightMask = smoothstep(0.55, 1.0, luma);
    float shadowMask = 1.0 - smoothstep(0.03, 0.48, luma);
    color *= 1.0 + (u_color1.w - 0.5) * highlightMask * 0.20;
    color += vec3((u_color2.x - 0.5) * shadowMask * 0.10);

    float gammaExponent = exp2((0.5 - u_color1.x) * 0.70);
    color = pow(max(color, vec3(0.0)), vec3(gammaExponent));

    float blackPoint = clamp(u_color2.z, 0.0, 0.95);
    float whitePoint = clamp(u_color2.y, blackPoint + 0.01, 1.0);
    color = (color - vec3(blackPoint)) / max(whitePoint - blackPoint, 0.01);
    return color;
}

vec3 applyLocalContrast(vec3 color, vec2 uv) {
    if (u_local0.x < 0.5 || u_local0.y <= 0.001) return color;

    vec2 radius = u_texelSize * mix(1.0, 7.0, clamp(u_local0.w, 0.0, 1.0));
    vec3 source = sampleOriginal(uv).rgb;
    vec3 localAverage = blurOriginal(uv, radius, u_master.y);
    vec3 detail = source - localAverage;
    float detailMask = smoothstep(0.015, 0.20, length(detail));
    float strength = clamp(u_local0.y, 0.0, 1.0) *
        mix(0.25, 0.80, clamp(u_local0.z, 0.0, 1.0));
    return color + detail * strength * detailMask;
}

vec3 applySpecular(vec3 color, vec2 uv) {
    if (u_specular0.x < 0.5 || u_specular0.y <= 0.001) return color;

    float radius = mix(1.0, 4.0, clamp(u_specular0.z, 0.0, 1.0));
    vec2 offset = u_texelSize * radius;
    vec3 source = sampleOriginal(uv).rgb;
    float center = luminance(source);
    float north = luminance(sampleOriginal(uv + vec2(0.0, offset.y)).rgb);
    float south = luminance(sampleOriginal(uv - vec2(0.0, offset.y)).rgb);
    float east = luminance(sampleOriginal(uv + vec2(offset.x, 0.0)).rgb);
    float west = luminance(sampleOriginal(uv - vec2(offset.x, 0.0)).rgb);
    float edge = abs(east - west) + abs(north - south);

    float threshold = mix(0.42, 0.96, clamp(u_specular0.w, 0.0, 1.0));
    float mask = smoothstep(threshold, threshold + 0.12, center + edge * 0.72);
    vec3 metallic = mix(vec3(1.0), adjustSaturation(source, 0.40), clamp(u_specular1.x, 0.0, 1.0) * 0.32);
    vec3 glass = mix(metallic, vec3(0.96, 0.985, 1.0), clamp(u_specular1.y, 0.0, 1.0) * 0.18);
    return color + glass * mask * clamp(u_specular0.y, 0.0, 1.0) * (0.035 + edge * 0.16);
}

vec3 applyDepthBlur(vec3 color, vec2 uv) {
    if (u_depth0.x < 0.5 || u_depth0.y <= 0.001) return color;

    float separation = mix(0.12, 0.78, clamp(u_depth0.w, 0.0, 1.0));
    float transition = mix(0.06, 0.34, clamp(u_depth1.x, 0.0, 1.0));
    float sceneLuma = luminance(sampleOriginal(uv).rgb);
    float verticalBackground = smoothstep(separation - transition, separation + transition, uv.y);
    float darkBackground = 1.0 - smoothstep(0.08, 0.48, sceneLuma);
    float playerProtection = exp(-dot(uv - vec2(0.24, 0.50), uv - vec2(0.24, 0.50)) * 48.0);
    float backgroundMask = clamp(
        max(verticalBackground * 0.42, darkBackground * 0.48) - playerProtection,
        0.0,
        1.0
    );

    vec2 blurRadius = u_texelSize * mix(1.0, 9.0, clamp(u_depth0.y, 0.0, 1.0));
    vec3 blurredSource = blurOriginal(uv, blurRadius, u_master.y);
    color = mix(color, blurredSource, backgroundMask * clamp(u_depth0.y, 0.0, 1.0) * 0.55);

    vec3 fineBlur = blurOriginal(uv, u_texelSize * 1.5, u_master.y);
    vec3 foregroundDetail = sampleOriginal(uv).rgb - fineBlur;
    color += foregroundDetail * clamp(u_depth0.z, 0.0, 1.0) * (1.0 - backgroundMask) * 0.22;
    return color;
}

vec3 applySharpen(vec3 color, vec2 uv) {
    if (u_sharpen0.x < 0.5 || u_sharpen0.y <= 0.001) return color;

    vec2 radius = u_texelSize * mix(0.75, 3.2, clamp(u_sharpen0.z, 0.0, 1.0));
    vec3 source = sampleOriginal(uv).rgb;
    vec3 blurred = blurOriginal(uv, radius, u_master.y);
    vec3 detail = source - blurred;
    float amount = length(detail);
    float threshold = mix(0.006, 0.14, clamp(u_sharpen0.w, 0.0, 1.0));
    float mask = smoothstep(threshold, threshold + 0.055, amount);
    return color + detail * clamp(u_sharpen0.y, 0.0, 1.0) * mask * 0.52;
}

float randomNoise(vec2 point) {
    return fract(sin(dot(point, vec2(12.9898, 78.233))) * 43758.5453);
}

void main() {
    vec2 uv = v_texCoord;
    vec4 sourceSample = sampleOriginal(uv);

    // Effect Intensity = 0 is an exact pass-through validation mode.
    if (u_pipelineFlags.z > 0.5 || u_master.x <= 0.0001) {
        gl_FragColor = sourceSample;
        return;
    }

    vec3 original = sourceSample.rgb;
    vec3 color = applyChromaticAberration(uv, original);
    color = applyLighting(color, uv);
    color = applyHDR(color);
    color = applyColorGrading(color);
    color = applyLocalContrast(color, uv);
    color = applySpecular(color, uv);
    color = applyDepthBlur(color, uv);
    color = applySharpen(color, uv);

    vec2 centered = uv - vec2(0.5);
    float radialDistance = length(centered) / 0.70710678;
    float vignetteMask = smoothstep(0.40, 1.0, radialDistance);
    color *= 1.0 - vignetteMask * clamp(u_finish0.x, 0.0, 1.0) * 0.42;

    if (u_finish0.z > 0.001) {
        float noise = randomNoise(
            uv * vec2(1920.0, 1080.0) + vec2(u_time * 37.0, u_time * 19.0)
        ) - 0.5;
        color += vec3(noise * clamp(u_finish0.z, 0.0, 1.0) * 0.022);
    }

    if (u_reactive0.x > 0.5) {
        float reactiveAmount = clamp(u_reactive0.y, 0.0, 1.0);
        float musicContribution = u_reactive1.z * clamp(u_reactive0.w, 0.0, 1.0);
        float speedContribution = u_reactive1.w * 0.16;
        float pulse = clamp(u_master.z + musicContribution + speedContribution, 0.0, 1.0);
        color *= 1.0 + pulse * reactiveAmount * 0.045;
        color += vec3(clamp(u_master.w, 0.0, 0.35) * reactiveAmount * 0.08);
    }

    color = safeColor(color);
    color = mix(original, color, clamp(u_master.x, 0.0, 1.0));
    gl_FragColor = vec4(clamp(color, 0.0, 1.0), sourceSample.a);
}
