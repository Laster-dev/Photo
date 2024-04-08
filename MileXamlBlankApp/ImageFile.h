

#include <vector>
#include <iostream>

struct IconDirEntry {
    uint8_t width;
    uint8_t height;
    uint8_t colorCount;
    uint8_t reserved;
    uint16_t planes;
    uint16_t bitCount;
    uint32_t bytesInRes;
    uint32_t imageOffset; // 相对于文件开始的偏移
};

// 解析并提取ICO中所有图标的函数
bool ExtractAllIcons(const std::vector<uint8_t>& imageData, std::vector<std::vector<uint8_t>>& iconsData) {
    if (imageData.size() < 6) return false; // 至少需要6字节来存储头部信息

    // 读取ICO头部信息
    uint16_t reserved = *reinterpret_cast<const uint16_t*>(&imageData[0]);
    uint16_t type = *reinterpret_cast<const uint16_t*>(&imageData[2]);
    uint16_t count = *reinterpret_cast<const uint16_t*>(&imageData[4]);

    if (reserved != 0 || type != 1) {
        // 不是有效的ICO文件
        return false;
    }

    // 遍历每个图标条目
    const size_t entrySize = 16; // 每个图标目录条目的大小
    for (uint16_t i = 0; i < count; ++i) {
        size_t entryOffset = 6 + i * entrySize;
        if (imageData.size() < entryOffset + entrySize) return false; // 确保数据包含完整的条目

        // 解析目录条目
        const IconDirEntry* entry = reinterpret_cast<const IconDirEntry*>(&imageData[entryOffset]);

        // 检查偏移量和图像大小是否有效
        if (imageData.size() < entry->imageOffset + entry->bytesInRes) return false;

        // 提取图像数据
        std::vector<uint8_t> iconData(imageData.begin() + entry->imageOffset, imageData.begin() + entry->imageOffset + entry->bytesInRes);
        iconsData.push_back(iconData); // 添加到结果集
    }

    return true;
}
///检测是不是ico
bool IsIcoFile(const std::vector<uint8_t>& data) {
    // 检查数据长度是否足够
    if (data.size() < 6) return false;

    // 读取并检查保留字段（必须为0）
    uint16_t reserved = static_cast<uint16_t>(data[0]) | static_cast<uint16_t>(data[1] << 8);
    if (reserved != 0) return false;

    // 读取并检查类型字段（对于ICO文件，这个值必须为1）
    uint16_t type = static_cast<uint16_t>(data[2]) | static_cast<uint16_t>(data[3] << 8);
    if (type != 1) return false;

    // 以上条件都满足，假定是ICO文件
    return true;
}
int exp() {
    std::vector<uint8_t> myImageData; // 假设这里已经加载了ICO文件数据
    std::vector<std::vector<uint8_t>> iconsData;

    if (ExtractAllIcons(myImageData, iconsData)) {
        std::cout << "成功提取所有图标，总数: " << iconsData.size() << std::endl;
        // 在这里可以进一步处理每个图标的数据
    }
    else {
        std::cerr << "提取图标失败。" << std::endl;
    }

    return 0;
}
