#pragma once
#include <Program_base.hpp>
#include <Render_pass.hpp>
#include <Buffer.hpp>
#include <Camera.hpp>
#include <Model.hpp>
#include <Aabb.hpp>
#include <Shader.hpp>
#include <Swapchain.hpp>
#include <Texture.hpp>

#include "Cubemap_render_target.hpp"
#include "Shell.hpp"

#include <queue>
#include <glm/gtc/type_ptr.hpp>

#include "shadow_map.vert.h"
#include "shadow_map.geom.h"
#include "shadow_map.frag.h"
#include "filter.vert.h"
#include "filter.frag.h"
#include "onscreen.vert.h"
#include "onscreen.frag.h"

class Program : public base::Program_base
{
public:
    Program(Prog_info *p_info,
            Shell *p_shell,
            const bool enable_validation,
            base::Camera *p_camera)
        : Program_base(p_info, p_shell, enable_validation),
        p_info_(p_info),
        p_shell_(p_shell),
        p_camera_(p_camera)
    {
        p_camera_->update_aspect(p_info->width(), p_info->height());
        req_phy_dev_features_.geometryShader=VK_TRUE;
    }

    ~Program() override
    {
        p_dev_->dev.waitIdle();
        destroy_pipelines_();
        destroy_descriptors_();
        destroy_frame_data_();
        destroy_swapchain_();
        destroy_offscreen_framebuffers_();
        destroy_render_passes_();
        destroy_shaders_();
        destroy_model_();
        destroy_command_pools_();
        destroy_back_buffers_();
    }

    void init() override
    {
        base::Program_base::init();
        init_back_buffers_();
        init_command_pools_();
        init_model_();
        init_shaders_();
        init_render_passes_();
        init_offscreen_framebuffers_();
        init_swapchain_();
        init_frame_data_();
        init_descriptors_();
        init_pipelines_();
    }

private:
    Prog_info * p_info_{nullptr};
    Shell *p_shell_{nullptr};
    base::Camera *p_camera_{nullptr};
    vk::Format depth_format_{vk::Format::eD16Unorm};
    vk::Format shadow_map_format_{vk::Format::eR32G32Sfloat};

    // ************************************************************************
    // back buffers
    // ************************************************************************

    struct Back_buffer
    {
        uint32_t swapchain_image_idx{0};
        vk::Semaphore swapchain_image_acquire_semaphore;
        vk::Semaphore onscreen_render_semaphore;
        vk::Fence present_queue_submit_fence;
    };
    std::deque<Back_buffer> back_buffers_;
    Back_buffer acquired_back_buf_;
    const uint32_t back_buf_count_=3;

    void init_back_buffers_()
    {
        for (auto i=0; i < back_buf_count_; i++) {
            Back_buffer back;
            back.swapchain_image_acquire_semaphore=p_dev_->dev.createSemaphore(vk::SemaphoreCreateInfo());
            back.onscreen_render_semaphore=p_dev_->dev.createSemaphore(vk::SemaphoreCreateInfo());
            back.present_queue_submit_fence=p_dev_->dev
                .createFence(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));
            back_buffers_.push_back(back);
        }
    }

    void destroy_back_buffers_()
    {
        while (!back_buffers_.empty()) {
            auto &back=back_buffers_.front();
            p_dev_->dev.destroySemaphore(back.swapchain_image_acquire_semaphore);
            p_dev_->dev.destroySemaphore(back.onscreen_render_semaphore);
            p_dev_->dev.destroyFence(back.present_queue_submit_fence);
            back_buffers_.pop_front();
        }
    }

    // ************************************************************************
    // command pools
    // ************************************************************************

    vk::CommandPool graphics_cmd_pool_;

    void init_command_pools_()
    {
        graphics_cmd_pool_=p_dev_->dev.createCommandPool(
            vk::CommandPoolCreateInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                                      p_phy_dev_->graphics_queue_family_idx));
    }

    void destroy_command_pools_()
    {
        p_dev_->dev.destroyCommandPool(graphics_cmd_pool_);
    }

    // ************************************************************************
    // model
    // ************************************************************************

    base::Model *p_model_{nullptr};

    void init_model_()
    {
        p_model_=new base::Model(p_phy_dev_, p_dev_, graphics_cmd_pool_);
        std::string model_dir=MODEL_DIR;
        //auto asset_path=model_dir + "/sibenik";
        //auto model_path=asset_path + "/sibenik_bubble.fbx";
        auto asset_path=model_dir + "/cornell";
        auto model_path=asset_path + "/cornell_angel.fbx";
        auto dummy_asset_path=model_dir + "/dummy";
        auto components=std::vector<base::Vertex_component >
        {
            base::VERT_COMP_POSITION,
            base::VERT_COMP_NORMAL
        };
        base::Vertex_layout layout(components);
        p_model_->load(model_path, layout);

        // update camera
        auto di=p_model_->aabb.get_diagonal();
        p_camera_->cam_far=10.f * glm::length(di);
        auto half_size=p_model_->aabb.get_half_size();
        p_camera_->eye_pos=half_size;
        p_camera_->target=half_size / 2.f;
        p_camera_->update();

        // update camera speed
        auto di_len=glm::length(di);
        p_shell_->pan_speed=di_len / 100.f;
        p_shell_->zoom_speed=di_len / 800.f;
    }

    void destroy_model_()
    {
        delete p_model_;
    }

    // ************************************************************************
    // shaders
    // ************************************************************************

    base::Shader *p_shadow_map_vs_;
    base::Shader *p_shadow_map_gs_;
    base::Shader *p_shadow_map_fs_;
    base::Shader *p_filter_vs_;
    base::Shader *p_filter_fs_;
    base::Shader *p_onscreen_vs_;
    base::Shader *p_onscreen_fs_;

    void init_shaders_()
    {
        p_shadow_map_vs_=new base::Shader(p_dev_, vk::ShaderStageFlagBits::eVertex);
        p_shadow_map_gs_=new base::Shader(p_dev_, vk::ShaderStageFlagBits::eGeometry);
        p_shadow_map_fs_=new base::Shader(p_dev_, vk::ShaderStageFlagBits::eFragment);
        p_filter_vs_=new base::Shader(p_dev_, vk::ShaderStageFlagBits::eVertex);
        p_filter_fs_=new base::Shader(p_dev_, vk::ShaderStageFlagBits::eFragment);
        p_onscreen_vs_=new base::Shader(p_dev_, vk::ShaderStageFlagBits::eVertex);
        p_onscreen_fs_=new base::Shader(p_dev_, vk::ShaderStageFlagBits::eFragment);
        p_shadow_map_vs_->generate(sizeof(shadow_map_vert), shadow_map_vert);
        p_shadow_map_gs_->generate(sizeof(shadow_map_geom), shadow_map_geom);
        p_shadow_map_fs_->generate(sizeof(shadow_map_frag), shadow_map_frag);
        p_filter_vs_->generate(sizeof(filter_vert), filter_vert);
        p_filter_fs_->generate(sizeof(filter_frag), filter_frag);
        p_onscreen_vs_->generate(sizeof(onscreen_vert), onscreen_vert);
        p_onscreen_fs_->generate(sizeof(onscreen_frag), onscreen_frag);
    }

    void destroy_shaders_()
    {
        delete p_shadow_map_vs_;
        delete p_shadow_map_gs_;
        delete p_shadow_map_fs_;
        delete p_filter_vs_;
        delete p_filter_fs_;
        delete p_onscreen_vs_;
        delete p_onscreen_fs_;
    }

    // ************************************************************************
    // render passes
    // ************************************************************************

    std::vector<vk::ClearValue> clear_values_={
        vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}),
        vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}),
        vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}),
        vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}),
        vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}),
        vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}),
        vk::ClearDepthStencilValue(1.0f, 0)};

    std::vector<vk::ClearValue> onscreen_clear_values_={
        vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}),
        vk::ClearDepthStencilValue(1.0f, 0)};

    base::Render_pass *p_shadow_rp_{nullptr};
    base::Render_pass *p_filter_rp_{nullptr};
    base::Render_pass *p_onscreen_rp_{nullptr};

    void init_render_passes_()
    {
        /********************* offscreen render pass ********************/

        {
            p_shadow_rp_=new base::Render_pass(p_dev_, 2, onscreen_clear_values_.data());
            p_filter_rp_=new base::Render_pass(p_dev_, 6, clear_values_.data());

            std::vector<vk::AttachmentDescription> attachments_shadow={
                {
                    {},
                    shadow_map_format_,
                    vk::SampleCountFlagBits::e1,
                    vk::AttachmentLoadOp::eClear,
                    vk::AttachmentStoreOp::eStore,
                    vk::AttachmentLoadOp::eDontCare,
                    vk::AttachmentStoreOp::eDontCare,
                    vk::ImageLayout::eUndefined,
                    vk::ImageLayout::eShaderReadOnlyOptimal
                }, {
                    {},
                    depth_format_,
                    vk::SampleCountFlagBits::e1,
                    vk::AttachmentLoadOp::eClear,
                    vk::AttachmentStoreOp::eDontCare,
                    vk::AttachmentLoadOp::eDontCare,
                    vk::AttachmentStoreOp::eDontCare,
                    vk::ImageLayout::eUndefined,
                    vk::ImageLayout::eDepthStencilAttachmentOptimal}};

            std::vector<vk::AttachmentDescription> attachments_filter(6, attachments_shadow[0]);

            vk::AttachmentReference ref_colors[6];
            for (uint32_t i=0; i < 6; i++) {
                ref_colors[i]={i, vk::ImageLayout::eColorAttachmentOptimal};
            }
            vk::AttachmentReference ref_depth{1, vk::ImageLayout::eDepthStencilAttachmentOptimal};

            std::vector<vk::SubpassDescription> subpass_descriptions={
                // shadow subpass 0 
                vk::SubpassDescription(vk::SubpassDescriptionFlags(),
                vk::PipelineBindPoint::eGraphics,
                0, nullptr, // input
                1, &ref_colors[0], // color
                nullptr, // resolve
                &ref_depth, // depth
                0, nullptr),// preserve

                // filter rp subpass 0 
                vk::SubpassDescription{
                vk::SubpassDescriptionFlags{},
                vk::PipelineBindPoint::eGraphics,
                0, nullptr,
                6, ref_colors, // color
                nullptr,
                nullptr,
                0, nullptr}
            };

            // layout transition between color attachment and shaderRead 
            vk::SubpassDependency dependencies[2]={
                vk::SubpassDependency(
                    VK_SUBPASS_EXTERNAL,
                    0,
                    vk::PipelineStageFlagBits::eBottomOfPipe,
                    vk::PipelineStageFlagBits::eColorAttachmentOutput,
                    vk::AccessFlagBits::eMemoryRead,
                    vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite,
                    vk::DependencyFlagBits::eByRegion),
                vk::SubpassDependency(
                    0,
                    VK_SUBPASS_EXTERNAL,
                    vk::PipelineStageFlagBits::eColorAttachmentOutput,
                    vk::PipelineStageFlagBits::eBottomOfPipe,
                    vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite,
                    vk::AccessFlagBits::eMemoryRead),
            };

            p_shadow_rp_->create(2,
                                 attachments_shadow.data(),
                                 1,
                                 subpass_descriptions.data(),
                                 2, dependencies);
            p_filter_rp_->create(6,
                                 attachments_filter.data(),
                                 1,
                                 &subpass_descriptions[1],
                                 2, dependencies);
        }

        /********************* onscreen render pass *********************/

        {
            p_onscreen_rp_=new base::Render_pass(p_dev_, 2, onscreen_clear_values_.data());

            std::vector<vk::AttachmentDescription> attachment_descriptions=
            {
                // color 
                {
                    {},
                    surface_format_.format,
                    vk::SampleCountFlagBits::e1,
                    vk::AttachmentLoadOp::eClear,
                    vk::AttachmentStoreOp::eStore,
                    vk::AttachmentLoadOp::eDontCare,
                    vk::AttachmentStoreOp::eDontCare,
                    vk::ImageLayout::eUndefined,
                    vk::ImageLayout::ePresentSrcKHR
                },
                // depth
                {
                    {},
                    depth_format_,
                    vk::SampleCountFlagBits::e1,
                    vk::AttachmentLoadOp::eClear,
                    vk::AttachmentStoreOp::eDontCare,
                    vk::AttachmentLoadOp::eDontCare,
                    vk::AttachmentStoreOp::eDontCare,
                    vk::ImageLayout::eUndefined,
                    vk::ImageLayout::eDepthStencilAttachmentOptimal
                },
            };

            static const vk::AttachmentReference ref_color
            {
                0,
                vk::ImageLayout::eColorAttachmentOptimal
            };
            static const vk::AttachmentReference ref_depth
            {
                1,
                vk::ImageLayout::eDepthStencilAttachmentOptimal
            };

            std::vector<vk::SubpassDescription> subpass_descriptions=
            {
                vk::SubpassDescription(
                    {},
                    vk::PipelineBindPoint::eGraphics,
                    0, nullptr, // input
                    1, &ref_color, // color
                    nullptr, // resolve
                    &ref_depth, // depth
                    0, nullptr // preserve
                )
            };

            /* subpass dependencies */

            std::vector<vk::SubpassDependency> dependencies=
            {
                {
                    VK_SUBPASS_EXTERNAL, // src
                    0, // dst
                    vk::PipelineStageFlagBits::eBottomOfPipe, // src stages
                    vk::PipelineStageFlagBits::eColorAttachmentOutput, // dst stages
                    vk::AccessFlagBits::eMemoryRead, // src access
                    vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite, // dst access
                    vk::DependencyFlagBits::eByRegion // dependency flags
                },
                {
                    0, // src
                    VK_SUBPASS_EXTERNAL, // dst
                    vk::PipelineStageFlagBits::eColorAttachmentOutput, // src stages
                    vk::PipelineStageFlagBits::eBottomOfPipe, // dst stages
                    vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite,
                    vk::AccessFlagBits::eMemoryRead, // dst access
                    vk::DependencyFlagBits::eByRegion // dependency flags
                }
            };

            p_onscreen_rp_->create(static_cast<uint32_t>(attachment_descriptions.size()),
                                   attachment_descriptions.data(),
                                   static_cast<uint32_t>(subpass_descriptions.size()),
                                   subpass_descriptions.data(),
                                   static_cast<uint32_t>(dependencies.size()),
                                   dependencies.data());

            // rp.framebuffer is set on frame
            p_shadow_rp_->rp_begin.renderArea.extent.width=p_info_->CUBEMAP_WIDTH;
            p_shadow_rp_->rp_begin.renderArea.extent.height=p_info_->CUBEMAP_HEIGHT;
            p_filter_rp_->rp_begin.renderArea.extent.width=p_info_->CUBEMAP_WIDTH;
            p_filter_rp_->rp_begin.renderArea.extent.height=p_info_->CUBEMAP_HEIGHT;
        }
    }

    void destroy_render_passes_()
    {
        delete p_shadow_rp_;
        delete p_filter_rp_;
        delete p_onscreen_rp_;
    }

    // ************************************************************************
    // offscreen framebuffers
    // ************************************************************************
    Cubemap_render_target *p_rt_shadow_depth_{nullptr};
    Cubemap_render_target *p_rt_shadow_color_a_{nullptr};
    Cubemap_render_target *p_rt_shadow_color_b_{nullptr};
    vk::Framebuffer shadow_fb_a_;
    vk::Framebuffer shadow_fb_b_;
    vk::Framebuffer filter_fb_a_;
    vk::Framebuffer filter_fb_b_;

    vk::Viewport offscreen_viewport_;
    vk::Rect2D offscreen_scissor_;

    void init_offscreen_framebuffers_()
    {
        offscreen_viewport_.minDepth=0.f;
        offscreen_viewport_.maxDepth=1.f;
        offscreen_viewport_.width=p_info_->CUBEMAP_WIDTH;
        offscreen_viewport_.height=p_info_->CUBEMAP_HEIGHT;
        offscreen_scissor_.extent.width=p_info_->CUBEMAP_WIDTH;
        offscreen_scissor_.extent.height=p_info_->CUBEMAP_HEIGHT;

        auto linear_filter_sampler=vk::SamplerCreateInfo({},
                                                           vk::Filter::eLinear,
                                                           vk::Filter::eLinear,
                                                           vk::SamplerMipmapMode::eLinear,
                                                           vk::SamplerAddressMode::eClampToEdge,
                                                           vk::SamplerAddressMode::eClampToEdge,
                                                           vk::SamplerAddressMode::eClampToEdge,
                                                           0.f,
                                                           VK_FALSE,
                                                           1.f,
                                                           VK_FALSE,
                                                           vk::CompareOp::eNever,
                                                           0.f, 1.f);

        p_rt_shadow_depth_=new Cubemap_render_target(p_phy_dev_,
                                                     p_dev_,
                                                     depth_format_,
                                                     {p_info_->CUBEMAP_WIDTH, p_info_->CUBEMAP_HEIGHT},
                                                     vk::ImageUsageFlagBits::eDepthStencilAttachment,
                                                     vk::SampleCountFlagBits::e1,
                                                     vk::ImageAspectFlagBits::eDepth);


        p_rt_shadow_color_a_=new Cubemap_render_target(p_phy_dev_,
                                                       p_dev_,
                                                       shadow_map_format_,
                                                       {p_info_->CUBEMAP_WIDTH, p_info_->CUBEMAP_HEIGHT},
                                                       vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
                                                       vk::SampleCountFlagBits::e1,
                                                       vk::ImageAspectFlagBits::eColor,
                                                       false, // has mip levels
                                                       true, // separate views
                                                       true, // create sampler
                                                       linear_filter_sampler, // cubemap uses linear filter
                                                       vk::ImageLayout::eShaderReadOnlyOptimal,
                                                       true, // tex2d array descriptors 
                                                       linear_filter_sampler); // filter pass use linear filter to improve gaussian blur quality 
        p_rt_shadow_color_b_=new Cubemap_render_target(p_phy_dev_,
                                                       p_dev_,
                                                       shadow_map_format_,
                                                       {p_info_->CUBEMAP_WIDTH, p_info_->CUBEMAP_HEIGHT},
                                                       vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
                                                       vk::SampleCountFlagBits::e1,
                                                       vk::ImageAspectFlagBits::eColor,
                                                       false, // has mip levels
                                                       true, // separate views
                                                       true, // create sampler
                                                       linear_filter_sampler, // cubemap uses linear filter
                                                       vk::ImageLayout::eShaderReadOnlyOptimal,
                                                       true, // tex 2d array descriptors 
                                                       linear_filter_sampler); // filter pass use linear filter to improve gaussian blur quality

        vk::ImageView shadow_pass_attachments_a[2]=
        {
            p_rt_shadow_color_a_->view,
            p_rt_shadow_depth_->view,
        };
        vk::ImageView filter_pass_attachments_a[6]=
        {
            p_rt_shadow_color_a_->views[0],
            p_rt_shadow_color_a_->views[1],
            p_rt_shadow_color_a_->views[2],
            p_rt_shadow_color_a_->views[3],
            p_rt_shadow_color_a_->views[4],
            p_rt_shadow_color_a_->views[5]
        };
        vk::ImageView shadow_pass_attachments_b[2]=
        {
            p_rt_shadow_color_b_->view,
            p_rt_shadow_depth_->view,
        };
        vk::ImageView filter_pass_attachments_b[6]=
        {
            p_rt_shadow_color_b_->views[0],
            p_rt_shadow_color_b_->views[1],
            p_rt_shadow_color_b_->views[2],
            p_rt_shadow_color_b_->views[3],
            p_rt_shadow_color_b_->views[4],
            p_rt_shadow_color_b_->views[5]
        };
        shadow_fb_a_=p_dev_->dev.createFramebuffer(
            vk::FramebufferCreateInfo({},
                                      p_shadow_rp_->rp,
                                      2,
                                      shadow_pass_attachments_a,
                                      p_info_->CUBEMAP_WIDTH,
                                      p_info_->CUBEMAP_HEIGHT,
                                      6)); // layers
        shadow_fb_b_=p_dev_->dev.createFramebuffer(
            vk::FramebufferCreateInfo({},
                                      p_shadow_rp_->rp,
                                      2,
                                      shadow_pass_attachments_b,
                                      p_info_->CUBEMAP_WIDTH,
                                      p_info_->CUBEMAP_HEIGHT,
                                      6)); // layers

        filter_fb_a_=p_dev_->dev.createFramebuffer(
            vk::FramebufferCreateInfo({},
                                      p_filter_rp_->rp,
                                      6,
                                      filter_pass_attachments_a,
                                      p_info_->CUBEMAP_WIDTH,
                                      p_info_->CUBEMAP_HEIGHT,
                                      1));
        filter_fb_b_=p_dev_->dev.createFramebuffer(
            vk::FramebufferCreateInfo({},
                                      p_filter_rp_->rp,
                                      6,
                                      filter_pass_attachments_b,
                                      p_info_->CUBEMAP_WIDTH,
                                      p_info_->CUBEMAP_HEIGHT,
                                      1));
    }

    void destroy_offscreen_framebuffers_()
    {
        p_dev_->dev.destroyFramebuffer(shadow_fb_a_);
        p_dev_->dev.destroyFramebuffer(shadow_fb_b_);
        p_dev_->dev.destroyFramebuffer(filter_fb_a_);
        p_dev_->dev.destroyFramebuffer(filter_fb_b_);
        delete p_rt_shadow_depth_;
        delete p_rt_shadow_color_a_;
        delete p_rt_shadow_color_b_;
    }

    // ************************************************************************
    // swapchain
    // ************************************************************************

    base::Swapchain *p_swapchain_{nullptr};

    void init_swapchain_()
    {
        p_swapchain_=new base::Swapchain(p_phy_dev_,
                                         p_dev_,
                                         surface_,
                                         surface_format_,
                                         depth_format_,
                                         back_buf_count_,
                                         p_onscreen_rp_);
        p_swapchain_->resize(p_info_->width(), p_info_->height());
        auto extent=p_swapchain_->curr_extent();
        if (extent.width != p_info_->width() || extent.height != p_info_->height()) {
            p_info_->on_resize(extent.width, extent.height);
        }
    }

    void destroy_swapchain_()
    {
        p_swapchain_->detach();
        delete p_swapchain_;
    }

    // ************************************************************************
    // Frame data
    // ************************************************************************

    struct Global_uniforms
    {
        glm::mat4 view;
        glm::mat4 model;
        glm::mat4 normal;
        glm::mat4 projection;
        glm::mat4 clip;
    } global_uniforms_;

    struct PLight_info
    {
        glm::vec3 pos;
        float range;
        glm::mat4 view0;
        glm::mat4 projection;
    } plight_info_;

    // push constants

    struct Filter_info
    {
        glm::vec2 direction;
        float resolution;
        float inv_resolution;
    } filter_info_;

    struct Frame_data
    {
        base::Buffer *p_global_uniforms{nullptr};
        base::Buffer *p_plight_info{nullptr};
        vk::DescriptorSet desc_set;

        vk::CommandBuffer cmd_buffer;
        vk::Fence submit_fence;
    };
    std::vector<Frame_data> frame_data_vec_;
    vk::DeviceMemory global_uniforms_mem_;
    vk::DeviceMemory plight_info_mem_;
    uint32_t frame_data_count_{0};
    uint32_t frame_data_idx_{0};

    vk::CommandBufferBeginInfo cmd_begin_info_;
    vk::SubmitInfo cmd_submit_info_;
    vk::PipelineStageFlags wait_stages_;

    void init_frame_data_()
    {
        frame_data_count_=back_buf_count_;
        frame_data_vec_.resize(frame_data_count_);

        // p_global_uniforms 
        {
            std::vector<base::Buffer *> p_bufs(frame_data_count_);
            vk::MemoryPropertyFlags host_visible_coherent{vk::MemoryPropertyFlagBits::eHostVisible |
                vk::MemoryPropertyFlagBits::eHostCoherent};
            vk::SharingMode sharing_mode{vk::SharingMode::eExclusive};
            uint32_t queue_family_count=1;
            uint32_t *p_queue_family=&p_phy_dev_->graphics_queue_family_idx;

            uint32_t idx=0;
            for (auto &data : frame_data_vec_) {
                data.p_global_uniforms=new base::Buffer(p_dev_,
                                                        vk::BufferUsageFlagBits::eUniformBuffer,
                                                        host_visible_coherent,
                                                        sizeof(Global_uniforms),
                                                        sharing_mode,
                                                        queue_family_count,
                                                        p_queue_family);
                data.p_global_uniforms->update_desc_buf_info(0, VK_WHOLE_SIZE);
                p_bufs[idx++]=data.p_global_uniforms;
            }
            base::allocate_and_bind_buffer_memory(p_phy_dev_,
                                                  p_dev_,
                                                  global_uniforms_mem_,
                                                  frame_data_count_,
                                                  p_bufs.data());
        }
        // p_plight_info
        {
            std::vector<base::Buffer *> p_bufs(frame_data_count_);
            vk::MemoryPropertyFlags host_visible_coherent{vk::MemoryPropertyFlagBits::eHostVisible |
                vk::MemoryPropertyFlagBits::eHostCoherent};
            vk::SharingMode sharing_mode{vk::SharingMode::eExclusive};
            uint32_t queue_family_count=1;
            uint32_t *p_queue_family=&p_phy_dev_->graphics_queue_family_idx;

            uint32_t idx=0;
            for (auto &data : frame_data_vec_) {
                data.p_plight_info=new base::Buffer(p_dev_,
                                                    vk::BufferUsageFlagBits::eUniformBuffer,
                                                    host_visible_coherent,
                                                    sizeof(PLight_info),
                                                    sharing_mode,
                                                    queue_family_count,
                                                    p_queue_family);
                data.p_plight_info->update_desc_buf_info(0, VK_WHOLE_SIZE);
                p_bufs[idx++]=data.p_plight_info;
            }
            base::allocate_and_bind_buffer_memory(p_phy_dev_,
                                                  p_dev_,
                                                  plight_info_mem_,
                                                  frame_data_count_,
                                                  p_bufs.data());
        }

        // cmd buffers
        {
            std::vector<vk::CommandBuffer> graphics_cmd_buffers(frame_data_count_);
            graphics_cmd_buffers=p_dev_->dev.allocateCommandBuffers(
                vk::CommandBufferAllocateInfo(graphics_cmd_pool_,
                                              vk::CommandBufferLevel::ePrimary,
                                              static_cast<uint32_t>(graphics_cmd_buffers.size())));
            uint32_t gidx=0;
            for (auto &data : frame_data_vec_) {
                data.cmd_buffer=graphics_cmd_buffers[gidx++];
                data.submit_fence=
                    p_dev_->dev.createFence(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));
            }
        }

        // cmd info
        {
            cmd_begin_info_=vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
            wait_stages_=vk::PipelineStageFlagBits::eColorAttachmentOutput;
            cmd_submit_info_=
            {
                1, nullptr,
                &wait_stages_,
                1, nullptr,
                1, nullptr
            };

        }
    }

    void destroy_frame_data_()
    {
        p_dev_->dev.freeMemory(global_uniforms_mem_);
        p_dev_->dev.freeMemory(plight_info_mem_);
        for (auto &data : frame_data_vec_) {
            delete data.p_global_uniforms;
            delete data.p_plight_info;
            p_dev_->dev.destroyFence(data.submit_fence);
        }
    }

    // ************************************************************************
    // descriptor sets
    // ************************************************************************

    vk::DescriptorPool desc_pool_;

    struct Descriptor_set_layouts
    {
        vk::DescriptorSetLayout frame_data; // global_uniform, plight_info
        vk::DescriptorSetLayout cubemap2d; // 1 texture2d array
    } desc_set_layouts_;

    struct Descriptor_sets
    {
        vk::DescriptorSet cubemap2d_a;
        vk::DescriptorSet cubemap2d_b;
    } desc_sets_;

    std::vector<vk::DescriptorSet> desc_sets_onscreen_;

    struct Pipeline_layouts
    {
        vk::PipelineLayout shadow; // frame data
        vk::PipelineLayout filter;
        vk::PipelineLayout onscreen; // frame data, cubemap2d
    } pipeline_layouts_;

    void init_descriptors_()
    {
        uint32_t sampler_descriptor_count=0;

        // layouts
        {
            // frame data
            vk::DescriptorSetLayoutBinding bindings[2]=
            {
                vk::DescriptorSetLayoutBinding{
                0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eGeometry | vk::ShaderStageFlagBits::eFragment
            },
                vk::DescriptorSetLayoutBinding{
                1, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eGeometry | vk::ShaderStageFlagBits::eFragment
            }};
            desc_set_layouts_.frame_data=p_dev_->dev.createDescriptorSetLayout(
                vk::DescriptorSetLayoutCreateInfo({}, 2, bindings));

            // cubemap2d 
            // 2d array
            bindings[0]=vk::DescriptorSetLayoutBinding{
                0, vk::DescriptorType::eCombinedImageSampler, 6, vk::ShaderStageFlagBits::eFragment
            };
            // cubemap
            bindings[1]=vk::DescriptorSetLayoutBinding{
                1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment
            };
            desc_set_layouts_.cubemap2d=p_dev_->dev.createDescriptorSetLayout(
                vk::DescriptorSetLayoutCreateInfo({}, 2, bindings));
            sampler_descriptor_count+=14;
        }

        // desc pool
        {
            std::vector<vk::DescriptorPoolSize> pool_sizes
            {
                vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, frame_data_count_ * 2),
                vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, sampler_descriptor_count)
            };

            desc_pool_=p_dev_->dev.createDescriptorPool(vk::DescriptorPoolCreateInfo({},
                                                                                     frame_data_count_ * 2 + sampler_descriptor_count,
                                                                                     static_cast<uint32_t>(pool_sizes.size()),
                                                                                     pool_sizes.data()));
        }

        // allocate and write desc sets
        {
            std::vector<vk::DescriptorSetLayout> set_layouts;
            std::vector<vk::WriteDescriptorSet> writes;

            for (auto i=0; i < frame_data_count_; i++) {
                set_layouts.emplace_back(desc_set_layouts_.frame_data);
            }
            set_layouts.emplace_back(desc_set_layouts_.cubemap2d); //a
            set_layouts.emplace_back(desc_set_layouts_.cubemap2d); //b

            std::vector<vk::DescriptorSet> desc_sets=p_dev_->dev.allocateDescriptorSets(
                vk::DescriptorSetAllocateInfo(
                    desc_pool_, static_cast<uint32_t>(set_layouts.size()), set_layouts.data()));

            uint32_t idx=0;
            for (auto &data : frame_data_vec_) {
                data.desc_set=desc_sets[idx++];

                writes.emplace_back(data.desc_set,
                                    0, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr,
                                    &data.p_global_uniforms->desc_buf_info, nullptr);
                writes.emplace_back(data.desc_set,
                                    1, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr,
                                    &data.p_plight_info->desc_buf_info, nullptr);
            }

            desc_sets_.cubemap2d_a=desc_sets[idx++];
            desc_sets_.cubemap2d_b=desc_sets[idx++];

            // cubemap2d a
            writes.emplace_back(desc_sets_.cubemap2d_a,
                                0, 0, 6,
                                vk::DescriptorType::eCombinedImageSampler,
                                p_rt_shadow_color_a_->desc_image_infos, nullptr, nullptr);
            writes.emplace_back(desc_sets_.cubemap2d_a,
                                1, 0, 1,
                                vk::DescriptorType::eCombinedImageSampler,
                                &p_rt_shadow_color_a_->desc_image_info, nullptr, nullptr);

            // cubemap2d b
            writes.emplace_back(desc_sets_.cubemap2d_b,
                                0, 0, 6,
                                vk::DescriptorType::eCombinedImageSampler,
                                p_rt_shadow_color_b_->desc_image_infos, nullptr, nullptr);
            writes.emplace_back(desc_sets_.cubemap2d_b,
                                1, 0, 1,
                                vk::DescriptorType::eCombinedImageSampler,
                                &p_rt_shadow_color_b_->desc_image_info, nullptr, nullptr);

            p_dev_->dev.updateDescriptorSets(static_cast<uint32_t>(writes.size()),
                                             writes.data(), 0, nullptr);
            writes.clear();

            desc_sets_onscreen_.resize(2); // frame data, cubemap
        }

        // pipeline layouts
        {
            pipeline_layouts_.shadow=p_dev_->dev.createPipelineLayout(
                vk::PipelineLayoutCreateInfo({}, 1, &desc_set_layouts_.frame_data));

            auto range=vk::PushConstantRange(vk::ShaderStageFlagBits::eFragment,
                                        0, sizeof(filter_info_));
            pipeline_layouts_.filter=p_dev_->dev.createPipelineLayout(
                vk::PipelineLayoutCreateInfo({}, 1, &desc_set_layouts_.cubemap2d,
                                             1, &range));
            vk::DescriptorSetLayout set_layouts[2]={
                desc_set_layouts_.frame_data,
                desc_set_layouts_.cubemap2d,
            };
            pipeline_layouts_.onscreen=p_dev_->dev.createPipelineLayout(
                vk::PipelineLayoutCreateInfo({}, 2, set_layouts));
        }
    }

    void destroy_descriptors_()
    {
        p_dev_->dev.destroyDescriptorPool(desc_pool_);
        p_dev_->dev.destroyDescriptorSetLayout(desc_set_layouts_.frame_data);
        p_dev_->dev.destroyDescriptorSetLayout(desc_set_layouts_.cubemap2d);
        p_dev_->dev.destroyPipelineLayout(pipeline_layouts_.shadow);
        p_dev_->dev.destroyPipelineLayout(pipeline_layouts_.filter);
        p_dev_->dev.destroyPipelineLayout(pipeline_layouts_.onscreen);
    }

    // ************************************************************************
    // pipelines
    // ************************************************************************

    struct Pipelines
    {
        vk::Pipeline shadow;
        vk::Pipeline filter;
        vk::Pipeline onscreen;
    } pipelines_;

    void init_pipelines_()
    {
        //pipeline shadow

        vk::PipelineInputAssemblyStateCreateInfo input_assembly_state(
            {},
            vk::PrimitiveTopology::eTriangleList, // topology
            VK_FALSE // primitive restart enable
        );

        vk::PipelineRasterizationStateCreateInfo rasterization_state(
            {},
            VK_FALSE, // depth clamp enable
            VK_FALSE, // rasterizer discard
            vk::PolygonMode::eFill, // polygon mode
            vk::CullModeFlagBits::eBack, // cull mode
            //vk::FrontFace::eCounterClockwise, // front face
            vk::FrontFace::eClockwise, // front face
            VK_FALSE, // depth bias
            0, 0, 0, 1.f);

        vk::PipelineDepthStencilStateCreateInfo depth_stencil_state(
            {},
            VK_TRUE, // depth test enable
            VK_TRUE, // depth write enable
            vk::CompareOp::eLessOrEqual, // depth compare op
            VK_FALSE, // depth bounds test enable
            VK_FALSE, // stencil test enable
            vk::StencilOpState(),
            vk::StencilOpState(),
            0.f, // min depth bounds
            0.f); // max depth bounds

        vk::PipelineColorBlendAttachmentState blend_attachment_state;
        blend_attachment_state.blendEnable=VK_FALSE;
        blend_attachment_state.colorWriteMask=
            vk::ColorComponentFlagBits::eR |
            vk::ColorComponentFlagBits::eG |
            vk::ColorComponentFlagBits::eB |
            vk::ColorComponentFlagBits::eA;

        vk::PipelineColorBlendStateCreateInfo color_blend_state(
            {},
            VK_FALSE,  // logic op enable
            vk::LogicOp::eClear, // logic op
            1, // attachment count
            &blend_attachment_state, // attachments
            std::array<float, 4> {1.f, 1.f, 1.f, 1.f} // blend constants
        );

        vk::PipelineMultisampleStateCreateInfo multisample_state(
            {},
            vk::SampleCountFlagBits::e1, // sample count
            VK_FALSE, // sample shading enable
            0.f, // min sample shading
            nullptr, // sample mask
            VK_FALSE, // alpha to coverage enable
            VK_FALSE);// alpha to one enable

        std::vector<vk::DynamicState> dynamic_states=
        {
            vk::DynamicState::eViewport,
            vk::DynamicState::eScissor
        };
        vk::PipelineDynamicStateCreateInfo dynamic_state_ci(
            {},
            2,
            dynamic_states.data());

        vk::PipelineViewportStateCreateInfo viewport_state(
            {},
            1, nullptr,
            1, nullptr);

        vk::PipelineShaderStageCreateInfo shader_stages[3];
        shader_stages[0]=p_shadow_map_vs_->create_pipeline_stage_info();
        shader_stages[1]=p_shadow_map_gs_->create_pipeline_stage_info();
        shader_stages[2]=p_shadow_map_fs_->create_pipeline_stage_info();

        vk::PipelineVertexInputStateCreateInfo vertex_input_state(
            {},
            1,
            &p_model_->vi_binding,
            1, // pos
            p_model_->vi_attribs.data());

        vk::GraphicsPipelineCreateInfo pipeline_ci(
            {},
            3,
            shader_stages,
            &vertex_input_state,
            &input_assembly_state,
            nullptr,
            &viewport_state,
            &rasterization_state,
            &multisample_state,
            &depth_stencil_state,
            &color_blend_state,
            &dynamic_state_ci,
            pipeline_layouts_.shadow,
            p_shadow_rp_->rp,
            0); // subpass

        pipelines_.shadow=p_dev_->dev.createGraphicsPipeline(
            nullptr, pipeline_ci);

        /* onscreen */

        vertex_input_state.vertexAttributeDescriptionCount=2; // pos, normal
        rasterization_state.frontFace=vk::FrontFace::eCounterClockwise;

        pipeline_ci.stageCount=2;
        shader_stages[0]=p_onscreen_vs_->create_pipeline_stage_info();
        shader_stages[1]=p_onscreen_fs_->create_pipeline_stage_info();

        color_blend_state.attachmentCount=1;

        pipeline_ci.layout=pipeline_layouts_.onscreen;
        pipeline_ci.renderPass=p_onscreen_rp_->rp;
        pipelines_.onscreen=p_dev_->dev.createGraphicsPipeline(nullptr, pipeline_ci);

        /* filter */

        depth_stencil_state.depthTestEnable=VK_FALSE;
        depth_stencil_state.depthWriteEnable=VK_FALSE;

        auto empty_vi=vk::PipelineVertexInputStateCreateInfo();
        pipeline_ci.pVertexInputState=&empty_vi;

        shader_stages[0]=p_filter_vs_->create_pipeline_stage_info();
        shader_stages[1]=p_filter_fs_->create_pipeline_stage_info();

        std::vector<vk::PipelineColorBlendAttachmentState> blend_attachment_states(6, blend_attachment_state);
        color_blend_state.attachmentCount=6;
        color_blend_state.pAttachments=blend_attachment_states.data();

        pipeline_ci.layout=pipeline_layouts_.filter;
        pipeline_ci.renderPass=p_filter_rp_->rp;
        pipelines_.filter=p_dev_->dev.createGraphicsPipeline(nullptr, pipeline_ci);
    }

    void destroy_pipelines_()
    {
        p_dev_->dev.destroyPipeline(pipelines_.shadow);
        p_dev_->dev.destroyPipeline(pipelines_.filter);
        p_dev_->dev.destroyPipeline(pipelines_.onscreen);
    }

    // ************************************************************************
    // on frame
    // ************************************************************************

    const glm::vec3 posx{1.f, 0.f, 0.f};
    const glm::vec3 posy{0.f, 1.f, 0.f};
    const float fovy{1.570796f}; // 90 deg

    void update_uniforms_(Frame_data &data, float elapsed_time)
    {
        // update host data
        global_uniforms_.view=p_camera_->view;
        global_uniforms_.model=p_model_->model_matrix;
        global_uniforms_.normal=p_model_->normal_matrix;
        global_uniforms_.projection=p_camera_->projection;
        global_uniforms_.clip=p_camera_->clip;

        if (!p_info_->pause_light) {
            const float radius=8.f;
            float t=elapsed_time / 3.f;
            plight_info_.pos={radius * cos(t), 5.f, radius * sin(t)};
        }
        plight_info_.range=1000.f;
        plight_info_.view0=glm::lookAt(plight_info_.pos, plight_info_.pos + posx, posy);
        plight_info_.projection=glm::perspective(fovy, 1.f, 0.1f, plight_info_.range);

        // memcpy to host visible memory
        {
            auto mapped=reinterpret_cast<Global_uniforms *>(data.p_global_uniforms->mapped);
            memcpy(mapped, &global_uniforms_, sizeof(Global_uniforms));
        }
        {
            auto mapped=reinterpret_cast<PLight_info *>(data.p_plight_info->mapped);
            memcpy(mapped, &plight_info_, sizeof(plight_info_));
        }

        // push constants
        filter_info_.resolution=static_cast<float>(p_info_->CUBEMAP_WIDTH);
        filter_info_.inv_resolution=1.f / filter_info_.resolution;
    }

    void acquire_back_buffer_() override
    {
        auto &back=back_buffers_.front();

        p_dev_->dev.waitForFences(1, &back.present_queue_submit_fence, VK_TRUE, UINT64_MAX);
        p_dev_->dev.resetFences(1, &back.present_queue_submit_fence);

        detect_window_resize_();

        vk::Result res=vk::Result::eTimeout;
        while (res != vk::Result::eSuccess) {

            res=p_dev_->dev.acquireNextImageKHR(
                p_swapchain_->swapchain,
                UINT64_MAX,
                back.swapchain_image_acquire_semaphore,
                vk::Fence(),
                &back.swapchain_image_idx);
            if (res == vk::Result::eErrorOutOfDateKHR) {
                p_swapchain_->resize(0, 0);
                p_shell_->post_quit_msg();
            }
            else {
                assert(res == vk::Result::eSuccess);
            }
        }

        acquired_back_buf_=back;
        back_buffers_.pop_front();
    }

    void present_back_buffer_(float elapsed_time) override
    {
        on_frame_(elapsed_time);

        auto &back=acquired_back_buf_;
        vk::PresentInfoKHR present_info(1, &back.onscreen_render_semaphore,
                                        1, &p_swapchain_->swapchain,
                                        &back.swapchain_image_idx);
        p_dev_->present_queue.presentKHR(present_info);
        p_dev_->present_queue.submit(0, nullptr, back.present_queue_submit_fence);

        back_buffers_.push_back(back);
    }

    void detect_window_resize_() const
    {
        if (p_info_->resize_flag) {
            p_info_->resize_flag=false;
            p_swapchain_->resize(p_info_->width(), p_info_->height());
        }
    }

    bool first_invocation=true;
    bool order_abaa=true;
    // if true,
    // render to target a, filter-x to b, filter-y to a, use a on screen 
    // if false, 
    // render to target b, filter-x to a, filter-y to b, use b on screen

    void on_frame_(float elapsed_time)
    {
        auto &back=acquired_back_buf_;
        auto &data=frame_data_vec_[frame_data_idx_];
        update_uniforms_(data, elapsed_time);
        const vk::DeviceSize vb_offset{0};

        // offscreen

        base::assert_success(p_dev_->dev.waitForFences(1,
                                                       &data.submit_fence,
                                                       VK_TRUE,
                                                       UINT64_MAX));
        p_dev_->dev.resetFences(1, &data.submit_fence);

        auto &cmd_buf=data.cmd_buffer;
        cmd_buf.begin(cmd_begin_info_);

        cmd_buf.setViewport(0, 1, &offscreen_viewport_);
        cmd_buf.setScissor(0, 1, &offscreen_scissor_);

        // render shadow map
        {
            p_shadow_rp_->update_framebuffer(order_abaa ? shadow_fb_a_ : shadow_fb_b_);
            cmd_buf.beginRenderPass(&p_shadow_rp_->rp_begin, vk::SubpassContents::eInline);
            cmd_buf.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                       pipeline_layouts_.shadow,
                                       0, 1, &data.desc_set,
                                       0, nullptr);
            cmd_buf.bindPipeline(vk::PipelineBindPoint::eGraphics,
                                 pipelines_.shadow);
            cmd_buf.bindVertexBuffers(p_model_->vi_bind_id, 1,
                                      &p_model_->p_vert_buffer->buf,
                                      &vb_offset);
            cmd_buf.bindIndexBuffer(p_model_->p_idx_buffer->buf,
                                    0,
                                    vk::IndexType::eUint32);
            cmd_buf.drawIndexed(p_model_->indices, 1, 0, 0, 0);
            cmd_buf.endRenderPass();
        } // shadow map

        vk::ImageMemoryBarrier barrier;
        // b from shader read to attachment
        if (!first_invocation) {
            barrier={vk::AccessFlagBits::eShaderRead,
                vk::AccessFlagBits::eColorAttachmentWrite,
                vk::ImageLayout::eShaderReadOnlyOptimal,
                vk::ImageLayout::eColorAttachmentOptimal,
                VK_QUEUE_FAMILY_IGNORED,
                VK_QUEUE_FAMILY_IGNORED,
                order_abaa ? p_rt_shadow_color_b_->image : p_rt_shadow_color_a_->image,
                vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor,
                                          0, 1,
                                          0, 1)};
            cmd_buf.pipelineBarrier(vk::PipelineStageFlagBits::eFragmentShader,
                                    vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                    vk::DependencyFlagBits::eByRegion,
                                    0, nullptr,
                                    0, nullptr,
                                    1, &barrier);

        }
        else {
            first_invocation=false;
        }

        // filter
        // x direction
        {
            p_filter_rp_->update_framebuffer(order_abaa ? filter_fb_b_ : filter_fb_a_);
            cmd_buf.beginRenderPass(&p_filter_rp_->rp_begin, vk::SubpassContents::eInline);
            cmd_buf.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                       pipeline_layouts_.filter,
                                       0, 1, order_abaa ? &desc_sets_.cubemap2d_a : &desc_sets_.cubemap2d_b,
                                       0, nullptr);
            filter_info_.direction={1.f, 0.f};
            cmd_buf.pushConstants(pipeline_layouts_.filter,
                                  vk::ShaderStageFlagBits::eFragment,
                                  0, sizeof(filter_info_),
                                  &filter_info_);
            cmd_buf.bindPipeline(vk::PipelineBindPoint::eGraphics,
                                 pipelines_.filter);
            cmd_buf.draw(3, 1, 0, 0);
            cmd_buf.endRenderPass();
        } // x direction

        // a from shader read to attachment
        barrier={vk::AccessFlagBits::eShaderRead,
            vk::AccessFlagBits::eColorAttachmentWrite,
            vk::ImageLayout::eShaderReadOnlyOptimal,
            vk::ImageLayout::eColorAttachmentOptimal,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            order_abaa ? p_rt_shadow_color_a_->image : p_rt_shadow_color_b_->image,
            vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor,
                                      0, 1,
                                      0, 1)};
        cmd_buf.pipelineBarrier(vk::PipelineStageFlagBits::eFragmentShader,
                                vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                vk::DependencyFlagBits::eByRegion,
                                0, nullptr,
                                0, nullptr,
                                1, &barrier);

        // y direction
        {
            p_filter_rp_->update_framebuffer(order_abaa ? filter_fb_a_ : filter_fb_b_);
            cmd_buf.beginRenderPass(&p_filter_rp_->rp_begin, vk::SubpassContents::eInline);
            cmd_buf.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                       pipeline_layouts_.filter,
                                       0, 1, order_abaa ? &desc_sets_.cubemap2d_b : &desc_sets_.cubemap2d_a,
                                       0, nullptr);
            filter_info_.direction={0.f, 1.f};
            cmd_buf.pushConstants(pipeline_layouts_.filter,
                                  vk::ShaderStageFlagBits::eFragment,
                                  0, sizeof(filter_info_),
                                  &filter_info_);
            cmd_buf.bindPipeline(vk::PipelineBindPoint::eGraphics,
                                 pipelines_.filter);
            cmd_buf.draw(3, 1, 0, 0);
            cmd_buf.endRenderPass();
        } // y direction

        // b from shader read to attachment
        barrier={vk::AccessFlagBits::eShaderRead,
            vk::AccessFlagBits::eColorAttachmentWrite,
            vk::ImageLayout::eShaderReadOnlyOptimal,
            vk::ImageLayout::eColorAttachmentOptimal,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            order_abaa ? p_rt_shadow_color_b_->image : p_rt_shadow_color_a_->image,
            vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor,
                                      0, 1,
                                      0, 1)};
        cmd_buf.pipelineBarrier(vk::PipelineStageFlagBits::eFragmentShader,
                                vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                vk::DependencyFlagBits::eByRegion,
                                0, nullptr,
                                0, nullptr,
                                1, &barrier);

        // onscreen

        cmd_buf.setViewport(0, 1, &p_swapchain_->onscreen_viewport);
        cmd_buf.setScissor(0, 1, &p_swapchain_->onscreen_scissor);

        p_onscreen_rp_->rp_begin.framebuffer=p_swapchain_->framebuffers[back.swapchain_image_idx];
        p_onscreen_rp_->rp_begin.renderArea.extent=p_swapchain_->curr_extent();

        cmd_buf.beginRenderPass(p_onscreen_rp_->rp_begin, vk::SubpassContents::eInline);

        cmd_buf.bindVertexBuffers(0, 1, &p_model_->p_vert_buffer->buf, &vb_offset);
        cmd_buf.bindIndexBuffer(p_model_->p_idx_buffer->buf, 0, vk::IndexType::eUint32);

        cmd_buf.bindPipeline(vk::PipelineBindPoint::eGraphics,
                             pipelines_.onscreen);
        desc_sets_onscreen_[0]=data.desc_set;
        desc_sets_onscreen_[1]=order_abaa ? desc_sets_.cubemap2d_a : desc_sets_.cubemap2d_b;
        cmd_buf.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                   pipeline_layouts_.onscreen,
                                   0, 2,
                                   desc_sets_onscreen_.data(),
                                   0, nullptr);
        cmd_buf.drawIndexed(p_model_->indices, 1, 0, 0, 0);

        cmd_buf.endRenderPass();
        cmd_buf.end();

        auto &submit_info=cmd_submit_info_;
        submit_info.pCommandBuffers=&cmd_buf;
        submit_info.pWaitSemaphores=&back.swapchain_image_acquire_semaphore;
        submit_info.pSignalSemaphores=&back.onscreen_render_semaphore;

        base::assert_success(p_dev_->graphics_queue.submit(
            1,
            &submit_info,
            data.submit_fence));

        frame_data_idx_=(frame_data_idx_ + 1) % frame_data_count_;
        order_abaa=!order_abaa;
    }
};
