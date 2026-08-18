#include "PipelineVertexBindingDescriptorBuilder.hpp"

#include <VkBindings/Enums.hpp>
#include <VkBindings/Structs.hpp>

#include <cstddef>
#include <cstdint>
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

    for (size_t binding = 0; binding < bindingDescriptions.size(); binding++) {
        const auto &bind = bindingDescriptions.at(binding);
        std::cout << "Binding: " << bind.binding << " rate: "
                  << (bind.inputRate == VkBindings::VertexInputRate::Vertex ? "vertex" : "instance")
                  << " stride: " << bind.stride << "\n";
        for (auto attr : attributeDescriptions) {
            if (attr.binding != binding)
                continue;
            std::cout << "\tlayout(location = " << attr.location << ") ";

            switch (attr.format) {
            // Single-component float formats
            case R8Unorm:
            case R8Snorm:
            case R8Uscaled:
            case R8Sscaled:
            case R8Srgb:
            case R16Unorm:
            case R16Snorm:
            case R16Uscaled:
            case R16Sscaled:
            case R16Sfloat:
            case R32Sfloat:
            case R64Sfloat:
                std::cout << "float";
                break;
            case R8Uint:
            case R16Uint:
            case R32Uint:
            case R64Uint:
                std::cout << "uint";
                break;
            case R8Sint:
            case R16Sint:
            case R32Sint:
            case R64Sint:
                std::cout << "int";
                break;
            // Two component formats (vec2)
            case R8G8Unorm:
            case R8G8Snorm:
            case R8G8Uscaled:
            case R8G8Sscaled:
            case R8G8Srgb:
            case R16G16Unorm:
            case R16G16Snorm:
            case R16G16Uscaled:
            case R16G16Sscaled:
            case R16G16Sfloat:
            case R32G32Sfloat:
            case R64G64Sfloat:
                std::cout << "vec2";
                break;
            case R8G8Uint:
            case R16G16Uint:
            case R32G32Uint:
            case R64G64Uint:
                std::cout << "uvec2";
                break;
            case R8G8Sint:
            case R16G16Sint:
            case R32G32Sint:
            case R64G64Sint:
                std::cout << "ivec2";
                break;
            // Three component formats (vec3)
            case R8G8B8Unorm:
            case R8G8B8Snorm:
            case R8G8B8Uscaled:
            case R8G8B8Sscaled:
            case R8G8B8Srgb:
            case B8G8R8Unorm:
            case B8G8R8Snorm:
            case B8G8R8Uscaled:
            case B8G8R8Sscaled:
            case B8G8R8Srgb:
            case R16G16B16Unorm:
            case R16G16B16Snorm:
            case R16G16B16Uscaled:
            case R16G16B16Sscaled:
            case R16G16B16Sfloat:
            case R32G32B32Sfloat:
            case R64G64B64Sfloat:
            case B10G11R11UfloatPack32:
            case E5B9G9R9UfloatPack32:
                std::cout << "vec3";
                break;
            case R8G8B8Uint:
            case B8G8R8Uint:
            case R16G16B16Uint:
            case R32G32B32Uint:
            case R64G64B64Uint:
                std::cout << "uvec3";
                break;
            case R8G8B8Sint:
            case B8G8R8Sint:
            case R16G16B16Sint:
            case R32G32B32Sint:
            case R64G64B64Sint:
                std::cout << "ivec3";
                break;
            // Four component formats (vec4)
            case R8G8B8A8Unorm:
            case R8G8B8A8Snorm:
            case R8G8B8A8Uscaled:
            case R8G8B8A8Sscaled:
            case R8G8B8A8Srgb:
            case B8G8R8A8Unorm:
            case B8G8R8A8Snorm:
            case B8G8R8A8Uscaled:
            case B8G8R8A8Sscaled:
            case B8G8R8A8Srgb:
            case A8B8G8R8UnormPack32:
            case A8B8G8R8SnormPack32:
            case A8B8G8R8UscaledPack32:
            case A8B8G8R8SscaledPack32:
            case A8B8G8R8SrgbPack32:
            case A2R10G10B10UnormPack32:
            case A2R10G10B10SnormPack32:
            case A2R10G10B10UscaledPack32:
            case A2R10G10B10SscaledPack32:
            case A2B10G10R10UnormPack32:
            case A2B10G10R10SnormPack32:
            case A2B10G10R10UscaledPack32:
            case A2B10G10R10SscaledPack32:
            case R16G16B16A16Unorm:
            case R16G16B16A16Snorm:
            case R16G16B16A16Uscaled:
            case R16G16B16A16Sscaled:
            case R16G16B16A16Sfloat:
            case R32G32B32A32Sfloat:
            case R64G64B64A64Sfloat:
            case R4G4B4A4UnormPack16:
            case B4G4R4A4UnormPack16:
            case R5G5B5A1UnormPack16:
            case B5G5R5A1UnormPack16:
            case A1R5G5B5UnormPack16:
            case A1B5G5R5UnormPack16:
            case A4R4G4B4UnormPack16:
            case A4B4G4R4UnormPack16:
            case R5G6B5UnormPack16:
            case B5G6R5UnormPack16:
                std::cout << "vec4";
                break;
            case R8G8B8A8Uint:
            case B8G8R8A8Uint:
            case A8B8G8R8UintPack32:
            case A2R10G10B10UintPack32:
            case A2B10G10R10UintPack32:
            case R16G16B16A16Uint:
            case R32G32B32A32Uint:
            case R64G64B64A64Uint:
                std::cout << "uvec4";
                break;
            case R8G8B8A8Sint:
            case B8G8R8A8Sint:
            case A8B8G8R8SintPack32:
            case A2R10G10B10SintPack32:
            case A2B10G10R10SintPack32:
            case R16G16B16A16Sint:
            case R32G32B32A32Sint:
            case R64G64B64A64Sint:
                std::cout << "ivec4";
                break;
            // Depth/Stencil formats
            case D16Unorm:
            case x8D24UnormPack32:
            case D32Sfloat:
                std::cout << "float"; // Depth
                break;
            case S8Uint:
                std::cout << "uint"; // Stencil
                break;
            case D16UnormS8Uint:
            case D24UnormS8Uint:
            case D32SfloatS8Uint:
            case R4G4UnormPack8:
                std::cout << "vec2";
                break;
            case A8Unorm:
                std::cout << "float";
                break;
            case R8BoolARM:
                std::cout << "bool";
                break;
            // Compressed formats (sampled as vec4)
            case Bc1RgbUnormBlock:
            case Bc1RgbSrgbBlock:
            case Bc1RgbaUnormBlock:
            case Bc1RgbaSrgbBlock:
            case Bc2UnormBlock:
            case Bc2SrgbBlock:
            case Bc3UnormBlock:
            case Bc3SrgbBlock:
            case Bc4UnormBlock:
            case Bc4SnormBlock:
            case Bc5UnormBlock:
            case Bc5SnormBlock:
            case Bc6HUfloatBlock:
            case Bc6HSfloatBlock:
            case Bc7UnormBlock:
            case Bc7SrgbBlock:
            case Etc2R8G8B8UnormBlock:
            case Etc2R8G8B8SrgbBlock:
            case Etc2R8G8B8A1UnormBlock:
            case Etc2R8G8B8A1SrgbBlock:
            case Etc2R8G8B8A8UnormBlock:
            case Etc2R8G8B8A8SrgbBlock:
            case EacR11UnormBlock:
            case EacR11SnormBlock:
            case EacR11G11UnormBlock:
            case EacR11G11SnormBlock:
            case Astc4x4UnormBlock:
            case Astc4x4SrgbBlock:
            case Astc4x4SfloatBlock:
            case Astc5x4UnormBlock:
            case Astc5x4SrgbBlock:
            case Astc5x4SfloatBlock:
            case Astc5x5UnormBlock:
            case Astc5x5SrgbBlock:
            case Astc5x5SfloatBlock:
            case Astc6x5UnormBlock:
            case Astc6x5SrgbBlock:
            case Astc6x5SfloatBlock:
            case Astc6x6UnormBlock:
            case Astc6x6SrgbBlock:
            case Astc6x6SfloatBlock:
            case Astc8x5UnormBlock:
            case Astc8x5SrgbBlock:
            case Astc8x5SfloatBlock:
            case Astc8x6UnormBlock:
            case Astc8x6SrgbBlock:
            case Astc8x6SfloatBlock:
            case Astc8x8UnormBlock:
            case Astc8x8SrgbBlock:
            case Astc8x8SfloatBlock:
            case Astc10x5UnormBlock:
            case Astc10x5SrgbBlock:
            case Astc10x5SfloatBlock:
            case Astc10x6UnormBlock:
            case Astc10x6SrgbBlock:
            case Astc10x6SfloatBlock:
            case Astc10x8UnormBlock:
            case Astc10x8SrgbBlock:
            case Astc10x8SfloatBlock:
            case Astc10x10UnormBlock:
            case Astc10x10SrgbBlock:
            case Astc10x10SfloatBlock:
            case Astc12x10UnormBlock:
            case Astc12x10SrgbBlock:
            case Astc12x10SfloatBlock:
            case Astc12x12UnormBlock:
            case Astc12x12SrgbBlock:
            case Astc12x12SfloatBlock:
            case Pvrtc12BppUnormBlockIMG:
            case Pvrtc14BppUnormBlockIMG:
            case Pvrtc22BppUnormBlockIMG:
            case Pvrtc24BppUnormBlockIMG:
            case Pvrtc12BppSrgbBlockIMG:
            case Pvrtc14BppSrgbBlockIMG:
            case Pvrtc22BppSrgbBlockIMG:
            case Pvrtc24BppSrgbBlockIMG:
                std::cout << "vec4"; // Compressed formats are sampled as vec4
                break;
            case Undefined:
            default:
                std::cout << "/* UNSUPPORTED FORMAT */";
                break;
            }
            std::cout << " (offset " << attr.offset << ")\n";
        }
    } // namespace VkUtils
}

} // namespace VkUtils
