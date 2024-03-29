/*
** EPITECH PROJECT, 2020
** Vulkan-Engine
** File description:
** Game.cpp
*/
#include "Game.hpp"
#include "EntityLib/Core/EntityLib.hpp"
#include "EntityLib/Core/GPUDisplay.hpp"
#include "EntityLib/Core/GPUEntityMgr.hpp"
#include "EntityCore/Tools/Tracer.hpp"
#include "Menu.hpp"
#include <fstream>
#include <cmath>
#include <algorithm>
#include <iostream>

const std::pair<unsigned char, unsigned char> Game::spawnability[6] {{BLOCK1, 1}, {LINE1, 7}, {MULTICOLOR, 3}, {BOMB1, 10}, {FISH1, 4}, {CANDY1, 75}};
const WeaponAttributes Game::weaponList[5] {{"laser", 500, 23}, {"blaster", 800, 16}, {"wave", 1200, 18}, {"eclatant", 3000, 25}, {"laserifier", 20000, 60}};
const VesselAttributes Game::vesselList[6] {{"classic", 1000, 2, 2, CLASSIC}, {"better", 4000, 3, 3, BETTER}, {"aerodynamic", 6000, 3, 1, AERODYNAMIC}, {"energised", 10000, 5, 12, ENERGISED}, {"optimal", 32000, 5, 2, OPTIMAL}, {"boosted", 56000, 10, 30, BOOSTED}};
const GeneratorAttributes Game::generatorList[6] {{"minimal", 500, 1200, 3, 50}, {"basic", 3000, 200, 5, 30}, {"good", 10000, 600, 20, 20}, {"highly_performant", 34000, 3000, 100, 10}, {"excellent", 96000, 10000, 500, 5}, {"perfect", 600000, 30000, 3000, 4}};
const RecoolerAttributes Game::recoolerList[10] {{"vent", 2000, 4, 100}, {"advanced vent", 4000, 8, 120}, {"simple water-cooling", 10000, 18, 1000}, {"water-based Closed-Loop Recooling System", 20000, 35, 2000}, {"coolant-based CLRS", 50000, 140, 10000}, {"heat exchanger", 120000, 270, 12000}, {"uncredible heat exchanger", 250000, 450, 18000}, {"heat-to-ray", 800000, 600, 300000}, {"hellium fission", 2400000, 1200, 1800000}, {"Xtreme Hellium Fission", 30000000, 2000, 3000000}};
const ShieldAttributes Game::shieldList[11] {{"better than nothing", 2000, 0.00025, 1, 0.625, 2.5}, {"maybe good", 4000, 0.0005, 2, 1.25, 5}, {"the stainless steel shield !", 8000, 0.001, 4, 2.5, 10}, {"auto-cleanable upgraded shield", 16000, 0.002, 8, 5, 20}, {"micro-technology shield", 32000, 0.004, 16, 10, 40}, {"electro-pulsed forcefield", 64000, 0.008, 32, 20, 80}, {"microfusion shield system", 128000, 0.016, 64, 40, 160}, {"quantum-based shield system", 256000, 0.04, 24, 100, 400}, {"multi-dimensionnal quantum-shield system", 512000, 0.08, 32, 200, 800}, {"useless shield", 1024000, 0.1, 40, 250, 1000}, {"The UseLess Shield !", 2048000, 0.12, 48, 300, 1200}};
const SpecialWeaponAttributes Game::specialList[10] {{"None", "Nothing", 0, 0}, {"big_laser", "powerfull", 3000, 20},
{"protecto", "destroy candies at proximity of you in all directions\nIt's my favorite special weapon in OpenTyrian", 6000, 10},
{"mini_transperseur", "unstoppable", 10000, 12}, {"transperseur", "unstoppable", 15000, 20}, {"super_transperseur", "unstoppable", 100000, 30},
{"Shield Booster", "Convert special point into shield energy at 50 units/s\nThis effect end when out of special point", 3000000, 0},
{"Enhanced Shield Booster", "Convert special point into shield energy at 50 units/s\nThis effect end when out of special point\nEnabled when shield is getting low", 10000000, 0},
{"HighP Shield Overclocker", "Convert special point into shield energy to maintain 20% shield energy\nProduce 5k heat and consume 1.5 special_point for each shield point generated", 25000000, 0},
{"Universal converter", "HighP Shield Overclocker functionnality\nconvert special point into coolant to maintain 25% coolant with 1:25k ratio\nconvert energy over 100% into special point at 20k:1 ratio\nIncrease special point limit from 100 to 160\nSpecial points over 150 are immediately consumed\n", 50000000, 0}};

Game::Game(const std::string &name, uint32_t version, int width, int height) : candyPosDist(20, width - 20), width(width), height(height)
{
    bool enableDebugLayers = true;
    bool drawLogs = true;
    bool saveLogs = false;

    std::ifstream config("config.txt");
    std::string str1, unused;
    while (config) {
        int value = -1;
        config >> str1 >> unused >> value;
        if (value == -1)
            break;
        if (str1 == "debug")
            drawLogs = value;
        if (str1 == "writeLog")
            saveLogs = value;
        if (str1 == "debugLayer")
            enableDebugLayers = value;
    }
    core = std::make_shared<EntityLib>(name.c_str(), version, width, height, enableDebugLayers, drawLogs, saveLogs);
    compute = std::make_unique<GPUEntityMgr>(core);
    display = std::make_unique<GPUDisplay>(core, *compute);
    compute->init();
    tracer = std::make_unique<Tracer>(100, 64);
    tracer->emplace(Trace::CHAR, &player1.alive, "player1", nullptr);
    tracer->emplace(Trace::FLOAT, &player1.energy, "ENERGY", nullptr);
    tracer->emplace(Trace::FLOAT, &player1.energyMax, "max", nullptr);
    tracer->emplace(Trace::FLOAT, &player1.energyRate, "rate", nullptr);
    tracer->emplace(Trace::FLOAT, &player1.shield, "SHIELD", nullptr);
    tracer->emplace(Trace::FLOAT, &player1.shieldMax, "max", nullptr);
    tracer->emplace(Trace::FLOAT, &player1.shieldRate, "rate", nullptr);
    tracer->emplace(Trace::FLOAT, &player1.coolant, "COOLANT", nullptr);
    tracer->emplace(Trace::FLOAT, &player1.coolantMax, "max", nullptr);
    tracer->emplace(Trace::FLOAT, &player1.coolantRate, "rate", nullptr);
    tracer->emplace(Trace::CHAR, &player2.alive, "player2", nullptr);
    tracer->emplace(Trace::FLOAT, &player2.energy, "ENERGY", nullptr);
    tracer->emplace(Trace::FLOAT, &player2.energyMax, "max", nullptr);
    tracer->emplace(Trace::FLOAT, &player2.energyRate, "rate", nullptr);
    tracer->emplace(Trace::FLOAT, &player2.shield, "SHIELD", nullptr);
    tracer->emplace(Trace::FLOAT, &player2.shieldMax, "max", nullptr);
    tracer->emplace(Trace::FLOAT, &player2.shieldRate, "rate", nullptr);
    tracer->emplace(Trace::FLOAT, &player2.coolant, "COOLANT", nullptr);
    tracer->emplace(Trace::FLOAT, &player2.coolantMax, "max", nullptr);
    tracer->emplace(Trace::FLOAT, &player2.coolantRate, "rate", nullptr);
    tracer->emplace(Trace::FLOAT, &difficultyCoef, "ScMult", nullptr);
}

Game::~Game()
{
    display->unpause();
    display.reset();
    compute.reset();
    core.reset();
}

void Game::init()
{
    core->loadFragment(LASERIFIER, 32, 128, 96, 148, 1, 1, 0, 0, F_OTHER);
    core->loadFragment(MIEL0, 0, 0, 16, 16, 6, 1, 32, 32, F_CANDY);
    core->loadFragment(MIEL1, 16, 0, 32, 16, 8, 1, 32, 32, F_MIEL1);
    core->loadFragment(MIEL2, 32, 0, 48, 16, 10, 1, 32, 32, F_MIEL2);
    core->loadFragment(MIEL3, 48, 0, 64, 16, 12, 1, 32, 32, F_MIEL3);
    core->loadFragment(MIEL4, 64, 0, 80, 16, 14, 1, 32, 32, F_MIEL4);
    core->loadFragment(MIEL5, 80, 0, 96, 16, 16, 1, 32, 32, F_MIEL5);
    core->loadFragment(LINE1, 0, 16, 16, 32, 2, 1, 32, 32, F_LINE);
    core->loadFragment(LINE2, 16, 16, 32, 32, 2, 1, 32, 32, F_LINE);
    core->loadFragment(LINE3, 32, 16, 48, 32, 2, 1, 32, 32, F_LINE);
    core->loadFragment(LINE4, 48, 16, 64, 32, 2, 1, 32, 32, F_LINE);
    core->loadFragment(MULTICOLOR, 64, 16, 80, 32, 2, 1, 32, 32, F_MULTICOLOR);
    core->loadFragment(CHOCOLAT, 80, 16, 96, 32, 1000000, 2, 32, 32, F_OTHER);
    core->loadFragment(CANDY1, 0, 32, 16, 48, 1, 1, 32, 32, F_CANDY);
    core->loadFragment(CANDY2, 16, 32, 32, 48, 1, 1, 32, 32, F_CANDY);
    core->loadFragment(CANDY3, 32, 32, 48, 48, 1, 1, 32, 32, F_CANDY);
    core->loadFragment(CANDY4, 48, 32, 64, 48, 1, 1, 32, 32, F_CANDY);
    core->loadFragment(BOMB1, 0, 48, 16, 64, 5, 1, 32, 32, F_CANDY);
    core->loadFragment(BOMB2, 16, 48, 32, 64, 5, 1, 32, 32, F_CANDY);
    core->loadFragment(BOMB3, 32, 48, 48, 64, 5, 1, 32, 32, F_CANDY);
    core->loadFragment(BOMB4, 48, 48, 64, 64, 5, 1, 32, 32, F_CANDY);
    core->loadFragment(BLOCK1, 64, 32, 80, 48, 600, 1, 32, 32, F_BLOCK);
    core->loadFragment(BLOCK2, 80, 32, 96, 48, 600, 1, 32, 32, F_BLOCK);
    core->loadFragment(BLOCK3, 64, 48, 80, 64, 600, 1, 32, 32, F_BLOCK);
    core->loadFragment(BLOCK4, 80, 48, 96, 64, 600, 1, 32, 32, F_BLOCK);
    core->loadFragment(FISH1, 0, 64, 16, 72, 2, 1, 32, 24, F_CANDY);
    core->loadFragment(FISH2, 16, 72, 32, 80, 2, 1, 32, 24, F_CANDY);
    core->loadFragment(FISH3, 0, 64, 16, 72, 2, 1, 32, 24, F_CANDY);
    core->loadFragment(FISH4, 16, 72, 32, 80, 2, 1, 32, 24, F_CANDY);
    core->loadFragment(BIG_LASER, 32, 64, 64, 80, 7, 1, 32, 16, F_OTHER);
    core->loadFragment(BIG_LASER2, 64, 64, 96, 80, 100, 4, 64, 32, F_OTHER);
    core->loadFragment(CLASSIC, 0, 80, 16, 96, 1, 1, 48, 40, F_PLAYER);
    core->loadFragment(BETTER, 16, 80, 32, 96, 1, 1, 48, 40, F_PLAYER);
    core->loadFragment(AERODYNAMIC, 32, 80, 48, 96, 1, 1, 48, 40, F_PLAYER);
    core->loadFragment(OPTIMAL, 48, 80, 64, 96, 1, 1, 48, 40, F_PLAYER);
    core->loadFragment(ENERGISED, 64, 80, 80, 96, 1, 1, 48, 40, F_PLAYER);
    core->loadFragment(BOOSTED, 80, 80, 96, 96, 1, 1, 48, 40, F_PLAYER);
    core->loadFragment(LASER, 0, 96, 16, 112, 1, 1, 0, 0, F_OTHER);
    core->loadFragment(LASER2, 16, 96, 32, 112, 30, 1, 0, 0, F_OTHER);
    core->loadFragment(ECLAT0, 32, 96, 48, 112, 1, 1, 0, 0, F_OTHER);
    core->loadFragment(ECLAT1, 48, 96, 64, 112, 1, 1, 0, 0, F_OTHER);
    core->loadFragment(ECLAT2, 64, 96, 80, 112, 1, 1, 0, 0, F_OTHER);
    core->loadFragment(ECLAT3, 80, 96, 96, 112, 1, 1, 0, 0, F_OTHER);
    core->loadFragment(BONUS0, 8, 112, 16, 120, 1, 0, 16, 16, F_PIECE_5);
    core->loadFragment(BONUS1, 0, 112, 8, 120, 1, 0, 16, 16, F_PIECE_1);
    core->loadFragment(BONUS2, 16, 112, 24, 120, 1, 0, 16, 16, F_PIECE_10);
    core->loadFragment(BONUS3, 24, 112, 32, 120, 1, 0, 16, 16, F_PIECE_25);
    core->loadFragment(BONUS4, 32, 112, 48, 120, 1, 0, 32, 16, F_SHIELD_BOOST);
    core->loadFragment(BONUS5, 0, 120, 16, 128, 1, 0, 32, 16, F_SPECIAL_BOOST);
    core->loadFragment(BONUS6, 16, 120, 32, 128, 1, 0, 32, 16, F_COOLANT_BOOST);
    core->loadFragment(BLASTER, 32, 120, 48, 128, 2, 1, 0, 0, F_OTHER);
    core->loadFragment(ECLATANT, 48, 112, 64, 128, 1, 1, 0, 0, F_ECLATANT);
    core->loadFragment(PROTECTO, 64, 112, 80, 128, 4, 1, 0, 0, F_OTHER);
    core->loadFragment(STRONG_WAVE, 80, 112, 96, 128, 2, 1, 0, 0, F_OTHER);
    core->loadFragment(WAVE, 0, 128, 8, 144, 1, 1, 0, 0, F_OTHER);
    core->loadFragment(BIG_WAVE, 16, 128, 32, 160, 4, 1, 0, 0, F_OTHER);
    core->loadFragment(MINI_TRANS, 8, 128, 16, 134, 1000000, 1, 0, 0, F_OTHER);
    core->loadFragment(TRANS, 0, 144, 16, 160, 1000000, 1, 0, 0, F_OTHER);
    core->loadFragment(SUPER_TRANS, 0, 160, 32, 184, 1000000, 1, 0, 0, F_OTHER);
    display->pause();
    display->start();
    float ldiag = cos(3.1415926/8+3.1415926/4)*7;
    float diag = cos(3.1415926/4)*7;
    float bdiag = cos(3.1415926/8)*7;
    circle[0] = 7;
    circle[1] = bdiag;
    circle[2] = diag;
    circle[3] = ldiag;
    circle[4] = 0;
    circle[5] = -ldiag;
    circle[6] = -diag;
    circle[7] = -bdiag;
    circle[8] = -7;
    circle[9] = -bdiag;
    circle[10] = -diag;
    circle[11] = -ldiag;
    circle[12] = 0;
    circle[13] = ldiag;
    circle[14] = diag;
    circle[15] = bdiag;
    player1.ptr = display->getJaugePtr();
    player2.ptr = player1.ptr + 20;
}

void Game::mainloop()
{
    SDL_Event event;
    auto delay = std::chrono::duration<int, std::ratio<1,1000000>>(1000000/100);
    auto clock = std::chrono::system_clock::now();
    while (notQuitting) {
        gameStart();
        while (alive & someone) {
            clock += delay;
            while (SDL_PollEvent(&event)) {
                switch (event.type) {
                    case SDL_QUIT:
                        alive = false;
                        notQuitting = false;
                        break;
                    case SDL_KEYDOWN:
                        switch (event.key.keysym.scancode) {
                            case SDL_SCANCODE_F4:
                                alive = false;
                                notQuitting = false;
                                break;
                            case SDL_SCANCODE_ESCAPE:
                            case SDL_SCANCODE_Z:
                            case SDL_SCANCODE_N:
                            case SDL_SCANCODE_P:
                                compute->pause();
                                break;
                            case SDL_SCANCODE_Q:
                                compute->limitFramerate(false);
                                break;
                            case SDL_SCANCODE_A:
                                player1.velX = -1;
                                break;
                            case SDL_SCANCODE_D:
                                player1.velX = 1;
                                break;
                            case SDL_SCANCODE_W:
                                player1.velY = -1;
                                break;
                            case SDL_SCANCODE_S:
                                player1.velY = 1;
                                break;
                            case SDL_SCANCODE_SPACE:
                                compute->unpause();
                                player1.shooting = true;
                                break;
                            case SDL_SCANCODE_TAB:
                                player1.useSpecial = true;
                                break;
                            case SDL_SCANCODE_KP_4:
                                player2.velX = -1;
                                break;
                            case SDL_SCANCODE_KP_6:
                                player2.velX = 1;
                                break;
                            case SDL_SCANCODE_KP_8:
                                player2.velY = -1;
                                break;
                            case SDL_SCANCODE_KP_5:
                                player2.velY = 1;
                                break;
                            case SDL_SCANCODE_KP_0:
                            case SDL_SCANCODE_RSHIFT:
                                player2.shooting = true;
                                break;
                            case SDL_SCANCODE_KP_PLUS:
                                player2.useSpecial = true;
                                break;
                            default:;
                        }
                        break;
                    case SDL_KEYUP:
                        switch (event.key.keysym.scancode) {
                            case SDL_SCANCODE_Q:
                                compute->limitFramerate(true);
                                break;
                            case SDL_SCANCODE_A:
                                if (player1.velX == -1)
                                    player1.velX = 0;
                                break;
                            case SDL_SCANCODE_D:
                                if (player1.velX == 1)
                                    player1.velX = 0;
                                break;
                            case SDL_SCANCODE_W:
                                if (player1.velY == -1)
                                    player1.velY = 0;
                                break;
                            case SDL_SCANCODE_S:
                                if (player1.velY == 1)
                                    player1.velY = 0;
                                break;
                            case SDL_SCANCODE_SPACE:
                                player1.shooting = false;
                                break;
                            case SDL_SCANCODE_KP_4:
                                if (player2.velX == -1)
                                    player2.velX = 0;
                                break;
                            case SDL_SCANCODE_KP_6:
                                if (player2.velX == 1)
                                    player2.velX = 0;
                                break;
                            case SDL_SCANCODE_KP_8:
                                if (player2.velY == -1)
                                    player2.velY = 0;
                                break;
                            case SDL_SCANCODE_KP_5:
                                if (player2.velY == 1)
                                    player2.velY = 0;
                                break;
                            case SDL_SCANCODE_RSHIFT:
                            case SDL_SCANCODE_KP_0:
                                player2.shooting = false;
                                break;
                            default:;
                        }
                        break;
                    default:;
                }
            }
            std::this_thread::sleep_until(clock);
        }
        gameEnd();
        bool waitingForStart = notQuitting;
        while (waitingForStart) {
            clock += delay;
            while (SDL_PollEvent(&event)) {
                switch (event.type) {
                    case SDL_QUIT:
                        notQuitting = false;
                        waitingForStart = false;
                        break;
                    case SDL_KEYDOWN:
                        switch (event.key.keysym.scancode) {
                            case SDL_SCANCODE_F4:
                                notQuitting = false;
                                waitingForStart = false;
                                break;
                            case SDL_SCANCODE_RETURN:
                                waitingForStart = false;
                                break;
                            case SDL_SCANCODE_ESCAPE:
                                display->pause();
                                notQuitting = openMenu(Menu::GENERAL);
                                display->unpause();
                                break;
                            case SDL_SCANCODE_Z:
                                display->pause();
                                notQuitting = openMenu(Menu::EQUIPMENT);
                                display->unpause();
                                break;
                            case SDL_SCANCODE_X:
                                display->pause();
                                notQuitting = openMenu(Menu::RESEARCH);
                                display->unpause();
                                break;
                            case SDL_SCANCODE_C:
                                display->pause();
                                notQuitting = openMenu(Menu::DISMANTLE);
                                display->unpause();
                                break;
                            default:;
                        }
                        break;
                    default:;
                }
            }
            std::this_thread::sleep_until(clock);
        }
        display->pause();
    }
}

void Game::gameStart()
{
    alive = true;
    someone = true;
    first = true;
    level = std::max(1, maxLevel - 7);
    candyTypeProbScale = (level < 4) ? 1 : level * 0.07 + 0.72;
    difficultyCoef = pow((float) level, 1.7f);
    tic = 0;

    initPlayer(player1, 0);
    if (nbPlayer == 2)
        initPlayer(player2, 1);
    display->section2 = (nbPlayer == 2);

    display->unpause();
    compute->start((void (*)(void *, GPUEntityMgr &)) &updateS, (void (*)(void *, GPUEntityMgr &)) &updatePlayerS, this);
    // tracer->start();
}

void Game::gameEnd()
{
    if (level > maxLevel)
        maxLevel = level;
    // tracer->stop();
    compute->stop();
    save();
}

void Game::spawn(GPUEntityMgr &engine)
{
    if (!(tic & 15)) {
        for (int i = (level < 4 ? level : 2 + level / 2); i > 0; --i) {
            int rdm = percentDist(rdevice) / candyTypeProbScale;
            int j = 0;
            while (spawnability[j].second < rdm)
                rdm -= spawnability[j++].second;
            const float vel = 0.5 + normDist(rdevice)*(1.f + level / 6.f);
            const int localPos = candyPosDist(rdevice);
            switch (spawnability[j].first) {
                case LINE1:
                    engine.pushCandy(F_LINE) = core->getFragment(LINE1 + (localPos & 3), width + 32, localPos, -vel);
                    break;
                case MULTICOLOR:
                    engine.pushCandy(F_MULTICOLOR) = core->getFragment(MULTICOLOR, width + 32, localPos, -vel);
                    break;
                case BLOCK1:
                    engine.pushCandy(F_BLOCK) = core->getFragment(BLOCK1 + (localPos & 3), width + 32, localPos, -vel);
                    break;
                case FISH1:
                    engine.pushCandy(F_CANDY) = core->getFragment(FISH1 + (localPos & 3), width + 32, localPos, -vel, 2048);
                    break;
                case BOMB1:
                    engine.pushCandy(F_CANDY) = core->getFragment(BOMB1 + (localPos & 3), width + 32, localPos, -vel);
                    break;
                case CANDY1:
                    engine.pushCandy(F_CANDY) = core->getFragment(CANDY1 + (localPos & 3), width + 32, localPos, -vel);
                    break;
                default:;
            }
        }
    }
}

void Game::update(GPUEntityMgr &engine)
{
    someone = false;
    if (first) {
        if (player1.alive) {
            auto &tmp = core->getFragment(player1.vesselAspect, player1.x, player1.y, 0, 0);
            tmp.health = player1.lastHealth;
            compute->pushPlayer(0) = tmp;
        }
        if (player2.alive) {
            auto &tmp = core->getFragment(player2.vesselAspect, player2.x, player2.y, 0, 0);
            tmp.health = player2.lastHealth;
            compute->pushPlayer(1) = tmp;
        }
    }
    if (tic == 2000) { // 20s
        tic = 0;
        level += 1;
        difficultyCoef = pow((float) level, 1.7f);
        candyTypeProbScale = (level < 4) ? 1 : level * 0.07 + 0.72;
    }
    for (int i = nbPlayer; i-- > 0; spawn(engine));
    if (player1.alive) {
        updatePlayer(player1, 0);
        someone = true;
    } else
        revive(player1, player2, 0);
    if (player2.alive) {
        updatePlayer(player2, 1);
        someone = true;
    } else if (nbPlayer > 1)
        revive(player2, player1, 1);
    ++tic;
    display->score1Max = player1.saved.maxScore;
    display->score2Max = player2.saved.maxScore;
    display->score1 = player1.score;
    display->score2 = player2.score;
    display->level1 = level;
    display->level1Max = maxLevel;
}

void Game::revive(Player &target, Player &saver, int idx)
{
    if (tic == RESPAWN_TIME) {
        target.alive = saver.alive;
        target.x = saver.x;
        target.y = saver.y;
        target.lastHealth = saver.lastHealth;
        target.energy = (saver.energy >= target.energyMax) ? target.energyMax : saver.energy;
        target.shield = (saver.shield >= target.shieldMax) ? target.shieldMax : saver.shield;
        target.coolant = (saver.coolant >= target.coolantMax) ? target.coolantMax : saver.coolant;
        target.special = (saver.special >= target.specialMax) ? target.specialMax : saver.special;
        auto &tmp = core->getFragment(target.vesselAspect, target.x, target.y, 0, 0);
        tmp.health = target.lastHealth;
        compute->pushPlayer(idx, true) = tmp;
    }
}

void Game::updatePlayer(Player &p, int idx)
{
    if (p.energy < p.energyMax && p.coolant >= p.energyHeatCost) {
        p.energy += p.energyRate;
        p.coolant -= p.energyRate;
    }
    if ((p.velX | p.velY) && p.energy >= p.moveEnergyCost) {
        p.energy -= p.moveEnergyCost;
        p.coolant -= p.moveHeatCost;
        p.x += p.velX * p.moveSpeed;
        p.y += p.velY * p.moveSpeed;
        if (p.x < 24)
            p.x = 24;
        if (p.x > width - 24)
            p.x = width - 24;
        if (p.y < 20)
            p.y = 20;
        if (p.y > height - 20)
            p.y = height - 20;
    }
    if (p.coolant < p.coolantMax)
        p.coolant += p.coolantRate;
    if (p.shield < p.shieldMax && p.energy > (p.energyMax / 4) && p.coolant > (p.coolantMax / 8)) {
        p.shield += p.shieldRate;
        p.energy -= p.shieldEnergyCost;
        p.coolant -= p.shieldHeatCost;
    }
    if (p.boost) {
        if (p.special >= 1) {
            p.special -= 1;
            p.shield += 1;
            if (p.shield > p.shieldMax)
                p.shield = p.shieldMax;
        } else
            p.boost = false;
    }
    // update position
    p.posX = p.x * core->worldScaleX - 1;
    p.posY = p.y * core->worldScaleY - 1;

    if (!(tic % ((p.saved.weapon == 4) ? 2 : 10)) && p.shooting)
        shoot(p);
    if (p.useSpecial) {
        useSpecial(p);
        p.useSpecial = false;
    }
    // x -0.025 y -0.0333333 + .00416666666666 * [0:3]
    auto &tmp = compute->pushPlayer(idx);
    tmp.posX = p.posX;
    float y = tmp.posY = p.posY;
    tmp.health = p.lastHealth;
    p.ptr[0].x = p.ptr[2].x = p.ptr[4].x = p.ptr[6].x = p.ptr[8].x = p.ptr[10].x = p.ptr[12].x = p.ptr[14].x = p.ptr[16].x = p.ptr[18].x = p.posX - 0.05;
    y -= 0.0333333333333333 * 3;
    p.ptr[0].y = p.ptr[3].y = p.ptr[4].y = p.ptr[7].y = y;
    y += 0.0041666666666667 * 2.1;
    p.ptr[5].y = p.ptr[6].y = p.ptr[8].y = p.ptr[11].y = y;
    y += 0.0041666666666667 * 2.1;
    p.ptr[9].y = p.ptr[10].y = p.ptr[12].y = p.ptr[15].y = y;
    y += 0.0041666666666667 * 2.1;
    p.ptr[13].y = p.ptr[14].y = p.ptr[16].y = p.ptr[19].y = y;
    y += 0.0041666666666667 * 2.1;
    p.ptr[17].y = p.ptr[18].y = p.ptr[1].y = p.ptr[2].y = y;
    p.ptr[1].x = p.ptr[3].x = p.posX - 0.05 + 0.1;
    p.ptr[5].x = p.ptr[7].x = p.posX - 0.05 + ((p.energy < p.energyMax) ? p.energy / p.energyMax * 0.1 : 0.1);
    p.ptr[9].x = p.ptr[11].x = p.posX - 0.05 + ((p.shield > 0) ? p.shield / p.shieldMax * 0.1 : 0);
    p.ptr[13].x = p.ptr[15].x = p.posX - 0.05 + ((p.coolant < p.coolantMax) ? ((p.coolant > 0) ? p.coolant / p.coolantMax * 0.1 : 0) : 0.1);
    p.ptr[17].x = p.ptr[19].x = p.posX - 0.05 + ((p.special < p.specialMax) ? p.special / p.specialMax * 0.1 : 0.1);
}

void Game::shoot(Player &p)
{
    auto &w = weaponList[p.saved.weapon];

    const int cons = w.baseEnergyConsumption << p.saved.weaponLevel;
    if (p.energy < cons)
        return;
    p.energy -= cons;
    // play shoot sound
    switch (p.saved.weapon) {
        case 0: // laser
            {
                auto &e = core->getFragment(LASER);
                const float dmg = e.damage;
                switch (p.saved.weaponLevel) { // level go from 0 to 8 both included
                    case 0:
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                        e.damage = -1 - p.saved.weaponLevel;
                        compute->pushPlayerShoot() = core->getFragment(LASER, p.x + 24, p.y, 7);
                        break;
                    case 5:
                    case 6:
                    case 7:
                        e.damage = -p.saved.weaponLevel;
                        compute->pushPlayerShoot() = core->getFragment(LASER, p.x + 24, p.y - 16, 7);
                        compute->pushPlayerShoot() = core->getFragment(LASER, p.x + 24, p.y + 16, 7);
                        --e.damage;
                        compute->pushPlayerShoot() = core->getFragment(LASER, p.x + 28, p.y, 7);
                        break;
                    case 8:
                        e.damage = -6;
                        compute->pushPlayerShoot() = core->getFragment(LASER, p.x + 24, p.y - 32, 7);
                        compute->pushPlayerShoot() = core->getFragment(LASER, p.x + 24, p.y + 32, 7);
                        e.damage = -7;
                        compute->pushPlayerShoot() = core->getFragment(LASER, p.x + 28, p.y - 16, 7);
                        compute->pushPlayerShoot() = core->getFragment(LASER, p.x + 28, p.y + 16, 7);
                        e.damage = -8;
                        compute->pushPlayerShoot() = core->getFragment(LASER, p.x + 32, p.y, 7);
                        break;
                }
                e.damage = dmg;
            }
            break;
        case 1: // blaster
        {
            const int x = p.x + 24;
            int y = p.y - 6 * p.saved.weaponLevel;
            for (int i = p.saved.weaponLevel; i > 0; --i) {
                compute->pushPlayerShoot() = core->getFragment(BLASTER, x, y, 10);
                y += 12;
            }
        }
            break;
        case 2: // wave
            switch (p.saved.weaponLevel) {
                case 0:
                case 1:
                case 2:
                    compute->pushPlayerShoot() = core->getFragment(WAVE + p.saved.weaponLevel, p.x + 24, p.y, 7);
                    break;
                case 3:
                case 4:
                    compute->pushPlayerShoot() = core->getFragment(WAVE + p.saved.weaponLevel - 3, p.x + 20, p.y - 26, 6, -1);
                    compute->pushPlayerShoot() = core->getFragment(BIG_WAVE, p.x + 28, p.y, 7);
                    compute->pushPlayerShoot() = core->getFragment(WAVE + p.saved.weaponLevel - 3, p.x + 20, p.y + 26, 6, 1);
                    break;
                case 5:
                    compute->pushPlayerShoot() = core->getFragment(BIG_WAVE, p.x + 20, p.y - 34, 6, -1);
                    compute->pushPlayerShoot() = core->getFragment(BIG_WAVE, p.x + 28, p.y, 7);
                    compute->pushPlayerShoot() = core->getFragment(BIG_WAVE, p.x + 20, p.y + 34, 6, 1);
                    break;
                case 6:
                case 7:
                    compute->pushPlayerShoot() = core->getFragment(WAVE + p.saved.weaponLevel - 6, p.x + 16, p.y - 60, 6, -2);
                    compute->pushPlayerShoot() = core->getFragment(BIG_WAVE, p.x + 24, p.y - 34, 7, -1);
                    compute->pushPlayerShoot() = core->getFragment(BIG_WAVE, p.x + 32, p.y, 8);
                    compute->pushPlayerShoot() = core->getFragment(BIG_WAVE, p.x + 24, p.y + 34, 7, 1);
                    compute->pushPlayerShoot() = core->getFragment(WAVE + p.saved.weaponLevel - 6, p.x + 16, p.y + 60, 6, 2);
                    break;
                case 8:
                    compute->pushPlayerShoot() = core->getFragment(BIG_WAVE, p.x + 16, p.y - 68, 6, -2);
                    compute->pushPlayerShoot() = core->getFragment(BIG_WAVE, p.x + 24, p.y - 34, 7, -1);
                    compute->pushPlayerShoot() = core->getFragment(BIG_WAVE, p.x + 32, p.y, 8);
                    compute->pushPlayerShoot() = core->getFragment(BIG_WAVE, p.x + 24, p.y + 34, 7, 1);
                    compute->pushPlayerShoot() = core->getFragment(BIG_WAVE, p.x + 20, p.y + 68, 6, 2);
                default:;
            }
            break;
        case 3: // eclatant
            switch (p.saved.weaponLevel) {
                case 0:
                case 1:
                case 2:
                case 3:
                case 4:
                    compute->pushPlayerShoot() = core->getFragment(ECLATANT, p.x + 24, p.y, 7);
                    break;
                case 5:
                case 6:
                case 7:
                    compute->pushPlayerShoot() = core->getFragment(ECLATANT, p.x + 24, p.y - 16, 7);
                    compute->pushPlayerShoot() = core->getFragment(ECLATANT, p.x + 24, p.y + 16, 7);
                    compute->pushPlayerShoot() = core->getFragment(ECLATANT, p.x + 28, p.y, 7);
                    break;
                case 8:
                    compute->pushPlayerShoot() = core->getFragment(ECLATANT, p.x + 24, p.y - 32, 7);
                    compute->pushPlayerShoot() = core->getFragment(ECLATANT, p.x + 24, p.y + 32, 7);
                    compute->pushPlayerShoot() = core->getFragment(ECLATANT, p.x + 28, p.y - 16, 7);
                    compute->pushPlayerShoot() = core->getFragment(ECLATANT, p.x + 28, p.y + 16, 7);
                    compute->pushPlayerShoot() = core->getFragment(ECLATANT, p.x + 32, p.y, 7);
                    break;
                default:;
            }
            break;
        case 4: // laserifier
            {
                auto &e = core->getFragment(LASERIFIER);
                switch (p.saved.weaponLevel) {
                    case 0:
                        e.damage = 0;
                        e.health = 0;
                        compute->pushPlayerShoot() = core->getFragment(LASERIFIER, p.x + 24, p.y, 20);
                        break;
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                        e.damage = -1;
                        e.health = (1 << (p.saved.weaponLevel - 1)) - 1;
                        compute->pushPlayerShoot() = core->getFragment(LASERIFIER, p.x + 24, p.y, 20);
                        break;
                    case 5:
                    case 6:
                        e.damage = -1;
                        e.health = (1 << (p.saved.weaponLevel - 3)) - 1;
                        compute->pushPlayerShoot() = core->getFragment(LASERIFIER, p.x + 20, p.y - 25, 20);
                        compute->pushPlayerShoot() = core->getFragment(LASERIFIER, p.x + 20, p.y + 25, 20);
                        e.health = (1 << (p.saved.weaponLevel - 2)) - 1;
                        compute->pushPlayerShoot() = core->getFragment(LASERIFIER, p.x + 24, p.y, 20);
                        break;
                    case 7:
                        e.damage = -2;
                        e.health = 7;
                        compute->pushPlayerShoot() = core->getFragment(LASERIFIER, p.x + 20, p.y - 25, 20);
                        compute->pushPlayerShoot() = core->getFragment(LASERIFIER, p.x + 20, p.y + 25, 20);
                        e.damage = -3;
                        e.health = 11;
                        compute->pushPlayerShoot() = core->getFragment(LASERIFIER, p.x + 24, p.y, 20);
                        break;
                    case 8:
                        e.damage = -3;
                        e.health = 5;
                        compute->pushPlayerShoot() = core->getFragment(LASERIFIER, p.x + 12, p.y - 50, 20);
                        compute->pushPlayerShoot() = core->getFragment(LASERIFIER, p.x + 12, p.y + 50, 20);
                        e.damage = -4;
                        e.health = 7;
                        compute->pushPlayerShoot() = core->getFragment(LASERIFIER, p.x + 20, p.y - 25, 20);
                        compute->pushPlayerShoot() = core->getFragment(LASERIFIER, p.x + 20, p.y + 25, 20);
                        e.damage = -6;
                        e.health = 11;
                        compute->pushPlayerShoot() = core->getFragment(LASERIFIER, p.x + 24, p.y, 20);
                        break;
                    default:;
                }
            }
            break;
    }
}

void Game::useSpecial(Player &p)
{
    auto &w = specialList[p.saved.special];

    if (p.special < w.specialCost)
        return;
    p.special -= w.specialCost;
    switch (p.saved.special) {
        case SW_BIG_LASER:
            compute->pushPlayerShoot() = core->dropFragment(BIG_LASER, p.posX, p.posY, 15);
            break;
        case SW_PROTECTO:
            compute->pushPlayerShoot() = core->dropFragment(PROTECTO, p.posX, p.posY, -1, -1);
            compute->pushPlayerShoot() = core->dropFragment(PROTECTO, p.posX, p.posY, 0, -1);
            compute->pushPlayerShoot() = core->dropFragment(PROTECTO, p.posX, p.posY, 1, -1);
            compute->pushPlayerShoot() = core->dropFragment(PROTECTO, p.posX, p.posY, 1, 0);
            compute->pushPlayerShoot() = core->dropFragment(PROTECTO, p.posX, p.posY, 1, 1);
            compute->pushPlayerShoot() = core->dropFragment(PROTECTO, p.posX, p.posY, 0, 1);
            compute->pushPlayerShoot() = core->dropFragment(PROTECTO, p.posX, p.posY, -1, 1);
            compute->pushPlayerShoot() = core->dropFragment(PROTECTO, p.posX, p.posY, -1, 0);
            break;
        case SW_MINI_TRANS:
            compute->pushPlayerShoot() = core->dropFragment(MINI_TRANS, p.posX, p.posY, 15);
            break;
        case SW_TRANS:
            compute->pushPlayerShoot() = core->dropFragment(TRANS, p.posX, p.posY, 15);
            break;
        case SW_SUPER_TRANS:
            compute->pushPlayerShoot() = core->dropFragment(SUPER_TRANS, p.posX, p.posY, 15);
            break;
        case SW_SHIELD_BOOSTER:
        case SW_ENHANCED_SHIELD_BOOSTER:
            p.boost = true;
            // if (p.special > 5)
                // play shield boost sound
            break;
        default:
            break;
    }
}

void Game::updatePlayer(GPUEntityMgr &engine)
{
    if (first) {
        first = false;
    } else {
        updatePlayerState(player1, 0);
        updatePlayerState(player2, 1);
    }
    for (int i = engine.nbDead; i-- > 0;) {
        switch (engine.deadFlags[i].second) {
            case F_OTHER:
                continue;
            case F_PIECE_1:
            {
                auto p = getClosest(engine.deadFlags[i].first);
                if (!p)
                    continue;
                // std::cout << "Piece 1\n";
                p->score += 1 * difficultyCoef;
                BONUS_EFFECT;
            }
            case F_PIECE_5:
            {
                auto p = getClosest(engine.deadFlags[i].first);
                if (!p)
                    continue;
                // std::cout << "Piece 5\n";
                p->score += 5 * difficultyCoef;
                BONUS_EFFECT;
            }
            case F_PIECE_10:
            {
                auto p = getClosest(engine.deadFlags[i].first);
                if (!p)
                    continue;
                // std::cout << "Piece 10\n";
                p->score += 10 * difficultyCoef;
                BONUS_EFFECT;
            }
            case F_PIECE_25:
            {
                auto p = getClosest(engine.deadFlags[i].first);
                if (!p)
                    continue;
                // std::cout << "Piece 25\n";
                p->score += 25 * difficultyCoef;
                BONUS_EFFECT;
            }
            case F_SHIELD_BOOST:
            {
                auto p = getClosest(engine.deadFlags[i].first);
                if (!p)
                    continue;
                // std::cout << "Shield boost\n";
                if (p->shield >= p->shieldMax) {
                    p->score += 100 * difficultyCoef;
                } else {
                    p->shield = 2 + p->shieldMax / 2;
                    p->energy = p->energyMax;
                }
                BONUS_EFFECT;
            }
            case F_SPECIAL_BOOST:
            {
                auto p = getClosest(engine.deadFlags[i].first);
                if (!p)
                    continue;
                // std::cout << "Special boost\n";
                p->shield += p->shieldMax / 6;
                p->energy = p->energyMax;
                p->special = p->specialMax;
                BONUS_EFFECT;
            }
            case F_COOLANT_BOOST:
            {
                auto p = getClosest(engine.deadFlags[i].first);
                if (!p)
                    continue;
                // std::cout << "Coolant boost\n";
            p->shield += p->shieldMax / 6;
                p->energy = p->energyMax;
                p->coolant = p->coolantRate * 500;
                BONUS_EFFECT;
            }
            case F_PLAYER:
                continue; // Why this flag ?
            case F_ECLATANT:
            {
                EntityState e = engine.readEntity(engine.deadFlags[i].first);
                engine.pushPlayerShoot() = core->dropFragment(ECLAT0, e.deadX, e.deadY, 1.5, 1.5);
                engine.pushPlayerShoot() = core->dropFragment(ECLAT1, e.deadX, e.deadY, -1.5, 1.5);
                engine.pushPlayerShoot() = core->dropFragment(ECLAT2, e.deadX, e.deadY, -1.5, -1.5);
                engine.pushPlayerShoot() = core->dropFragment(ECLAT3, e.deadX, e.deadY, 1.5, -1.5);
                continue;
            }
            case F_MULTICOLOR:
            {
                EntityState e = engine.readEntity(engine.deadFlags[i].first);
                for (int i = 0; i < 16; ++i) {
                    engine.pushCandyShoot() = core->dropFragment(LASER, e.deadX, e.deadY, circle[i], circle[(i - 4) & 15]);
                }
                continue;
            }
            case F_LINE:
            {
                EntityState e = engine.readEntity(engine.deadFlags[i].first);
                engine.pushCandyShoot() = core->dropFragment(BIG_LASER, e.deadX, e.deadY, -15);
                engine.pushCandyShoot() = core->dropFragment(BIG_LASER, e.deadX, e.deadY, 15);
                continue;
            }
            case F_BLOCK:
            {
                EntityState e = engine.readEntity(engine.deadFlags[i].first);
                for (int i = 0; i < 16; ++i) {
                    if (!(i & 1)) {
                        engine.pushBonus() = core->dropFragment(BONUS3, e.deadX, e.deadY, circle[i] / 28 - 2, circle[(i - 4) & 15] / 28);
                    }
                    engine.pushBonus() = core->dropFragment(LASER2, e.deadX, e.deadY, circle[i], circle[(i - 4) & 15]);
                }
                engine.pushBonus() = core->dropFragment(BONUS2, e.deadX, e.deadY, -2);
                engine.pushBonus() = core->dropFragment(BIG_LASER2, e.deadX, e.deadY, -18);
                engine.pushBonus() = core->dropFragment(BIG_LASER2, e.deadX, e.deadY, 18);
                continue;
            }
            case F_CANDY:
            {
                int r = bonusDist(rdevice);
                if (r < 650)
                    continue;
                EntityState e = engine.readEntity(engine.deadFlags[i].first);
                if (r < 675)
                    engine.pushBonus(F_PIECE_1) = core->dropFragment(BONUS0, e.deadX, e.deadY, -2);
                else if (r < 690)
                    engine.pushBonus(F_PIECE_5) = core->dropFragment(BONUS1, e.deadX, e.deadY, -2);
                else if (r < 695)
                    engine.pushBonus(F_PIECE_10) = core->dropFragment(BONUS2, e.deadX, e.deadY, -2);
                else if (r < 698)
                    engine.pushBonus(F_PIECE_25) = core->dropFragment(BONUS3, e.deadX, e.deadY, -2);
                else if (r < 699)
                    engine.pushBonus(F_SHIELD_BOOST) = core->dropFragment(BONUS4, e.deadX, e.deadY, -2);
                else if (r < 700)
                    engine.pushBonus(F_SPECIAL_BOOST) = core->dropFragment(BONUS5, e.deadX, e.deadY, -2);
                else
                    engine.pushBonus(F_COOLANT_BOOST) = core->dropFragment(BONUS6, e.deadX, e.deadY, -2);
                continue;
            }
            default: // MIEL1-5 useless while there is no level loaded
                continue;
        }
    }
}

void Game::updatePlayerState(Player &p, int i)
{
    if (!p.alive)
        return;
    const int newShield = compute->readPlayer(i).health;
    if (newShield != p.lastHealth) {
        if (newShield < 0) {
            if (tic != RESPAWN_TIME + 1) {
                p.alive = false;
                p.score -= p.score / generatorList[p.saved.generator].deathLossRatio;
                if (p.score < p.saved.maxScore) {
                    p.score = p.saved.maxScore;
                } else {
                    p.saved.maxScore = p.score;
                }
                p.x = -100;
                compute->pushPlayer(i).posX = -100;
                tic = 0;
                // play death sound
                return;
            }
        } else {
            // play shield sound
            p.shield += newShield - p.lastHealth;
        }
    }
    if (p.shield < p.shieldMax / 5 && p.highPOverclocker) {
        float loc_recover = std::min({p.shieldMax/5.f - p.shield, p.special/1.5f, p.coolant/5000.f});
        p.coolant -= 5000*loc_recover;
        p.special -= 1.5*loc_recover;
        p.shield += loc_recover;
    }
    compute->pushPlayer(i).health = p.lastHealth = (int) (p.shield + 1);
}

Player *Game::getClosest(short idx)
{
    auto &readback = compute->readEntity(idx);
    const float x1 = readback.deadX;
    const float y1 = readback.deadY;
    if (player1.alive) {
        const float x2 = (x1 - player1.posX) / ((24 + 16 + player1.moveSpeed) * core->worldScaleX);
        const float y2 = (y1 - player1.posY) / ((20 + 8 + player1.moveSpeed) * core->worldScaleY);
        if (x2 * x2 + y2 * y2 <= 1) {
            if (player2.alive) {
                const float _x2 = (x1 - player2.posX) / ((24 + 16 + player2.moveSpeed) * core->worldScaleX);
                const float _y2 = (y1 - player2.posY) / ((20 + 8 + player2.moveSpeed) * core->worldScaleY);
                if (_x2 * _x2 + _y2 * _y2 < x2 * x2 + y2 * y2)
                    return &player2;
            }
            return &player1;
        }
    }
    if (player2.alive) {
        const float x2 = (x1 - player2.posX) / ((24 + 16 + player2.moveSpeed) * core->worldScaleX);
        const float y2 = (y1 - player2.posY) / ((20 + 8 + player2.moveSpeed) * core->worldScaleY);
        if (x2 * x2 + y2 * y2 <= 1)
            return &player2;
    }
    return nullptr;
}

void Game::load(int slot, int playerCount)
{
    usedSlot = slot;

    std::ifstream file(std::string("saves/slot") + std::to_string(slot) + ".dat", std::ifstream::binary);
    if (file) {
        file.read((char *) &recursion, 9);
        file.read((char *) &player1, SIZEOF_SAVED_DATA);
        player1.score = player1.saved.maxScore;
        if (nbPlayer == 2) {
            file.read((char *) &player2, SIZEOF_SAVED_DATA);
            player2.score = player2.saved.maxScore;
        }
    } else {
        std::cout << "Slot not found, creating one\n";
        recursion = 0;
        level = 1;
        maxLevel = level;
        nbPlayer = playerCount;
        for (int i = 0; i < nbPlayer; ++i) {
            SavedDatas &s = (i == 0) ? player1.saved : player2.saved;
            s = SavedDatas({0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0});
        }
    }
    recursionGainFactor = (nbPlayer == 1) ? 1 : 0.55;
}

void Game::save()
{
    std::ofstream file(std::string("saves/slot") + std::to_string(usedSlot) + ".dat", std::ofstream::binary);

    if (file) {
        file.write((char *) &recursion, 9);
        file.write((char *) &player1, SIZEOF_SAVED_DATA);
        if (nbPlayer == 2)
            file.write((char *) &player2, SIZEOF_SAVED_DATA);
    } else {
        std::cout << "Failed to create save\n";
    }
}

void Game::initPlayer(Player &p, int idx)
{
    p.x = 48;
    p.y = height * (idx + 1) / (nbPlayer + 1);
    auto &v = vesselList[p.saved.vessel];
    auto &g = generatorList[p.saved.generator];
    auto &r = recoolerList[p.saved.recooler];
    auto &s = shieldList[p.saved.shield];
    p.moveSpeed = v.speed;
    p.moveEnergyCost = v.energyConsumption;
    p.moveHeatCost = v.energyConsumption * 2;
    p.energyMax = g.energyCapacity;
    p.energy = p.energyMax / 4;
    p.energyRate = g.energyRate;
    p.energyHeatCost = g.energyRate;
    p.shield = level * 2.4;
    p.shieldRate = s.shieldRate;
    p.shieldEnergyCost = s.energyConsumption;
    p.shieldHeatCost = s.heat;
    p.shieldMax = s.shieldCapacity;
    if (p.shield > p.shieldMax)
        p.shield = p.shieldMax;
    p.coolantMax = r.recoolingCapacity;
    p.coolant = p.coolantMax / 8;
    p.coolantRate = r.recoolingRate;
    p.uniconvert = (p.saved.special == 9);
    p.special = 50;
    p.specialMax = (p.uniconvert) ? 160 : 100;
    p.velX = 0;
    p.velY = 0;
    p.alive = true;
    p.lastHealth = p.shield + 1;
    p.highPOverclocker = p.uniconvert | (p.saved.special == 8);
    p.shooting = false;
    p.useSpecial = false;
    p.boost = false;
    p.vesselAspect = vesselList[p.saved.vessel].aspect;
}

bool Game::openMenu(int type)
{
    Menu menu(this, core, player1, player2, maxLevel, recursion, nbPlayer);
    bool ret = menu.mainloop(type);
    save();
    return ret;
}

int64_t Game::getRecursionGain() const
{
    switch (nbPlayer) {
        case 1:
            return (vesselList[player1.saved.vessel].cost + weaponList[player1.saved.weapon].cost * (1 << player1.saved.weaponLevel) + specialList[player1.saved.special].cost + shieldList[player1.saved.shield].cost + generatorList[player1.saved.generator].cost + recoolerList[player1.saved.recooler].cost
            - std::min(0, (int) ((startScore + recursion / recursionBaseScoreRatio) - player1.saved.maxScore))) * recursionGainFactor;
        case 2:
            return (vesselList[player1.saved.vessel].cost + weaponList[player1.saved.weapon].cost * (1 << player1.saved.weaponLevel) + specialList[player1.saved.special].cost + shieldList[player1.saved.shield].cost + generatorList[player1.saved.generator].cost + recoolerList[player1.saved.recooler].cost
            + vesselList[player2.saved.vessel].cost + weaponList[player2.saved.weapon].cost * (1 << player2.saved.weaponLevel) + specialList[player2.saved.special].cost + shieldList[player2.saved.shield].cost + generatorList[player2.saved.generator].cost + recoolerList[player2.saved.recooler].cost
             - std::min(0, (int) ((startScore + recursion / recursionBaseScoreRatio) * 2 - player1.saved.maxScore + player2.saved.maxScore))) * recursionGainFactor;
        default:
            return 0;
    }
}

int64_t Game::getMaxedRecursionGain() const
{
    switch (nbPlayer) {
        case 1:
            return (vesselList[player1.saved.vesselUnlock].cost + weaponList[player1.saved.weaponUnlock].cost * (1 << player1.saved.weaponLevelUnlock) + specialList[player1.saved.specialUnlock].cost + shieldList[player1.saved.shieldUnlock].cost + generatorList[player1.saved.generatorUnlock].cost + recoolerList[player1.saved.recoolerUnlock].cost) * recursionGainFactor;
        case 2:
            return (vesselList[player1.saved.vesselUnlock].cost + weaponList[player1.saved.weaponUnlock].cost * (1 << player1.saved.weaponLevelUnlock) + specialList[player1.saved.specialUnlock].cost + shieldList[player1.saved.shieldUnlock].cost + generatorList[player1.saved.generatorUnlock].cost + recoolerList[player1.saved.recoolerUnlock].cost
            + vesselList[player2.saved.vesselUnlock].cost + weaponList[player2.saved.weaponUnlock].cost * (1 << player2.saved.weaponLevelUnlock) + specialList[player2.saved.specialUnlock].cost + shieldList[player2.saved.shieldUnlock].cost + generatorList[player2.saved.generatorUnlock].cost + recoolerList[player2.saved.recoolerUnlock].cost) * recursionGainFactor;
        default:
            return 0;
    }
}

void Game::makeRecursion()
{
    recursion += getRecursionGain();
    switch (nbPlayer) {
        case 2:
            player2.score = recursion * recursionBaseScoreRatio;
            player2.saved.maxScore = player2.score;
            player2.saved.vessel = 0;
            player2.saved.weapon = 0;
            player2.saved.weaponLevel = 0;
            player2.saved.special = 0;
            player2.saved.generator = 1;
            player2.saved.shield = 0;
            player2.saved.recooler = 0;
            initPlayer(player2, 1);
            [[fallthrough]];
        case 1:
            player1.score = recursion * recursionBaseScoreRatio;
            player1.saved.maxScore = player1.score;
            player1.saved.vessel = 0;
            player1.saved.weapon = 0;
            player1.saved.weaponLevel = 0;
            player1.saved.special = 0;
            player1.saved.generator = 1;
            player1.saved.shield = 0;
            player1.saved.recooler = 0;
            initPlayer(player1, 0);
            [[fallthrough]];
        default:
            maxLevel = 1;
            level = maxLevel;
            tic = 0;
    }
}

int64_t Game::getScoreAfterRecursion() const
{
    return (recursion + getRecursionGain()) * recursionBaseScoreRatio;
}
