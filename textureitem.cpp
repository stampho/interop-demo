#include "textureitem.h"

#include <QDebug>
#include <QImage>
#include <QSGImageNode>
#include <rhi/qrhi.h>

// TODO: Check if Vulkan is available at build time?
#include <QVulkanDeviceFunctions>
#include <QVulkanFunctions>
#include <QVulkanInstance>

TextureItem::TextureItem(QQuickItem *parent)
    : QQuickItem(parent)
{
    setFlags(QQuickItem::ItemHasContents);

    if (parent) {
        setSize(parent->size());
        connect(parent, &QQuickItem::widthChanged, this, &TextureItem::onSizeChanged);
        connect(parent, &QQuickItem::heightChanged, this, &TextureItem::onSizeChanged);
    }
}

QSGNode *TextureItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    QSGImageNode *node = static_cast<QSGImageNode *>(oldNode);
    if (!node) {
        node = window()->createImageNode();
        node->setOwnsTexture(true);
    }

    QRhi *rhi = window()->rhi();
    if (!rhi) {
        qWarning("RHI is not available!");
        return oldNode;
    }

    if (rhi->backend() != QRhi::Vulkan) {
        // TODO: Implement all backends.
        qWarning("Only Vulkan backend is supported!");
        return oldNode;
    }

    const QRhiVulkanNativeHandles *vulkanHandles =
        static_cast<const QRhiVulkanNativeHandles *>(rhi->nativeHandles());
    QVulkanFunctions *f = vulkanHandles->inst->functions();
    QVulkanDeviceFunctions *df = vulkanHandles->inst->deviceFunctions(vulkanHandles->dev);
    VkResult result;

    VkImageCreateInfo imageCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .extent = {
            .width = static_cast<uint32_t>(size().width()),
            .height = static_cast<uint32_t>(size().height()),
            .depth = 1,
        },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_LINEAR,
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    VkImage image = VK_NULL_HANDLE;
    result = df->vkCreateImage(vulkanHandles->dev, &imageCreateInfo, nullptr, &image);
    if (result != VK_SUCCESS) {
        qWarning() << "vkCreateImage failed with result:" << result;
        return oldNode;
    }

    VkMemoryRequirements requirements;
    df->vkGetImageMemoryRequirements(vulkanHandles->dev, image, &requirements);
    if (!requirements.memoryTypeBits) {
        qWarning() << "vkGetImageMemoryRequirements failed.";
        return oldNode;
    }

    VkPhysicalDeviceMemoryProperties memoryProperties;
    f->vkGetPhysicalDeviceMemoryProperties(vulkanHandles->physDev, &memoryProperties);
    constexpr VkMemoryPropertyFlags flags =
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    const uint32_t memoryCount = memoryProperties.memoryTypeCount;
    uint32_t memoryTypeIndex = memoryCount;
    for (uint32_t i = 0; i < memoryCount; ++i) {
        if (((1u << i) & requirements.memoryTypeBits) == 0)
            continue;
        if ((memoryProperties.memoryTypes[i].propertyFlags & flags) != flags)
            continue;
        memoryTypeIndex = i;
        break;
    }

    if (memoryTypeIndex >= memoryCount) {
        qWarning("Cannot find valid memory type index.");
        return oldNode;
    }

    VkMemoryAllocateInfo memoryAllocateInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = nullptr,
        .allocationSize = requirements.size,
        .memoryTypeIndex = memoryTypeIndex,
    };

    VkDeviceMemory imageMemory = VK_NULL_HANDLE;
    result = df->vkAllocateMemory(vulkanHandles->dev, &memoryAllocateInfo, nullptr, &imageMemory);
    if (result != VK_SUCCESS) {
        qWarning() << "vkAllocateMemory failed with result:" << result;
        return oldNode;
    }

    df->vkBindImageMemory(vulkanHandles->dev, image, imageMemory, 0);

    void *data;
    VkDeviceSize imageSize = size().width() * size().height() * 4;

    df->vkMapMemory(vulkanHandles->dev, imageMemory, 0, imageSize, 0, &data);
    QImage qimage(size().toSize(), QImage::Format_RGBA8888);
    qimage.fill(QColor("red"));
    memcpy(data, qimage.bits(), static_cast<size_t>(imageSize));
    df->vkUnmapMemory(vulkanHandles->dev, imageMemory);

    QQuickWindow::CreateTextureOptions texOpts;
    texOpts.setFlag(QQuickWindow::TextureHasAlphaChannel);
    QSGTexture *texture = QNativeInterface::QSGVulkanTexture::fromNative(image, VK_IMAGE_LAYOUT_UNDEFINED,
                                                                         window(), size().toSize(), texOpts);

    // TODO: Destroy Image and ImageMemory

    if (!texture) {
        qWarning("Failed to create QSGTexture from VkImage.");
        return oldNode;
    }

    node->setRect(0, 0, width(), height());
    node->setTexture(texture);
    return node;
}

void TextureItem::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickItem::geometryChange(newGeometry, oldGeometry);
}

void TextureItem::onSizeChanged()
{
    if (parentItem() && size() != parentItem()->size()) {
        setSize(parentItem()->size());
    }
}
