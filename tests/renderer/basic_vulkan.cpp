#include <catch.hpp>

#include "FSNG/modules/renderer/GLFWVulkanSpace.hpp"
#include "FSNG/modules/renderer/CreateWindow.hpp"
#include "PathSpace.hpp"


using namespace FSNG;

TEST_CASE("Basic Vulkan") {
    PathSpaceTE space = PathSpace{};

    SECTION("Basic Window") {
        space.insert("/graphics", PathSpaceTE(GLFWVulkanSpace()));
        //space.insert("/graphics/windows/main", CreateWindow{.title="Main", .fullscreen=false});
    }
}