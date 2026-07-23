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
    return dot(max(color, vec3(0.0)), vec3(0.2126, 0.7152, 0.0722));
}

float maximum3(vec3 value) {
    return max(value.r, max(value.g, value.b));
}

vec4 sampleScene(vec2 uv) {
    return texture2D(CC_Texture0, clamp(uv, vec2(0.0), vec2(1.0)));
}

vec3 blurScene(vec2 uv, vec2 offset, float quality) {
    vec3 sum = sampleScene(uv).rgb * 4.0;
    float weight = 4.0;

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

    if (quality > 0.58) {
        vec2 farOffset = offset * 2.0;
        sum += sampleScene(uv + vec2(farOffset.x, 0.0)).rgb;
        sum += sampleScene(uv - vec2(farOffset.x, 0.0)).rgb;
        sum += sampleScene(uv + vec2(0.0, farOffset.y)).rgb;
        sum += sampleScene(uv - vec2(0.0, farOffset.y)).rgb;
        weight += 4.0;
    }

    if (quality > 0.86) {
        vec2 farOffset = offset * 2.75;
        sum += sampleScene(uv + farOffset).rgb;
        sum += sampleScene(uv - farOffset).rgb;
        sum += sampleScene(uv + vec2(farOffset.x, -farOffset.y)).rgb;
        sum += sampleScene(uv + vec2(-farOffset.x, farOffset.y)).rgb;
        weight += 4.0;
    }

    return sum / max(weight, 0.0001);
}

float thresholdMask(vec3 color, float threshold, float knee) {
    float brightness = max(luminance(color), maximum3(color) * 0.72);
    float width = mix(0.01, 0.22, clamp(knee, 0.0, 1.0));
    return smoothstep(threshold - width, threshold + width, brightness);
}

vec3 balancedLightColor(vec3 color, float coloredAmount) {
    color = clamp(color, 0.0, 1.0);
    float luma = luminance(color);
    vec3 neutral = vec3(luma);
    vec3 colored = mix(neutral, color, clamp(coloredAmount, 0.0, 1.0));
    float peak = maximum3(colored);
    if (peak > 1.0) colored /= peak;
    return max(colored, vec3(0.0));
}

vec3 bloomContribution(vec2 uv) {
    if (u_bloom0.x < 0.5 || u_bloom0.y <= 0.001) return vec3(0.0);

    float threshold = mix(0.42, 0.98, clamp(u_bloom0.z, 0.0, 1.0));
    float radius = mix(1.0, 15.0, clamp(u_bloom0.w, 0.0, 1.0));
    float quality = max(u_bloom1.y, u_bloom1.z);
    vec2 offset = u_texelSize * radius;

    vec3 sum = vec3(0.0);
    float weight = 0.0;

    vec3 center = sampleScene(uv).rgb;
    float centerMask = thresholdMask(center, threshold, u_bloom1.x);
    sum += balancedLightColor(center, 0.20) * centerMask * 4.0;
    weight += 4.0;

    vec3 c1 = sampleScene(uv + vec2(offset.x, 0.0)).rgb;
    vec3 c2 = sampleScene(uv - vec2(offset.x, 0.0)).rgb;
    vec3 c3 = sampleScene(uv + vec2(0.0, offset.y)).rgb;
    vec3 c4 = sampleScene(uv - vec2(0.0, offset.y)).rgb;
    sum += balancedLightColor(c1, 0.20) * thresholdMask(c1, threshold, u_bloom1.x);
    sum += balancedLightColor(c2, 0.20) * thresholdMask(c2, threshold, u_bloom1.x);
    sum += balancedLightColor(c3, 0.20) * thresholdMask(c3, threshold, u_bloom1.x);
    sum += balancedLightColor(c4, 0.20) * thresholdMask(c4, threshold, u_bloom1.x);
    weight += 4.0;

    if (quality > 0.18) {
        vec3 d1 = sampleScene(uv + offset).rgb;
        vec3 d2 = sampleScene(uv - offset).rgb;
        vec3 d3 = sampleScene(uv + vec2(offset.x, -offset.y)).rgb;
        vec3 d4 = sampleScene(uv + vec2(-offset.x, offset.y)).rgb;
        sum += balancedLightColor(d1, 0.20) * thresholdMask(d1, threshold, u_bloom1.x);
        sum += balancedLightColor(d2, 0.20) * thresholdMask(d2, threshold, u_bloom1.x);
        sum += balancedLightColor(d3, 0.20) * thresholdMask(d3, threshold, u_bloom1.x);
        sum += balancedLightColor(d4, 0.20) * thresholdMask(d4, threshold, u_bloom1.x);
        weight += 4.0;
    }

    if (quality > 0.62) {
        vec2 farOffset = offset * 2.0;
        vec3 f1 = sampleScene(uv + vec2(farOffset.x, 0.0)).rgb;
        vec3 f2 = sampleScene(uv - vec2(farOffset.x, 0.0)).rgb;
        vec3 f3 = sampleScene(uv + vec2(0.0, farOffset.y)).rgb;
        vec3 f4 = sampleScene(uv - vec2(0.0, farOffset.y)).rgb;
        sum += balancedLightColor(f1, 0.18) * thresholdMask(f1, threshold, u_bloom1.x);
        sum += balancedLightColor(f2, 0.18) * thresholdMask(f2, threshold, u_bloom1.x);
        sum += balancedLightColor(f3, 0.18) * thresholdMask(f3, threshold, u_bloom1.x);
        sum += balancedLightColor(f4, 0.18) * thresholdMask(f4, threshold, u_bloom1.x);
        weight += 4.0;
    }

    return (sum / max(weight, 0.0001)) * clamp(u_bloom0.y, 0.0, 1.0) * 0.62;
}

vec3 emissiveContribution(vec2 uv, vec3 source, vec3 localBlur) {
    if (u_emissive0.x < 0.5 || u_emissive0.y <= 0.001) return vec3(0.0);

    float threshold = mix(0.38, 0.96, clamp(u_emissive0.z, 0.0, 1.0));
    float radius = mix(1.0, 12.0, clamp(u_emissive0.w, 0.0, 1.0));
    vec3 blurred = blurScene(uv, u_texelSize * radius, max(u_bloom1.z, 0.25));

    float brightMask = thresholdMask(blurred, threshold, 0.18);
    float edgeMask = clamp(length(source - localBlur) * 2.0, 0.0, 1.0);
    float playerMask = exp(-dot(uv - vec2(0.24, 0.50), uv - vec2(0.24, 0.50)) * 34.0);
    float particleMask = brightMask * edgeMask;
    float categoryMask = clamp(
        u_emissive1.y * playerMask +
        u_emissive1.z * max(edgeMask, 0.12) +
        u_emissive1.w * particleMask,
        0.0,
        1.0
    );

    float coloredAmount = mix(0.20, 0.58, clamp(u_emissive1.x, 0.0, 1.0));
    vec3 emission = balancedLightColor(blurred, coloredAmount);
    return emission * brightMask * categoryMask * clamp(u_emissive0.y, 0.0, 1.0) * 0.55;
}

float ambientOcclusion(vec2 uv, vec3 source) {
    if (u_ao0.x < 0.5 || u_ao0.y <= 0.001) return 0.0;

    float radius = mix(1.0, 8.0, clamp(u_ao0.z, 0.0, 1.0));
    vec2 offset = u_texelSize * radius;
    float center = luminance(source);
    float surrounding =
        luminance(sampleScene(uv + vec2(offset.x, 0.0)).rgb) +
        luminance(sampleScene(uv - vec2(offset.x, 0.0)).rgb) +
        luminance(sampleScene(uv + vec2(0.0, offset.y)).rgb) +
        luminance(sampleScene(uv - vec2(0.0, offset.y)).rgb);
    surrounding *= 0.25;

    if (max(u_ao1.x, u_ao1.y) > 0.42) {
        float diagonal =
            luminance(sampleScene(uv + offset).rgb) +
            luminance(sampleScene(uv - offset).rgb) +
            luminance(sampleScene(uv + vec2(offset.x, -offset.y)).rgb) +
            luminance(sampleScene(uv + vec2(-offset.x, offset.y)).rgb);
        surrounding = mix(surrounding, diagonal * 0.25, 0.35);
    }

    float cavity = max(surrounding - center, 0.0);
    float edge = abs(surrounding - center);
    float raw = clamp(cavity * 1.10 + edge * 0.22, 0.0, 1.0);
    float darkness = mix(0.08, 0.42, clamp(u_ao0.w, 0.0, 1.0));
    return min(raw * clamp(u_ao0.y, 0.0, 1.0) * darkness, 0.22);
}

vec3 reflectionContribution(vec2 uv, vec3 source, vec3 localBlur) {
    if (u_reflection0.x < 0.5 || u_reflection0.y <= 0.001) return vec3(0.0);

    float distanceOffset = (clamp(u_reflection1.x, 0.0, 1.0) - 0.5) * 0.12;
    float wave = sin(uv.y * 38.0 + u_time * 1.20) * clamp(u_reflection1.y, 0.0, 1.0) * 0.007;
    vec2 reflectionUV = vec2(
        uv.x + wave,
        clamp(1.0 - uv.y + distanceOffset, 0.0, 1.0)
    );

    vec2 blurOffset = u_texelSize * mix(1.0, 9.0, clamp(u_reflection0.w, 0.0, 1.0));
    vec3 reflected = blurScene(reflectionUV, blurOffset, max(u_bloom1.z, 0.22));
    reflected = mix(vec3(luminance(reflected)), reflected, 0.42);

    float playerMask = exp(-dot(reflectionUV - vec2(0.24, 0.50), reflectionUV - vec2(0.24, 0.50)) * 30.0);
    float edgeMask = clamp(length(source - localBlur) * 2.0, 0.0, 1.0);
    float particleMask = smoothstep(0.72, 0.98, luminance(reflected)) * edgeMask;
    float categoryMask = clamp(
        u_reflection1.z * playerMask +
        u_reflection1.w * max(edgeMask, 0.12) +
        u_reflection2.x * particleMask,
        0.0,
        1.0
    );

    float lowerScreenMask = 1.0 - smoothstep(0.18, 0.82, uv.y);
    float strength = clamp(u_reflection0.y, 0.0, 1.0) * clamp(u_reflection0.z, 0.0, 1.0);
    return balancedLightColor(reflected, 0.36) * categoryMask * max(lowerScreenMask, 0.06) * strength * 0.32;
}

vec3 lightRaysContribution(vec2 uv) {
    if (u_rays0.x < 0.5 || u_rays0.y <= 0.001) return vec3(0.0);

    vec2 center = clamp(u_rays1.zw, vec2(0.0), vec2(1.0));
    vec2 direction = center - uv;
    float lengthFactor = mix(0.03, 0.30, clamp(u_rays0.z, 0.0, 1.0));
    float threshold = mix(0.52, 0.98, clamp(u_rays1.y, 0.0, 1.0));
    float decay = mix(0.62, 0.90, 1.0 - clamp(u_rays1.x, 0.0, 1.0));
    float blurScale = mix(0.75, 1.25, clamp(u_rays0.w, 0.0, 1.0));

    vec3 rays = vec3(0.0);
    float weight = 0.0;
    float currentDecay = 1.0;

    vec2 step1 = direction * lengthFactor * 0.16 * blurScale;
    vec2 step2 = direction * lengthFactor * 0.34 * blurScale;
    vec2 step3 = direction * lengthFactor * 0.54 * blurScale;
    vec2 step4 = direction * lengthFactor * 0.76 * blurScale;
    vec2 step5 = direction * lengthFactor * 1.00 * blurScale;

    vec3 c1 = sampleScene(uv + step1).rgb;
    currentDecay *= decay;
    rays += balancedLightColor(c1, 0.18) * thresholdMask(c1, threshold, 0.12) * currentDecay;
    weight += currentDecay;

    vec3 c2 = sampleScene(uv + step2).rgb;
    currentDecay *= decay;
    rays += balancedLightColor(c2, 0.18) * thresholdMask(c2, threshold, 0.12) * currentDecay;
    weight += currentDecay;

    vec3 c3 = sampleScene(uv + step3).rgb;
    currentDecay *= decay;
    rays += balancedLightColor(c3, 0.18) * thresholdMask(c3, threshold, 0.12) * currentDecay;
    weight += currentDecay;

    vec3 c4 = sampleScene(uv + step4).rgb;
    currentDecay *= decay;
    rays += balancedLightColor(c4, 0.18) * thresholdMask(c4, threshold, 0.12) * currentDecay;
    weight += currentDecay;

    if (u_bloom1.z > 0.52) {
        vec3 c5 = sampleScene(uv + step5).rgb;
        currentDecay *= decay;
        rays += balancedLightColor(c5, 0.18) * thresholdMask(c5, threshold, 0.12) * currentDecay;
        weight += currentDecay;
    }

    return rays / max(weight, 0.0001) * clamp(u_rays0.y, 0.0, 1.0) * 0.42;
}

void main() {
    vec4 sourceSample = sampleScene(v_texCoord);
    vec3 source = sourceSample.rgb;
    vec3 localBlur = blurScene(v_texCoord, u_texelSize * 2.0, max(u_bloom1.z, 0.20));

    float ao = ambientOcclusion(v_texCoord, source);
    vec3 additive =
        reflectionContribution(v_texCoord, source, localBlur) +
        emissiveContribution(v_texCoord, source, localBlur) +
        bloomContribution(v_texCoord) +
        lightRaysContribution(v_texCoord);

    float peak = maximum3(additive);
    if (peak > 1.0) additive /= peak;

    // RGB contains additive lighting only. Alpha contains AO only. The final
    // pass always starts from the untouched full-resolution framebuffer.
    gl_FragColor = vec4(clamp(additive, 0.0, 1.0), clamp(ao, 0.0, 0.25));
}
