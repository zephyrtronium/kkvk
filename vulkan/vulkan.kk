// Low-level Vulkan API wrapper.
module vulkan/vulkan

import std/num/int32

extern import
  c { vcpkg="vulkan-headers" }

extern import
  c file "vulkan-inline.c"

abstract value struct vk
  obj: any

abstract value struct vk-version
  v: int32

extern vk-make-api-version(variant: int32, major: int32, minor: int32, patch: int32): int32
  c inline "(VK_MAKE_API_VERSION($1, $2, $3, $4))"

// Create a Vulkan API version.
// This is a thin wrapper around the VK_MAKE_API_VERSION macro;
// it does not check that its inputs are in valid ranges.
pub fun api-version(variant: int, major: int, minor: int, patch: int): vk-version
  // TODO(zeph): validate bit widths
  Vk-Version(vk-make-api-version(variant.int32, major.int32, minor.int32, patch.int32))

// Predefine the important values of vk-version.
val vk-version-v10 = vk-make-api-version(0.int32, 1.int32, 0.int32, 0.int32)
val vk-version-v11 = vk-make-api-version(0.int32, 1.int32, 1.int32, 0.int32)
val vk-version-v12 = vk-make-api-version(0.int32, 1.int32, 2.int32, 0.int32)
val vk-version-v13 = vk-make-api-version(0.int32, 1.int32, 3.int32, 0.int32)
// API version 1.0.
pub val version-v10 = Vk-Version(vk-version-v10)
// API version 1.1.
pub val version-v11 = Vk-Version(vk-version-v11)
// API version 1.2.
pub val version-v12 = Vk-Version(vk-version-v12)
// API version 1.3.
pub val version-v13 = Vk-Version(vk-version-v13)

extern vk-create-instance(info: instance-create, app: maybe<application>): any
  c "kk_vulkan_create_instance"

// VkCreateInstance.
pub fun vk/create(info: instance-create, app: maybe<application>): vk
  Vk(vk-create-instance(info, app))

// VkInstanceCreateInfo.
pub struct instance-create
  enabled-layers: list<string> = Nil
  enabled-extensions: list<string> = Nil
  enumerate-portability: bool = False
  next: list<instance-create-next> = Nil

// Types that can appear in VkInstanceCreateInfo's pNext chain.
pub type instance-create-next
  // TODO

// VkApplicationInfo.
pub struct application
  application-name: string
  application-version: int32
  engine-name: string
  engine-version: int32
  api-version: vk-version = version-v10
