// The material surfaces are images now, so what the shader samples is checked
// here: the four patterned materials name a file, the four flat ones name none,
// and every named file is in textures/ and is a PNG the renderer can upload.
//
// No window and no GL: the PNG header is read out of the file itself, which is
// enough to catch a renamed, truncated or wrong-sized surface on a build machine.

#include "Part.h"

#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <set>
#include <string>
#include <vector>

namespace {

int failures = 0;

void check(bool condition, const std::string& what) {
    if (!condition) {
        std::printf("FAIL: %s\n", what.c_str());
        ++failures;
    }
}

// The renderer looks beside the program, under textures/ and two folders up; the
// tests run a few levels under the checkout, so the folder is found by walking up
// from the working directory and from the executable.
std::filesystem::path findTexturesFolder(const char* argv0) {
    std::error_code ignored;
    std::vector<std::filesystem::path> starts;
    starts.push_back(std::filesystem::current_path(ignored));
    starts.push_back(std::filesystem::absolute(argv0, ignored).parent_path());

    for (const std::filesystem::path& start : starts) {
        std::filesystem::path folder = start;
        for (int hop = 0; hop < 4; ++hop) {
            const std::filesystem::path candidate = folder / "textures";
            if (std::filesystem::exists(candidate / "material-brick.png")) {
                return candidate;
            }
            folder = folder.parent_path();
            if (folder.empty()) {
                break;
            }
        }
    }
    return std::filesystem::path();
}

std::vector<unsigned char> readFile(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return {};
    }
    return std::vector<unsigned char>((std::istreambuf_iterator<char>(file)),
                                      std::istreambuf_iterator<char>());
}

std::uint32_t read32(const unsigned char* bytes) {
    return (static_cast<std::uint32_t>(bytes[0]) << 24) |
           (static_cast<std::uint32_t>(bytes[1]) << 16) |
           (static_cast<std::uint32_t>(bytes[2]) << 8) |
           static_cast<std::uint32_t>(bytes[3]);
}

void testNames() {
    std::set<std::string> seen;
    for (int index = 0; index < kMaterialCount; ++index) {
        const Material material = static_cast<Material>(index);
        const std::string name = materialName(material);
        const std::string file = linear::material::textureFile(material);
        check(!name.empty(), "material " + std::to_string(index) + " has a name");
        if (file.empty()) {
            continue;
        }
        check(file.size() > 4 && file.compare(file.size() - 4, 4, ".png") == 0,
              name + " names a .png: " + file);
        // A material's image is the build's own, but the rule is the same one a
        // part texture follows: a plain relative file name, never a path.
        check(file.find('/') == std::string::npos && file.find('\\') == std::string::npos,
              name + " names a file, not a path: " + file);
        check(file.find("..") == std::string::npos, name + " cannot climb out of textures/");
        check(seen.insert(file).second, file + " is used by one material only");
    }

    // The four flat materials are a colour and a reflectance, so they must not ask
    // for an image and a renderer must not look for one.
    check(std::string(linear::material::textureFile(Material::Plastic)).empty(),
          "plastic has no surface image");
    check(std::string(linear::material::textureFile(Material::SmoothPlastic)).empty(),
          "smooth plastic has no surface image");
    check(std::string(linear::material::textureFile(Material::Neon)).empty(),
          "neon has no surface image");
    check(std::string(linear::material::textureFile(Material::Metal)).empty(),
          "metal has no surface image");
}

void testFiles(const std::filesystem::path& folder) {
    check(!folder.empty(), "the textures folder is beside the tests or a few levels above them");
    if (folder.empty()) {
        return;
    }

    for (int index = 0; index < kMaterialCount; ++index) {
        const Material material = static_cast<Material>(index);
        const std::string file = linear::material::textureFile(material);
        if (file.empty()) {
            continue;
        }
        const std::string name = materialName(material);

        const std::filesystem::path path = folder / file;
        check(std::filesystem::exists(path), name + "'s surface ships: " + file);
        const std::vector<unsigned char> bytes = readFile(path);
        if (bytes.size() < 33) {
            check(false, name + "'s surface is a file with a PNG header in it");
            continue;
        }

        const unsigned char signature[8] = {0x89, 'P', 'N', 'G', '\r', '\n', 0x1A, '\n'};
        bool png = true;
        for (int i = 0; i < 8; ++i) {
            png = png && bytes[static_cast<std::size_t>(i)] == signature[i];
        }
        check(png, name + "'s surface is a PNG");
        if (!png) {
            continue;
        }

        // The header the renderer's loader reads: dimensions, bit depth and
        // colour type. An image at the wrong size would tile at the wrong scale.
        const std::uint32_t width = read32(bytes.data() + 16);
        const std::uint32_t height = read32(bytes.data() + 20);
        const int bits = bytes[24];
        const int colour = bytes[25];
        check(width == 256 && height == 256,
              name + "'s surface is 256x256 (it is " + std::to_string(width) + "x" +
                  std::to_string(height) + ")");
        check(bits == 8, name + "'s surface is 8 bits per channel");
        check(colour == 2 || colour == 6,
              name + "'s surface is RGB or RGBA (colour type " + std::to_string(colour) + ")");
    }
}

}  // namespace

int main(int argc, char** argv) {
    testNames();
    testFiles(findTexturesFolder(argc > 0 ? argv[0] : ""));

    if (failures == 0) {
        std::printf("material_texture_tests: all checks passed\n");
        return 0;
    }
    std::printf("material_texture_tests: %d check(s) failed\n", failures);
    return 1;
}
