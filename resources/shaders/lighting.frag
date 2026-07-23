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
    float width = max(knee * 0.22, 0.008);
    float lower = threshold - width;
    float upper = threshold + width;
    float soft = smoothstep(lower, upper, value);
    return max(value - threshold, 0.0) + soft * width * 0.55;
}

vec3 lightHue(vec3 color, float coloredAmount) {
    float luma = luminance(color);
    float peak = max(maximum3(color), 0.001);
    vec3 normalizedHue = color / peak;
    vec3 neutral = vec3(max(luma, peak * 0.58));
    return mix(neutral, normalizedHue * max(luma, peak * 0.72), coloredAmount);
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

    if (quality > 0.52) {
        vec2 farOffset = offset * 2.0;
        sum += sampleScene(uv + vec2(farOffset.x, 0.0)).rgb;
        sum += sampleScene(uv - vec2(farOffset.x, 0.0)).rgb;
        sum += sampleScene(uv + vec2(0.0, farOffset.y)).rgb;
        sum += sampleScene(uv - vec2(0.0, farOffset.y)).rgb;
        weight += 4.0;
    }

    if (quality > 0.82) {
        vec2 farOffset = offset * 2.75;
        sum += sampleScene(uv + farOffset).rgb;
        sum += sampleScene(uv - farOffset).rgb;
        sum += sampleScene(uv + vec2(farOffset.x, -farOffset.y)).rgb;
        sum += sampleScene(uv + vec2(-farOffset.x, farOffset.y)).rgb;
        weight += 4.0;
    }

    return sum / weight;
}

vec3 extractedHighlight(vec2 uv, float threshold, float knee, float colorAmount) {
    vec3 color = sampleScene(uv).rgb;
    float mask = softThreshold(luminance(color), threshold, knee);
    return lightHue(color, colorAmount) * mask;
}

vec3 bloomContribution(vec2 uv) {
    if (u_bloom0.x < 0.5 || u_bloom0.y <= 0.001) return vec3(0.0);

    float threshold = mix(0.35, 1.08, u_bloom0.z);
    float radius = mix(1.0, 17.0, u_bloom0.w);
    float quality = max(u_bloom1.y, u_bloom1.z);
    float colorAmount = mix(0.16, 0.34, clamp(u_emissive1.x, 0.0, 1.0));
    vec2 offset = u_texelSize * radius;

    vec3 sum = extractedHighlight(uv, threshold, u_bloom1.x, colorAmount) * 2.0;
    float weight = 2.0;

    sum += extractedHighlight(uv + vec2(offset.x, 0.0), threshold, u_bloom1.x, colorAmount);
    sum += extractedHighlight(uv - vec2(offset.x, 0.0), threshold, u_bloom1.x, colorAmount);
    sum += extractedHighlight(uv + vec2(0.0, offset.y), threshold, u_bloom1.x, colorAmount);
    sum += extractedHighlight(uv - vec2(0.0, offset.y), threshold, u_bloom1.x, colorAmount);
    weight += 4.0;

    if (quality > 0.18) {
        sum += extractedHighlight(uv + offset, threshold, u_bloom1.x, colorAmount);
        sum += extractedHighlight(uv - offset, threshold, u_bloom1.x, colorAmount);
        sum += extractedHighlight(uv + vec2(offset.x, -offset.y), threshold, u_bloom1.x, colorAmount);
        sum += extractedHighlight(uv + vec2(-offset.x, offset.y), threshold, u_bloom1.x, colorAmount);
        weight += 4.0;
    }

    if (quality > 0.55) {
        vec2 farOffset = offset * 2.1;
        sum += extractedHighlight(uv + vec2(farOffset.x, 0.0), threshold, u_bloom1.x, colorAmount);
        sum += extractedHighlight(uv - vec2(farOffset.x, 0.0), threshold, u_bloom1.x, colorAmount);
        sum += extractedHighlight(uv + vec2(0.0, farOffset.y), threshold, u_bloom1.x, colorAmount);
        sum += extractedHighlight(uv - vec2(0.0, farOffset.y), threshold, u_bloom1.x, colorAmount);
        weight += 4.0;
    }

    if (quality > 0.84) {
        vec2 farOffset = offset * 2.8;
        sum += extractedHighlight(uv + farOffset, threshold, u_bloom1.x, colorAmount);
        sum += extractedHighlight(uv - farOffset, threshold, u_bloom1.x, colorAmount);
        weight += 2.0;
    }

    return (sum / weight) * u_bloom0.y * 0.92;
}

vec3 emissiveContribution(vec2 uv, vec3 source, vec3 localBlur) {
    if (u_emissive0.x < 0.5 || u_emissive0.y <= 0.001) return vec3(0.0);

    float threshold = mix(0.30, 1.00, u_emissive0.z);
    float radius = mix(1.0, 13.0, u_emissive0.w);
    vec3 blurred = qualityBlur(uv, u_texelSize * radius, max(u_bloom1.z, 0.25));

    float sourceLuma = luminance(source);
    float glowLuma = luminance(blurred);
    float chroma = maximum3(blurred) - minimum3(blurred);
    float brightMask = smoothstep(threshold, threshold + 0.20, glowLuma);
    float edgeMask = clamp(length(source - localBlur) * 2.5, 0.0, 1.0);

    float playerMask = exp(
        -dot(uv - vec2(0.24, 0.50), uv - vec2(0.24, 0.50)) * 34.0
    );
    float particleMask = clamp(
        smoothstep(threshold, threshold + 0.14, sourceLuma) * edgeMask * 1.4,
        0.0,
        1.0
    );
    float categoryMask = clamp(
        u_emissive1.y * playerMask +
        u_emissive1.z * max(edgeMask, 0.14) +
        u_emissive1.w * particleMask,
        0.0,
        1.0
    );

    float colorAmount = mix(0.24, 0.52, u_emissive1.x);
    vec3 emissionColor = lightHue(blurred, colorAmount);
    float colorEnergy = 1.0 + chroma * u_emissive1.x * 0.35;

    return emissionColor * brightMask * categoryMask *
        u_emissive0.y * colorEnergy * 0.84;
}

float ambientOcclusion(vec2 uv, vec3 source) {
    if (u_ao0.x < 0.5 || u_ao0.y <= 0.001) return 0.0;

    float radius = mix(1.0, 9.0, u_ao0.z);
    vec2 offset = u_texelSize * radius;
    float center = luminance(source);

    float surrounding =
        luminance(sampleScene(uv + vec2(offset.x, 0.0)).rgb) +
        luminance(sampleScene(uv - vec2(offset.x, 0.0)).rgb) +
        luminance(sampleScene(uv + vec2(0.0, offset.y)).rgb) +
        luminance(sampleScene(uv - vec2(0.0, offset.y)).rgb);
    surrounding *= 0.25;

    if (max(u_ao1.x, u_ao1.y) > 0.38) {
        float diagonal =
            luminance(sampleScene(uv + offset).rgb) +
            luminance(sampleScene(uv - offset).rgb) +
            luminance(sampleScene(uv + vec2(offset.x, -offset.y)).rgb) +
            luminance(sampleScene(uv + vec2(-offset.x, offset.y)).rgb);
        surrounding = mix(surrounding, diagonal * 0.25, 0.38);
    }

    float cavity = max(surrounding - center, 0.0);
    float edge = abs(surrounding - center);
    float occlusion = clamp(cavity * 1.25 + edge * 0.30, 0.0, 1.0);
    return min(occlusion * u_ao0.y * mix(0.12, 0.72, u_ao0.w), 0.26);
}

vec3 reflectionContribution(vec2 uv, vec3 source, vec3 localBlur) {
    if (u_reflection0.x < 0.5 || u_reflection0.y <= 0.001) return vec3(0.0);

    float distanceOffset = (u_reflection1.x - 0.5) * 0.16;
    float wave = sin(uv.y * 42.0 + u_time * 1.35) * u_reflection1.y * 0.010;
    vec2 reflectionUV = vec2(
        uv.x + wave,
        clamp(1.0 - uv.y + distanceOffset, 0.001, 0.999)
    );

    vec2 blurOffset = u_texelSize * mix(1.0, 10.0, u_reflection0.w);
    vec3 reflected = qualityBlur(reflectionUV, blurOffset, max(u_bloom1.z, 0.22));
    reflected = mix(vec3(luminance(reflected)), reflected, 0.58);

    float playerMask = exp(
        -dot(reflectionUV - vec2(0.24, 0.50), reflectionUV - vec2(0.24, 0.50)) * 30.0
    );
    float edgeMask = clamp(length(source - localBlur) * 2.4, 0.0, 1.0);
    float particleMask = smoothstep(0.70, 0.98, luminance(reflected)) * edgeMask;
    float categoryMask = clamp(
        u_reflection1.z * playerMask +
        u_reflection1.w * max(edgeMask, 0.15) +
        u_reflection2.x * particleMask,
        0.0,
        1.0
    );

    float lowerScreenMask = 1.0 - smoothstep(0.18, 0.78, uv.y);
    return reflected * categoryMask * max(lowerScreenMask, 0.08) *
        u_reflection0.y * u_reflection0.z * 0.48;
}

vec3 lightRaysContribution(vec2 uv) {
    if (u_rays0.x < 0.5 || u_rays0.y <= 0.001) return vec3(0.0);

    vec2 center = u_rays1.zw;
    vec2 direction = center - uv;
    float lengthFactor = mix(0.03, 0.38, u_rays0.z);
    float threshold = mix(0.42, 1.08, u_rays1.y);
    float decay = mix(0.60, 0.92, 1.0 - u_rays1.x);
    float blurScale = mix(0.70, 1.35, u_rays0.w);

    vec3 rays = vec3(0.0);
    float weight = 0.0;
    float currentDecay = 1.0;

    vec2 step1 = direction * lengthFactor * 0.14 * blurScale;
    vec2 step2 = direction * lengthFactor * 0.28 * blurScale;
    vec2 step3 = direction * lengthFactor * 0.44 * blurScale;
    vec2 step4 = direction * lengthFactor * 0.62 * blurScale;
    vec2 step5 = direction * lengthFactor * 0.82 * blurScale;
    vec2 step6 = direction * lengthFactor * 1.04 * blurScale;

    vec3 c1 = sampleScene(uv + step1).rgb;
    currentDecay *= decay;
    rays += lightHue(c1, 0.22) * smoothstep(threshold, threshold + 0.16, luminance(c1)) * currentDecay;
    weight += currentDecay;

    vec3 c2 = sampleScene(uv + step2).rgb;
    currentDecay *= decay;
    rays += lightHue(c2, 0.22) * smoothstep(threshold, threshold + 0.16, luminance(c2)) * currentDecay;
    weight += currentDecay;

    vec3 c3 = sampleScene(uv + step3).rgb;
    currentDecay *= decay;
    rays += lightHue(c3, 0.22) * smoothstep(threshold, threshold + 0.16, luminance(c3)) * currentDecay;
    weight += currentDecay;

    vec3 c4 = sampleScene(uv + step4).rgb;
    currentDecay *= decay;
    rays += lightHue(c4, 0.22) * smoothstep(threshold, threshold + 0.16, luminance(c4)) * currentDecay;
    weight += currentDecay;

    if (u_bloom1.z > 0.25) {
        vec3 c5 = sampleScene(uv + step5).rgb;
        currentDecay *= decay;
        rays += lightHue(c5, 0.22) * smoothstep(threshold, threshold + 0.16, luminance(c5)) * currentDecay;
        weight += currentDecay;
    }

    if (u_bloom1.z > 0.62) {
        vec3 c6 = sampleScene(uv + step6).rgb;
        currentDecay *= decay;
        rays += lightHue(c6, 0.22) * smoothstep(threshold, threshold + 0.16, luminance(c6)) * currentDecay;
        weight += currentDecay;
    }

    return rays / max(weight, 0.0001) * u_rays0.y * 0.72;
}

void main() {
    vec4 sourceSample = sampleScene(v_texCoord);
    vec3 source = sourceSample.rgb;
    vec3 localBlur = qualityBlur(
        v_texCoord,
        u_texelSize * 2.0,
        max(u_bloom1.z, 0.20)
    );

    float ao = ambientOcclusion(v_texCoord, source);
    vec3 base = source * (1.0 - ao);

    vec3 lighting =
        reflectionContribution(v_texCoord, source, localBlur) +
        emissiveContribution(v_texCoord, source, localBlur) +
        bloomContribution(v_texCoord) +
        lightRaysContribution(v_texCoord);

    float lightingPeak = maximum3(lighting);
    if (lightingPeak > 1.65) {
        lighting *= 1.65 / lightingPeak;
    }

    // Screen-like compositing keeps bright colored Geometry Dash backgrounds
    // from becoming solid magenta, yellow or cyan while preserving luminous
    // edges and white highlights.
    vec3 color = base + lighting * (vec3(1.0) - clamp(base, 0.0, 1.0) * 0.52);
    gl_FragColor = vec4(clamp(color, vec3(0.0), vec3(2.5)), sourceSample.a);
}
