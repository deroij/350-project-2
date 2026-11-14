#include <fstream>

#include "CameraSystem.h"
#include "CollisionComponent.h"
#include "CollisionLateSystem.h"
#include "CollisionSystem.h"
#include "ECSEngine.h"
#include "EntityManager.h"
#include "GravitySystem.h"
#include "InputComponent.h"
#include "InputSystem.h"
#include "LocationComponent.h"
#include "MathUtil.h"
#include "MovementSystem.h"
#include "ScoreComponent.h"
#include "ScoreSystem.h"
#include "SpawnComponent.h"
#include "SpawnSystem.h"
#include "SpriteComponent.h"
#include "SpriteManager.h"
#include "SpriteSystem.h"


std::string gResourcePath = "../assets/";
#define COMPONENTS                                                                             \
    ECSEngine::LocationComponent, ECSEngine::MovementComponent, ECSEngine::CollisionComponent, \
        ECSEngine::SpriteComponent, ECSEngine::SpawnComponent, ECSEngine::CameraComponent,     \
        ECSEngine::CameraFollower, ECSEngine::InputComponent, ECSEngine::GravityComponent,     \
        ECSEngine::CameraShake, ECSEngine::ScoreComponent, ECSEngine::RespawnComponent
using Engine = ECSEngine::ECSEngine<COMPONENTS>;

struct TileDef {
    ECSEngine::SpriteID sprite;
    ECSEngine::Rect bounds;   
};

void LoadMap(Engine& engine, const std::string& path, const std::string& file) {
    std::ifstream in(path + file);
    if(!in){
        std::cerr << "Could not open map: " << path + file << "\n";
        return;
    }

    auto& sprites = engine.GetSpriteManager();
    auto& entities = engine.GetEntityManager();

    std::string word;
    int dictType;
    in >> word >> dictType;

    std::unordered_map<char, TileDef> dict;

    TileDef starDef;
    starDef.sprite = sprites.RegisterTexture(path + "spritesheet-tiles-default.png", ECSEngine::Rect{{640, 320}, 64, 64});
    starDef.bounds = ECSEngine::Rect{{0,0}, 64, 64};
    dict['s'] = starDef;

    std::string token;
    while(in >> token && token != "map"){
        char c  = token[0];
        std::string image;
        float sx, sy, sw, sh;
        in >> image >> sx >> sy >> sw >> sh;

        TileDef def;
        def.sprite = sprites.RegisterTexture(path + image, ECSEngine::Rect{{sx, sy}, sw, sh});
        def.bounds = ECSEngine::Rect{{0,0}, 0, 0};

        if(dictType == 2){
            float bx, by, bw, bh;
            in >> bx >> by >> bw >> bh;
            def.bounds = ECSEngine::Rect{{bx, by}, bw, bh};
        }
        dict[c] = def;
    }

    float originX, originY, tileW, tileH;
    int gridW, gridH;
    in >> word >> originX >> originY;
    in >> word >> tileW >> tileH;
    in >> word >> gridW >> gridH;

    for(int row = 0; row < gridH; ++row){
        std::string line;
        in >> line;
        for(int col = 0; col < static_cast<int>(line.size()); ++col){
            char c = line[col];
            float x = originX + col * tileW;
            float y = originY + row * tileH;

            if(c == 'S'){
                auto e = entities.CreateEntity("spawn");

                ECSEngine::LocationComponent loc;
                loc.loc.x = x;
                loc.loc.y = y;
                entities.AddComponent(e, loc);

                ECSEngine::SpawnComponent spawn;
                spawn.entityID = e;
                spawn.spawnType = "star";
                spawn.spawnTextureID = dict['s'].sprite;
                spawn.timeBetweenSpawns = 180;
                spawn.timeToNextSpawn = 0;
                spawn.totalSpawns = 12;
                entities.AddComponent(e, spawn);

                ECSEngine::SpriteComponent spriteComponent;
                spriteComponent.textureID = dict['S'].sprite;
                spriteComponent.bounds = dict['S'].bounds;
                spriteComponent.worldSpace = true;
                entities.AddComponent(e, spriteComponent);

                continue;
            }

            auto it = dict.find(c);
            if(it == dict.end()){
                continue;
            }

            auto e = entities.CreateEntity("tile");

            ECSEngine::LocationComponent loc;
            loc.loc.x = x;
            loc.loc.y = y;
            entities.AddComponent(e, loc);

            ECSEngine::SpriteComponent spriteComponent;
            spriteComponent.textureID = it->second.sprite;
            spriteComponent.bounds = ECSEngine::Rect{{0,0}, tileW, tileH};
            spriteComponent.worldSpace = true;
            entities.AddComponent(e, spriteComponent);

            if(dictType == 2){
                ECSEngine::CollisionComponent cc{};
                cc.staticEntity = 1;
                cc.collisionBox = ECSEngine::Rect{
                    {x + it->second.bounds.topLeft.x, y + it->second.bounds.topLeft.y},
                    it->second.bounds.width, it->second.bounds.height};
                if (c == 'x'){
                    cc.doesDamage = true;
                }
                entities.AddComponent(e, cc);
            }
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc >= 3 && argv[1] == std::string("-path")) {
        gResourcePath = argv[2];
    }
    std::cout << "Usage: " << argv[0] << " [-path resource_path]\n";
    std::cout << "Using resource path: " << gResourcePath << "\n";

    // Engine test code
    Engine engine(1024, 768, "Test Engine");
    engine.GetWindowManager().GetWindow()->setFramerateLimit(60);
    //Framerate limit because I was going waaayyyy to fast too soon
    auto& sprites = engine.GetSpriteManager();
    auto& entities = engine.GetEntityManager();

    // Load sounds

    // Load maps
    LoadMap(engine, gResourcePath, "sky.map");
    LoadMap(engine, gResourcePath, "world.map");

    // Place player
    ECSEngine::SpriteID playerTile = sprites.RegisterTexture(gResourcePath + "spritesheet-characters-default.png", ECSEngine::Rect{{0, 640}, 128, 128});
    auto player = entities.CreateEntity("player");

    ECSEngine::LocationComponent playerLoc;
    playerLoc.loc.x = 700.0;
    playerLoc.loc.y = 100.0;
    entities.AddComponent(player, playerLoc);

    ECSEngine::SpriteComponent playerSprite;
    playerSprite.textureID = playerTile;
    playerSprite.bounds = ECSEngine::Rect{{-32,-64}, 64, 64};
    playerSprite.worldSpace = true;
    entities.AddComponent(player, playerSprite);

    ECSEngine::MovementComponent playerMovement;
    playerMovement.velocity.x = 0;
    playerMovement.velocity.y = 0;
    entities.AddComponent(player, playerMovement);

    ECSEngine::Point2D gravityAccel;
    gravityAccel.x = 0;
    gravityAccel.y = 2;
    entities.AddComponent(player, ECSEngine::GravityComponent(gravityAccel));

    entities.AddComponent(player, ECSEngine::InputComponent(player));

    ECSEngine::CollisionComponent playerCollision{};
    playerCollision.collisionBox = ECSEngine::Rect{{playerLoc.loc.x, playerLoc.loc.y}, 64, 64};
    entities.AddComponent(player, playerCollision);

    ECSEngine::RespawnComponent playerRespawn{};
    playerRespawn.loc.x = 700.0;
    playerRespawn.loc.y = 100.0;
    entities.AddComponent(player, playerRespawn);

    // Setup score component
    auto scoreManager = entities.CreateEntity("scoreM");

    ECSEngine::ScoreComponent scoreComp{};
    for (int i = 0; i < 10; i++){
        ECSEngine::SpriteID number = sprites.RegisterTexture(gResourcePath + "spritesheet-tiles-default.png", ECSEngine::Rect{{832, 832 - 64 * i}, 64, 64});
        scoreComp.digitTextureID[i] = number;
    }

    for (int i = 0; i < 3; i++){
        auto score = entities.CreateEntity("score" + std::to_string(i));
        ECSEngine::LocationComponent scoreLoc{};
        scoreLoc.loc.x = 128 - 64*i;
        scoreLoc.loc.y = 0;
        entities.AddComponent(score, scoreLoc);
        
        ECSEngine::SpriteComponent sprite{};
        sprite.worldSpace = false;
        sprite.bounds = ECSEngine::Rect{ECSEngine::Point2D(0,0), 64, 64};
        sprite.textureID = scoreComp.digitTextureID[0];
        entities.AddComponent(score, sprite);

        scoreComp.digitID[i] = score;
    }
    entities.AddComponent(scoreManager, scoreComp);

    // Setup Camera
    auto cameraEntity = entities.CreateEntity("camera");
    ECSEngine::CameraShake cameraShake{};
    entities.AddComponent(cameraEntity, cameraShake);

    ECSEngine::CameraComponent cameraComp;
    cameraComp.worldUnitsPerPixel = 1.0f;
    cameraComp.speed = 0.1f;
    cameraComp.position.x = 512;
    cameraComp.position.y = 384;
    cameraComp.hardExclusion = ECSEngine::Rect{{102.4f, 0.0f}, 819.2f, 768.0f};
    cameraComp.softExclusion = ECSEngine::Rect{{307.2f, 0.0f}, 409.6f, 768.0f};
    entities.AddComponent(cameraEntity, cameraComp);

    ECSEngine::CameraFollower follow;
    follow.entityID = player;
    entities.AddComponent(cameraEntity, follow);

    // Add all systems
    auto& sm = engine.GetSystemManager();
    sm.AddSystem(std::make_unique<ECSEngine::CollisionLateSystem<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::InputSystem<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::GravitySystem<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::MovementSystem<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::CollisionSystem<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::ScoreSystem<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::CameraSystem<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::SpriteSystem<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::SpawnSystem<COMPONENTS>>());

    engine.Run();
    return 0;
}
