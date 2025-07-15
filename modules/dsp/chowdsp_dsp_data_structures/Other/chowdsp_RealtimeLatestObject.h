#pragma once

namespace chowdsp
{
JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4324) // structure was padded due to alignment specifier

/**
 * An object that can be written to from the UI thread and read from
 * the audio thread, and is wait-free on both threads. Here's how it works:
 *
 * We keep an atomic pointer to the object on the UI thread. Whenever the UI
 * thread wants to write a new object, it swaps it with the atomic pointer,
 * and frees the old object. Then on the audio thread, we keep a raw pointer
 * (non-atomic), and swap it with the atomic pointer (written from the UI thread)
 * if not null.
 *
 * Now we have a pointer on the audio thread that needs to be freed,
 * so we add that pointer to an atomic linked-list (the "zombie" list),
 * which can be freed from the UI thread. So now when we do a write we
 * first clear the zombie list. However, in order to make it easier
 * for the audio thread to push onto the zombie list, the UI thread
 * always leaves the last zombie alive as a lis "stub" for the audio
 * thread to build off of.
 */
template <typename T>
struct RealtimeLatestObject
{
    static_assert (std::is_aggregate_v<T> || std::is_default_constructible_v<T>);

    RealtimeLatestObject()
    {
        allocator.backing_buffer = allocator_backing_buffer;
        allocator.free_all();
    }

    ~RealtimeLatestObject()
    {
        allocator.backing_buffer = {};
    }

    /**
     * Initializes the object with a given starting value.
     * [main thread]
     */
    template <typename... Args>
    void init (Args&&... args)
    {
        auto* new_obj = allocate (std::forward<Args> (args)...);

        // Did you forget to call deinit()
        jassert (main_thread_object.load (std::memory_order_relaxed) == nullptr);

        // We don't care about memory order here because the audio thread isn't running yet!
        audio_thread_object = new_obj;

        // Allocate an extra object for the free list
        zombie_list_head = allocate();
        zombie_list_tail.store (zombie_list_head, std::memory_order_relaxed);
    }

    /**
     * Reset's the object state.
     * This must be called before destruction unless T is trivially destructible.
     * [main thread]
     */
    void deinit()
    {
        // We don't care about memory order here because the audio thread isn't running anymore!
        deallocate (main_thread_object.exchange (nullptr, std::memory_order_relaxed));
        deallocate (audio_thread_object);

        // delete the entire free list
        clear_zombie_list();
        deallocate (zombie_list_tail.load (std::memory_order_relaxed));

        // reset everything to nullptr
        main_thread_object.store (nullptr, std::memory_order_relaxed);
        audio_thread_object = nullptr;
        zombie_list_head = nullptr;
        zombie_list_tail.store (nullptr, std::memory_order_relaxed);
    }

    /**
     * Writes a new value to the object.
     * [main thread]
     */
    template <typename... Args>
    void write (Args&&... args)
    {
        clear_zombie_list();

        auto* new_obj = allocate (std::forward<Args> (args)...);
        auto* obj_to_free = main_thread_object.exchange (new_obj, std::memory_order_release);
        deallocate (obj_to_free); // free the old object!
    }

    /**
     * Read's the latest value from the object.
     * [audio thread]
     */
    T& read()
    {
        if (main_thread_object.load (std::memory_order_acquire) != nullptr)
        {
            auto* obj_to_free = audio_thread_object;
            audio_thread_object = main_thread_object.exchange (nullptr, std::memory_order_release);

            // push obj_to_free onto the free list
            zombie_list_tail.load (std::memory_order_acquire)->next = obj_to_free;
            zombie_list_tail.store (obj_to_free, std::memory_order_release);
        }

        // return audio_thread_object
        return audio_thread_object->obj;
    }

    /**
     * Free's outstanding objects from the "zombie" list.
     * [main thread]
     */
    void clear_zombie_list()
    {
        while (zombie_list_head != zombie_list_tail.load (std::memory_order_acquire))
        {
            auto* obj_to_free = zombie_list_head;
            zombie_list_head = zombie_list_head->next;
            assert (zombie_list_head != nullptr); // we messed up!
            deallocate (obj_to_free);
        }
    }

private:
    struct Node
    {
        T obj {};
        Node* next {};
    };

    template <typename... Args>
    Node* allocate (Args&&... args)
    {
        return allocator.template allocate<Node> ({ T { std::forward<Args> (args)... } });
    }

    void deallocate (Node* node)
    {
        if (node == nullptr)
            return;

        allocator.free (node);
    }

    std::atomic<Node*> main_thread_object {}; // written on main thread / read on audio thread
    Node* audio_thread_object {}; // written/read on audio thread
    Node* zombie_list_head {}; // written/read on main thread
    std::atomic<Node*> zombie_list_tail {}; // written on audio thread / read on main thread

    ObjectPool<Node> allocator { 4 };
    alignas (alignof (Node)) std::array<std::byte, sizeof (Node) * 4> allocator_backing_buffer {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RealtimeLatestObject)
};
JUCE_END_IGNORE_WARNINGS_MSVC
} // namespace chowdsp
