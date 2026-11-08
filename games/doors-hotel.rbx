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
-- The hotel is a path, not a corridor. Every room has an entry point and a
-- heading, and the rooms are walked in order: a straight room runs its length and
-- hands the heading on unchanged, while a corner room is square and ends in its
-- side wall, so the next room starts there with the heading turned a quarter
-- turn. Nothing is rotated - a room owns a local frame of (along, across) and
-- places every part in it, so a room on either axis is the same code with the
-- axes swapped. The corridor therefore bends, and the door onto the next room is
-- in the wall the player is facing when the bend arrives.
--
-- What a room is comes from SEED and the room's number, so the same seed builds
-- the same hotel every run and a different seed builds a different one with no
-- world file to edit. A room is one of four kinds:
--   plain   - a door, a wardrobe, a lamp and whatever furniture the seed stacks in
--   divider - a wall across it with the way through on one side, so it turns you
--   pillar  - a column in the middle, so you pick a side to walk down
--   corner  - the corridor turns left or right, the door in the side wall
-- The wardrobe side, the lighting, the paint and the furniture are seeded too.
--
-- Rooms are built two ahead of the deepest player and taken away two behind
-- them, so a hundred-door hotel costs only the rooms someone can actually see;
-- the ones behind are gone for good, which is why the hotel never lets you walk
-- back out of it. A room someone is standing in is never taken away.
--
-- Touch the pad in front of a door to lift it, step into the wardrobe beside it
-- to hide, and reach the elevator past door 100 to leave. Now and then an entity
-- starts behind the deepest player and runs down the path past them: anyone
-- standing in the open when it goes by takes the hit, anyone inside a wardrobe
-- survives. A death costs the run, not the progress: the player comes back at the
-- last door they opened, and doors stay open.

-- The walk below is allowed to bend back on itself, and a seed whose path
-- re-enters an earlier corridor puts two rooms in the same place. 71234 did,
-- twice, which the trainer's top-down view draws as one corridor crossed by
-- another. This seed's hundred rooms all stand clear of each other; moving the
-- pitch, the door count or this number means walking the path again.
local SEED = 71241
local DOORS = 100
local PITCH = 30 -- a straight room
local CORNER_LEN = 26 -- a corner room: square, so its exit wall is a room wide
local DOOR1_Z = -22
local LOBBY_END_Z = 26.5
local HALF = 13 -- half the corridor's width

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
local WARN_COLOR = Vector3.new(1, 0.1, 0.1)
local OPEN_DOOR_Y = 14.25

local rush = workspace:FindFirstChild("Rush")
local sign = workspace:FindFirstChild("HotelSign")

-- The warning is the lights above the lobby door: the hotel's sign and the
-- lobby's own lamp. They go red a few seconds before the entity comes and back
-- to their own colour once it has passed. A script's colour change reaches the
-- clients, so the warning is visible from inside the hotel and not only in the
-- server's console.
local signColor = Vector3.new(0.9, 0.75, 0.35)
if sign ~= nil then
    signColor = sign.Color
end
local lobbyLight = nil
local lobbyColor = LAMP_COLOR

local setLights = function(color)
    if sign ~= nil then
        sign.Color = color
    end
    if lobbyLight ~= nil then
        lobbyLight.Color = color
    end
end

local clearLights = function()
    if sign ~= nil then
        sign.Color = signColor
    end
    if lobbyLight ~= nil then
        lobbyLight.Color = lobbyColor
    end
end

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

-- Headings in quarters of a turn: -z (into the hotel), +x, +z, -x. A room's
-- across axis is its heading turned a quarter to the right, so a right turn is
-- the next entry in these tables.
local DIR_X = { 0, 1, 0, -1 }
local DIR_Z = { -1, 0, 1, 0 }

local dirX = {}
local dirZ = {}
local acrX = {}
local acrZ = {}
local entryX = {}
local entryZ = {}
local roomLen = {}
local turnSide = {} -- 0 straight, 1 turns right, -1 turns left
local roomKind = {} -- 1 plain, 2 corner, 3 divider, 4 pillar
local wardrobeSide = {}
local wardrobeU = {}
local centreX = {}
local centreZ = {}
local startDist = {} -- distance along the path at the room's door
local endX = 0
local endZ = 0

-- The whole path is laid out before a single part exists. Rooms are 30 apart
-- when they run straight on and 26 square when the hotel turns, so the path is
-- walked once here and every room keeps its own entry, heading and length.
local buildPath = function()
    local d = 1
    local ex = 0
    local ez = DOOR1_Z
    local dist = 0
    local sinceTurn = 0

    for index = 1, DOORS do
        dirX[index] = DIR_X[d]
        dirZ[index] = DIR_Z[d]
        acrX[index] = -dirZ[index]
        acrZ[index] = dirX[index]
        entryX[index] = ex
        entryZ[index] = ez
        startDist[index] = dist

        -- A turn needs straight rooms on either side of it: two corners in a
        -- row would be a staircase nobody can see down, and the first and last
        -- rooms keep the lobby and the elevator where the world file expects.
        local side = 0
        if sinceTurn >= 3 and index > 3 and index < DOORS - 2 then
            if chance(index, 21, 24) then
                side = pick(index, 22, 2)
                if side == 2 then
                    side = -1
                end
            end
        end
        sinceTurn = sinceTurn + 1

        local kind = 1
        local len = PITCH
        if side ~= 0 then
            kind = 2
            len = CORNER_LEN
            sinceTurn = 0
        else
            local rolled = pick(index, 23, 3)
            if rolled == 2 then
                kind = 3
            end
            if rolled == 3 then
                kind = 4
            end
        end

        roomKind[index] = kind
        turnSide[index] = side
        roomLen[index] = len
        wardrobeU[index] = len * 0.5

        -- A corner's wardrobe goes on the wall the corridor does not leave
        -- through, so it never stands in the doorway the player has to turn into.
        local ws = 1
        if pick(index, 2, 2) == 2 then
            ws = -1
        end
        if side ~= 0 then
            ws = -side
        end
        wardrobeSide[index] = ws

        centreX[index] = ex + dirX[index] * (len * 0.5)
        centreZ[index] = ez + dirZ[index] * (len * 0.5)

        dist = dist + len
        if side == 0 then
            ex = ex + dirX[index] * len
            ez = ez + dirZ[index] * len
        else
            ex = ex + dirX[index] * (len * 0.5) + acrX[index] * (HALF * side)
            ez = ez + dirZ[index] * (len * 0.5) + acrZ[index] * (HALF * side)
            d = d + side
            if d > 4 then
                d = d - 4
            end
            if d < 1 then
                d = d + 4
            end
        end
    end

    endX = entryX[DOORS] + dirX[DOORS] * roomLen[DOORS]
    endZ = entryZ[DOORS] + dirZ[DOORS] * roomLen[DOORS]
end

-- Every part a room owns, by name stem. Destroying a room walks this list, so a
-- room that was built with a rug and a divider is taken away with all of them.
local STEMS = {
    "RoomFloor", "RoomCeiling", "RoomWallWest", "RoomWallEast", "RoomBack",
    "PartitionWest", "PartitionEast", "Lintel", "Door", "DoorPad", "Light",
    "WardrobeBack", "WardrobeNorth", "WardrobeSouth", "WardrobeTop", "HidePad",
    "RoomProp1", "RoomProp2", "RoomProp3", "RoomRug", "RoomPainting",
    "RoomLamp2", "Divider", "Pillar"
}
local STEM_COUNT = 24

local built = {}
local present = {}
local progress = {}
local respawnAt = {}
local respawnRoomIndex = {}
local playerS = {} -- each player's distance along the path, refreshed every beat
local currentRoom = 1

-- Scratch for the two queries everything else is written in terms of: which room
-- a point is in, and where a point on the path is in the world.
local foundRoom = 0
local foundU = 0
local foundV = 0
local probeRoom = 1
local probeX = 0
local probeZ = 0
local markX = 0
local markZ = 0

local rushS = 0
local rushEndS = 0
local rushSpeed = RUSH_SPEED
local rushActive = false
local rushAxis = 1
local cooldown = 14
local warned = false

-- A room's local frame: u runs from the door into the room, v runs to the right
-- of the heading. Both queries are the same two dot products.
local locateRoom = function(position)
    local best = 0
    for index = 1, DOORS do
        if present[index] then
            local dx = position.x - entryX[index]
            local dz = position.z - entryZ[index]
            local u = dx * dirX[index] + dz * dirZ[index]
            local v = dx * acrX[index] + dz * acrZ[index]
            if u >= -0.5 and u <= roomLen[index] + 0.5 and math.abs(v) <= HALF + 0.5 then
                if index > best then
                    best = index
                    foundU = u
                    foundV = v
                end
            end
        end
    end
    foundRoom = best
    return best
end

-- The point a path distance lands on. The centreline of a corner runs to the
-- middle of the room and then out through the side wall.
local locatePath = function(distance)
    local s = distance
    if s < 0 then
        s = 0
    end
    local index = 1
    while index < DOORS and s >= startDist[index] + roomLen[index] do
        index = index + 1
    end
    local d = s - startDist[index]
    local u = d
    local v = 0
    if turnSide[index] ~= 0 and d > CORNER_LEN * 0.5 then
        u = CORNER_LEN * 0.5
        v = (d - CORNER_LEN * 0.5) * turnSide[index]
    end
    probeRoom = index
    probeX = entryX[index] + dirX[index] * u + acrX[index] * v
    probeZ = entryZ[index] + dirZ[index] * u + acrZ[index] * v
end

local roomDistance = function(index, u, v)
    local d = u
    if turnSide[index] ~= 0 and u > CORNER_LEN * 0.5 then
        d = CORNER_LEN * 0.5 + math.abs(v)
    end
    return startDist[index] + d
end

-- A part in a room's own frame. Creating it on either axis is the same call: the
-- size is given along the heading and across it, and the world size swaps with
-- the axes.
local place = function(index, name, u, v, y, su, sv, sy, color)
    local hx = math.abs(dirX[index])
    local hz = math.abs(dirZ[index])
    local rx = math.abs(acrX[index])
    local rz = math.abs(acrZ[index])
    local x = entryX[index] + dirX[index] * u + acrX[index] * v
    local z = entryZ[index] + dirZ[index] * u + acrZ[index] * v
    return workspace:CreatePart(name, Vector3.new(x, y, z),
                                Vector3.new(hx * su + rx * sv, sy, hz * su + rz * sv), color)
end

local setupDoor = function(pad, door, index)
    pad.Touched:Connect(function(hit)
        door.Position = Vector3.new(entryX[index], OPEN_DOOR_Y, entryZ[index])
        workspace:PlaySound("door", Vector3.new(entryX[index], OPEN_DOOR_Y, entryZ[index]), 0.8)
        progress[hit.Name] = index
        if index % 10 == 0 then
            print(hit.Name .. " opened door " .. index .. " of " .. DOORS)
        end
    end)
end

local setupWardrobe = function(pad, index)
    pad.Touched:Connect(function(hit)
        -- Touching the same pad again from inside steps back out. The room's own
        -- frame decides which way, so a player never gets stuck in the wardrobe
        -- or is thrown through a wall on a corner.
        local position = hit.Position
        local dx = position.x - entryX[index]
        local dz = position.z - entryZ[index]
        local v = dx * acrX[index] + dz * acrZ[index]
        local side = wardrobeSide[index]
        local u = wardrobeU[index]
        local placeV = CLOSET_STEP_OUT * side
        if v * side > CLOSET_MIN_X then
            placeV = CLOSET_STEP_OUT * side
        else
            placeV = CLOSET_STAND * side
        end
        markX = entryX[index] + dirX[index] * u + acrX[index] * placeV
        markZ = entryZ[index] + dirZ[index] * u + acrZ[index] * placeV
        workspace:PlaySound("wardrobe", Vector3.new(markX, CLOSET_STAND_Y, markZ), 0.6)
        hit:Teleport(Vector3.new(markX, CLOSET_STAND_Y, markZ))
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
    local len = roomLen[index]
    local side = turnSide[index]
    local kind = roomKind[index]
    local ws = wardrobeSide[index]

    -- Seeded variation. The corridor and the doorway keep their measurements
    -- because the doors and the avatar depend on them; what changes is the
    -- wardrobe side, the lighting, the paint and what is stacked in the room.
    local floorTint = pick(index, 3, 3) - 1
    local wallTint = pick(index, 4, 3) - 1
    local floorColor = Vector3.new(0.11 + floorTint * 0.015, 0.095 + floorTint * 0.01,
                                   0.115 + floorTint * 0.012)
    local wallColor = Vector3.new(0.16 + wallTint * 0.013, 0.15 + wallTint * 0.011,
                                  0.17 + wallTint * 0.012)
    local ceilingColor = Vector3.new(0.09, 0.09, 0.1 + wallTint * 0.008)
    local doorColor = Vector3.new(0.4, 0.24 + pick(index, 5, 3) * 0.035, 0.15)

    place(index, "RoomFloor" .. index, len * 0.5, 0, -0.5, len, HALF * 2, 1, floorColor)
    place(index, "RoomCeiling" .. index, len * 0.5, 0, 12.5, len, HALF * 2, 1, ceilingColor)

    if side == 0 then
        place(index, "RoomWallWest" .. index, len * 0.5, -HALF - 0.5, 6, len, 1, 12, wallColor)
        place(index, "RoomWallEast" .. index, len * 0.5, HALF + 0.5, 6, len, 1, 12, wallColor)
    else
        -- The wall the corridor leaves through is the next room's entry wall, so
        -- this room builds only the side it keeps and the far end.
        place(index, "RoomWallWest" .. index, len * 0.5, -side * (HALF + 0.5), 6, len, 1, 12,
              wallColor)
        place(index, "RoomBack" .. index, len + 0.5, 0, 6, 1, HALF * 2, 12, wallColor)
    end

    place(index, "PartitionWest" .. index, 0, -8, 6, 1, 10, 12, wallColor)
    place(index, "PartitionEast" .. index, 0, 8, 6, 1, 10, 12, wallColor)
    place(index, "Lintel" .. index, 0, 0, 10.65, 1, 6, 2.7, wallColor)

    local door = place(index, "Door" .. index, 0, 0, 4.65, 0.6, 6, 9.3, doorColor)
    local pad = place(index, "DoorPad" .. index, -1.3, 0, 0.5, 1, 6, 1, Vector3.new(0, 0, 0))
    pad.Transparency = 1
    pad.CanCollide = false

    place(index, "Light" .. index, len * 0.5, 0, 11.6, 0.8, 8, 0.3, lampColorFor(index))
    if chance(index, 7, 30) then
        place(index, "RoomLamp2" .. index, len * 0.5 + len * 0.28, 0, 11.6, 0.6, 4, 0.3,
              LAMP_COLOR)
    end

    place(index, "WardrobeBack" .. index, wardrobeU[index], 12.6 * ws, 4, 4, 0.4, 8, WOOD_COLOR)
    place(index, "WardrobeNorth" .. index, wardrobeU[index] + 1.8, 11.2 * ws, 4, 0.4, 2.4, 8,
          WOOD_COLOR)
    place(index, "WardrobeSouth" .. index, wardrobeU[index] - 1.8, 11.2 * ws, 4, 0.4, 2.4, 8,
          WOOD_COLOR)
    place(index, "WardrobeTop" .. index, wardrobeU[index], 11.2 * ws, 8, 4, 2.4, 0.4, WOOD_COLOR)

    local hide = place(index, "HidePad" .. index, wardrobeU[index], 9.8 * ws, 0.5, 3.6, 0.8, 1,
                       Vector3.new(0, 0, 0))
    hide.Transparency = 1
    hide.CanCollide = false

    -- The kind of room it is decides where the room can be walked through, so the
    -- furniture it gets is kept out of that route as well.
    if kind == 3 then
        -- A wall across the room with the way through down one side.
        local gap = -1
        if pick(index, 24, 2) == 2 then
            gap = 1
        end
        place(index, "Divider" .. index, len * 0.35, gap * 4.5, 6, 1, 17, 12, wallColor)
    end

    if kind == 4 then
        -- A column in the middle: the walking lane splits either side of it and
        -- it stops below the lamp.
        place(index, "Pillar" .. index, len * 0.5, 0, 5.5, 6, 8, 11, wallColor)
    end

    -- Furniture. The lane, the space in front of the door and the doorway out of
    -- the room stay clear, so a crate can never wall the player in or hide the
    -- pad that opens the next door.
    local props = pick(index, 8, 4) - 1
    for n = 1, props do
        local spread = (pick(index, 30 + n, 5) - 3) * 2.4
        local propU = len * 0.5 + spread
        local propV = 7.5
        if pick(index, 20 + n, 2) == 2 then
            propV = -7.5
        end
        if kind == 2 then
            propU = len * 0.24 + spread * 0.6
            propV = -7.5 * side
        end
        if kind == 3 then
            propU = len * 0.72 + spread
        end
        if kind == 4 then
            propU = len * 0.76 + spread * 0.6
        end
        local height = 1.1 + pick(index, 40 + n, 3) * 0.5
        place(index, "RoomProp" .. n .. index, propU, propV, height * 0.5, 1.5, 1.5, height,
              WOOD_COLOR)
    end

    if chance(index, 9, 35) and kind ~= 4 then
        local rug = place(index, "RoomRug" .. index, len * 0.5, 0, 0.06, 10, 8, 0.12,
                          Vector3.new(0.3, 0.12 + floorTint * 0.02, 0.14))
        rug.CanCollide = false
    end

    if chance(index, 11, 40) then
        local px = 6.2
        if pick(index, 12, 2) == 2 then
            px = -6.2
        end
        local painting = place(index, "RoomPainting" .. index, 0.45, px, 8, 0.12, 3, 2,
                               Vector3.new(0.5, 0.36, 0.2))
        painting.CanCollide = false
    end

    setupDoor(pad, door, index)
    setupWardrobe(hide, index)

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

local inside = function(index, position)
    local dx = position.x - entryX[index]
    local dz = position.z - entryZ[index]
    local u = dx * dirX[index] + dz * dirZ[index]
    local v = dx * acrX[index] + dz * acrZ[index]
    return u >= -0.5 and u <= roomLen[index] + 0.5 and math.abs(v) <= HALF + 0.5
end

-- The window: build what is coming, take away what is gone. A room someone is
-- standing in is never taken away, and a room is only ever built once, so a run
-- of a hundred doors never holds more than a handful of rooms worth of parts.
local keepWindow = function(plist, count, deepest)
    currentRoom = deepest
    if currentRoom < 1 then
        currentRoom = 1
    end
    if currentRoom > DOORS then
        currentRoom = DOORS
    end

    for index = currentRoom - BEHIND, currentRoom + AHEAD do
        if index >= 1 and index <= DOORS and not built[index] then
            buildRoom(index)
        end
    end

    for index = 1, DOORS do
        if present[index] and index < currentRoom - BEHIND then
            local busy = false
            for i = 1, count do
                if inside(index, plist[i].Position) then
                    busy = true
                end
            end
            if not busy then
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
    lobbyLight = workspace:CreatePart("LobbyLight", Vector3.new(0, 11.6, 8),
                                     Vector3.new(8, 0.3, 0.8), LAMP_COLOR)
    lobbyColor = lobbyLight.Color

    -- The elevator is the far wall of the last room, wherever the path has ended.
    place(DOORS, "ElevatorBackWall", roomLen[DOORS] + 0.5, 0, 6, 1, 28, 12,
          Vector3.new(0.17, 0.16, 0.18))
    place(DOORS, "ElevatorLight", roomLen[DOORS] - 6, 0, 11.6, 0.6, 6, 0.3,
          Vector3.new(0.85, 1, 0.75))

    local elevator = place(DOORS, "Elevator", roomLen[DOORS] - 4, 0, 4.5, 0.4, 6, 9,
                           Vector3.new(0.15, 0.75, 0.35))
    elevator.Transparency = 1
    elevator.CanCollide = false
    elevator.Touched:Connect(function(hit)
        workspace:PlaySound("elevator", hit.Position, 0.9)
        print(hit.Name .. " reached door " .. DOORS .. " and left the hotel")
        progress[hit.Name] = 0
        respawnAt[hit.Name] = 0
        hit:Teleport(LOBBY_SPAWN)
    end)
end

local parkRush = function()
    if rush ~= nil then
        rush.Position = Vector3.new(0, RUSH_PARK_Y, RUSH_PARK_Z)
        rush.Size = Vector3.new(24, 9, 1.6)
    end
    rushAxis = 1
end

local hiddenAt = function(position)
    local index = locateRoom(position)
    if index < 1 then
        return false
    end
    local side = wardrobeSide[index]
    if foundV * side < CLOSET_MIN_X or foundV * side > CLOSET_MAX_X then
        return false
    end
    return math.abs(foundU - wardrobeU[index]) < CLOSET_HALF_Z
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

buildPath()
buildLobby()
currentRoom = 1
for index = 1, AHEAD + 1 do
    buildRoom(index)
end
parkRush()
print("Doors seed " .. SEED .. ": " .. DOORS .. " doors down a path that turns, rooms built " ..
      AHEAD .. " ahead and taken away " .. BEHIND .. " behind")

RunService.Heartbeat:Connect(function(dt)
    local plist = Players:GetPlayers()
    local count = Players.PlayerCount

    -- Where every player is on the path, and the deepest room anyone has
    -- reached: the window, the entity and the respawn all read from these.
    local deepest = 0
    local deepestS = 0
    for i = 1, count do
        local player = plist[i]
        local index = locateRoom(player.Position)
        local s = 0
        if index >= 1 then
            s = roomDistance(index, foundU, foundV)
        end
        playerS[player.Name] = s
        if index > deepest then
            deepest = index
        end
        if s > deepestS then
            deepestS = s
        end
    end

    keepWindow(plist, count, deepest)

    for i = 1, count do
        local player = plist[i]
        local name = player.Name
        local waiting = respawnAt[name]
        if waiting ~= nil and waiting > 0 then
            respawnAt[name] = waiting - dt
            if waiting - dt <= 0 then
                local index = respawnRoomIndex[name]
                if index == nil then
                    index = 1
                end
                player:Teleport(Vector3.new(centreX[index], ROOM_SPAWN_Y, centreZ[index]))
            end
        end
    end

    if rushActive then
        rushS = rushS + rushSpeed * dt
        locatePath(rushS)
        if rush ~= nil then
            local axis = 1
            if dirX[probeRoom] ~= 0 then
                axis = 2
            end
            if axis ~= rushAxis then
                rushAxis = axis
                if axis == 1 then
                    rush.Size = Vector3.new(24, 9, 1.6)
                else
                    rush.Size = Vector3.new(1.6, 9, 24)
                end
            end
            rush.Position = Vector3.new(probeX, RUSH_PARK_Y, probeZ)
        end

        for i = 1, count do
            local player = plist[i]
            local position = player.Position
            local s = playerS[player.Name]
            if s == nil then
                s = 0
            end
            if math.abs(s - rushS) < RUSH_HIT_DEPTH then
                if not hiddenAt(position) then
                    local name = player.Name
                    if respawnAt[name] == nil or respawnAt[name] <= 0 then
                        local reached = progress[name]
                        if reached == nil or reached < 1 then
                            reached = 1
                        end
                        respawnRoomIndex[name] = respawnRoom(reached)
                        respawnAt[name] = RESPAWN_DELAY
                        workspace:PlaySound("death", position, 1)
                        player:TakeDamage(DEATH_DAMAGE)
                    end
                end
            end
        end

        if rushS >= rushEndS then
            rushActive = false
            cooldown = 12 + math.random() * 8
            warned = false
            parkRush()
            clearLights()
        end
    else
        cooldown = cooldown - dt
        if cooldown < 3 and not warned then
            warned = true
            setLights(WARN_COLOR)
            -- The warning is the only cue before the entity, so it gets the
            -- growl as well as the red lights.
            workspace:PlaySound("entity", Vector3.new(0, 6, deepestS), 0.7)
            print("The lights above the lobby door turn red")
        end
        if cooldown <= 0 then
            if deepest < 1 then
                deepest = 1
            end
            rushSpeed = RUSH_SPEED + math.min(18, deepest * 0.18)
            rushS = deepestS - RUSH_LEAD
            rushEndS = deepestS + RUSH_REACH
            rushActive = true
            warned = false
            locatePath(rushS)
            if rush ~= nil then
                rush.Position = Vector3.new(probeX, RUSH_PARK_Y, probeZ)
            end
            print("Something is running down the corridor towards door " .. deepest)
        end
    end
end)
