#ifdef GL_ES
precision highp float;
#endif

varying vec2 v_texCoord;

uniform sampler2D CC_Texture0;
uniform vec2 u_texelSize;
uniform float u_time;

uniform vec4 u_bloom0;
uniform vec4 u_bloom1;
uniform vec4 u_emissive0;
uniform vec4 u_emissive1;
uniform vec4 u_ao0;
uniform vec4 u_ao1;
uniform vec4 u_reflection0;
uniform vec4 u_reflection1;
uniform vec4 u_reflection2;
uniform vec4 u_rays0;
uniform vec4 u_rays1;

float luminance(vec3 color) {
    return dot(color, vec3(0.2126, 0.7152, 0.0722));
}

float maximum3(vec3 value) {
    return max(value.r, max(value.g, value.b));
}

float minimum3(vec3 value) {
    return min(value.r, min(value.g, value.b));
}

vec4 sampleScene(vec2 uv) {
    return texture2D(CC_Texture0, clamp(uv, vec2(0.001), vec2(0.999)));
}

float softThreshold(float value, float threshold, float knee) {
    float safeKnee = max(knee * 0.35, 0.0001);
    float soft = clamp(
        (value - threshold + safeKnee) / (2.0 * safeKnee),
        0.0,
        1.0
    );
    soft = soft * soft * (3.0 - 2.0 * soft);
    return max(value - threshold, 0.0) + soft * safeKnee;
}

vec3 extractedHighlight(vec2 uv, float threshold, float knee) {
    vec3 color = sampleScene(uv).rgb;
    float mask = softThreshold(luminance(color), threshold, knee);
    return color * mask;
}

vec3 qualityBlur(vec2 uv, vec2 offset, float quality) {
    vec3 sum = sampleScene(uv).rgb * 2.0;
    float weight = 2.0;

    sum += sampleScene(uv + vec2(offset.x, 0.0)).rgb;
    sum += sampleScene(uv - vec2(offset.x, 0.0)).rgb;
    sum += sampleScene(uv + vec2(0.0, offset.y)).rgb;
    sum += sampleScene(uv - vec2(0.0, offset.y)).rgb;
    weight += 4.0;

    if (quality > 0.18) {
        sum += sampleScene(uv + offset).rgb;
        sum += sampleScene(uv - offset).rgb;
        sum += sampleScene(uv + vec2(offset.x, -offset.y)).rgb;
        sum += sampleScene(uv + vec2(-offset.x, offset.y)).rgb;
        weight += 4.0;
    }

    if (quality > 0.48) {
        vec2 farOffset = offset * 2.0;
        sum += sampleScene(uv + vec2(farOffset.x, 0.0)).rgb;
        sum += sampleScene(uv - vec2(farOffset.x, 0.0)).rgb;
        sum += sampleScene(uv + vec2(0.0, farOffset.y)).rgb;
        sum += sampleScene(uv - vec2(0.0, farOffset.y)).rgb;
        weight += 4.0;
    }

    if (quality > 0.78) {
        vec2 farOffset = offset * 2.5;
        sum += sampleScene(uv + farOffset).rgb;
        sum += sampleScene(uv - farOffset).rgb;
        sum += sampleScene(uv + vec2(farOffset.x, -farOffset.y)).rgb;
        sum += sampleScene(uv + vec2(-farOffset.x, farOffset.y)).rgb;
        weight += 4.0;
    }

    return sum / weight;
}

vec3 bloomContribution(vec2 uv) {
    if (u_bloom0.x < 0.5 || u_bloom0.y <= 0.001) return vec3(0.0);

    float intensity = u_bloom0.y;
    float threshold = mix(0.12, 1.35, u_bloom0.z);
    float radius = mix(1.0, 18.0, u_bloom0.w);
    float knee = u_bloom1.x;
    float quality = max(u_bloom1.y, u_bloom1.z);
    vec2 offset = u_texelSize * radius;

    vec3 sum = extractedHighlight(uv, threshold, knee) * 2.0;
    float weight = 2.0;
    sum += extractedHighlight(uv + vec2(offset.x, 0.0), threshold, knee);
    sum += extractedHighlight(uv - vec2(offset.x, 0.0), threshold, knee);
    sum += extractedHighlight(uv + vec2(0.0, offset.y), threshold, knee);
    sum += extractedHighlight(uv - vec2(0.0, offset.y), threshold, knee);
    weight += 4.0;

    if (quality > 0.18) {
        sum += extractedHighlight(uv + offset, threshold, knee);
        sum += extractedHighlight(uv - offset, threshold, knee);
        sum += extractedHighlight(uv + vec2(offset.x, -offset.y), threshold, knee);
        sum += extractedHighlight(uv + vec2(-offset.x, offset.y), threshold, knee);
        weight += 4.0;
    }
    if (quality > 0.52) {
        vec2 farOffset = offset * 2.0;
        sum += extractedHighlight(uv + vec2(farOffset.x, 0.0), threshold, knee);
        sum += extractedHighlight(uv - vec2(farOffset.x, 0.0), threshold, knee);
        sum += extractedHighlight(uv + vec2(0.0, farOffset.y), threshold, knee);
        sum += extractedHighlight(uv - vec2(0.0, farOffset.y), threshold, knee);
        weight += 4.0;
    }
    if (quality > 0.82) {
        vec2 farOffset = offset * 2.75;
        sum += extractedHighlight(uv + farOffset, threshold, knee);
        sum += extractedHighlight(uv - farOffset, threshold, knee);
        sum += extractedHighlight(uv + vec2(farOffset.x, -farOffset.y), threshold, knee);
        sum += extractedHighlight(uv + vec2(-farOffset.x, farOffset.y), threshold, knee);
        weight += 4.0;
    }
    return (sum / weight) * intensity * 2.4;
}

vec3 emissiveContribution(vec2 uv, vec3 source, vec3 localBlur) {
    if (u_emissive0.x < 0.5 || u_emissive0.y <= 0.001) return vec3(0.0);

    float threshold = mix(0.10, 1.10, u_emissive0.z);
    float radius = mix(1.0, 14.0, u_emissive0.w);
    vec3 glowColor = qualityBlur(uv, u_texelSize * radius, max(u_bloom1.z, 0.25));

    float glowLuma = luminance(glowColor);
    float sourceLuma = luminance(source);
    float chroma = maximum3(glowColor) - minimum3(glowColor);
    float colorAffinity = clamp(chroma * 1.7 + maximum3(glowColor) * 0.35, 0.0, 1.5);
    float brightMask = smoothstep(threshold, threshold + 0.22, glowLuma);

    float playerMask = exp(-dot(uv - vec2(0.24, 0.50), uv - vec2(0.24, 0.50)) * 32.0);
    float edgeMask = clamp(length(source - localBlur) * 3.5, 0.0, 1.0);
    float particleMask = clamp(
        smoothstep(threshold, threshold + 0.16, sourceLuma) * edgeMask * 1.8,
        0.0,
        1.0
    );
    float categoryMask = clamp(
        u_emissive1.y * playerMask +
        u_emissive1.z * max(edgeMask, 0.18) +
        u_emissive1.w * particleMask,
        0.0,
        1.0
    );

    vec3 boostedColor = mix(
        glowColor,
        glowColor * (1.0 + colorAffinity * 1.8),
        u_emissive1.x
    );
    return boostedColor * brightMask * categoryMask * u_emissive0.y * 1.8;
}

float ambientOcclusion(vec2 uv, vec3 source) {
    if (u_ao0.x < 0.5 || u_ao0.y <= 0.001) return 0.0;

    float radius = mix(1.0, 10.0, u_ao0.z);
    vec2 offset = u_texelSize * radius;
    float center = luminance(source);
    float nearAverage =
        luminance(sampleScene(uv + vec2(offset.x, 0.0)).rgb) +
        luminance(sampleScene(uv - vec2(offset.x, 0.0)).rgb) +
        luminance(sampleScene(uv + vec2(0.0, offset.y)).rgb) +
        luminance(sampleScene(uv - vec2(0.0, offset.y)).rgb);
    nearAverage *= 0.25;

    float diagonalAverage = nearAverage;
    if (max(u_ao1.x, u_ao1.y) > 0.32) {
        diagonalAverage =
            luminance(sampleScene(uv + offset).rgb) +
            luminance(sampleScene(uv - offset).rgb) +
            luminance(sampleScene(uv + vec2(offset.x, -offset.y)).rgb) +
            luminance(sampleScene(uv + vec2(-offset.x, offset.y)).rgb);
        diagonalAverage *= 0.25;
    }

    float surrounding = mix(nearAverage, diagonalAverage, 0.45);
    float cavity = max(surrounding - center, 0.0);
    float edge = abs(surrounding - center);
    float occlusion = clamp(cavity * 1.8 + edge * 0.55, 0.0, 1.0);
    return occlusion * u_ao0.y * mix(0.25, 1.0, u_ao0.w);
}

vec3 reflectionContribution(vec2 uv, vec3 source, vec3 localBlur) {
    if (u_reflection0.x < 0.5 || u_reflection0.y <= 0.001) return vec3(0.0);

    float distanceOffset = (u_reflection1.x - 0.5) * 0.22;
    float wave = sin((uv.y * 45.0 + u_time * 1.7)) * u_reflection1.y * 0.018;
    vec2 reflectionUV = vec2(
        uv.x + wave,
        clamp(1.0 - uv.y + distanceOffset, 0.001, 0.999)
    );

    vec2 blurOffset = u_texelSize * mix(1.0, 12.0, u_reflection0.w);
    vec3 reflected = qualityBlur(reflectionUV, blurOffset, max(u_bloom1.z, 0.25));
    float playerMask = exp(
        -dot(reflectionUV - vec2(0.24, 0.50), reflectionUV - vec2(0.24, 0.50)) * 30.0
    );
    float edgeMask = clamp(length(source - localBlur) * 3.0, 0.0, 1.0);
    float particleMask = clamp(
        smoothstep(0.65, 0.95, luminance(reflected)) * edgeMask * 1.5,
        0.0,
        1.0
    );
    float categoryMask = clamp(
        u_reflection1.z * playerMask +
        u_reflection1.w * max(edgeMask, 0.20) +
        u_reflection2.x * particleMask,
        0.0,
        1.0
    );
    float lowerScreenMask = smoothstep(0.62, 0.05, uv.y);
    return reflected * categoryMask * max(lowerScreenMask, 0.12) *
        u_reflection0.y * u_reflection0.z;
}

vec3 lightRaysContribution(vec2 uv) {
    if (u_rays0.x < 0.5 || u_rays0.y <= 0.001) return vec3(0.0);

    vec2 center = u_rays1.zw;
    vec2 direction = center - uv;
    float lengthFactor = mix(0.03, 0.42, u_rays0.z);
    float blur = mix(0.65, 1.65, u_rays0.w);
    float threshold = mix(0.25, 1.20, u_rays1.y);
    float decay = mix(0.58, 0.94, 1.0 - u_rays1.x);

    vec3 rays = vec3(0.0);
    float weight = 0.0;
    float currentDecay = 1.0;
    vec2 step1 = direction * lengthFactor * 0.12 * blur;
    vec2 step2 = direction * lengthFactor * 0.24 * blur;
    vec2 step3 = direction * lengthFactor * 0.36 * blur;
    vec2 step4 = direction * lengthFactor * 0.50 * blur;
    vec2 step5 = direction * lengthFactor * 0.66 * blur;
    vec2 step6 = direction * lengthFactor * 0.82 * blur;
    vec2 step7 = direction * lengthFactor;
    vec2 step8 = direction * lengthFactor * 1.18 * blur;

    vec3 c1 = sampleScene(uv + step1).rgb;
    currentDecay *= decay;
    rays += c1 * smoothstep(threshold, threshold + 0.18, luminance(c1)) * currentDecay;
    weight += currentDecay;
    vec3 c2 = sampleScene(uv + step2).rgb;
    currentDecay *= decay;
    rays += c2 * smoothstep(threshold, threshold + 0.18, luminance(c2)) * currentDecay;
    weight += currentDecay;
    vec3 c3 = sampleScene(uv + step3).rgb;
    currentDecay *= decay;
    rays += c3 * smoothstep(threshold, threshold + 0.18, luminance(c3)) * currentDecay;
    weight += currentDecay;
    vec3 c4 = sampleScene(uv + step4).rgb;
    currentDecay *= decay;
    rays += c4 * smoothstep(threshold, threshold + 0.18, luminance(c4)) * currentDecay;
    weight += currentDecay;

    if (u_bloom1.z > 0.18) {
        vec3 c5 = sampleScene(uv + step5).rgb;
        currentDecay *= decay;
        rays += c5 * smoothstep(threshold, threshold + 0.18, luminance(c5)) * currentDecay;
        weight += currentDecay;
        vec3 c6 = sampleScene(uv + step6).rgb;
        currentDecay *= decay;
        rays += c6 * smoothstep(threshold, threshold + 0.18, luminance(c6)) * currentDecay;
        weight += currentDecay;
    }
    if (u_bloom1.z > 0.58) {
        vec3 c7 = sampleScene(uv + step7).rgb;
        currentDecay *= decay;
        rays += c7 * smoothstep(threshold, threshold + 0.18, luminance(c7)) * currentDecay;
        weight += currentDecay;
        vec3 c8 = sampleScene(uv + step8).rgb;
        currentDecay *= decay;
        rays += c8 * smoothstep(threshold, threshold + 0.18, luminance(c8)) * currentDecay;
        weight += currentDecay;
    }
    return rays / max(weight, 0.0001) * u_rays0.y * 1.8;
}

void main() {
    vec4 sourceSample = sampleScene(v_texCoord);
    vec3 source = sourceSample.rgb;
    vec3 localBlur = qualityBlur(v_texCoord, u_texelSize * 2.0, max(u_bloom1.z, 0.20));

    float ao = ambientOcclusion(v_texCoord, source);
    vec3 color = source * (1.0 - ao);
    color += reflectionContribution(v_texCoord, source, localBlur);
    color += emissiveContribution(v_texCoord, source, localBlur);
    color += bloomContribution(v_texCoord);
    color += lightRaysContribution(v_texCoord);

    gl_FragColor = vec4(max(color, vec3(0.0)), sourceSample.a);
}
