#pragma once
#include <Prog_info_base.hpp>
#include <algorithm>
#include <string>

class Prog_info : public base::Prog_info_base
{
public:
    uint32_t MAX_WIDTH{1920};
    uint32_t MAX_HEIGHT{1080};
    uint32_t CUBEMAP_WIDTH{512};
    uint32_t CUBEMAP_HEIGHT{512};
    bool pause_light{false};

    Prog_info()
    {
    }

    uint32_t width() const override
    {
        return width_;
    }

    uint32_t height() const override
    {
        return height_;
    }

    const std::string& prog_name() const override
    {
        return prog_name_;
    }

    void on_resize(uint32_t width, uint32_t height) override
    {
        width_=std::min(MAX_WIDTH, width);
        height_=std::min(MAX_HEIGHT, height);
        resize_flag=true;
    }

private:
    uint32_t width_{800};
    uint32_t height_{600};
    std::string prog_name_{"vsm_vk"};
};