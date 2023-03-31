#include <vector>
#include <thread>
#include <atomic>
#include <functional>

typedef void(__cdecl *ComputeShader)(int3, int3, int3);

static std::atomic<int32_t> group_thread_counter;

//=====================================================================================================================
static void InitGroupBarrier(int concurrent_thread_count)
{
    std::printf("Barrier: init (%d)\n", concurrent_thread_count);
    group_thread_counter = concurrent_thread_count;
}

//=====================================================================================================================
static void GroupMemoryBarrier()
{
    // Notify thread group scheduler that we've reached this part of the code
    group_thread_counter.fetch_sub(1);
    std::printf("Barrier: waiting (%d)\n", group_thread_counter.load());

    if (group_thread_counter.load() == 0)
    {
        assert(false);
    }

    do
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    } while (group_thread_counter.load() > 0);

    std::printf("Barrier: done\n");
}

//=====================================================================================================================
static void parallel_for(
    unsigned group_size,
    std::function<void(int id)> functor,
    bool use_threads = true)
{
    unsigned nb_threads = group_size;

    std::vector<std::thread> my_threads(nb_threads);

    if (use_threads)
    {
        InitGroupBarrier(nb_threads);

        // Multithread execution
        for (unsigned i = 0; i < nb_threads; ++i)
        {
            my_threads[i] = std::thread(functor, i);
        }
    }
    else
    {
        // Single thread execution (for easy debugging)
        for (unsigned i = 0; i < nb_threads; ++i)
        {
            InitGroupBarrier(1);

            functor(i);
        }
    }

    // Wait for the other thread to finish their task
    if (use_threads)
    {
        std::for_each(my_threads.begin(), my_threads.end(), std::mem_fn(&std::thread::join));
    }
}

//=====================================================================================================================
class ThreadGroup
{
public:

    ThreadGroup(int3 group_id, int3 group_size)
        :
        m_group_id(group_id),
        m_group_size(group_size)
    {

    }

    ~ThreadGroup() {

    }

    void Execute(void(__cdecl* shader)(int3, int3, int3)) {

        int group_thread_count = m_group_size.x * m_group_size.y * m_group_size.z;

        parallel_for(group_thread_count, [&](int j)
        {
            int3 group_thread_id;
            group_thread_id.x = j % m_group_size.x;
            group_thread_id.y = j / m_group_size.x;
            group_thread_id.z = j / (m_group_size.x * m_group_size.y);

            const int3 dispatch_thread_id = group_thread_id + (m_group_id * m_group_size);
            shader(dispatch_thread_id, group_thread_id, m_group_id);
        });

        for (int j = 0; j < group_thread_count; ++j)
        {

            int3 group_thread_id;
            group_thread_id.x = j % m_group_size.x;
            group_thread_id.y = j / m_group_size.x;
            group_thread_id.z = j / (m_group_size.x * m_group_size.y);

            const int3 dispatch_thread_id = group_thread_id + (m_group_id * m_group_size);
            shader(dispatch_thread_id, group_thread_id, m_group_id);
        }
    }

private:

    const int3 m_group_id;
    const int3 m_group_size;
    std::vector<std::thread> m_threads;
}; 

//=====================================================================================================================
class Dispatch
{
    public:
        Dispatch(int3 thread_group_count, int3 group_size)
            :
            m_group_size(group_size),
            m_group_count(thread_group_count),
            m_thread_count(group_size.x * thread_group_count.x,
                           group_size.y * thread_group_count.y, 
                           group_size.z * thread_group_count.z)
        {
            m_groups.resize(thread_group_count.x * thread_group_count.y * thread_group_count.z);
        }

        ~Dispatch() {

        }

        void Execute(void (__cdecl* shader)(int3, int3,int3)) {

            int group_count = m_group_count.x * m_group_count.y * m_group_count.z;

            for (int i = 0; i < group_count; ++i)
            {
                int3 group_id;
                group_id.x = i % m_group_count.x;
                group_id.y = i / m_group_count.x;
                group_id.z = i / (m_group_count.x * m_group_count.y);

                m_groups[i] = std::make_unique<ThreadGroup>(group_id, m_group_size);
                m_groups[i]->Execute(shader);
            }
        }

    private:

        std::vector<std::unique_ptr<ThreadGroup>> m_groups;
        const int3 m_group_count;
        const int3 m_group_size;
        const int3 m_thread_count;
};