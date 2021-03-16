#include"tResource.h"
#include"utils.hpp"
#include"CommandBufferBase.h"
namespace tEngine {
    VkImageView tImageView::get_render_target_view(unsigned layer) const
    {
        // Transient images just have one layer.
        if (info.image->get_create_info().domain == ImageDomain::Transient)
            return view;

        VK_ASSERT(layer < get_create_info().layers);

        if (render_target_views.empty())
            return view;
        else
        {
            VK_ASSERT(layer < render_target_views.size());
            return render_target_views[layer];
        }
    }
 
  

  
	tSwapChain::tSwapChain( Device* device,vk::SwapchainKHR const& swapChain):device(device), swapChain(swapChain){

	}
}