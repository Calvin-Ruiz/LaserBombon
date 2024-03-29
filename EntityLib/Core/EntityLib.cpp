/*
** EPITECH PROJECT, 2020
** Vulkan-Engine
** File description:
** EntityLib.cpp
*/
#include "EntityLib.hpp"
#include "GPUEntityMgr.hpp"
#include "EntityCore/Core/VulkanMgr.hpp"
#include "EntityCore/Core/BufferMgr.hpp"
#include "EntityCore/Core/RenderMgr.hpp"
#include "EntityCore/Core/FrameMgr.hpp"
#include "EntityCore/Resource/Texture.hpp"

EntityLib::EntityLib(const char *AppName, uint32_t appVersion, int width, int height, bool enableDebugLayers, bool drawLogs, bool saveLogs) : worldScaleX(2.f / width), worldScaleY(2.f / height)
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK);
    width *= WIN_SIZE_SCALING;
    height *= WIN_SIZE_SCALING;
    window = SDL_CreateWindow(AppName, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_VULKAN|SDL_WINDOW_SHOWN);
    VkPhysicalDeviceFeatures features{};
    features.samplerAnisotropy = VK_TRUE;
    features.sampleRateShading = VK_TRUE;
    features.vertexPipelineStoresAndAtomics = VK_TRUE;
    master = std::make_unique<VulkanMgr>(AppName, appVersion, window, width, height, QueueRequirement{2, 1, 1, 0, 0}, features, VkPhysicalDeviceFeatures{}, 64, enableDebugLayers, drawLogs, saveLogs, "cache/", 3);
    localBuffer = std::make_unique<BufferMgr>(*master, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 0, 136*1024);
    localBuffer->setName("Dynamic staging buffer");
    Texture::setTextureDir("./textures/");
    entityMap = std::make_unique<Texture>(*master, *localBuffer, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, "entityMap.png");
    entityMap->init();
    entityMap->getDimensions(mapWidth, mapHeight);
    mapScaleX = 1.f / mapWidth;
    mapScaleY = 1.f / mapHeight;
    renderMgr = std::make_unique<RenderMgr>(*master);
    if (WINDOWLESS) {
        renderMgr->bindColor(renderMgr->attach(VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    } else {
        renderMgr->bindColor(renderMgr->attach(VK_FORMAT_B8G8R8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    }
    renderMgr->addDependency(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, VK_DEPENDENCY_BY_REGION_BIT);
    renderMgr->pushLayer();
    renderMgr->addSelfDependency(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT);
    renderMgr->addDependency(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, VK_DEPENDENCY_BY_REGION_BIT);
    renderMgr->build();
    graphicQueueFamily = master->acquireQueue(graphicQueue, VulkanMgr::QueueType::GRAPHIC, "GPUDisplay/MenuMgr");
    if (graphicQueueFamily == nullptr) {
        // This is an impossible scenario
        master->putLog("Missing graphic queue (unpossible case)", LogType::ERROR);
    }

    if (WINDOWLESS) {
        // Create color attachment
        for (int i = 0; i < 3; ++i) {
            frameDraw.push_back(std::make_unique<Texture>(*master, width, height, VK_SAMPLE_COUNT_1_BIT, "Color attachment " + std::to_string(i + 1), VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT));
            frameDraw.back()->use();
        }
    }

    auto tmp = std::make_unique<FrameMgr>(*master, *renderMgr, frames.size(), width, height, std::to_string(frames.size()));
    if (WINDOWLESS) {
        tmp->bind(0, *frameDraw[frames.size()]);
    } else {
        tmp->bind(0, master->getSwapchainView()[frames.size()]);
    }
    tmp->build(graphicQueueFamily->id, true);
    frames.push_back(std::move(tmp));

    tmp = std::make_unique<FrameMgr>(*master, *renderMgr, frames.size(), width, height, std::to_string(frames.size()));
    if (WINDOWLESS) {
        tmp->bind(0, *frameDraw[frames.size()]);
    } else {
        tmp->bind(0, master->getSwapchainView()[frames.size()]);
    }
    tmp->build(graphicQueueFamily->id, true);
    frames.push_back(std::move(tmp));

    tmp = std::make_unique<FrameMgr>(*master, *renderMgr, frames.size(), width, height, std::to_string(frames.size()));
    if (WINDOWLESS) {
        tmp->bind(0, *frameDraw[frames.size()]);
    } else {
        tmp->bind(0, master->getSwapchainView()[frames.size()]);
    }
    tmp->build(graphicQueueFamily->id, true);
    frames.push_back(std::move(tmp));
}

EntityLib::~EntityLib()
{
    localBuffer.reset();
    entityMap.reset();
    renderMgr.reset();
    frameDraw.clear();
    frames.clear();
    master.reset();
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void EntityLib::loadFragment(int idx, int texX1, int texY1, int texX2, int texY2, int shield, int damage, int width, int height, unsigned char flag)
{
    if (idx >= fragments.size()) {
        fragments.resize(idx + 1);
        flags.resize(idx + 1);
    }
    auto &tmp = fragments[idx];
    if (!width)
        width = texX2 - texX1;
    if (!height)
        height = texY2 - texY1;
    tmp.health = shield - 1;
    tmp.damage = -damage;
    tmp.sizeX = width * worldScaleX / 2;
    tmp.sizeY = height * worldScaleY / 2;
    tmp.texX1 = (texX1 + 0.5f) * mapScaleX + 0.00001f;
    tmp.texY1 = (texY1 + 0.5f) * mapScaleY + 0.00001f;
    tmp.texX2 = (texX2 - 0.5f) * mapScaleX - 0.00002f;
    tmp.texY2 = (texY2 - 0.5f) * mapScaleY - 0.00002f;
    tmp.aliveNow = VK_FALSE;
    tmp.newlyInserted = VK_TRUE;
    flags[idx] = flag;
}

EntityData &EntityLib::getFragment(int idx)
{
    return fragments[idx];
}

EntityData &EntityLib::getFragment(int idx, int x, int y, int velX, int velY)
{
    auto &tmp = fragments[idx];

    tmp.posX = x * worldScaleX - 1;
    tmp.posY = y * worldScaleY - 1;
    tmp.velX = velX * worldScaleX;
    tmp.velY = velY * worldScaleY;
    return tmp;
}

EntityData &EntityLib::dropFragment(int idx, float x, float y, float velX, float velY)
{
    auto &tmp = fragments[idx];

    tmp.posX = x;
    tmp.posY = y;
    tmp.velX = velX * worldScaleX;
    tmp.velY = velY * worldScaleY;
    return tmp;
}

void EntityLib::setFragmentPos(EntityData &entity, int x, int y)
{
    entity.posX = x * worldScaleX - 1;
    entity.posY = y * worldScaleY - 1;
}

std::string EntityLib::toText(int64_t nbr)
{
    std::string str;

    if (nbr < 0) {
        nbr = -nbr;
        str = "-";
    }
    if (nbr < 10000) {
        return str + std::to_string(nbr);
    }
    const unsigned char ranks[] = {' ', 'k', 'M', 'G', 'T'};
    unsigned char rank = 0;
    while (nbr >= 100000) {
        ++rank;
        nbr /= 1000;
    }
    unsigned char cut = 0;
    while (nbr >= 1000) {
        ++cut;
        nbr /= 10;
    }
    std::string tmp = std::to_string(nbr);
    switch (cut) {
        case 0:
            str += tmp;
            str.push_back(ranks[rank]);
            break;
        case 1:
            str.push_back(tmp[0]);
            str.push_back('.');
            str.push_back(tmp[1]);
            str.push_back(tmp[2]);
            str.push_back(ranks[rank + 1]);
            break;
        case 2:
            str.push_back(tmp[0]);
            str.push_back(tmp[1]);
            str.push_back('.');
            str.push_back(tmp[2]);
            str.push_back(ranks[rank + 1]);
            break;
    }
    return str;
}
