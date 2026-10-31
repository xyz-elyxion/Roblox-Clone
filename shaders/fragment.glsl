#version 330 core
out vec4 FragColor;

in vec3 PartColor;
in vec3 Normal;
in vec3 FragPos;
in vec4 FragPosLightSpace;
in float Alpha;
in vec3 LocalPos;
in vec3 LocalNormal;
in mat3 LocalBasis;

uniform vec3 viewPos;
uniform float reflectance;
uniform samplerCube skybox;
uniform sampler2D shadowMap;
uniform sampler2D faceTexture; // Face texture for camera parts
uniform float hasTexture; // Whether to use face texture
uniform vec3 lightPos; // The sun: its direction from the origin is the light direction
uniform vec3 partSize; // Added size for scaling studs
uniform float isCamera;  // Flag for camera part visualization
uniform float material;  // Material enum value, see Material in include/Part.h
uniform sampler2D partTexture; // Tiled image, when the part names one
uniform float hasPartTexture;

// Material ids, mirroring the Material enum.
const float MAT_PLASTIC = 0.0;
const float MAT_SMOOTH_PLASTIC = 1.0;
const float MAT_NEON = 2.0;
const float MAT_METAL = 3.0;
const float MAT_WOOD = 4.0;
const float MAT_GRASS = 5.0;
const float MAT_CONCRETE = 6.0;
const float MAT_BRICK = 7.0;

// Part colours and the skybox are stored as sRGB, the space the palette is
// picked in. Lighting maths has to happen in linear light and be encoded once on
// the way out; doing it on the sRGB values themselves is what made every part
// look flat and the shadows grey and muddy.
vec3 toLinear(vec3 c) { return pow(max(c, vec3(0.0)), vec3(2.2)); }
vec3 toSrgb(vec3 c) { return pow(max(c, vec3(0.0)), vec3(1.0 / 2.2)); }

// The skybox is only bound while a game is being drawn; Studio runs the same
// shader without one, and a sampler with nothing behind it can read back
// anything at all, so the samples are clamped to a real colour range.
vec3 sampleSky(vec3 dir) { return clamp(texture(skybox, dir).rgb, 0.0, 1.0); }

float hash21(vec2 p) {
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
}

float valueNoise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    float a = hash21(i);
    float b = hash21(i + vec2(1.0, 0.0));
    float c = hash21(i + vec2(0.0, 1.0));
    float d = hash21(i + vec2(1.0, 1.0));
    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

// Box mapping in stud units: every face samples the same image without needing
// the mesh to carry UVs, and the scale comes from the part's size so a small
// brick and a large platform show tiles of the same physical size.
vec2 boxUv(vec3 localPos, vec3 localNormal) {
    vec3 p = localPos * partSize;
    vec3 n = abs(localNormal);
    if (n.y >= n.x && n.y >= n.z) return p.xz;
    if (n.x >= n.z) return p.zy;
    return p.xy;
}

// Procedural surface patterns for the flat materials. They are deliberately
// cheap and coarse: the point is that a wall reads as brick and a plank reads as
// wood from across the map, not that it survives a close-up shader review.
vec3 materialPattern(float matId, vec2 uv, vec3 p) {
    if (abs(matId - MAT_WOOD) < 0.5) {
        float rings = sin(p.y * 6.5 + valueNoise(p.xz * 1.3) * 4.0);
        float grain = valueNoise(vec2(p.x * 2.0, p.y * 22.0));
        return vec3(0.84 + 0.16 * rings) * (0.92 + 0.08 * grain);
    }
    if (abs(matId - MAT_GRASS) < 0.5) {
        float clumps = valueNoise(p.xz * 2.2) * 0.65 + valueNoise(p.xz * 8.0) * 0.35;
        return mix(vec3(0.74, 0.88, 0.70), vec3(1.10, 1.06, 0.94), clumps);
    }
    if (abs(matId - MAT_CONCRETE) < 0.5) {
        float speck = valueNoise(p.xz * 13.0 + p.y * 5.0);
        float blotch = valueNoise(p.xz * 2.6 + p.y * 1.4);
        return vec3(0.84 + 0.10 * speck + 0.06 * blotch);
    }
    if (abs(matId - MAT_BRICK) < 0.5) {
        vec2 brickUv = uv * vec2(0.5, 1.0);
        float row = floor(brickUv.y);
        brickUv.x += mod(row, 2.0) * 0.5;
        vec2 cell = fract(brickUv);
        float mortar = smoothstep(0.0, 0.05, cell.x) * smoothstep(0.0, 0.05, 1.0 - cell.x) *
                       smoothstep(0.0, 0.09, cell.y) * smoothstep(0.0, 0.09, 1.0 - cell.y);
        float tint = 0.86 + 0.14 * hash21(floor(brickUv));
        return mix(vec3(0.68, 0.64, 0.60), vec3(tint), mortar);
    }
    return vec3(1.0);
}

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    // Nothing behind the sun's box is shadowed, and the shadow fades out at the
    // edges of the box instead of stopping on a hard line.
    if (projCoords.z > 1.0)
        return 0.0;

    vec2 edge = smoothstep(vec2(0.0), vec2(0.06), projCoords.xy) *
                smoothstep(vec2(0.0), vec2(0.06), vec2(1.0) - projCoords.xy);
    float boundsFade = edge.x * edge.y;
    if (boundsFade <= 0.0)
        return 0.0;

    float currentDepth = projCoords.z;
    // Slope-scaled bias in the sun's depth range. The old 0.05 floor was a
    // fiftieth of the whole shadow frustum, so shadows sat visibly above the
    // surface that cast them.
    float cosTheta = max(dot(normal, lightDir), 0.0);
    float bias = clamp(0.0035 * (1.0 - cosTheta), 0.0004, 0.004);

    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0));
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }

    return (shadow / 9.0) * boundsFade;
}

void main() {
    // These four were declared const, which GLSL 3.30 only allows for constant
    // expressions. A uniform, a texture lookup helper and an attribute-derived
    // value are none of those, so drivers that enforce the spec refused to build
    // the shader at all and drew nothing but the sky.
    float matId = floor(material + 0.5);
    vec3 albedo = toLinear(PartColor);
    vec3 N = normalize(Normal);
    vec3 V = normalize(viewPos - FragPos);

    vec3 surfacePos = LocalPos * partSize;
    vec2 surfaceUv = boxUv(LocalPos, LocalNormal);

    bool isFacePart = isCamera > 0.5;
    // Studs belong to plastic bricks. A smooth plastic, metal or grass surface
    // studded like a brick was the giveaway that materials were only a colour.
    bool studded = !isFacePart && matId < 0.5;

    float surfaceShade = 1.0;

    // --- Studs and inlets ---
    // These used to be painted circles. Tilting the normal towards the stud rim
    // lets the sun shade them like the raised cylinder they represent, and a
    // contact shadow where the stud meets the face sells the height.
    if (studded && LocalNormal.y > 0.5) {
        vec2 uv = LocalPos.xz * partSize.xz;
        vec2 cell = fract(uv) - 0.5;
        float r = length(cell);

        if (r < 0.30) {
            float t = clamp(r / 0.30, 0.0, 1.0);
            float dome = sqrt(max(0.0, 1.0 - t * t));
            vec3 tiltLocal = normalize(vec3(cell.x * 1.7, dome * 2.4, cell.y * 1.7));
            N = normalize(LocalBasis * tiltLocal);
            surfaceShade = mix(1.0, 0.9, smoothstep(0.7, 1.0, t));
        } else if (r < 0.36) {
            surfaceShade = mix(0.74, 1.0, smoothstep(0.30, 0.36, r));
        }
    } else if (studded && LocalNormal.y < -0.9) {
        vec2 uv = LocalPos.xz * partSize.xz;
        vec2 cell = fract(uv) - 0.5;
        float r = length(cell);

        if (r < 0.30) {
            float t = clamp(r / 0.30, 0.0, 1.0);
            float dome = sqrt(max(0.0, 1.0 - t * t));
            // Inlets are a groove, so their normals lean the other way.
            vec3 tiltLocal = normalize(vec3(cell.x * 1.7, -dome * 2.4, cell.y * 1.7));
            N = normalize(LocalBasis * tiltLocal);
            surfaceShade = mix(0.62, 0.78, t);
        } else if (r < 0.36) {
            surfaceShade = mix(1.06, 1.0, smoothstep(0.30, 0.36, r));
        }
    }

    // --- Texture ---
    // One tile per two studs, multiplied by the part colour so a texture can be
    // tinted the same way a plain part is.
    if (hasPartTexture > 0.5) {
        vec4 texel = texture(partTexture, surfaceUv * 0.5);
        albedo = toLinear(texel.rgb) * albedo;
    }

    albedo *= materialPattern(matId, surfaceUv, surfacePos);

    // --- Lighting parameters per material ---
    float gloss = clamp(reflectance, 0.0, 1.0);
    float shininess = mix(64.0, 8.0, gloss);
    float specStrength = 0.04 + 0.55 * gloss;
    float ambientStrength = 0.42;
    float shadowStrength = 1.0;

    if (abs(matId - MAT_SMOOTH_PLASTIC) < 0.5) {
        gloss = max(gloss, 0.20);
        shininess = mix(96.0, 32.0, gloss);
        specStrength = 0.28 + 0.55 * gloss;
    } else if (abs(matId - MAT_NEON) < 0.5) {
        ambientStrength = 0.0;
        shadowStrength = 0.0;
        specStrength = 0.0;
    } else if (abs(matId - MAT_METAL) < 0.5) {
        gloss = max(gloss, 0.60);
        shininess = 28.0;
        specStrength = 0.80;
        ambientStrength = 0.30;
    } else if (matId > MAT_METAL + 0.5) {
        specStrength = 0.03;
        shininess = 14.0;
        ambientStrength = 0.46;
    }

    // The sun is the light position, and the shadow camera looks from that same
    // point at the origin, so the light direction is the same across the world.
    // Deriving it per fragment turned it into a point light and lit identical
    // builds differently depending on where they stood.
    vec3 L = normalize(lightPos);
    vec3 sunColor = vec3(1.0, 0.97, 0.92);

    // --- Ambient ---
    // Sky above, bounced ground light below: what gives stacked bricks a lit top
    // and a darker underside without any extra geometry.
    vec3 skyTint = toLinear(sampleSky(vec3(0.0, 1.0, 0.0)));
    vec3 groundTint = toLinear(sampleSky(vec3(0.0, -1.0, 0.0)));
    vec3 ambientTint = mix(groundTint, skyTint, 0.5 + 0.5 * N.y);
    // Kept mostly neutral so a solid-colour skybox does not wash the world blue.
    ambientTint = mix(vec3(1.0), ambientTint, 0.55);
    vec3 ambient = albedo * surfaceShade * ambientTint * ambientStrength;

    // --- Sun ---
    float ndl = max(dot(N, L), 0.0);
    float shadow = ShadowCalculation(FragPosLightSpace, N, L) * shadowStrength;

    vec3 diffuse = albedo * surfaceShade * sunColor * ndl;

    // Matte plastic, glossier as reflectance rises. Blinn-Phong with a low base
    // strength keeps parts from looking wet the way a flat 0.5 highlight did.
    vec3 H = normalize(L + V);
    float spec = pow(max(dot(N, H), 0.0), shininess);
    vec3 specular = sunColor * spec * specStrength * ndl;

    vec3 finalColor = ambient + (diffuse + specular) * (1.0 - shadow);

    // --- Reflection map ---
    vec3 R = reflect(-V, N);
    vec3 reflectionColor = toLinear(sampleSky(R));
    if (abs(matId - MAT_METAL) < 0.5) {
        // Metal reflects its own colour; that tint is what separates it from a
        // very shiny plastic.
        reflectionColor *= albedo * 2.0 + 0.2;
    }
    finalColor = mix(finalColor, reflectionColor, clamp(gloss, 0.0, 0.9));

    // Neon emits instead of being lit, so it reads as a light source in a dark
    // map rather than as a bright plastic.
    if (abs(matId - MAT_NEON) < 0.5) {
        finalColor = albedo * (1.55 + 0.45 * ndl) + albedo * 0.2;
    }

    // --- Camera Part / Head Part Face Visualization ---
    if (isFacePart) {
        // Show face on +Z face (front face in local space)
        // The head rotates, so +Z should be the front
        if (LocalNormal.z > 0.9 && hasTexture > 0.5) {
            // For +Z face, use x and y coordinates directly
            vec2 uv = LocalPos.xy + vec2(0.5);
            uv.y = 1.0 - uv.y; // Flip Y for correct orientation
            uv = clamp(uv, vec2(0.0), vec2(1.0));

            vec4 texColor = texture(faceTexture, uv);

            // Only apply texture where it has content (alpha > threshold), and
            // shade it so a face in shadow does not glow like a lamp.
            if (texColor.a > 0.1) {
                vec3 faceLit = toLinear(texColor.rgb) * mix(0.55, 1.0, 1.0 - shadow);
                finalColor = mix(finalColor, faceLit, texColor.a);
            }
        }
    }

    FragColor = vec4(clamp(toSrgb(finalColor), 0.0, 1.0), Alpha);
}
