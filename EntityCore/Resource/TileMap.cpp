#include "EntityCore/Core/VulkanMgr.hpp"
#include "EntityCore/Core/BufferMgr.hpp"
#include "EntityCore/Core/MemoryManager.hpp"
#include "TileMap.hpp"
#include <cstring>

TileMap::TileMap(VulkanMgr &master, BufferMgr &mgr, const std::string &name, VkImageUsageFlags usage, VkFormat format) :
    Texture(master, mgr, usage, name, format)
{
    switch (format) {
        case VK_FORMAT_R8G8B8A8_UNORM:
        case VK_FORMAT_B8G8R8A8_UNORM:
            break;
        default:
            assert(false); // Other format are not currently supported
    }
}

TileMap::~TileMap()
{
    if (map)
        delete (map - 1);
}

bool TileMap::createMap(uint32_t width, uint32_t height)
{
    if (map)
        delete (map - 1);
    info.extent.width = width;
    info.extent.height = height;
    createSurface();
    createImage();
    ptr = (int *) acquireStagingMemoryPtr();
    width += CHUNK_SIZE - 1;
    width /= CHUNK_SIZE;
    height += CHUNK_SIZE - 1;
    height /= CHUNK_SIZE;
    lineSize = width + 1;
    map = new uint16_t[lineSize * height + 1];
    map[0] = 0;
    for (uint32_t i = 0; i++ < height;) {
        map[i * lineSize] = 0;
        propagate(map + i * lineSize);
    }
    end = (++map) + lineSize * height;
    loaded = false;
    return true;
}

void TileMap::writeSurface(SubTexture &surface, const void *data)
{
    writeAt(surface.x, surface.y, surface.width, surface.height, (const int *) data);
}

void TileMap::writeInSurface(SubTexture &surface, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const void *data)
{
    writeAt(surface.x + x, surface.y + y, width, height, (const int *) data);
}

void TileMap::writeAt(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const int *data)
{
    VkDeviceSize offset = x + info.extent.width * y;
    writes.push_back({offset * 4, info.extent.width, 0, {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1}, {x, y, 0}, {width, height, 1}});
    if ((width | x) & 1) {
        int *dst = ptr + offset;
        const uint16_t nextLine = lineSize - width;
        while (height--) {
            for (uint16_t i = width; i--;)
                *(dst++) = *(data++);
            dst += nextLine;
        }
    } else {
        // Memory alignment optimal for 64-bit copy
        long *dst = (long *) (ptr + offset);
        const long *src = (const long *) data;
        const uint16_t nextLine = (lineSize - width) / 2;
        width /= 2;
        while (height--) {
            for (uint16_t i = width; i--;)
                *(dst++) = *(src++);
            dst += nextLine;
        }
    }
}

SubTexture TileMap::makeSubTexture(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
    float w = info.extent.width * 2;
    float h = info.extent.height * 2;
    SubTexture tex {x, y, width, height,
        (x * 2 + 1) / w,
        (y * 2 + 1) / h,
        ((x + width) * 2 - 1) / w,
        ((y + height) * 2 - 1) / h};
    reserveSpace(x, y, width, height);
    return tex;
}

void TileMap::uploadChanges(VkCommandBuffer cmd, bool includeTransition, VkImageLayout layout)
{
    if (includeTransition) {
        VkImageMemoryBarrier barrier {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, nullptr, VK_ACCESS_HOST_WRITE_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, loaded ? layout : VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, image, {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 0, 1}};
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
        vkCmdCopyBufferToImage(cmd, staging.buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, writes.size(), writes.data());
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = 0;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = layout;
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
        loaded = true;
    } else {
        vkCmdCopyBufferToImage(cmd, staging.buffer, image, layout, writes.size(), writes.data());
    }
    writes.clear();
}

SubTexture TileMap::acquireSurface(uint32_t width, uint32_t height)
{
    uint16_t *ptr = map;
    uint16_t *end = this->end - lineSize * (height - 1);
    uint32_t layer;
    uint16_t *subptr;
    const uint16_t tmpWidth = (width + (CHUNK_SIZE - 1)) / CHUNK_SIZE;
    const uint16_t tmpHeight = (height + (CHUNK_SIZE - 1)) / CHUNK_SIZE;
    while (ptr > end) {
        subptr = ptr;
        layer = tmpHeight;
        while (layer--) {
            if (*subptr >= tmpWidth) {
                subptr += lineSize;
            } else {
                ptr += (*subptr) + 1;
                continue;
            }
        }
        uint32_t pos = ptr - map;
        return makeSubTexture((pos % lineSize) * CHUNK_SIZE, (pos / lineSize) * CHUNK_SIZE, width, height);
    }
    return {};
}

void TileMap::releaseSurface(SubTexture &surface)
{
    releaseSpace(surface.x, surface.y, surface.width, surface.height);
}

void TileMap::reserveSpace(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    width += x % CHUNK_SIZE + CHUNK_SIZE - 1;
    height += y % CHUNK_SIZE + CHUNK_SIZE - 1;
    x /= CHUNK_SIZE;
    y /= CHUNK_SIZE;
    width /= CHUNK_SIZE;
    height /= CHUNK_SIZE;
    width *= 2;
    uint16_t *ptr = map + x + y * lineSize;
    while (height--) {
        memset(ptr, 0, width);
        propagate(ptr);
        ptr += lineSize;
    }
}

void TileMap::releaseSpace(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    width += x % CHUNK_SIZE + CHUNK_SIZE - 1;
    height += y % CHUNK_SIZE + CHUNK_SIZE - 1;
    x /= CHUNK_SIZE;
    y /= CHUNK_SIZE;
    width /= CHUNK_SIZE;
    height /= CHUNK_SIZE;
    uint16_t *ptr = map + x + width + y * lineSize;
    while (height--) {
        propagate(ptr, width);
        ptr += lineSize;
    }
}