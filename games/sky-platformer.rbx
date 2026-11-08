-- Sky Platformer
-- Runs on the game server and in Studio. Moves the three action platforms,
-- sends players back to spawn when they touch the lava, and celebrates the goal.

local moveA = workspace:FindFirstChild("MoveA")
local moveB = workspace:FindFirstChild("MoveB")
local spinner = workspace:FindFirstChild("Spinner")
local spawn = workspace:FindFirstChild("SpawnPoint")
local lava = workspace:FindFirstChild("Lava")
local goal = workspace:FindFirstChild("Goal")
local elapsed = 0

RunService.Heartbeat:Connect(function(dt)
    elapsed = elapsed + dt

    if moveA ~= nil then
        moveA.Position = Vector3.new(math.sin(elapsed) * 7, 5, -2)
    end

    if moveB ~= nil then
        moveB.Position = Vector3.new(0, 9 + math.sin(elapsed * 1.6) * 2, -24)
    end

    if spinner ~= nil then
        spinner.Position = Vector3.new(8 + math.cos(elapsed * 2) * 4, 14, -34)
    end
end)

if lava ~= nil and spawn ~= nil then
    lava.Touched:Connect(function(hit)
        workspace:PlaySound("lava", lava.Position, 0.7)
        print("Back to spawn")
        hit:Teleport(spawn.Position)
    end)
end

if goal ~= nil and spawn ~= nil then
    goal.Touched:Connect(function(hit)
        workspace:PlaySound("chime", goal.Position, 0.8)
        print("You reached the sky goal!")
        hit:Teleport(spawn.Position)
    end)
end
