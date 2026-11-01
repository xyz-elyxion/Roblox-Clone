-- Doors: The Hotel
-- The hotel is built here, not drawn into the world file. Every room, its door,
-- its wardrobe, the corridor between them and the elevator at the end are
-- generated when the script loads, and the run is 100 doors long. The world file
-- only holds the lobby spawn, the hotel sign and the entity's body.
--
-- Generation needs parts that were never in the world file, so the world script
-- exposes workspace:CreatePart(name, position, size, color). A created part is
-- anchored and collidable and carries the same properties as a world part, so the
-- rest of this script works on a generated room exactly like on a painted one.
--
-- Touch the pad in front of a door to lift it, step into the wardrobe beside it
-- to hide, and reach the elevator past door 100 to leave. Now and then an entity
-- starts behind the deepest player and runs past them: anyone standing in the
-- open when it goes by takes the hit, anyone inside a wardrobe survives. A death
-- costs the run, not the progress: the player comes back at the last door they
-- opened, and doors stay open.

local DOORS = 100
local PITCH = 30
local DOOR1_Z = -22
local END_Z = DOOR1_Z - PITCH * DOORS
local LOBBY_END_Z = 26.5

local LOBBY_SPAWN = Vector3.new(0, 6.2, 0)
local ROOM_SPAWN_Y = 6.2

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

local WALL_COLOR = Vector3.new(0.17, 0.16, 0.18)
local FLOOR_COLOR = Vector3.new(0.12, 0.1, 0.12)
local CEILING_COLOR = Vector3.new(0.09, 0.09, 0.1)
local WOOD_COLOR = Vector3.new(0.34, 0.22, 0.12)
local DOOR_COLOR = Vector3.new(0.4, 0.26, 0.15)
local LAMP_COLOR = Vector3.new(0.92, 0.88, 0.72)
local OPEN_DOOR_Y = 14.25

local rush = workspace:FindFirstChild("Rush")
local sign = workspace:FindFirstChild("HotelSign")

-- Which side a room's wardrobe was generated on, so the hiding check and the
-- step-in/step-out teleport agree with the geometry that was actually built.
local closetSide = {}
local progress = {}
local respawnAt = {}
local respawnZ = {}

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

local buildRoom = function(index)
    local dz = doorZ(index)
    local centre = roomCentreZ(index)
    local side = 1
    if math.random(1, 2) == 2 then
        side = -1
    end
    closetSide[index] = side

    workspace:CreatePart("RoomFloor" .. index, Vector3.new(0, -0.5, centre),
                         Vector3.new(26, 1, PITCH), FLOOR_COLOR)
    workspace:CreatePart("RoomCeiling" .. index, Vector3.new(0, 12.5, centre),
                         Vector3.new(26, 1, PITCH), CEILING_COLOR)
    workspace:CreatePart("RoomWallWest" .. index, Vector3.new(-13.5, 6, centre),
                         Vector3.new(1, 12, PITCH), WALL_COLOR)
    workspace:CreatePart("RoomWallEast" .. index, Vector3.new(13.5, 6, centre),
                         Vector3.new(1, 12, PITCH), WALL_COLOR)

    workspace:CreatePart("PartitionWest" .. index, Vector3.new(-8, 6, dz),
                         Vector3.new(10, 12, 1), WALL_COLOR)
    workspace:CreatePart("PartitionEast" .. index, Vector3.new(8, 6, dz),
                         Vector3.new(10, 12, 1), WALL_COLOR)
    workspace:CreatePart("Lintel" .. index, Vector3.new(0, 10.65, dz),
                         Vector3.new(6, 2.7, 1), WALL_COLOR)

    local door = workspace:CreatePart("Door" .. index, Vector3.new(0, 4.65, dz),
                                      Vector3.new(6, 9.3, 0.6), DOOR_COLOR)
    local pad = workspace:CreatePart("DoorPad" .. index, Vector3.new(0, 0.5, dz + 1.3),
                                     Vector3.new(6, 1, 1), Vector3.new(0, 0, 0))
    pad.Transparency = 1
    pad.CanCollide = false

    workspace:CreatePart("Light" .. index, Vector3.new(0, 11.6, centre),
                         Vector3.new(8, 0.3, 0.8), LAMP_COLOR)

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

    setupDoor(pad, door, index, dz)
    setupWardrobe(hide, index, centre, side)
end

local buildLobby = function()
    local centre = (LOBBY_END_Z + DOOR1_Z) * 0.5
    local length = LOBBY_END_Z - DOOR1_Z

    workspace:CreatePart("LobbyFloor", Vector3.new(0, -0.5, centre),
                         Vector3.new(26, 1, length), FLOOR_COLOR)
    workspace:CreatePart("LobbyCeiling", Vector3.new(0, 12.5, centre),
                         Vector3.new(26, 1, length), CEILING_COLOR)
    workspace:CreatePart("LobbyWallWest", Vector3.new(-13.5, 6, centre),
                         Vector3.new(1, 12, length), WALL_COLOR)
    workspace:CreatePart("LobbyWallEast", Vector3.new(13.5, 6, centre),
                         Vector3.new(1, 12, length), WALL_COLOR)
    workspace:CreatePart("LobbyBackWall", Vector3.new(0, 6, LOBBY_END_Z + 0.5),
                         Vector3.new(28, 12, 1), WALL_COLOR)
    workspace:CreatePart("LobbyLight", Vector3.new(0, 11.6, 8),
                         Vector3.new(8, 0.3, 0.8), LAMP_COLOR)
    workspace:CreatePart("ElevatorBackWall", Vector3.new(0, 6, END_Z - 0.5),
                         Vector3.new(28, 12, 1), WALL_COLOR)
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

for index = 1, DOORS do
    buildRoom(index)
end
buildLobby()
print("The hotel opened with " .. DOORS .. " generated doors")

RunService.Heartbeat:Connect(function(dt)
    local plist = Players:GetPlayers()
    local count = Players.PlayerCount

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
                        respawnZ[name] = roomCentreZ(reached)
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
