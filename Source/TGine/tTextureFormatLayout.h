#pragma once
/* Copyright (c) 2017-2020 Hans-Kristian Arntzen
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include"vulkan/vulkan.hpp"
#include<vector>
//#include"tDevice.h"
namespace tEngine {
	class TextureFormatLayout
	{
	public:
		void set_1d(VkFormat format, uint32_t width, uint32_t array_layers = 1, uint32_t mip_levels = 1);
		void set_2d(VkFormat format, uint32_t width, uint32_t height, uint32_t array_layers = 1, uint32_t mip_levels = 1);
		void set_3d(VkFormat format, uint32_t width, uint32_t height, uint32_t depth, uint32_t mip_levels = 1);

		static uint32_t format_block_size(VkFormat format, VkImageAspectFlags aspect);
		static void format_block_dim(VkFormat format, uint32_t& width, uint32_t& height);
		static uint32_t num_miplevels(uint32_t width, uint32_t height = 1, uint32_t depth = 1);

		void set_buffer(void* buffer, size_t size);
		inline void* get_buffer()
		{
			return buffer;
		}

		uint32_t get_width(uint32_t mip = 0) const;
		uint32_t get_height(uint32_t mip = 0) const;
		uint32_t get_depth(uint32_t mip = 0) const;
		uint32_t get_levels() const;
		uint32_t get_layers() const;
		uint32_t get_block_stride() const;
		uint32_t get_block_dim_x() const;
		uint32_t get_block_dim_y() const;
		VkImageType get_image_type() const;
		VkFormat get_format() const;

		size_t get_required_size() const;

		size_t row_byte_stride(uint32_t row_length) const;
		size_t layer_byte_stride(uint32_t row_length, size_t row_byte_stride) const;

		inline size_t get_row_size(uint32_t mip) const
		{
			return mips[mip].block_row_length * block_stride;
		}

		inline size_t get_layer_size(uint32_t mip) const
		{
			return mips[mip].block_image_height * get_row_size(mip);
		}

		struct MipInfo
		{
			size_t offset = 0;
			uint32_t width = 1;
			uint32_t height = 1;
			uint32_t depth = 1;

			uint32_t block_image_height = 0;
			uint32_t block_row_length = 0;
			uint32_t image_height = 0;
			uint32_t row_length = 0;
		};

		const MipInfo& get_mip_info(uint32_t mip) const;

		inline void* data(uint32_t layer = 0, uint32_t mip = 0) const
		{
			assert(buffer);
			assert(buffer_size == required_size);
			auto& mip_info = mips[mip];
			uint8_t* slice = buffer + mip_info.offset;
			slice += block_stride * layer * mip_info.block_row_length * mip_info.block_image_height;
			return slice;
		}

		template <typename Attribute>
		inline Attribute* data_generic(uint32_t x, uint32_t y, uint32_t slice_index, uint32_t mip = 0) const
		{
			auto& mip_info = mips[mip];
			Attribute* slice = reinterpret_cast<Attribute*>(buffer + mip_info.offset);
			slice += slice_index * mip_info.block_row_length * mip_info.block_image_height;
			slice += y * mip_info.block_row_length;
			slice += x;
			return slice;
		}

		inline void* data_opaque(uint32_t x, uint32_t y, uint32_t slice_index, uint32_t mip = 0) const
		{
			auto& mip_info = mips[mip];
			uint8_t* slice = buffer + mip_info.offset;
			size_t off = slice_index * mip_info.block_row_length * mip_info.block_image_height;
			off += y * mip_info.block_row_length;
			off += x;
			return slice + off * block_stride;
		}

		template <typename Attribute>
		inline Attribute* data_generic() const
		{
			return data_generic<Attribute>(0, 0, 0, 0);
		}

		template <typename Attribute>
		inline Attribute* data_1d(uint32_t x, uint32_t layer = 0, uint32_t mip = 0) const
		{
			assert(sizeof(Attribute) == block_stride);
			assert(buffer);
			assert(image_type == VK_IMAGE_TYPE_1D);
			assert(buffer_size == required_size);
			return data_generic<Attribute>(x, 0, layer, mip);
		}

		template <typename Attribute>
		inline Attribute* data_2d(uint32_t x, uint32_t y, uint32_t layer = 0, uint32_t mip = 0) const
		{
			assert(sizeof(Attribute) == block_stride);
			assert(buffer);
			assert(image_type == VK_IMAGE_TYPE_2D);
			assert(buffer_size == required_size);
			return data_generic<Attribute>(x, y, layer, mip);
		}

		template <typename Attribute>
		inline Attribute* data_3d(uint32_t x, uint32_t y, uint32_t z, uint32_t mip = 0) const
		{
			assert(sizeof(Attribute) == block_stride);
			assert(buffer);
			assert(image_type == VK_IMAGE_TYPE_3D);
			assert(buffer_size == required_size);
			return data_generic<Attribute>(x, y, z, mip);
		}

		void build_buffer_image_copies(std::vector<vk::BufferImageCopy>& copies) const;

	private:
		uint8_t* buffer = nullptr;
		size_t buffer_size = 0;

		VkImageType image_type = VK_IMAGE_TYPE_MAX_ENUM;
		VkFormat format = VK_FORMAT_UNDEFINED;
		size_t required_size = 0;

		uint32_t block_stride = 1;
		uint32_t mip_levels = 1;
		uint32_t array_layers = 1;
		uint32_t block_dim_x = 1;
		uint32_t block_dim_y = 1;

		MipInfo mips[16];

		void fill_mipinfo(uint32_t width, uint32_t height, uint32_t depth);
	};
	enum class FormatCompressionType
	{
		Uncompressed,
		BC,
		ETC,
		ASTC
	};

	static inline FormatCompressionType format_compression_type(VkFormat format)
	{
		switch (format)
		{
		case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
		case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
		case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
		case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
		case VK_FORMAT_BC2_SRGB_BLOCK:
		case VK_FORMAT_BC2_UNORM_BLOCK:
		case VK_FORMAT_BC3_SRGB_BLOCK:
		case VK_FORMAT_BC3_UNORM_BLOCK:
		case VK_FORMAT_BC4_UNORM_BLOCK:
		case VK_FORMAT_BC4_SNORM_BLOCK:
		case VK_FORMAT_BC5_UNORM_BLOCK:
		case VK_FORMAT_BC5_SNORM_BLOCK:
		case VK_FORMAT_BC6H_SFLOAT_BLOCK:
		case VK_FORMAT_BC6H_UFLOAT_BLOCK:
		case VK_FORMAT_BC7_SRGB_BLOCK:
		case VK_FORMAT_BC7_UNORM_BLOCK:
			return FormatCompressionType::BC;

		case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:
		case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:
		case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
		case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
		case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
		case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
		case VK_FORMAT_EAC_R11G11_SNORM_BLOCK:
		case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:
		case VK_FORMAT_EAC_R11_SNORM_BLOCK:
		case VK_FORMAT_EAC_R11_UNORM_BLOCK:
			return FormatCompressionType::ETC;

#define astc_fmt(w, h) \
	case VK_FORMAT_ASTC_##w##x##h##_UNORM_BLOCK: \
	case VK_FORMAT_ASTC_##w##x##h##_SRGB_BLOCK: \
	case VK_FORMAT_ASTC_##w##x##h##_SFLOAT_BLOCK_EXT
		astc_fmt(4, 4):
			astc_fmt(5, 4) :
				astc_fmt(5, 5) :
				astc_fmt(6, 5) :
				astc_fmt(6, 6) :
				astc_fmt(8, 5) :
				astc_fmt(8, 6) :
				astc_fmt(8, 8) :
				astc_fmt(10, 5) :
				astc_fmt(10, 6) :
				astc_fmt(10, 8) :
				astc_fmt(10, 10) :
				astc_fmt(12, 10) :
				astc_fmt(12, 12) :
				return FormatCompressionType::ASTC;
#undef astc_fmt

		default:
			return FormatCompressionType::Uncompressed;
		}
	}

	static inline bool format_is_srgb(VkFormat format)
	{
		switch (format)
		{
		case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
		case VK_FORMAT_R8G8B8A8_SRGB:
		case VK_FORMAT_B8G8R8A8_SRGB:
		case VK_FORMAT_R8_SRGB:
		case VK_FORMAT_R8G8_SRGB:
		case VK_FORMAT_R8G8B8_SRGB:
		case VK_FORMAT_B8G8R8_SRGB:
		case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
		case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
		case VK_FORMAT_BC2_SRGB_BLOCK:
		case VK_FORMAT_BC3_SRGB_BLOCK:
		case VK_FORMAT_BC7_SRGB_BLOCK:
		case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
		case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:
		case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
		case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
		case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:
		case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:
		case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:
		case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:
		case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:
		case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:
		case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:
		case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:
		case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:
		case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:
		case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:
		case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:
		case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:
			return true;

		default:
			return false;
		}
	}

	static inline bool format_has_depth_aspect(VkFormat format)
	{
		switch (format)
		{
		case VK_FORMAT_D16_UNORM:
		case VK_FORMAT_D16_UNORM_S8_UINT:
		case VK_FORMAT_D24_UNORM_S8_UINT:
		case VK_FORMAT_D32_SFLOAT:
		case VK_FORMAT_X8_D24_UNORM_PACK32:
		case VK_FORMAT_D32_SFLOAT_S8_UINT:
			return true;

		default:
			return false;
		}
	}

	static inline bool format_has_stencil_aspect(VkFormat format)
	{
		switch (format)
		{
		case VK_FORMAT_D16_UNORM_S8_UINT:
		case VK_FORMAT_D24_UNORM_S8_UINT:
		case VK_FORMAT_D32_SFLOAT_S8_UINT:
		case VK_FORMAT_S8_UINT:
			return true;

		default:
			return false;
		}
	}

	static inline bool format_has_depth_or_stencil_aspect(VkFormat format)
	{
		return format_has_depth_aspect(format) || format_has_stencil_aspect(format);
	}

	static inline VkImageAspectFlags format_to_aspect_mask(VkFormat format)
	{
		switch (format)
		{
		case VK_FORMAT_UNDEFINED:
			return 0;

		case VK_FORMAT_S8_UINT:
			return VK_IMAGE_ASPECT_STENCIL_BIT;

		case VK_FORMAT_D16_UNORM_S8_UINT:
		case VK_FORMAT_D24_UNORM_S8_UINT:
		case VK_FORMAT_D32_SFLOAT_S8_UINT:
			return VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_DEPTH_BIT;

		case VK_FORMAT_D16_UNORM:
		case VK_FORMAT_D32_SFLOAT:
		case VK_FORMAT_X8_D24_UNORM_PACK32:
			return VK_IMAGE_ASPECT_DEPTH_BIT;

		default:
			return VK_IMAGE_ASPECT_COLOR_BIT;
		}
	}

	static inline void format_align_dim(VkFormat format, uint32_t& width, uint32_t& height)
	{
		uint32_t align_width, align_height;
		TextureFormatLayout::format_block_dim(format, align_width, align_height);
		width = ((width + align_width - 1) / align_width) * align_width;
		height = ((height + align_height - 1) / align_height) * align_height;
	}

	static inline void format_num_blocks(VkFormat format, uint32_t& width, uint32_t& height)
	{
		uint32_t align_width, align_height;
		TextureFormatLayout::format_block_dim(format, align_width, align_height);
		width = (width + align_width - 1) / align_width;
		height = (height + align_height - 1) / align_height;
	}

	static inline VkDeviceSize format_get_layer_size(VkFormat format, VkImageAspectFlags aspect, unsigned width, unsigned height, unsigned depth)
	{
		uint32_t blocks_x = width;
		uint32_t blocks_y = height;
		format_num_blocks(format, blocks_x, blocks_y);
		format_align_dim(format, width, height);

		VkDeviceSize size = TextureFormatLayout::format_block_size(format, aspect) * depth * blocks_x * blocks_y;
		return size;
	}

	enum class YCbCrFormat
	{
		YUV420P_3PLANE,
		YUV444P_3PLANE,
		YUV422P_3PLANE,
		Count
	};

	static inline unsigned format_ycbcr_num_planes(VkFormat format)
	{
		switch (format)
		{
		case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
		case VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM:
		case VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM:
		case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM:
		case VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM:
		case VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM:
		case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16:
		case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16:
		case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16:
		case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16:
		case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16:
		case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16:
			return 3;

		case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
		case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM:
		case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM:
		case VK_FORMAT_G16_B16R16_2PLANE_422_UNORM:
		case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:
		case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16:
		case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16:
		case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16:
			return 2;

		default:
			return 1;
		}
	}

	static inline unsigned format_ycbcr_num_planes(YCbCrFormat format)
	{
		switch (format)
		{
		case YCbCrFormat::YUV420P_3PLANE:
		case YCbCrFormat::YUV422P_3PLANE:
		case YCbCrFormat::YUV444P_3PLANE:
			return 3;

		default:
			return 0;
		}
	}

	static inline void format_ycbcr_downsample_dimensions(VkFormat format, VkImageAspectFlags aspect, uint32_t& width, uint32_t& height)
	{
		if (aspect == VK_IMAGE_ASPECT_PLANE_0_BIT)
			return;

		switch (format)
		{
#define fmt(x, sub0, sub1) \
	case VK_FORMAT_##x: \
		width >>= sub0; \
		height >>= sub1; \
		break

			fmt(G8_B8_R8_3PLANE_420_UNORM, 1, 1);
			fmt(G8_B8R8_2PLANE_420_UNORM, 1, 1);
			fmt(G8_B8_R8_3PLANE_422_UNORM, 1, 0);
			fmt(G8_B8R8_2PLANE_422_UNORM, 1, 0);
			fmt(G8_B8_R8_3PLANE_444_UNORM, 0, 0);

			fmt(G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16, 1, 1);
			fmt(G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16, 1, 0);
			fmt(G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16, 0, 0);
			fmt(G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16, 1, 1);
			fmt(G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16, 1, 0);

			fmt(G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16, 1, 1);
			fmt(G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16, 1, 0);
			fmt(G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16, 0, 0);
			fmt(G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16, 1, 1);
			fmt(G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16, 1, 0);

			fmt(G16_B16_R16_3PLANE_420_UNORM, 1, 1);
			fmt(G16_B16_R16_3PLANE_422_UNORM, 1, 0);
			fmt(G16_B16_R16_3PLANE_444_UNORM, 0, 0);
			fmt(G16_B16R16_2PLANE_420_UNORM, 1, 1);
			fmt(G16_B16R16_2PLANE_422_UNORM, 1, 0);

		default:
			break;
		}
#undef fmt
	}

	static inline unsigned format_ycbcr_downsample_ratio_log2(YCbCrFormat format, unsigned dim, unsigned plane)
	{
		switch (format)
		{
		case YCbCrFormat::YUV420P_3PLANE:
			return plane > 0 ? 1 : 0;
		case YCbCrFormat::YUV422P_3PLANE:
			return plane > 0 && dim == 0 ? 1 : 0;

		default:
			return 0;
		}
	}

	static inline VkFormat format_ycbcr_plane_vk_format(YCbCrFormat format, unsigned plane)
	{
		switch (format)
		{
		case YCbCrFormat::YUV420P_3PLANE:
			return VK_FORMAT_R8_UNORM;
		case YCbCrFormat::YUV422P_3PLANE:
			return plane > 0 ? VK_FORMAT_R8G8_UNORM : VK_FORMAT_R8_UNORM;
		case YCbCrFormat::YUV444P_3PLANE:
			return VK_FORMAT_R8_UNORM;

		default:
			return VK_FORMAT_UNDEFINED;
		}
	}

	static inline VkFormat format_ycbcr_planar_vk_format(YCbCrFormat format)
	{
		switch (format)
		{
		case YCbCrFormat::YUV420P_3PLANE:
			return VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;
		case YCbCrFormat::YUV422P_3PLANE:
			return VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM;
		case YCbCrFormat::YUV444P_3PLANE:
			return VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM;

		default:
			return VK_FORMAT_UNDEFINED;
		}
	}
	static inline VkPipelineStageFlags image_usage_to_possible_stages(VkImageUsageFlags usage)
	{
		VkPipelineStageFlags flags = 0;

		if (usage & (VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT))
			flags |= VK_PIPELINE_STAGE_TRANSFER_BIT;
		if (usage & VK_IMAGE_USAGE_SAMPLED_BIT)
			flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		if (usage & VK_IMAGE_USAGE_STORAGE_BIT)
			flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
			flags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
			flags |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		if (usage & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT)
			flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

		if (usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT)
		{
			VkPipelineStageFlags possible = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
				VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
				VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

			if (usage & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT)
				possible |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

			flags &= possible;
		}

		return flags;
	}

	static inline VkAccessFlags image_layout_to_possible_access(VkImageLayout layout)
	{
		switch (layout)
		{
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			return VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
			return VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			return VK_ACCESS_TRANSFER_READ_BIT;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			return VK_ACCESS_TRANSFER_WRITE_BIT;
		default:
			return ~0u;
		}
	}

	static inline VkAccessFlags image_usage_to_possible_access(VkImageUsageFlags usage)
	{
		VkAccessFlags flags = 0;

		if (usage & (VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT))
			flags |= VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
		if (usage & VK_IMAGE_USAGE_SAMPLED_BIT)
			flags |= VK_ACCESS_SHADER_READ_BIT;
		if (usage & VK_IMAGE_USAGE_STORAGE_BIT)
			flags |= VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
		if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
			flags |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
		if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
			flags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
		if (usage & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT)
			flags |= VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;

		// Transient attachments can only be attachments, and never other resources.
		if (usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT)
		{
			flags &= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
				VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
		}

		return flags;
	}

	static inline uint32_t image_num_miplevels(const VkExtent3D& extent)
	{
		uint32_t size = std::max(std::max(extent.width, extent.height), extent.depth);
		uint32_t levels = 0;
		while (size)
		{
			levels++;
			size >>= 1;
		}
		return levels;
	}

	static inline VkFormatFeatureFlags image_usage_to_features(VkImageUsageFlags usage)
	{
		VkFormatFeatureFlags flags = 0;
		if (usage & VK_IMAGE_USAGE_SAMPLED_BIT)
			flags |= VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;
		if (usage & VK_IMAGE_USAGE_STORAGE_BIT)
			flags |= VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT;
		if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
			flags |= VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT;
		if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
			flags |= VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

		return flags;
	}



}