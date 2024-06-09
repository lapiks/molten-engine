#include <vulkan/vulkan.h>

namespace vkutil {
  // image manipulation helper functions
  // transition image from a layout to another
  void transition_image(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout);
  // copy an image to another
  void copy_image_to_image(VkCommandBuffer cmd, VkImage source, VkImage destination, VkExtent2D srcSize, VkExtent2D dstSize);
  //------------------------------------
  // 
  // Init helper functions for images
  // for image
  VkImageCreateInfo image_create_info(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent);
  // for image view
  VkImageViewCreateInfo imageview_create_info(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags);
  // for image subresource range
  VkImageSubresourceRange image_subresource_range(VkImageAspectFlags aspectMask);
  //---------------------------------
}