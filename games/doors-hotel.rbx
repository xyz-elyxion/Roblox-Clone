-- Doors: The Hotel
-- The hotel is built here, not drawn into the world file. The world file holds
-- only the lobby spawn, the sign and the entity's body.
--
-- Generation needs parts that were never in the world file, so the world script
-- exposes workspace:CreatePart(name, position, size, color) and part:Destroy().
-- A created part is anchored and collidable and carries the same properties as a
-- world part, so the rest of this script works on a generated room exactly like
-- on a painted one.
--
-- Two things make the hotel: a seed and a window. Everything about a room - which
-- wall its wardrobe is on, how its floor and lamp are lit, what is stacked in it
-- - is derived from SEED and the room's number, so the same seed builds the same
-- hotel every run and a different seed builds a different one with no world file
-- to edit. Rooms are built two ahead of the deepest player and taken away two
-- behind them, so a hundred-door hotel costs only the rooms someone can actually
-- see; the ones behind are gone for good, which is why the hotel never lets you
-- walk back out of it.
--
-- Touch the pad in front of a door to lift it, step into the wardrobe beside it
-- to hide, and reach the elevator past door 100 to leave. Now and then an entity
-- starts behind the deepest player and runs past them: anyone standing in the
-- open when it goes by takes the hit, anyone inside a wardrobe survives. A death
-- costs the run, not the progress: the player comes back at the last door they
-- opened, and doors stay open.

local SEED = 71234
local DOORS = 100
local PITCH = 30
local DOOR1_Z = -22
local END_Z = DOOR1_Z - PITCH * DOORS
local LOBBY_END_Z = 26.5

local LOBBY_SPAWN = Vector3.new(0, 6.2, 0)
local ROOM_SPAWN_Y = 6.2

local AHEAD = 2 -- rooms built past the deepest player
local BEHIND = 2 -- rooms kept behind them before they are taken away

local RUSH_PARK_Z = 40
local RUSH_PARK_Y = 4.5
local RUSH_SPEED = 40
local RUSH_LEAD = 40
local RUSH_REACH = 170
local RUSH_HIT_DEPTH = 4.5
local RESPAWN_DELAY = 3.6
local DEATH_DAMAGE = 100

local CLOSET_MIN_X = 9.9
local CLOSET_MAX_X = 12.5
local CLOSET_HALF_Z = 1.8
local CLOSET_STAND = 11.4
local CLOSET_STEP_OUT = 7.8
local CLOSET_STAND_Y = 5

local WOOD_COLOR = Vector3.new(0.34, 0.22, 0.12)
local LAMP_COLOR = Vector3.new(0.92, 0.88, 0.72)
local OPEN_DOOR_Y = 14.25

local rush = workspace:FindFirstChild("Rush")
local sign = workspace:FindFirstChild("HotelSign")

-- The seed is mixed with the room number instead of drawn from a stream, so a
-- room looks the same whether the player reached it now or in an hour, and the
-- rooms do not have to be built in order for that to hold.
local hash = function(index, salt)
    local n = (index * 73856093 + salt * 19349663 + SEED * 2654435761) % 1000003
    n = (n * 1103515245 + 12345) % 2147483648
    return n
end

-- 1..count
local pick = function(index, salt, count)
    return hash(index, salt) % count + 1
end

local chance = function(index, salt, percent)
    return hash(index, salt) % 100 < percent
end

-- Every part a room owns, by name stem. Destroying a room walks this list, so a
-- room that was built with a rug and three crates is taken away with all of them.
local STEMS = {
    "RoomFloor", "RoomCeiling", "RoomWallWest", "RoomWallEast",
    "PartitionWest", "PartitionEast", "Lintel", "Door", "DoorPad", "Light",
    "WardrobeBack", "WardrobeNorth", "WardrobeSouth", "WardrobeTop", "HidePad",
    "RoomProp1", "RoomProp2", "RoomProp3", "RoomRug", "RoomPainting",
    "RoomLamp2"
}
local STEM_COUNT = 21

-- Which side a room's wardrobe was generated on, so the hiding check and the
-- step-in/step-out teleport agree with the geometry that was actually built.
local closetSide = {}
local built = {}
local present = {}
local progress = {}
local respawnAt = {}
local respawnZ = {}
local currentRoom = 1

local rushZ = RUSH_PARK_Z
local rushEndZ = RUSH_PARK_Z
local rushSpeed = RUSH_SPEED
local rushActive = false
local cooldown = 14
local warned = false

local sideOf = function(index)
    if closetSide[index] == -1 then
        return -1
    end
    return 1
end

local doorZ = function(index)
    return DOOR1_Z - PITCH * (index - 1)
end

local roomCentreZ = function(index)
    return doorZ(index) - PITCH * 0.5
end

local roomAt = function(z)
    local index = math.floor((DOOR1_Z - PITCH * 0.5 - z) / PITCH + 0.5) + 1
    if index < 1 then
        return 1
    end
    if index > DOORS then
        return DOORS
    end
    return index
end

local hiddenAt = function(position)
    local index = roomAt(position.z)
    local x = position.x * sideOf(index)
    if x < CLOSET_MIN_X or x > CLOSET_MAX_X then
        return false
    end
    return math.abs(position.z - roomCentreZ(index)) < CLOSET_HALF_Z
end

-- The room someone can come back to: the last door they opened, but never one
-- the hotel has already taken away. The oldest kept room is always there, so a
-- respawn always lands on a floor.
local respawnRoom = function(reached)
    local index = reached
    if index < currentRoom - BEHIND then
        index = currentRoom - BEHIND
    end
    if index < 1 then
        index = 1
    end
    if index > DOORS then
        index = DOORS
    end
    for step = 1, DOORS do
        if present[index] or index >= DOORS then
            return index
        end
        index = index + 1
    end
    return index
end

-- The entity follows the deepest player, so a run of a hundred doors is not a
-- hundred doors of walking before anything happens.
local furthestZ = function()
    local deepest = 0
    local plist = Players:GetPlayers()
    for i = 1, Players.PlayerCount do
        local z = plist[i].Position.z
        if z < deepest then
            deepest = z
        end
    end
    return deepest
end

local setupDoor = function(pad, door, index, dz)
    pad.Touched:Connect(function(hit)
        door.Position = Vector3.new(0, OPEN_DOOR_Y, dz)
        progress[hit.Name] = index
        if index % 10 == 0 then
            print(hit.Name .. " opened door " .. index .. " of " .. DOORS)
        end
    end)
end

local setupWardrobe = function(pad, index, centre, side)
    pad.Touched:Connect(function(hit)
        -- Touching the same pad again from inside steps back out. Geometry
        -- decides which way, so a player never gets stuck in the wardrobe.
        if hit.Position.x * side > CLOSET_MIN_X then
            hit:Teleport(Vector3.new(CLOSET_STEP_OUT * side, CLOSET_STAND_Y, centre))
        else
            hit:Teleport(Vector3.new(CLOSET_STAND * side, CLOSET_STAND_Y, centre))
        end
    end)
end

local lampColorFor = function(index)
    local tint = pick(index, 6, 4)
    if tint == 1 then
        return Vector3.new(0.95, 0.86, 0.62)
    end
    if tint == 2 then
        return Vector3.new(0.74, 0.84, 0.96)
    end
    if tint == 3 then
        return Vector3.new(0.82, 0.96, 0.8)
    end
    return Vector3.new(0.96, 0.74, 0.66)
end

local buildRoom = function(index)
    local dz = doorZ(index)
    local centre = roomCentreZ(index)

    -- Seeded variation. The corridor and the doorway keep their measurements
    -- because the doors and the avatar depend on them; what changes is the
    -- wardrobe side, the lighting, the paint and what is stacked in the room.
    local side = 1
    if pick(index, 2, 2) == 2 then
        side = -1
    end
    closetSide[index] = side

    local floorTint = pick(index, 3, 3) - 1
    local wallTint = pick(index, 4, 3) - 1
    local floorColor = Vector3.new(0.11 + floorTint * 0.015, 0.095 + floorTint * 0.01,
                                   0.115 + floorTint * 0.012)
    local wallColor = Vector3.new(0.16 + wallTint * 0.013, 0.15 + wallTint * 0.011,
                                  0.17 + wallTint * 0.012)
    local ceilingColor = Vector3.new(0.09, 0.09, 0.1 + wallTint * 0.008)
    local doorColor = Vector3.new(0.4, 0.24 + pick(index, 5, 3) * 0.035, 0.15)

    workspace:CreatePart("RoomFloor" .. index, Vector3.new(0, -0.5, centre),
                         Vector3.new(26, 1, PITCH), floorColor)
    workspace:CreatePart("RoomCeiling" .. index, Vector3.new(0, 12.5, centre),
                         Vector3.new(26, 1, PITCH), ceilingColor)
    workspace:CreatePart("RoomWallWest" .. index, Vector3.new(-13.5, 6, centre),
                         Vector3.new(1, 12, PITCH), wallColor)
    workspace:CreatePart("RoomWallEast" .. index, Vector3.new(13.5, 6, centre),
                         Vector3.new(1, 12, PITCH), wallColor)

    workspace:CreatePart("PartitionWest" .. index, Vector3.new(-8, 6, dz),
                         Vector3.new(10, 12, 1), wallColor)
    workspace:CreatePart("PartitionEast" .. index, Vector3.new(8, 6, dz),
                         Vector3.new(10, 12, 1), wallColor)
    workspace:CreatePart("Lintel" .. index, Vector3.new(0, 10.65, dz),
                         Vector3.new(6, 2.7, 1), wallColor)

    local door = workspace:CreatePart("Door" .. index, Vector3.new(0, 4.65, dz),
                                      Vector3.new(6, 9.3, 0.6), doorColor)
    local pad = workspace:CreatePart("DoorPad" .. index, Vector3.new(0, 0.5, dz + 1.3),
                                     Vector3.new(6, 1, 1), Vector3.new(0, 0, 0))
    pad.Transparency = 1
    pad.CanCollide = false

    workspace:CreatePart("Light" .. index, Vector3.new(0, 11.6, centre),
                         Vector3.new(8, 0.3, 0.8), lampColorFor(index))
    if chance(index, 7, 30) then
        workspace:CreatePart("RoomLamp2" .. index, Vector3.new(0, 11.6, centre - PITCH * 0.28),
                             Vector3.new(4, 0.3, 0.6), LAMP_COLOR)
    end

    workspace:CreatePart("WardrobeBack" .. index, Vector3.new(12.6 * side, 4, centre),
                         Vector3.new(0.4, 8, 4), WOOD_COLOR)
    workspace:CreatePart("WardrobeNorth" .. index,
                         Vector3.new(11.2 * side, 4, centre + 1.8),
                         Vector3.new(2.4, 8, 0.4), WOOD_COLOR)
    workspace:CreatePart("WardrobeSouth" .. index,
                         Vector3.new(11.2 * side, 4, centre - 1.8),
                         Vector3.new(2.4, 8, 0.4), WOOD_COLOR)
    workspace:CreatePart("WardrobeTop" .. index, Vector3.new(11.2 * side, 8, centre),
                         Vector3.new(2.4, 0.4, 4), WOOD_COLOR)

    local hide = workspace:CreatePart("HidePad" .. index, Vector3.new(9.8 * side, 0.5, centre),
                                      Vector3.new(0.8, 1, 3.6), Vector3.new(0, 0, 0))
    hide.Transparency = 1
    hide.CanCollide = false

    -- Furniture. The walking lane down the middle and the space in front of the
    -- door stay clear, so a crate can never wall the player in or hide the pad.
    local props = pick(index, 8, 4) - 1
    for n = 1, props do
        local px = 7.5
        if pick(index, 20 + n, 2) == 2 then
            px = -7.5
        end
        local pz = centre + (pick(index, 30 + n, 5) - 3) * 2.4
        local height = 1.1 + pick(index, 40 + n, 3) * 0.5
        workspace:CreatePart("RoomProp" .. n .. index, Vector3.new(px, height * 0.5, pz),
                             Vector3.new(1.5, height, 1.5), WOOD_COLOR)
    end

    if chance(index, 9, 35) then
        local rug = workspace:CreatePart("RoomRug" .. index, Vector3.new(0, 0.06, centre),
                                         Vector3.new(8, 0.12, 10),
                                         Vector3.new(0.3, 0.12 + floorTint * 0.02, 0.14))
        rug.CanCollide = false
    end

    if chance(index, 11, 40) then
        local px = 6.2
        if pick(index, 12, 2) == 2 then
            px = -6.2
        end
        local painting = workspace:CreatePart("RoomPainting" .. index,
                                              Vector3.new(px, 8, dz + 0.45),
                                              Vector3.new(3, 2, 0.12),
                                              Vector3.new(0.5, 0.36, 0.2))
        painting.CanCollide = false
    end

    setupDoor(pad, door, index, dz)
    setupWardrobe(hide, index, centre, side)

    built[index] = true
    present[index] = true
end

local destroyRoom = function(index)
    for i = 1, STEM_COUNT do
        local part = workspace:FindFirstChild(STEMS[i] .. index)
        if part ~= nil then
            part:Destroy()
        end
    end
    present[index] = false
end

local occupied = function(index, plist, count)
    local centre = roomCentreZ(index)
    for i = 1, count do
        local z = plist[i].Position.z
        if z < centre + PITCH * 0.5 + 1.5 and z > centre - PITCH * 0.5 - 1.5 then
            return true
        end
    end
    return false
end

-- The window: build what is coming, take away what is gone. A room someone is
-- standing in is never taken away, and a room is only ever built once, so a run
-- of a hundred doors never holds more than a handful of rooms worth of parts.
local keepWindow = function(plist, count)
    local deepest = 0
    for i = 1, count do
        local z = plist[i].Position.z
        if z < deepest then
            deepest = z
        end
    end

    currentRoom = roomAt(deepest)

    for index = currentRoom - BEHIND, currentRoom + AHEAD do
        if index >= 1 and index <= DOORS and not built[index] then
            buildRoom(index)
        end
    end

    for index = 1, DOORS do
        if present[index] and index < currentRoom - BEHIND then
            if not occupied(index, plist, count) then
                destroyRoom(index)
            end
        end
    end
end

local buildLobby = function()
    local centre = (LOBBY_END_Z + DOOR1_Z) * 0.5
    local length = LOBBY_END_Z - DOOR1_Z

    workspace:CreatePart("LobbyFloor", Vector3.new(0, -0.5, centre),
                         Vector3.new(26, 1, length), Vector3.new(0.12, 0.1, 0.12))
    workspace:CreatePart("LobbyCeiling", Vector3.new(0, 12.5, centre),
                         Vector3.new(26, 1, length), Vector3.new(0.09, 0.09, 0.1))
    workspace:CreatePart("LobbyWallWest", Vector3.new(-13.5, 6, centre),
                         Vector3.new(1, 12, length), Vector3.new(0.17, 0.16, 0.18))
    workspace:CreatePart("LobbyWallEast", Vector3.new(13.5, 6, centre),
                         Vector3.new(1, 12, length), Vector3.new(0.17, 0.16, 0.18))
    workspace:CreatePart("LobbyBackWall", Vector3.new(0, 6, LOBBY_END_Z + 0.5),
                         Vector3.new(28, 12, 1), Vector3.new(0.17, 0.16, 0.18))
    workspace:CreatePart("LobbyLight", Vector3.new(0, 11.6, 8),
                         Vector3.new(8, 0.3, 0.8), LAMP_COLOR)
    workspace:CreatePart("ElevatorBackWall", Vector3.new(0, 6, END_Z - 0.5),
                         Vector3.new(28, 12, 1), Vector3.new(0.17, 0.16, 0.18))
    workspace:CreatePart("ElevatorLight", Vector3.new(0, 11.6, END_Z + 6),
                         Vector3.new(6, 0.3, 0.6), Vector3.new(0.85, 1, 0.75))

    local elevator = workspace:CreatePart("Elevator", Vector3.new(0, 4.5, END_Z + 4),
                                          Vector3.new(6, 9, 0.4),
                                          Vector3.new(0.15, 0.75, 0.35))
    elevator.Transparency = 1
    elevator.CanCollide = false
    elevator.Touched:Connect(function(hit)
        print(hit.Name .. " reached door " .. DOORS .. " and left the hotel")
        progress[hit.Name] = 0
        respawnAt[hit.Name] = 0
        hit:Teleport(LOBBY_SPAWN)
    end)
end

buildLobby()
currentRoom = 1
for index = 1, AHEAD + 1 do
    buildRoom(index)
end
print("Doors seed " .. SEED .. ": rooms are built " .. AHEAD .. " ahead and taken away " ..
      BEHIND .. " behind, " .. DOORS .. " doors in all")

RunService.Heartbeat:Connect(function(dt)
    local plist = Players:GetPlayers()
    local count = Players.PlayerCount

    keepWindow(plist, count)

    for i = 1, count do
        local player = plist[i]
        local name = player.Name
        local waiting = respawnAt[name]
        if waiting ~= nil and waiting > 0 then
            respawnAt[name] = waiting - dt
            if waiting - dt <= 0 then
                player:Teleport(Vector3.new(0, ROOM_SPAWN_Y, respawnZ[name]))
            end
        end
    end

    if rushActive then
        rushZ = rushZ - rushSpeed * dt
        if rush ~= nil then
            rush.Position = Vector3.new(0, RUSH_PARK_Y, rushZ)
        end

        for i = 1, count do
            local player = plist[i]
            local position = player.Position
            if math.abs(position.z - rushZ) < RUSH_HIT_DEPTH then
                if not hiddenAt(position) then
                    local name = player.Name
                    if respawnAt[name] == nil or respawnAt[name] <= 0 then
                        local reached = progress[name]
                        if reached == nil or reached < 1 then
                            reached = 1
                        end
                        respawnZ[name] = roomCentreZ(respawnRoom(reached))
                        respawnAt[name] = RESPAWN_DELAY
                        player:TakeDamage(DEATH_DAMAGE)
                    end
                end
            end
        end

        if rushZ <= rushEndZ then
            rushActive = false
            rushZ = RUSH_PARK_Z
            cooldown = 12 + math.random() * 8
            warned = false
            if rush ~= nil then
                rush.Position = Vector3.new(0, RUSH_PARK_Y, RUSH_PARK_Z)
            end
            if sign ~= nil then
                sign.Color = Vector3.new(0.9, 0.75, 0.35)
            end
        end
    else
        cooldown = cooldown - dt
        if cooldown < 3 and not warned then
            warned = true
            if sign ~= nil then
                sign.Color = Vector3.new(1, 0.1, 0.1)
            end
            print("The lights above the lobby door turn red")
        end
        if cooldown <= 0 then
            local deepest = furthestZ()
            local depth = roomAt(deepest)
            rushSpeed = RUSH_SPEED + math.min(18, depth * 0.18)
            rushZ = deepest + RUSH_LEAD
            rushEndZ = deepest - RUSH_REACH
            rushActive = true
            warned = false
            if rush ~= nil then
                rush.Position = Vector3.new(0, RUSH_PARK_Y, rushZ)
            end
            print("Something is running down the corridor towards door " .. depth)
        end
    end
end)
