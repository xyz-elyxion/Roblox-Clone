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
    vec3 albedo = toLinear(PartColor);
    vec3 N = normalize(Normal);
    vec3 V = normalize(viewPos - FragPos);

    // The sun is the light position, and the shadow camera looks from that same
    // point at the origin, so the light direction is the same across the world.
    // Deriving it per fragment turned it into a point light and lit identical
    // builds differently depending on where they stood.
    vec3 L = normalize(lightPos);
    vec3 sunColor = vec3(1.0, 0.97, 0.92);

    bool isFacePart = isCamera > 0.5;
    float surfaceShade = 1.0;

    // --- Studs and inlets ---
    // These used to be painted circles. Tilting the normal towards the stud rim
    // lets the sun shade them like the raised cylinder they represent, and a
    // contact shadow where the stud meets the face sells the height.
    if (!isFacePart && LocalNormal.y > 0.5) {
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
    } else if (!isFacePart && LocalNormal.y < -0.9) {
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

    // --- Ambient ---
    // Sky above, bounced ground light below: what gives stacked bricks a lit top
    // and a darker underside without any extra geometry.
    vec3 skyTint = toLinear(sampleSky(vec3(0.0, 1.0, 0.0)));
    vec3 groundTint = toLinear(sampleSky(vec3(0.0, -1.0, 0.0)));
    vec3 ambientTint = mix(groundTint, skyTint, 0.5 + 0.5 * N.y);
    // Kept mostly neutral so a solid-colour skybox does not wash the world blue.
    ambientTint = mix(vec3(1.0), ambientTint, 0.55);
    vec3 ambient = albedo * surfaceShade * ambientTint * 0.42;

    // --- Sun ---
    float ndl = max(dot(N, L), 0.0);
    float shadow = ShadowCalculation(FragPosLightSpace, N, L);

    vec3 diffuse = albedo * surfaceShade * sunColor * ndl;

    // Matte plastic, glossier as reflectance rises. Blinn-Phong with a low base
    // strength keeps parts from looking wet the way a flat 0.5 highlight did.
    vec3 H = normalize(L + V);
    float gloss = clamp(reflectance, 0.0, 1.0);
    float shininess = mix(64.0, 8.0, gloss);
    float specStrength = 0.04 + 0.55 * gloss;
    float spec = pow(max(dot(N, H), 0.0), shininess);
    vec3 specular = sunColor * spec * specStrength * ndl;

    vec3 finalColor = ambient + (diffuse + specular) * (1.0 - shadow);

    // --- Reflection map ---
    vec3 R = reflect(-V, N);
    vec3 reflectionColor = toLinear(sampleSky(R));
    finalColor = mix(finalColor, reflectionColor, gloss);

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
