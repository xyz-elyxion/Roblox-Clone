-- Turbo Circuit
-- Press E beside the car to get in, W/S for throttle and brake, A/D to steer and
-- E again to get out. The car is simulated by the game server, so this script
-- only counts laps: the driver crosses the finish line once per circuit.

local finish = workspace:FindFirstChild("FinishLine")
local lapCount = 0

if finish ~= nil then
    finish.Touched:Connect(function(hit)
        lapCount = lapCount + 1
        workspace:PlaySound("lap", finish.Position, 0.7)
        print("Lap " .. lapCount .. " complete")
    end)
end
