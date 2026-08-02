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

VkBindings::PipelineVertexInputStateCreateInfo
PipelineVertexBindingDescriptorBuilder::getVertexInputInfo() {
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
    for (size_t b = 0; b < bindingDescriptions.size(); b++) {
        const auto &bind = bindingDescriptions[b];
        std::cout << "Binding: " << bind.binding << " rate: "
                  << (bind.inputRate == VkBindings::VertexInputRate::eVertex ? "vertex"
                                                                             : "instance")
                  << " stride: " << bind.stride << "\n";
        for (size_t l = 0; l < attributeDescriptions.size(); l++) {
            const auto &attr = attributeDescriptions[l];
            if (attr.binding != b)
                continue;
            std::cout << "\tlayout(location = " << attr.location << ") ";

            // (Updated switch cases to use VkBindings::Format enums)
            switch (attr.format) {
            // Single-component float formats
            case VkBindings::Format::eR8Unorm:
            case VkBindings::Format::eR8Snorm:
            case VkBindings::Format::eR8Uscaled:
            case VkBindings::Format::eR8Sscaled:
            case VkBindings::Format::eR8Srgb:
            case VkBindings::Format::eR16Unorm:
            case VkBindings::Format::eR16Snorm:
            case VkBindings::Format::eR16Uscaled:
            case VkBindings::Format::eR16Sscaled:
            case VkBindings::Format::eR16Sfloat:
            case VkBindings::Format::eR32Sfloat:
            case VkBindings::Format::eR64Sfloat:
                std::cout << "float";
                break;
            case VkBindings::Format::eR8Uint:
            case VkBindings::Format::eR16Uint:
            case VkBindings::Format::eR32Uint:
            case VkBindings::Format::eR64Uint:
                std::cout << "uint";
                break;
            case VkBindings::Format::eR8Sint:
            case VkBindings::Format::eR16Sint:
            case VkBindings::Format::eR32Sint:
            case VkBindings::Format::eR64Sint:
                std::cout << "int";
                break;
            // Two component formats (vec2)
            case VkBindings::Format::eR8G8Unorm:
            case VkBindings::Format::eR8G8Snorm:
            case VkBindings::Format::eR8G8Uscaled:
            case VkBindings::Format::eR8G8Sscaled:
            case VkBindings::Format::eR8G8Srgb:
            case VkBindings::Format::eR16G16Unorm:
            case VkBindings::Format::eR16G16Snorm:
            case VkBindings::Format::eR16G16Uscaled:
            case VkBindings::Format::eR16G16Sscaled:
            case VkBindings::Format::eR16G16Sfloat:
            case VkBindings::Format::eR32G32Sfloat:
            case VkBindings::Format::eR64G64Sfloat:
                std::cout << "vec2";
                break;
            case VkBindings::Format::eR8G8Uint:
            case VkBindings::Format::eR16G16Uint:
            case VkBindings::Format::eR32G32Uint:
            case VkBindings::Format::eR64G64Uint:
                std::cout << "uvec2";
                break;
            case VkBindings::Format::eR8G8Sint:
            case VkBindings::Format::eR16G16Sint:
            case VkBindings::Format::eR32G32Sint:
            case VkBindings::Format::eR64G64Sint:
                std::cout << "ivec2";
                break;
            // Three component formats (vec3)
            case VkBindings::Format::eR8G8B8Unorm:
            case VkBindings::Format::eR8G8B8Snorm:
            case VkBindings::Format::eR8G8B8Uscaled:
            case VkBindings::Format::eR8G8B8Sscaled:
            case VkBindings::Format::eR8G8B8Srgb:
            case VkBindings::Format::eB8G8R8Unorm:
            case VkBindings::Format::eB8G8R8Snorm:
            case VkBindings::Format::eB8G8R8Uscaled:
            case VkBindings::Format::eB8G8R8Sscaled:
            case VkBindings::Format::eB8G8R8Srgb:
            case VkBindings::Format::eR16G16B16Unorm:
            case VkBindings::Format::eR16G16B16Snorm:
            case VkBindings::Format::eR16G16B16Uscaled:
            case VkBindings::Format::eR16G16B16Sscaled:
            case VkBindings::Format::eR16G16B16Sfloat:
            case VkBindings::Format::eR32G32B32Sfloat:
            case VkBindings::Format::eR64G64B64Sfloat:
            case VkBindings::Format::eB10G11R11UfloatPack32:
            case VkBindings::Format::eE5B9G9R9UfloatPack32:
                std::cout << "vec3";
                break;
            case VkBindings::Format::eR8G8B8Uint:
            case VkBindings::Format::eB8G8R8Uint:
            case VkBindings::Format::eR16G16B16Uint:
            case VkBindings::Format::eR32G32B32Uint:
            case VkBindings::Format::eR64G64B64Uint:
                std::cout << "uvec3";
                break;
            case VkBindings::Format::eR8G8B8Sint:
            case VkBindings::Format::eB8G8R8Sint:
            case VkBindings::Format::eR16G16B16Sint:
            case VkBindings::Format::eR32G32B32Sint:
            case VkBindings::Format::eR64G64B64Sint:
                std::cout << "ivec3";
                break;
            // Four component formats (vec4)
            case VkBindings::Format::eR8G8B8A8Unorm:
            case VkBindings::Format::eR8G8B8A8Snorm:
            case VkBindings::Format::eR8G8B8A8Uscaled:
            case VkBindings::Format::eR8G8B8A8Sscaled:
            case VkBindings::Format::eR8G8B8A8Srgb:
            case VkBindings::Format::eB8G8R8A8Unorm:
            case VkBindings::Format::eB8G8R8A8Snorm:
            case VkBindings::Format::eB8G8R8A8Uscaled:
            case VkBindings::Format::eB8G8R8A8Sscaled:
            case VkBindings::Format::eB8G8R8A8Srgb:
            case VkBindings::Format::eA8B8G8R8UnormPack32:
            case VkBindings::Format::eA8B8G8R8SnormPack32:
            case VkBindings::Format::eA8B8G8R8UscaledPack32:
            case VkBindings::Format::eA8B8G8R8SscaledPack32:
            case VkBindings::Format::eA8B8G8R8SrgbPack32:
            case VkBindings::Format::eA2R10G10B10UnormPack32:
            case VkBindings::Format::eA2R10G10B10SnormPack32:
            case VkBindings::Format::eA2R10G10B10UscaledPack32:
            case VkBindings::Format::eA2R10G10B10SscaledPack32:
            case VkBindings::Format::eA2B10G10R10UnormPack32:
            case VkBindings::Format::eA2B10G10R10SnormPack32:
            case VkBindings::Format::eA2B10G10R10UscaledPack32:
            case VkBindings::Format::eA2B10G10R10SscaledPack32:
            case VkBindings::Format::eR16G16B16A16Unorm:
            case VkBindings::Format::eR16G16B16A16Snorm:
            case VkBindings::Format::eR16G16B16A16Uscaled:
            case VkBindings::Format::eR16G16B16A16Sscaled:
            case VkBindings::Format::eR16G16B16A16Sfloat:
            case VkBindings::Format::eR32G32B32A32Sfloat:
            case VkBindings::Format::eR64G64B64A64Sfloat:
            case VkBindings::Format::eR4G4B4A4UnormPack16:
            case VkBindings::Format::eB4G4R4A4UnormPack16:
            case VkBindings::Format::eR5G5B5A1UnormPack16:
            case VkBindings::Format::eB5G5R5A1UnormPack16:
            case VkBindings::Format::eA1R5G5B5UnormPack16:
            case VkBindings::Format::eA1B5G5R5UnormPack16:
            case VkBindings::Format::eA4R4G4B4UnormPack16:
            case VkBindings::Format::eA4B4G4R4UnormPack16:
            case VkBindings::Format::eR5G6B5UnormPack16:
            case VkBindings::Format::eB5G6R5UnormPack16:
                std::cout << "vec4";
                break;
            case VkBindings::Format::eR8G8B8A8Uint:
            case VkBindings::Format::eB8G8R8A8Uint:
            case VkBindings::Format::eA8B8G8R8UintPack32:
            case VkBindings::Format::eA2R10G10B10UintPack32:
            case VkBindings::Format::eA2B10G10R10UintPack32:
            case VkBindings::Format::eR16G16B16A16Uint:
            case VkBindings::Format::eR32G32B32A32Uint:
            case VkBindings::Format::eR64G64B64A64Uint:
                std::cout << "uvec4";
                break;
            case VkBindings::Format::eR8G8B8A8Sint:
            case VkBindings::Format::eB8G8R8A8Sint:
            case VkBindings::Format::eA8B8G8R8SintPack32:
            case VkBindings::Format::eA2R10G10B10SintPack32:
            case VkBindings::Format::eA2B10G10R10SintPack32:
            case VkBindings::Format::eR16G16B16A16Sint:
            case VkBindings::Format::eR32G32B32A32Sint:
            case VkBindings::Format::eR64G64B64A64Sint:
                std::cout << "ivec4";
                break;
            // Depth/Stencil formats
            case VkBindings::Format::eD16Unorm:
            case VkBindings::Format::ex8D24UnormPack32:
            case VkBindings::Format::eD32Sfloat:
                std::cout << "float"; // Depth
                break;
            case VkBindings::Format::eS8Uint:
                std::cout << "uint"; // Stencil
                break;
            case VkBindings::Format::eD16UnormS8Uint:
            case VkBindings::Format::eD24UnormS8Uint:
            case VkBindings::Format::eD32SfloatS8Uint:
                std::cout << "vec2"; // Depth + Stencil (typically accessed separately)
                break;
            // Special formats
            case VkBindings::Format::eR4G4UnormPack8:
                std::cout << "vec2";
                break;
            case VkBindings::Format::eA8Unorm:
                std::cout << "float";
                break;
            case VkBindings::Format::eR8BoolARM:
                std::cout << "bool";
                break;
            // Compressed formats (sampled as vec4)
            case VkBindings::Format::eBc1RgbUnormBlock:
            case VkBindings::Format::eBc1RgbSrgbBlock:
            case VkBindings::Format::eBc1RgbaUnormBlock:
            case VkBindings::Format::eBc1RgbaSrgbBlock:
            case VkBindings::Format::eBc2UnormBlock:
            case VkBindings::Format::eBc2SrgbBlock:
            case VkBindings::Format::eBc3UnormBlock:
            case VkBindings::Format::eBc3SrgbBlock:
            case VkBindings::Format::eBc4UnormBlock:
            case VkBindings::Format::eBc4SnormBlock:
            case VkBindings::Format::eBc5UnormBlock:
            case VkBindings::Format::eBc5SnormBlock:
            case VkBindings::Format::eBc6HUfloatBlock:
            case VkBindings::Format::eBc6HSfloatBlock:
            case VkBindings::Format::eBc7UnormBlock:
            case VkBindings::Format::eBc7SrgbBlock:
            case VkBindings::Format::eEtc2R8G8B8UnormBlock:
            case VkBindings::Format::eEtc2R8G8B8SrgbBlock:
            case VkBindings::Format::eEtc2R8G8B8A1UnormBlock:
            case VkBindings::Format::eEtc2R8G8B8A1SrgbBlock:
            case VkBindings::Format::eEtc2R8G8B8A8UnormBlock:
            case VkBindings::Format::eEtc2R8G8B8A8SrgbBlock:
            case VkBindings::Format::eEacR11UnormBlock:
            case VkBindings::Format::eEacR11SnormBlock:
            case VkBindings::Format::eEacR11G11UnormBlock:
            case VkBindings::Format::eEacR11G11SnormBlock:
            case VkBindings::Format::eAstc4x4UnormBlock:
            case VkBindings::Format::eAstc4x4SrgbBlock:
            case VkBindings::Format::eAstc4x4SfloatBlock:
            case VkBindings::Format::eAstc5x4UnormBlock:
            case VkBindings::Format::eAstc5x4SrgbBlock:
            case VkBindings::Format::eAstc5x4SfloatBlock:
            case VkBindings::Format::eAstc5x5UnormBlock:
            case VkBindings::Format::eAstc5x5SrgbBlock:
            case VkBindings::Format::eAstc5x5SfloatBlock:
            case VkBindings::Format::eAstc6x5UnormBlock:
            case VkBindings::Format::eAstc6x5SrgbBlock:
            case VkBindings::Format::eAstc6x5SfloatBlock:
            case VkBindings::Format::eAstc6x6UnormBlock:
            case VkBindings::Format::eAstc6x6SrgbBlock:
            case VkBindings::Format::eAstc6x6SfloatBlock:
            case VkBindings::Format::eAstc8x5UnormBlock:
            case VkBindings::Format::eAstc8x5SrgbBlock:
            case VkBindings::Format::eAstc8x5SfloatBlock:
            case VkBindings::Format::eAstc8x6UnormBlock:
            case VkBindings::Format::eAstc8x6SrgbBlock:
            case VkBindings::Format::eAstc8x6SfloatBlock:
            case VkBindings::Format::eAstc8x8UnormBlock:
            case VkBindings::Format::eAstc8x8SrgbBlock:
            case VkBindings::Format::eAstc8x8SfloatBlock:
            case VkBindings::Format::eAstc10x5UnormBlock:
            case VkBindings::Format::eAstc10x5SrgbBlock:
            case VkBindings::Format::eAstc10x5SfloatBlock:
            case VkBindings::Format::eAstc10x6UnormBlock:
            case VkBindings::Format::eAstc10x6SrgbBlock:
            case VkBindings::Format::eAstc10x6SfloatBlock:
            case VkBindings::Format::eAstc10x8UnormBlock:
            case VkBindings::Format::eAstc10x8SrgbBlock:
            case VkBindings::Format::eAstc10x8SfloatBlock:
            case VkBindings::Format::eAstc10x10UnormBlock:
            case VkBindings::Format::eAstc10x10SrgbBlock:
            case VkBindings::Format::eAstc10x10SfloatBlock:
            case VkBindings::Format::eAstc12x10UnormBlock:
            case VkBindings::Format::eAstc12x10SrgbBlock:
            case VkBindings::Format::eAstc12x10SfloatBlock:
            case VkBindings::Format::eAstc12x12UnormBlock:
            case VkBindings::Format::eAstc12x12SrgbBlock:
            case VkBindings::Format::eAstc12x12SfloatBlock:
            case VkBindings::Format::ePvrtc12BppUnormBlockIMG:
            case VkBindings::Format::ePvrtc14BppUnormBlockIMG:
            case VkBindings::Format::ePvrtc22BppUnormBlockIMG:
            case VkBindings::Format::ePvrtc24BppUnormBlockIMG:
            case VkBindings::Format::ePvrtc12BppSrgbBlockIMG:
            case VkBindings::Format::ePvrtc14BppSrgbBlockIMG:
            case VkBindings::Format::ePvrtc22BppSrgbBlockIMG:
            case VkBindings::Format::ePvrtc24BppSrgbBlockIMG:
                std::cout << "vec4"; // Compressed formats are sampled as vec4
                break;
            case VkBindings::Format::eUndefined:
            default:
                std::cout << "/* UNSUPPORTED FORMAT */";
                break;
            }
            std::cout << " (offset " << attr.offset << ")\n";
        }
    }
}

} // namespace VkUtils
