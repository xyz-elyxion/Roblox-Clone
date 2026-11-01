-- Doors: The Hotel
-- One round is a dark corridor of eight rooms. Walk into the pad in front of a
-- room's door to slide it up, step into a wardrobe to hide, and reach the
-- elevator at the far end. Now and then an entity sweeps the corridor from the
-- lobby end to the elevator: anyone standing in the open takes the hit, anyone
-- inside a wardrobe survives it. Dying returns you to the lobby, and the doors
-- you already opened stay open, so a retry is quick.
--
-- Everything here is script state. The world file only supplies geometry and the
-- named parts below.

local ROOMS = 8
local ROOM1_Z = -34
local PITCH = 30

local LOBBY_SPAWN = Vector3.new(0, 6.2, 0)
local RUSH_PARK_Z = 40
local RUSH_PARK_Y = 4.5
local RUSH_SPEED = 42
local RUSH_HIT_DEPTH = 4.5

-- Wardrobe pocket, in world units. The pad sits at x 9.8, the pocket runs from
-- the pad to the back panel, so a position past the pad is "inside".
local CLOSET_MIN_X = 9.9
local CLOSET_MAX_X = 12.5
local CLOSET_HALF_Z = 1.8
local CLOSET_STAND = 11.4
local CLOSET_STEP_OUT = 7.8
local CLOSET_STAND_Y = 5

local rush = workspace:FindFirstChild("Rush")
local sign = workspace:FindFirstChild("HotelSign")
local elevator = workspace:FindFirstChild("Elevator")

local rushZ = RUSH_PARK_Z
local rushActive = false
local cooldown = 12
local warned = false

local roomZ = function(index)
    return ROOM1_Z - PITCH * (index - 1)
end

local hiddenAt = function(position)
    for i = 1, ROOMS do
        local cz = roomZ(i)
        if position.x > CLOSET_MIN_X and position.x < CLOSET_MAX_X then
            if position.z > cz - CLOSET_HALF_Z and position.z < cz + CLOSET_HALF_Z then
                return true
            end
        end
    end
    return false
end

local setupDoor = function(pad, door, openAt)
    if pad == nil or door == nil then
        return
    end
    pad.Touched:Connect(function(hit)
        door.Position = openAt
    end)
end

local setupCloset = function(pad, closetZ)
    if pad == nil then
        return
    end
    pad.Touched:Connect(function(hit)
        -- Touching the same pad again from inside steps back out. Geometry
        -- decides which way, so a player never gets stuck in the wardrobe.
        if hit.Position.x > CLOSET_MIN_X then
            hit:Teleport(Vector3.new(CLOSET_STEP_OUT, CLOSET_STAND_Y, closetZ))
        else
            hit:Teleport(Vector3.new(CLOSET_STAND, CLOSET_STAND_Y, closetZ))
        end
    end)
end

setupDoor(workspace:FindFirstChild("DoorPad1"), workspace:FindFirstChild("Door1"),
          Vector3.new(0, 14.25, -22))
setupDoor(workspace:FindFirstChild("DoorPad2"), workspace:FindFirstChild("Door2"),
          Vector3.new(0, 14.25, -52))
setupDoor(workspace:FindFirstChild("DoorPad3"), workspace:FindFirstChild("Door3"),
          Vector3.new(0, 14.25, -82))
setupDoor(workspace:FindFirstChild("DoorPad4"), workspace:FindFirstChild("Door4"),
          Vector3.new(0, 14.25, -112))
setupDoor(workspace:FindFirstChild("DoorPad5"), workspace:FindFirstChild("Door5"),
          Vector3.new(0, 14.25, -142))
setupDoor(workspace:FindFirstChild("DoorPad6"), workspace:FindFirstChild("Door6"),
          Vector3.new(0, 14.25, -172))
setupDoor(workspace:FindFirstChild("DoorPad7"), workspace:FindFirstChild("Door7"),
          Vector3.new(0, 14.25, -202))
setupDoor(workspace:FindFirstChild("DoorPad8"), workspace:FindFirstChild("Door8"),
          Vector3.new(0, 14.25, -232))

setupCloset(workspace:FindFirstChild("HidePad1"), -34)
setupCloset(workspace:FindFirstChild("HidePad2"), -64)
setupCloset(workspace:FindFirstChild("HidePad3"), -94)
setupCloset(workspace:FindFirstChild("HidePad4"), -124)
setupCloset(workspace:FindFirstChild("HidePad5"), -154)
setupCloset(workspace:FindFirstChild("HidePad6"), -184)
setupCloset(workspace:FindFirstChild("HidePad7"), -214)
setupCloset(workspace:FindFirstChild("HidePad8"), -244)

if elevator ~= nil then
    elevator.Touched:Connect(function(hit)
        print(hit.Name .. " reached the elevator and survived the hotel")
        hit:Teleport(LOBBY_SPAWN)
    end)
end

RunService.Heartbeat:Connect(function(dt)
    if rushActive then
        rushZ = rushZ - RUSH_SPEED * dt
        if rush ~= nil then
            rush.Position = Vector3.new(0, RUSH_PARK_Y, rushZ)
        end

        local plist = Players:GetPlayers()
        local count = Players.PlayerCount
        for i = 1, count do
            local player = plist[i]
            local position = player.Position
            if math.abs(position.z - rushZ) < RUSH_HIT_DEPTH then
                if not hiddenAt(position) then
                    player:TakeDamage(100)
                end
            end
        end

        if rushZ < -270 then
            rushActive = false
            rushZ = RUSH_PARK_Z
            cooldown = 10 + math.random() * 8
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
            rushActive = true
            warned = false
            rushZ = RUSH_PARK_Z - 4
            if rush ~= nil then
                rush.Position = Vector3.new(0, RUSH_PARK_Y, rushZ)
            end
            print("Something is running down the corridor")
        end
    end
end)
