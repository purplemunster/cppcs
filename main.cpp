#include <cstdio>

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_XYZW_ONLY
#include "glm/glm.hpp"
#include "glm/ext.hpp"
using namespace glm;

#include "dispatch.h"
#include "byte_address_buffer.h"

static RWByteAddressBuffer DstBuffer;

//=====================================================================================================================
static void cs_main(
    int3 dispatch_thread_id, int3 group_thread_id, int3 group_id)
{
    std::printf("[%d, %d, %d] [%d, %d, %d] [%d, %d, %d]\n",
        dispatch_thread_id.x, dispatch_thread_id.y, dispatch_thread_id.z,
        group_id.x, group_id.y, group_id.z,
        group_thread_id.x, group_thread_id.y, group_thread_id.z
        );

    int offset = dispatch_thread_id.x * sizeof(float);
    DstBuffer.Store<float>(offset, dispatch_thread_id.x * 1.0f);

    GroupMemoryBarrier();
};

//=====================================================================================================================
int main(int argc, char* argv[])
{
    DstBuffer.Allocate(1024);

    int3 thread_group_count;
    thread_group_count.x = 6;
    thread_group_count.y = 3;
    thread_group_count.z = 1;

    Dispatch dispatch(thread_group_count, int3(32, 1, 1));
    dispatch.Execute(&cs_main);

    return 1;
}