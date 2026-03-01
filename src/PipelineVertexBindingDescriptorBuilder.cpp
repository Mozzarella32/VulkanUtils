#include "PipelineVertexBindingDescriptorBuilder.hpp"
#include "Structures.hpp"

#include <iostream>

namespace VkUtils {

void PipelineVertexBindingDescriptorBuilder::addBinding(
    VkVertexInputBindingDescription bindingDescription) {
    currentBinding = nextBinding++;
    bindingDescription.binding = currentBinding;
    bindingDescriptions.emplace_back(bindingDescription);
}

void PipelineVertexBindingDescriptorBuilder::addAttribute(
    VkVertexInputAttributeDescription attributeDescription) {
    attributeDescription.binding = currentBinding;
    attributeDescription.location = currentLocation++;
    attributeDescriptions.emplace_back(attributeDescription);
}

VkPipelineVertexInputStateCreateInfo PipelineVertexBindingDescriptorBuilder::getVertexInputInfo() {
    auto vertexInputInfo = VkBindings::Init<VkPipelineVertexInputStateCreateInfo>();
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
                  << (bind.inputRate == VK_VERTEX_INPUT_RATE_VERTEX ? "vertex" : "instance")
                  << " stride: " << bind.stride << "\n";
        for (size_t l = 0; l < attributeDescriptions.size(); l++) {
            const auto &attr = attributeDescriptions[l];
            if (attr.binding != b)
                continue;
            std::cout << "\tlayout(location = " << attr.location << ") ";

            switch (attr.format) {
            // Single component formats
            case VK_FORMAT_R8_UNORM:
            case VK_FORMAT_R8_SNORM:
            case VK_FORMAT_R8_USCALED:
            case VK_FORMAT_R8_SSCALED:
            case VK_FORMAT_R8_SRGB:
            case VK_FORMAT_R16_UNORM:
            case VK_FORMAT_R16_SNORM:
            case VK_FORMAT_R16_USCALED:
            case VK_FORMAT_R16_SSCALED:
            case VK_FORMAT_R16_SFLOAT:
            case VK_FORMAT_R32_SFLOAT:
            case VK_FORMAT_R64_SFLOAT:
                std::cout << "float";
                break;
            case VK_FORMAT_R8_UINT:
            case VK_FORMAT_R16_UINT:
            case VK_FORMAT_R32_UINT:
            case VK_FORMAT_R64_UINT:
                std::cout << "uint";
                break;
            case VK_FORMAT_R8_SINT:
            case VK_FORMAT_R16_SINT:
            case VK_FORMAT_R32_SINT:
            case VK_FORMAT_R64_SINT:
                std::cout << "int";
                break;
            // Two component formats (vec2)
            case VK_FORMAT_R8G8_UNORM:
            case VK_FORMAT_R8G8_SNORM:
            case VK_FORMAT_R8G8_USCALED:
            case VK_FORMAT_R8G8_SSCALED:
            case VK_FORMAT_R8G8_SRGB:
            case VK_FORMAT_R16G16_UNORM:
            case VK_FORMAT_R16G16_SNORM:
            case VK_FORMAT_R16G16_USCALED:
            case VK_FORMAT_R16G16_SSCALED:
            case VK_FORMAT_R16G16_SFLOAT:
            case VK_FORMAT_R32G32_SFLOAT:
            case VK_FORMAT_R64G64_SFLOAT:
                std::cout << "vec2";
                break;
            case VK_FORMAT_R8G8_UINT:
            case VK_FORMAT_R16G16_UINT:
            case VK_FORMAT_R32G32_UINT:
            case VK_FORMAT_R64G64_UINT:
                std::cout << "uvec2";
                break;
            case VK_FORMAT_R8G8_SINT:
            case VK_FORMAT_R16G16_SINT:
            case VK_FORMAT_R32G32_SINT:
            case VK_FORMAT_R64G64_SINT:
                std::cout << "ivec2";
                break;
            // Three component formats (vec3)
            case VK_FORMAT_R8G8B8_UNORM:
            case VK_FORMAT_R8G8B8_SNORM:
            case VK_FORMAT_R8G8B8_USCALED:
            case VK_FORMAT_R8G8B8_SSCALED:
            case VK_FORMAT_R8G8B8_SRGB:
            case VK_FORMAT_B8G8R8_UNORM:
            case VK_FORMAT_B8G8R8_SNORM:
            case VK_FORMAT_B8G8R8_USCALED:
            case VK_FORMAT_B8G8R8_SSCALED:
            case VK_FORMAT_B8G8R8_SRGB:
            case VK_FORMAT_R16G16B16_UNORM:
            case VK_FORMAT_R16G16B16_SNORM:
            case VK_FORMAT_R16G16B16_USCALED:
            case VK_FORMAT_R16G16B16_SSCALED:
            case VK_FORMAT_R16G16B16_SFLOAT:
            case VK_FORMAT_R32G32B32_SFLOAT:
            case VK_FORMAT_R64G64B64_SFLOAT:
            case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
            case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
                std::cout << "vec3";
                break;
            case VK_FORMAT_R8G8B8_UINT:
            case VK_FORMAT_B8G8R8_UINT:
            case VK_FORMAT_R16G16B16_UINT:
            case VK_FORMAT_R32G32B32_UINT:
            case VK_FORMAT_R64G64B64_UINT:
                std::cout << "uvec3";
                break;
            case VK_FORMAT_R8G8B8_SINT:
            case VK_FORMAT_B8G8R8_SINT:
            case VK_FORMAT_R16G16B16_SINT:
            case VK_FORMAT_R32G32B32_SINT:
            case VK_FORMAT_R64G64B64_SINT:
                std::cout << "ivec3";
                break;
            // Four component formats (vec4)
            case VK_FORMAT_R8G8B8A8_UNORM:
            case VK_FORMAT_R8G8B8A8_SNORM:
            case VK_FORMAT_R8G8B8A8_USCALED:
            case VK_FORMAT_R8G8B8A8_SSCALED:
            case VK_FORMAT_R8G8B8A8_SRGB:
            case VK_FORMAT_B8G8R8A8_UNORM:
            case VK_FORMAT_B8G8R8A8_SNORM:
            case VK_FORMAT_B8G8R8A8_USCALED:
            case VK_FORMAT_B8G8R8A8_SSCALED:
            case VK_FORMAT_B8G8R8A8_SRGB:
            case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
            case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
            case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
            case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
            case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
            case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
            case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
            case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
            case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
            case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
            case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
            case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
            case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
            case VK_FORMAT_R16G16B16A16_UNORM:
            case VK_FORMAT_R16G16B16A16_SNORM:
            case VK_FORMAT_R16G16B16A16_USCALED:
            case VK_FORMAT_R16G16B16A16_SSCALED:
            case VK_FORMAT_R16G16B16A16_SFLOAT:
            case VK_FORMAT_R32G32B32A32_SFLOAT:
            case VK_FORMAT_R64G64B64A64_SFLOAT:
            case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
            case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
            case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
            case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
            case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
            case VK_FORMAT_A1B5G5R5_UNORM_PACK16:
            case VK_FORMAT_A4R4G4B4_UNORM_PACK16:
            case VK_FORMAT_A4B4G4R4_UNORM_PACK16:
            case VK_FORMAT_R5G6B5_UNORM_PACK16:
            case VK_FORMAT_B5G6R5_UNORM_PACK16:
                std::cout << "vec4";
                break;
            case VK_FORMAT_R8G8B8A8_UINT:
            case VK_FORMAT_B8G8R8A8_UINT:
            case VK_FORMAT_A8B8G8R8_UINT_PACK32:
            case VK_FORMAT_A2R10G10B10_UINT_PACK32:
            case VK_FORMAT_A2B10G10R10_UINT_PACK32:
            case VK_FORMAT_R16G16B16A16_UINT:
            case VK_FORMAT_R32G32B32A32_UINT:
            case VK_FORMAT_R64G64B64A64_UINT:
                std::cout << "uvec4";
                break;
            case VK_FORMAT_R8G8B8A8_SINT:
            case VK_FORMAT_B8G8R8A8_SINT:
            case VK_FORMAT_A8B8G8R8_SINT_PACK32:
            case VK_FORMAT_A2R10G10B10_SINT_PACK32:
            case VK_FORMAT_A2B10G10R10_SINT_PACK32:
            case VK_FORMAT_R16G16B16A16_SINT:
            case VK_FORMAT_R32G32B32A32_SINT:
            case VK_FORMAT_R64G64B64A64_SINT:
                std::cout << "ivec4";
                break;
            // Depth/Stencil formats
            case VK_FORMAT_D16_UNORM:
            case VK_FORMAT_X8_D24_UNORM_PACK32:
            case VK_FORMAT_D32_SFLOAT:
                std::cout << "float"; // Depth
                break;
            case VK_FORMAT_S8_UINT:
                std::cout << "uint"; // Stencil
                break;
            case VK_FORMAT_D16_UNORM_S8_UINT:
            case VK_FORMAT_D24_UNORM_S8_UINT:
            case VK_FORMAT_D32_SFLOAT_S8_UINT:
                std::cout << "vec2"; // Depth + Stencil (typically accessed separately)
                break;
            // Special formats
            case VK_FORMAT_R4G4_UNORM_PACK8:
                std::cout << "vec2";
                break;
            case VK_FORMAT_A8_UNORM:
                std::cout << "float";
                break;
            case VK_FORMAT_R8_BOOL_ARM:
                std::cout << "bool";
                break;
            // Compressed formats (sampled as vec4)
            case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
            case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
            case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
            case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
            case VK_FORMAT_BC2_UNORM_BLOCK:
            case VK_FORMAT_BC2_SRGB_BLOCK:
            case VK_FORMAT_BC3_UNORM_BLOCK:
            case VK_FORMAT_BC3_SRGB_BLOCK:
            case VK_FORMAT_BC4_UNORM_BLOCK:
            case VK_FORMAT_BC4_SNORM_BLOCK:
            case VK_FORMAT_BC5_UNORM_BLOCK:
            case VK_FORMAT_BC5_SNORM_BLOCK:
            case VK_FORMAT_BC6H_UFLOAT_BLOCK:
            case VK_FORMAT_BC6H_SFLOAT_BLOCK:
            case VK_FORMAT_BC7_UNORM_BLOCK:
            case VK_FORMAT_BC7_SRGB_BLOCK:
            case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
            case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
            case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:
            case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:
            case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
            case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
            case VK_FORMAT_EAC_R11_UNORM_BLOCK:
            case VK_FORMAT_EAC_R11_SNORM_BLOCK:
            case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:
            case VK_FORMAT_EAC_R11G11_SNORM_BLOCK:
            case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:
            case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
            case VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK:
            case VK_FORMAT_ASTC_5x4_UNORM_BLOCK:
            case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:
            case VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK:
            case VK_FORMAT_ASTC_5x5_UNORM_BLOCK:
            case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:
            case VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK:
            case VK_FORMAT_ASTC_6x5_UNORM_BLOCK:
            case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:
            case VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK:
            case VK_FORMAT_ASTC_6x6_UNORM_BLOCK:
            case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:
            case VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK:
            case VK_FORMAT_ASTC_8x5_UNORM_BLOCK:
            case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:
            case VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK:
            case VK_FORMAT_ASTC_8x6_UNORM_BLOCK:
            case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:
            case VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK:
            case VK_FORMAT_ASTC_8x8_UNORM_BLOCK:
            case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:
            case VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK:
            case VK_FORMAT_ASTC_10x5_UNORM_BLOCK:
            case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:
            case VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK:
            case VK_FORMAT_ASTC_10x6_UNORM_BLOCK:
            case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:
            case VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK:
            case VK_FORMAT_ASTC_10x8_UNORM_BLOCK:
            case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:
            case VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK:
            case VK_FORMAT_ASTC_10x10_UNORM_BLOCK:
            case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:
            case VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK:
            case VK_FORMAT_ASTC_12x10_UNORM_BLOCK:
            case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:
            case VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK:
            case VK_FORMAT_ASTC_12x12_UNORM_BLOCK:
            case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:
            case VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK:
            case VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG:
            case VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG:
            case VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG:
            case VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG:
            case VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG:
            case VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG:
            case VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG:
            case VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG:
                std::cout << "vec4"; // Compressed formats are sampled as vec4
                break;
            case VK_FORMAT_UNDEFINED:
            default:
                std::cout << "/* UNSUPPORTED FORMAT */";
                break;
            }
            std::cout << " (offset " << attr.offset << ")\n";
        }
    }
}

} // namespace VkUtils
