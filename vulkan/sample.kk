module vulkan/sample

import std/core/console
import std/num/int32
import vulkan/vulkan

pub fun main()
  val r = vulkan/initialize()
  println("initialization result: " ++ r.show)
  if !r.is-success then return ()
  val app = Application("test application", 1.int32, "kkvk", 0.int32, version-v13)
  ignore(create(Instance-Create(), Just(app)))