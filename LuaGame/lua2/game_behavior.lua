
-- Lua caches that returned value so any subsequent calls to require(...) will reuse the same result and won’t re-run the file.
local phase_load = require("phase_load")
local phase_1 = require("phase_1")
local phase_2 = require("phase_2")

game = {
    --game_entity = {},
    config = {
        player_speed = 10.0,
        bounds = { left = -5, right = 10, bottom = -5, top = 5 },
        is_out_of_bounds = function(self, x, y)
            return x < self.bounds.left or x > self.bounds.right or y < self.bounds.bottom or y > self.bounds.top
        end,
        sounds = {
            music_title = "music_title",
            music_end = "music_end",
            music_lvl1 = "music_lvl1",
            music_lvl2 = "music_lvl2",
            music_lvl3 = "music_lvl3",
    
            projectile_fire1 = "fire1",
            player_death = "player_death",
            element_explode = "player_death"
        }, 
        enemy_kill_count = 0,
        player_deaths = 0,

        phases = {phase_1, phase_2},
        current_phase_index = -1,
        next_phase_index = 1,
        current_phase = nil,
        phase_is_loading = false, -- ???
        
        meta = {
            player_speed = {inspectable = true, serializable = true}
        }
    },

    batch_id = nil, -- -> PHASE

    meta = {
        config = {inspectable = true, serializable = true}
    }
}

function game:switch_to_next_phase()
    self.config.next_phase_index = self.config.next_phase_index + 1
end

function game:switch_phase(phase)
    
    if self.current_phase then
        self.current_phase:stop()
        --self.current_phase:destroy()
    end
    
    self.current_phase = phase
    if self.current_phase then
        self.current_phase:run()
    end

end

function game:init()
    engine.log("game:init()")
end

function game:destroy()
    engine.log('game:destroy() #' .. self.id())
end

function game:run()

    -- Start loading the next level
    --phase_load:run(self.config.phases[self.config.next_phase_index])
    --self:phase_is_loading = true



    -- TEST
    -- Queue a batch of assets
    self.batch_id = engine.queue_assets({"sound1.wav", "model1.fbx", "texture1.png"})


    --[[
        engine.log(string.format(
            'player_behavior:run() entity = %s, projectile_pool_behavior = %s, game_behavior = %s',
            self.id(),
            tostring(self.projectile_pool),
            tostring(self.game)
            ))
            ]]
    engine.log('game:run() #' .. self.id())
end

function game:stop()
    engine.log('game:stop() #' .. self.id())

    -- stop current_phase (so it can deallocate resources)
    --self:current_phase:stop()
end

function game:update(dt)

    --[[
    if self:phase_is_loading then
        if phase_load:is_done() then
            if phase_load:did_loading_succeed() then
                print("All assets in batch " .. self.batch_id .. " loaded successfully!")
            else
                print("Some assets in batch " .. self.batch_id .. " failed to load.")
            end
        else
            local progress = engine.get_loading_progress(self.batch_id)
            print(string.format("Loading progress: %.2f%%", progress * 100))
        end
    end

    if self.current_phase then
        
        self.current_phase:update(dt)

        if self.current_phase:has_finished() then
            self.config.current_phase_index = self.config.current_phase_index + 1

            if self.config.phases[self.config.current_phase_index] then
                self:switch_phase(self.config.phases[self.config.current_phase_index])
            else
                print('All phases completed.')
                self.current_phase:destroy()
                self.current_phase = nil
            end
        end
    end
    ]]

    --self:switch_phase(self.config.phases[self.config.next_phase_index])
    -- + run phase_loader while waiting for the next phase to finish loading


    -- TEST
    if engine.is_loading_complete(self.batch_id) then
        -- Loading is complete
        if engine.did_loading_succeed(self.batch_id) then
            print("All assets in batch " .. self.batch_id .. " loaded successfully!")
        else
            print("Some assets in batch " .. self.batch_id .. " failed to load.")
        end
    else
        -- Loading is still in progress
        local progress = engine.get_loading_progress(self.batch_id)
        print(string.format("Loading progress: %.2f%%", progress * 100))
    end

--[[
while not engine.is_loading_complete(batch_id) do
    local progress = engine.get_loading_progress(batch_id)
    print("Loading progress: " .. (progress * 100) .. "%")
    os.execute("sleep 0.1") -- Replace with your engine's frame delay function
end

-- Check if all assets loaded successfully
if engine.did_loading_succeed(batch_id) then
    print("All assets in batch " .. batch_id .. " loaded successfully!")
else
    print("Some assets in batch " .. batch_id .. " failed to load.")
end
]]

--[[
    LOAD LOADING_SCREEN_BEHAVIOR. Fast & requires no/cheap resources.
        As an entity: run(), update() called from Core. Inspectable
        As a local script: do run(), update() etc from here.
    Now, "load" the next level and wait for it to finish loading (fetching (if loaded) or importing assets).
            What exactly does this mean?
            1. The level entity / script is created
            2. Which function is responsible for starting the loading process: run(), load()
            3. IsReady = a script is still loading and is not ready to be updated
                Who query this? The game entity? The Core?
                If update() should not be called until isReady = true, it must be Core.
    When it's done, remove the loading screen
]]

--[[
int main()
{
    AssetLoader asset_loader;

    // Queue up some asset loading tasks
    auto sound_future = asset_loader.queue_task([] { load_sound("sound1.wav"); });
    auto model_future = asset_loader.queue_task([] { load_model("model1.obj"); });

    // Simulate a loading screen
    while (!asset_loader.is_loading_complete())
    {
        std::cout << "Loading assets...\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    // Wait for futures to ensure all exceptions are handled
    sound_future.get();
    model_future.get();

    std::cout << "All assets loaded!\n";
    return 0;
}
]]
end

game:init()
return game