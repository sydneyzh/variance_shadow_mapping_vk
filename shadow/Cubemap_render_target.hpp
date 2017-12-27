#pragma once
#include <vulkan/vulkan.hpp>
#include <Physical_device.hpp>
#include <Device.hpp>

class Cubemap_render_target
{
public:
    vk::Image image;
    vk::DeviceMemory mem;
    vk::ImageView view;
    vk::ImageView view_with_mip_levels;
    vk::ImageView views[6];
    vk::Format format;
    vk::Sampler sampler;
    vk::Sampler tex2d_array_sampler;
    vk::DescriptorImageInfo desc_image_info;
    vk::DescriptorImageInfo desc_image_infos[6];
    int mip_levels=1;
    Cubemap_render_target(base::Physical_device* p_phy_dev,
                          base::Device* p_dev,
                          const vk::Format format,
                          const vk::Extent2D extent,
                          const vk::ImageUsageFlags usage,
                          const vk::SampleCountFlagBits sample_count=vk::SampleCountFlagBits::e1,
                          // view
                          const vk::ImageAspectFlagBits aspect_flag=vk::ImageAspectFlagBits::eColor,
                          bool has_mip_levels=false,
                          bool separate_views=false,
                          // sampler 
                          bool create_sampler=false,
                          const vk::SamplerCreateInfo sampler_create_info={},
                          const vk::ImageLayout layout={},
                          bool tex2d_array_descriptors=false,
                          const vk::SamplerCreateInfo tex2d_sampler_info={})
        :p_phy_dev_(p_phy_dev),
        p_dev_(p_dev),
        format(format),
        has_mip_levels_(has_mip_levels),
        has_separate_views_(separate_views),
        has_tex_2d_descriptors_(tex2d_array_descriptors)
    {
        if (has_tex_2d_descriptors_) {
            assert(create_sampler);
            assert(has_separate_views_);
        }

        create_image_(extent, usage, sample_count);
        allocate_and_bind_memory_();
        create_view_(format, aspect_flag);
        if (create_sampler) create_sampler_(sampler_create_info, layout, tex2d_sampler_info);
    }
    ~Cubemap_render_target()
    {
        if (sampler) p_dev_->dev.destroySampler(sampler);
        if (has_tex_2d_descriptors_) {
            p_dev_->dev.destroySampler(tex2d_array_sampler);
        }
        if (view) p_dev_->dev.destroyImageView(view);
        if (has_mip_levels_ && view_with_mip_levels) p_dev_->dev.destroyImageView(view_with_mip_levels);
        if (has_separate_views_) {
            for (auto i=0; i < 6; i++) {
                if (views[i]) p_dev_->dev.destroyImageView(views[i]);
            }
        }
        if (image) p_dev_->dev.destroyImage(image);
        if (mem) p_dev_->dev.freeMemory(mem);
    }

private:
    base::Physical_device* p_phy_dev_;
    base::Device* p_dev_;
    bool has_mip_levels_;
    bool has_separate_views_;
    bool has_tex_2d_descriptors_;

    void create_image_(const vk::Extent2D extent, const vk::ImageUsageFlags& usage, const vk::SampleCountFlagBits& sample_count)
    {
        if (has_mip_levels_) mip_levels=floor(log2(std::max(extent.width, extent.height))) + 1;
        image=p_dev_->dev.createImage(
            vk::ImageCreateInfo(vk::ImageCreateFlagBits::eCubeCompatible,
                                vk::ImageType::e2D,
                                format,
                                {extent.width, extent.height, 1},
                                mip_levels,
                                6,
                                sample_count,
                                vk::ImageTiling::eOptimal,
                                usage));
    }
    void create_view_(const vk::Format& format, const vk::ImageAspectFlagBits aspect_flag)
    {
        // for framebuffer attachment
        view=p_dev_->dev.createImageView(
            vk::ImageViewCreateInfo({},
                                    image,
                                    vk::ImageViewType::eCube,
                                    format,
                                    vk::ComponentMapping(vk::ComponentSwizzle::eR,
                                                         vk::ComponentSwizzle::eG,
                                                         vk::ComponentSwizzle::eB,
                                                         vk::ComponentSwizzle::eA),
                                    vk::ImageSubresourceRange{aspect_flag,
                                    0, 1,
                                    0, 6}));
        if (has_mip_levels_) {
            view_with_mip_levels=p_dev_->dev.createImageView(
                vk::ImageViewCreateInfo({},
                                        image,
                                        vk::ImageViewType::eCube,
                                        format,
                                        vk::ComponentMapping(vk::ComponentSwizzle::eR,
                                                             vk::ComponentSwizzle::eG,
                                                             vk::ComponentSwizzle::eB,
                                                             vk::ComponentSwizzle::eA),
                                        vk::ImageSubresourceRange{aspect_flag,
                                        0, static_cast<uint32_t>(mip_levels),
                                        0, 6}));
        }
        if (has_separate_views_) {
            for (uint32_t i=0; i < 6; i++) {
                // for framebuffer attachment
                views[i]=p_dev_->dev.createImageView(
                    vk::ImageViewCreateInfo({},
                                            image,
                                            vk::ImageViewType::e2D,
                                            format,
                                            vk::ComponentMapping(vk::ComponentSwizzle::eR,
                                                                 vk::ComponentSwizzle::eG,
                                                                 vk::ComponentSwizzle::eB,
                                                                 vk::ComponentSwizzle::eA),
                                            vk::ImageSubresourceRange{aspect_flag,
                                            0, 1,
                                            i, 1}));
            }
        }
    }
    void allocate_and_bind_memory_()
    {
        vk::MemoryRequirements mem_reqs=p_dev_->dev.getImageMemoryRequirements(image);
        mem=p_dev_->dev.allocateMemory(
            vk::MemoryAllocateInfo(mem_reqs.size,
                                   p_phy_dev_->get_memory_type_index(mem_reqs.memoryTypeBits,
                                                                     vk::MemoryPropertyFlagBits::eDeviceLocal)));
        p_dev_->dev.bindImageMemory(image, mem, 0);
    }
    void create_sampler_(const vk::SamplerCreateInfo& sampler_create_info, const vk::ImageLayout& layout, const vk::SamplerCreateInfo& tex2d_array_sampler_info)
    {
        sampler=p_dev_->dev.createSampler(sampler_create_info);
        if (has_mip_levels_) {
            desc_image_info={sampler, view_with_mip_levels, layout};
        }
        else {
            desc_image_info={sampler, view, layout};
        }
        if (has_tex_2d_descriptors_) {
            tex2d_array_sampler=p_dev_->dev.createSampler(tex2d_array_sampler_info);
            for (auto i=0; i < 6; i++) {
                desc_image_infos[i]={tex2d_array_sampler, views[i], layout};
            }
        }
    }
};
