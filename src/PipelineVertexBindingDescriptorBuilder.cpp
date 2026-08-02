#include "PipelineVertexBindingDescriptorBuilder.hpp"
#include "VkBindings/Enums.hpp"
#include <VkBindings/Structs.hpp>

#include <iostream>

namespace VkUtils {

void PipelineVertexBindingDescriptorBuilder::addBinding(
    VkBindings::VertexInputBindingDescription bindingDescription) {
    currentBinding = nextBinding++;
    bindingDescription.binding = currentBinding;
    bindingDescriptions.emplace_back(bindingDescription);
}

void PipelineVertexBindingDescriptorBuilder::addAttribute(
    VkBindings::VertexInputAttributeDescription attributeDescription) {
    attributeDescription.binding = currentBinding;
    attributeDescription.location = currentLocation++;
    attributeDescriptions.emplace_back(attributeDescription);
}

auto PipelineVertexBindingDescriptorBuilder::getVertexInputInfo()
    -> VkBindings::PipelineVertexInputStateCreateInfo {
    VkBindings::PipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.vertexBindingDescriptionCount =
        static_cast<uint32_t>(bindingDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
    vertexInputInfo.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
    return vertexInputInfo;
}

void PipelineVertexBindingDescriptorBuilder::print() const {
    using enum VkBindings::Format;

    for (size_t b = 0; b < bindingDescriptions.size(); b++) {
        const auto &bind = bindingDescriptions[b];
        std::cout << "Binding: " << bind.binding << " rate: "
                  << (bind.inputRate == VkBindings::VertexInputRate::eVertex ? "vertex"
                                                                             : "instance")
                  << " stride: " << bind.stride << "\n";
        for (auto attr : attributeDescriptions) {
            if (attr.binding != b)
                continue;
            std::cout << "\tlayout(location = " << attr.location << ") ";

            switch (attr.format) {
            // Single-component float formats
            case eR8Unorm:
            case eR8Snorm:
            case eR8Uscaled:
            case eR8Sscaled:
            case eR8Srgb:
            case eR16Unorm:
            case eR16Snorm:
            case eR16Uscaled:
            case eR16Sscaled:
            case eR16Sfloat:
            case eR32Sfloat:
            case eR64Sfloat:
                std::cout << "float";
                break;
            case eR8Uint:
            case eR16Uint:
            case eR32Uint:
            case eR64Uint:
                std::cout << "uint";
                break;
            case eR8Sint:
            case eR16Sint:
            case eR32Sint:
            case eR64Sint:
                std::cout << "int";
                break;
            // Two component formats (vec2)
            case eR8G8Unorm:
            case eR8G8Snorm:
            case eR8G8Uscaled:
            case eR8G8Sscaled:
            case eR8G8Srgb:
            case eR16G16Unorm:
            case eR16G16Snorm:
            case eR16G16Uscaled:
            case eR16G16Sscaled:
            case eR16G16Sfloat:
            case eR32G32Sfloat:
            case eR64G64Sfloat:
                std::cout << "vec2";
                break;
            case eR8G8Uint:
            case eR16G16Uint:
            case eR32G32Uint:
            case eR64G64Uint:
                std::cout << "uvec2";
                break;
            case eR8G8Sint:
            case eR16G16Sint:
            case eR32G32Sint:
            case eR64G64Sint:
                std::cout << "ivec2";
                break;
            // Three component formats (vec3)
            case eR8G8B8Unorm:
            case eR8G8B8Snorm:
            case eR8G8B8Uscaled:
            case eR8G8B8Sscaled:
            case eR8G8B8Srgb:
            case eB8G8R8Unorm:
            case eB8G8R8Snorm:
            case eB8G8R8Uscaled:
            case eB8G8R8Sscaled:
            case eB8G8R8Srgb:
            case eR16G16B16Unorm:
            case eR16G16B16Snorm:
            case eR16G16B16Uscaled:
            case eR16G16B16Sscaled:
            case eR16G16B16Sfloat:
            case eR32G32B32Sfloat:
            case eR64G64B64Sfloat:
            case eB10G11R11UfloatPack32:
            case eE5B9G9R9UfloatPack32:
                std::cout << "vec3";
                break;
            case eR8G8B8Uint:
            case eB8G8R8Uint:
            case eR16G16B16Uint:
            case eR32G32B32Uint:
            case eR64G64B64Uint:
                std::cout << "uvec3";
                break;
            case eR8G8B8Sint:
            case eB8G8R8Sint:
            case eR16G16B16Sint:
            case eR32G32B32Sint:
            case eR64G64B64Sint:
                std::cout << "ivec3";
                break;
            // Four component formats (vec4)
            case eR8G8B8A8Unorm:
            case eR8G8B8A8Snorm:
            case eR8G8B8A8Uscaled:
            case eR8G8B8A8Sscaled:
            case eR8G8B8A8Srgb:
            case eB8G8R8A8Unorm:
            case eB8G8R8A8Snorm:
            case eB8G8R8A8Uscaled:
            case eB8G8R8A8Sscaled:
            case eB8G8R8A8Srgb:
            case eA8B8G8R8UnormPack32:
            case eA8B8G8R8SnormPack32:
            case eA8B8G8R8UscaledPack32:
            case eA8B8G8R8SscaledPack32:
            case eA8B8G8R8SrgbPack32:
            case eA2R10G10B10UnormPack32:
            case eA2R10G10B10SnormPack32:
            case eA2R10G10B10UscaledPack32:
            case eA2R10G10B10SscaledPack32:
            case eA2B10G10R10UnormPack32:
            case eA2B10G10R10SnormPack32:
            case eA2B10G10R10UscaledPack32:
            case eA2B10G10R10SscaledPack32:
            case eR16G16B16A16Unorm:
            case eR16G16B16A16Snorm:
            case eR16G16B16A16Uscaled:
            case eR16G16B16A16Sscaled:
            case eR16G16B16A16Sfloat:
            case eR32G32B32A32Sfloat:
            case eR64G64B64A64Sfloat:
            case eR4G4B4A4UnormPack16:
            case eB4G4R4A4UnormPack16:
            case eR5G5B5A1UnormPack16:
            case eB5G5R5A1UnormPack16:
            case eA1R5G5B5UnormPack16:
            case eA1B5G5R5UnormPack16:
            case eA4R4G4B4UnormPack16:
            case eA4B4G4R4UnormPack16:
            case eR5G6B5UnormPack16:
            case eB5G6R5UnormPack16:
                std::cout << "vec4";
                break;
            case eR8G8B8A8Uint:
            case eB8G8R8A8Uint:
            case eA8B8G8R8UintPack32:
            case eA2R10G10B10UintPack32:
            case eA2B10G10R10UintPack32:
            case eR16G16B16A16Uint:
            case eR32G32B32A32Uint:
            case eR64G64B64A64Uint:
                std::cout << "uvec4";
                break;
            case eR8G8B8A8Sint:
            case eB8G8R8A8Sint:
            case eA8B8G8R8SintPack32:
            case eA2R10G10B10SintPack32:
            case eA2B10G10R10SintPack32:
            case eR16G16B16A16Sint:
            case eR32G32B32A32Sint:
            case eR64G64B64A64Sint:
                std::cout << "ivec4";
                break;
            // Depth/Stencil formats
            case eD16Unorm:
            case ex8D24UnormPack32:
            case eD32Sfloat:
                std::cout << "float"; // Depth
                break;
            case eS8Uint:
                std::cout << "uint"; // Stencil
                break;
            case eD16UnormS8Uint:
            case eD24UnormS8Uint:
            case eD32SfloatS8Uint:
                std::cout << "vec2"; // Depth + Stencil (typically accessed separately)
                break;
            // Special formats
            case eR4G4UnormPack8:
                std::cout << "vec2";
                break;
            case eA8Unorm:
                std::cout << "float";
                break;
            case eR8BoolARM:
                std::cout << "bool";
                break;
            // Compressed formats (sampled as vec4)
            case eBc1RgbUnormBlock:
            case eBc1RgbSrgbBlock:
            case eBc1RgbaUnormBlock:
            case eBc1RgbaSrgbBlock:
            case eBc2UnormBlock:
            case eBc2SrgbBlock:
            case eBc3UnormBlock:
            case eBc3SrgbBlock:
            case eBc4UnormBlock:
            case eBc4SnormBlock:
            case eBc5UnormBlock:
            case eBc5SnormBlock:
            case eBc6HUfloatBlock:
            case eBc6HSfloatBlock:
            case eBc7UnormBlock:
            case eBc7SrgbBlock:
            case eEtc2R8G8B8UnormBlock:
            case eEtc2R8G8B8SrgbBlock:
            case eEtc2R8G8B8A1UnormBlock:
            case eEtc2R8G8B8A1SrgbBlock:
            case eEtc2R8G8B8A8UnormBlock:
            case eEtc2R8G8B8A8SrgbBlock:
            case eEacR11UnormBlock:
            case eEacR11SnormBlock:
            case eEacR11G11UnormBlock:
            case eEacR11G11SnormBlock:
            case eAstc4x4UnormBlock:
            case eAstc4x4SrgbBlock:
            case eAstc4x4SfloatBlock:
            case eAstc5x4UnormBlock:
            case eAstc5x4SrgbBlock:
            case eAstc5x4SfloatBlock:
            case eAstc5x5UnormBlock:
            case eAstc5x5SrgbBlock:
            case eAstc5x5SfloatBlock:
            case eAstc6x5UnormBlock:
            case eAstc6x5SrgbBlock:
            case eAstc6x5SfloatBlock:
            case eAstc6x6UnormBlock:
            case eAstc6x6SrgbBlock:
            case eAstc6x6SfloatBlock:
            case eAstc8x5UnormBlock:
            case eAstc8x5SrgbBlock:
            case eAstc8x5SfloatBlock:
            case eAstc8x6UnormBlock:
            case eAstc8x6SrgbBlock:
            case eAstc8x6SfloatBlock:
            case eAstc8x8UnormBlock:
            case eAstc8x8SrgbBlock:
            case eAstc8x8SfloatBlock:
            case eAstc10x5UnormBlock:
            case eAstc10x5SrgbBlock:
            case eAstc10x5SfloatBlock:
            case eAstc10x6UnormBlock:
            case eAstc10x6SrgbBlock:
            case eAstc10x6SfloatBlock:
            case eAstc10x8UnormBlock:
            case eAstc10x8SrgbBlock:
            case eAstc10x8SfloatBlock:
            case eAstc10x10UnormBlock:
            case eAstc10x10SrgbBlock:
            case eAstc10x10SfloatBlock:
            case eAstc12x10UnormBlock:
            case eAstc12x10SrgbBlock:
            case eAstc12x10SfloatBlock:
            case eAstc12x12UnormBlock:
            case eAstc12x12SrgbBlock:
            case eAstc12x12SfloatBlock:
            case ePvrtc12BppUnormBlockIMG:
            case ePvrtc14BppUnormBlockIMG:
            case ePvrtc22BppUnormBlockIMG:
            case ePvrtc24BppUnormBlockIMG:
            case ePvrtc12BppSrgbBlockIMG:
            case ePvrtc14BppSrgbBlockIMG:
            case ePvrtc22BppSrgbBlockIMG:
            case ePvrtc24BppSrgbBlockIMG:
                std::cout << "vec4"; // Compressed formats are sampled as vec4
                break;
            case eUndefined:
            default:
                std::cout << "/* UNSUPPORTED FORMAT */";
                break;
            }
            std::cout << " (offset " << attr.offset << ")\n";
        }
    } // namespace VkUtils
}

} // namespace VkUtils
