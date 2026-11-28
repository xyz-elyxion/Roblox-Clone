// The world API as a script sees it: the service tree, Lighting, the two player
// events, and the two ways a script reaches a sound.
//
// A part list, an interpreter and no physics are enough to run every one of these,
// which is what keeps the world's own API testable on a machine with no window and
// no sound card: the host's handlers stand in for the server and the clients.

#include "SoundBank.h"
#include "WorldScript.h"
#include "WorldServices.h"

#include <cmath>
#include <cstdio>
#include <cstdint>
#include <deque>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace {

int failures = 0;

// Every sound is a file, so the test brings the two names it asks for: what is
// under test here is the script's API, not the loader. A folder of its own also
// keeps the refusal of a name this build does not have honest.
void writeTestSound(const std::filesystem::path& path) {
    const int frames = 64;
    std::vector<unsigned char> file;
    const auto put16 = [&file](unsigned int value) {
        file.push_back(static_cast<unsigned char>(value & 0xFFu));
        file.push_back(static_cast<unsigned char>((value >> 8) & 0xFFu));
    };
    const auto put32 = [&file](unsigned int value) {
        for (int byte = 0; byte < 4; ++byte) {
            file.push_back(static_cast<unsigned char>((value >> (byte * 8)) & 0xFFu));
        }
    };
    const auto tag = [&file](const char* text) {
        for (int index = 0; index < 4; ++index) {
            file.push_back(static_cast<unsigned char>(text[index]));
        }
    };

    tag("RIFF");
    put32(static_cast<unsigned int>(36 + frames * 2));
    tag("WAVE");
    tag("fmt ");
    put32(16);
    put16(1);  // PCM
    put16(1);  // mono
    put32(static_cast<unsigned int>(linear::audio::kSampleRate));
    put32(static_cast<unsigned int>(linear::audio::kSampleRate * 2));
    put16(2);
    put16(16);
    tag("data");
    put32(static_cast<unsigned int>(frames * 2));
    for (int frame = 0; frame < frames; ++frame) {
        put16(800);
    }

    std::ofstream out(path, std::ios::binary);
    out.write(reinterpret_cast<const char*>(file.data()),
              static_cast<std::streamsize>(file.size()));
}

void useTestSounds() {
    const std::filesystem::path folder =
        std::filesystem::temp_directory_path() / "linear-world-script-sounds";
    std::filesystem::create_directories(folder);
    writeTestSound(folder / "door.wav");
    writeTestSound(folder / "jump.wav");
    linear::audio::setSoundDirectory(folder.string());
}

struct Harness {
    std::deque<Part> parts;
    linear::script::Interpreter interpreter;
    linear::worldscript::WorldRuntime runtime;

    // What the host was asked to do.
    bool sounds = false;
    std::string lastSound;
    float lastVolume = -1.0f;
    bool lastSpatial = false;

    bool environments = false;
    float fogX = -1.0f;
    float fogDensity = -1.0f;

    // The players the host reports, and what it was told about their score.
    std::vector<linear::worldscript::ScriptPlayer> playerSnapshot;
    int scoreChanges = 0;
    std::string lastScored;
    int lastScore = 0;

    std::string lastError;

    Harness() : runtime(parts, nullptr, interpreter) {
        parts.emplace_back(linearmath::vec3(0.0f), linearmath::vec3(8.0f, 1.0f, 8.0f),
                           linearmath::vec3(0.4f), ShapeType::Cube, "Ground");
        runtime.setSoundHandler([this](const std::string& name, const linearmath::vec3&,
                                       float volume, bool spatial) {
            sounds = true;
            lastSound = name;
            lastVolume = volume;
            lastSpatial = spatial;
        });
        runtime.setEnvironmentHandler([this](const linearmath::vec3& color, float density) {
            environments = true;
            fogX = color.x;
            fogDensity = density;
        });
        runtime.setPlayerSource([this]() { return playerSnapshot; });
        runtime.setScoreHandler([this](const std::string& name, int score) {
            scoreChanges += 1;
            lastScored = name;
            lastScore = score;
        });
    }

    bool run(const std::string& source) {
        lastError.clear();
        return runtime.runScript(source, "world script test", lastError);
    }
};

void check(bool condition, const std::string& what) {
    if (!condition) {
        std::printf("FAIL: %s\n", what.c_str());
        ++failures;
    }
}

// Run a script that has to work. The message is built after the run, because the
// order of evaluation of two arguments is not something to rely on.
void checkRuns(Harness& harness, const std::string& source, const std::string& what) {
    if (harness.run(source)) {
        return;
    }
    std::printf("FAIL: %s: %s\n", what.c_str(), harness.lastError.c_str());
    ++failures;
}

// Run a script that has to fail, and check that its failure says the thing the
// person reading the error needs to know.
void checkRefused(Harness& harness, const std::string& source, const std::string& what,
                  const std::string& mustMention) {
    if (harness.run(source)) {
        std::printf("FAIL: %s: the script was accepted\n", what.c_str());
        ++failures;
        return;
    }
    if (mustMention.empty() || harness.lastError.find(mustMention) != std::string::npos) {
        return;
    }
    std::printf("FAIL: %s: '%s' is not in '%s'\n", what.c_str(), mustMention.c_str(),
                harness.lastError.c_str());
    ++failures;
}

void testServiceTable() {
    const std::vector<linear::worldscript::Service>& services = linear::worldscript::services();
    check(services.size() == 14, "the tree lists fourteen services");
    check(std::string(services.front().name) == "Workspace", "Workspace is listed first");
    check(std::string(services.back().name) == "TextChatService",
          "TextChatService is listed last");

    for (const linear::worldscript::Service& service : services) {
        const std::string name = service.name;
        check(!name.empty(), "every service has a name");
        check(std::string(service.note).size() > 0, name + " says what it holds");
        check(linear::worldscript::findService(name) == &service, name + " is found by name");
    }

    check(linear::worldscript::findService("workspace") == nullptr,
          "service names are case sensitive");
    check(linear::worldscript::findService("") == nullptr, "an empty name is no service");
    check(linear::worldscript::findService("Lighting2") == nullptr,
          "a name that is not in the tree is no service");
}

void testGetService() {
    Harness harness;

    // Every name in the tree resolves to something, generated from the table so a
    // service added on one side and not the other cannot pass.
    std::string source;
    for (const linear::worldscript::Service& service : linear::worldscript::services()) {
        source += "if game:GetService(\"";
        source += service.name;
        source += "\") == nil then error(\"";
        source += service.name;
        source += " is nil\") end\n";
    }
    checkRuns(harness, source, "every service in the tree resolves");

    checkRefused(harness, "game:GetService(\"SoundService2\")",
                 "a service that is not in the tree fails the script", "there is no service");
    checkRefused(harness, "game:GetService(\"SoundService2\")",
                 "the failure lists the services this build has", "SoundService");
    checkRefused(harness, "game:GetService(1)", "a service name has to be a string",
                 "GetService expects a service name");
    checkRefused(harness, "game:BindToClose()", "game only offers the lookup it has",
                 "GetService");

    checkRuns(harness, "if game.Workspace == nil then error(\"no workspace\") end",
              "game.Workspace is still the workspace");
    checkRuns(harness, "if game.Name == nil then error(\"no name\") end",
              "game still has a name");
    checkRuns(harness,
              "local workspace = game:GetService(\"Workspace\")\n"
              "if workspace:FindFirstChild(\"Ground\") == nil then error(\"no Ground\") end",
              "the workspace from the tree is the world's own");
    checkRuns(harness,
              "if game:GetService(\"Workspace\").Name == nil then error(\"no name\") end",
              "a service answers with its own name");
}

void testLighting() {
    Harness harness;

    checkRuns(harness,
              "local lighting = game:GetService(\"Lighting\")\n"
              "lighting.FogDensity = 0.05\n"
              "lighting.FogColor = Vector3.new(0.5, 1.0, 0.25)",
              "Lighting takes a new fog");
    check(harness.environments, "a fog change reaches the host");
    check(std::fabs(harness.fogX - 0.5f) < 1e-5f, "the new fog colour reaches the host");
    check(std::fabs(harness.fogDensity - 0.05f) < 1e-5f, "the new density reaches the host");

    // Read back through a second lookup: the service carries state, so two calls
    // have to be the same object and not two copies that disagree.
    checkRuns(harness,
              "local lighting = game:GetService(\"Lighting\")\n"
              "if lighting.FogDensity < 0.04 then error(\"density did not stay\") end\n"
              "if lighting.FogColor.Y < 0.9 then error(\"colour did not stay\") end",
              "Lighting reads back what it was given");

    // Past the point where a client stops thickening the haze, so a script cannot
    // ask for fog that hides the world outright. The comparison allows for a float
    // that comes back as the double it widens to.
    checkRuns(harness,
              "local lighting = game:GetService(\"Lighting\")\n"
              "lighting.FogDensity = 5.0\n"
              "if lighting.FogDensity > 0.2001 then error(\"density was not clamped\") end",
              "the fog density is held at what a client can draw");
}

void testSounds() {
    useTestSounds();
    Harness harness;

    checkRuns(harness, "game:GetService(\"SoundService\"):Play(\"door\")",
              "SoundService:Play plays a built-in sound");
    check(harness.sounds && harness.lastSound == "door", "the sound reaches the host by name");
    check(!harness.lastSpatial, "a sound with no position is not spatial");

    checkRuns(harness, "workspace:PlaySound(\"jump\", Vector3.new(1, 2, 3), 0.5)",
              "workspace:PlaySound is unchanged");
    check(harness.lastSound == "jump", "the workspace's sound reaches the host too");
    check(harness.lastSpatial, "a sound with a position is spatial");
    check(std::fabs(harness.lastVolume - 0.5f) < 1e-5f, "the volume arrives");

    checkRefused(harness, "game:GetService(\"SoundService\"):Play(\"nope\")",
                 "a sound this build does not have fails the script", "there is no sound");
    checkRefused(harness, "game:GetService(\"SoundService\"):Stop()",
                 "SoundService only offers what it has", "SoundService has no method 'Stop'");
}

// The two events a round loop is built from, and the number the world keeps for a
// player. Both are driven by the list the host supplies, so a host that already
// knows its players does not have to report the moments separately.
void testPlayers() {
    Harness harness;
    harness.playerSnapshot.push_back(
        linear::worldscript::ScriptPlayer{"Ana", linearmath::vec3(0.0f), 0});

    checkRuns(harness,
              "added = \"\"\n"
              "removed = \"\"\n"
              "lastPlayer = nil\n"
              "Players.PlayerAdded:Connect(function(player)\n"
              "    added = added .. player.Name .. \",\"\n"
              "    lastPlayer = player\n"
              "end)\n"
              "Players.PlayerRemoving:Connect(function(player)\n"
              "    removed = removed .. player.Name .. \",\"\n"
              "end)",
              "a script connects to the two player events");

    // The players who were in the game when the script started are the first
    // PlayerAdded: that is what makes the events enough to count a match without
    // asking every tick whether the list changed.
    harness.runtime.heartbeat(0.016f);
    checkRuns(harness, "if added ~= \"Ana,\" then error(added) end",
              "PlayerAdded fires for a player already in the game");
    checkRuns(harness, "if removed ~= \"\" then error(removed) end", "nothing has left yet");

    harness.playerSnapshot.push_back(
        linear::worldscript::ScriptPlayer{"Bo", linearmath::vec3(0.0f), 0});
    harness.runtime.heartbeat(0.016f);
    checkRuns(harness, "if added ~= \"Ana,Bo,\" then error(added) end",
              "a join reports the player who joined and not the one who was there");

    harness.playerSnapshot.erase(harness.playerSnapshot.begin());
    harness.runtime.heartbeat(0.016f);
    checkRuns(harness, "if removed ~= \"Ana,\" then error(removed) end",
              "a leave reports the player who left");
    checkRuns(harness, "if Players.PlayerCount ~= 1 then error(\"count\") end",
              "the count follows the list");

    // The score is written by the script and owned by the host, the same way
    // damage and teleports are: the script says what, the host decides whether
    // anyone hears it.
    checkRuns(harness, "lastPlayer.Score = 7", "a script writes a score");
    check(harness.scoreChanges == 1, "the score reaches the host once");
    check(harness.lastScored == "Bo", "the score is named after the player it belongs to");
    check(harness.lastScore == 7, "the value the script wrote is the value the host got");
    checkRuns(harness, "if lastPlayer.Score ~= 7 then error(\"score did not stay\") end",
              "a player reads back the score it was given");

    // A score the host already holds - a join in the middle of a match - is what
    // the script reads, so the HUD and the script cannot disagree.
    harness.playerSnapshot.back().score = 4;
    harness.runtime.heartbeat(0.016f);
    checkRuns(harness, "if lastPlayer.Score ~= 4 then error(\"host score did not arrive\") end",
              "a score the host holds is what the script reads");

    checkRefused(harness, "lastPlayer.Score = \"high\"", "a score has to be a number", "");
}

void testPlaceholders() {
    Harness harness;

    // A service this build does not fill is still an object with a name, so code
    // that only walks the tree works.
    checkRuns(harness, "if game:GetService(\"Teams\").Name == nil then error(\"no name\") end",
              "a placeholder service still has a name");
    checkRefused(harness, "game:GetService(\"MaterialService\"):GetMaterial()",
                 "a method a placeholder does not have fails the script", "MaterialService");
    checkRefused(harness, "game:GetService(\"MaterialService\"):GetMaterial()",
                 "the failure says what this build keeps instead", "no service");
    checkRefused(harness, "game:GetService(\"Teams\").Score = 1",
                 "a property a placeholder does not have fails the script", "");
}

}  // namespace

int main() {
    testServiceTable();
    testGetService();
    testLighting();
    testSounds();
    testPlayers();
    testPlaceholders();

    if (failures == 0) {
        std::printf("world_script_tests: all checks passed\n");
        return 0;
    }
    std::printf("world_script_tests: %d check(s) failed\n", failures);
    return 1;
}
