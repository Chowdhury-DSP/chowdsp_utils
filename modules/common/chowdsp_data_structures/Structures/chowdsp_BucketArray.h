#pragma once

namespace chowdsp
{
/**
 * A "Bucket Array" based loosely on the one implemented in the Jai programming language.
 *
 * The idea is that "buckets" are dynamically allocated, but individual items are
 * statically allocated within the buckets. The most important feature of this container
 * (in my opinion), is that pointers to items in the bucket array are _always_ valid,
 * until the item is removed from the array, or `reset()` is called on the whole array.
 */
template <typename T, size_t items_per_bucket>
class BucketArray
{
public:
    struct Bucket
    {
        std::array<bool, items_per_bucket> occupied {};
        std::array<RawObject<T>, items_per_bucket> items {};

        size_t lowest_maybe_not_occupied = 0; // Index that _may_ not be occupied, to decrease search space when adding elements.
        size_t bucket_index = 0;

        size_t count = 0;
    };

    struct BucketLocator
    {
        size_t bucket_index;
        size_t slot_index;
        static constexpr size_t INVALID_SLOT_INDEX = std::numeric_limits<size_t>::max();
    };

    BucketArray() = default;
    BucketArray (const BucketArray&) = delete;
    BucketArray& operator= (const BucketArray&) = delete;
    BucketArray (BucketArray&&) noexcept = default;
    BucketArray& operator= (BucketArray&&) noexcept = default;
    ~BucketArray() { reset(); }

    /** Deletes any items in the array, and deallocates all memory being used. */
    void reset()
    {
        for (auto& bucket : all_buckets)
        {
            for (auto [item, is_occupied] : zip (bucket->items, bucket->occupied))
            {
                if (is_occupied)
                    item.destruct();
            }
        }

        all_buckets.clear();
        unfull_buckets.clear();
        count = 0;
    }

    /** Returns the number of items in the array. */
    [[nodiscard]] size_t size() const noexcept { return count; }

    /** Adds an item to the array (by copy) */
    std::pair<BucketLocator, T*> add (const T& item)
    {
        return find_and_occupy_empty_slot (item);
    }

    /** Adds an item to the array (by move) */
    std::pair<BucketLocator, T*> add (T&& item)
    {
        return find_and_occupy_empty_slot (std::move (item));
    }

    /** Constructs an item in the array in-place. */
    template <typename... Args>
    std::pair<BucketLocator, T*> emplace (Args&&... args)
    {
        return find_and_occupy_empty_slot (std::forward<Args...> (args...));
    }

    std::pair<BucketLocator, T*> emplace()
    {
        return find_and_occupy_empty_slot();
    }

    /** Returns an element found by the given locator. */
    T* find (BucketLocator locator)
    {
        if (locator.bucket_index >= all_buckets.size() || locator.slot_index >= items_per_bucket)
            return nullptr;

        auto& bucket = all_buckets[locator.bucket_index];
        jassert (bucket->occupied[locator.slot_index] == true);
        return bucket->items[locator.slot_index].data();
    }

    /**
     * Returns a locator for an element. If the element is not present
     * in the array, then locator.slot_index will equal BucketLocator::INVALID_SLOT_INDEX.
     */
    BucketLocator get_locator (const T* item) const
    {
        for (const auto& bucket : all_buckets)
        {
            for (const auto [idx, bucket_item] : enumerate (bucket->items))
            {
                if (item == bucket_item.data())
                    return BucketLocator { bucket->bucket_index, idx };
            }
        }

        jassertfalse; // locator not found!
        return { 0, BucketLocator::INVALID_SLOT_INDEX };
    }

    /** Iterates over the items in the array (by reference). */
    template <typename Action>
    void doForAll (Action&& action)
    {
        for (auto& bucket : all_buckets)
        {
            for (auto [bucket_item, is_occupied] : zip (bucket->items, bucket->occupied))
            {
                if (! is_occupied)
                    continue;
                action (bucket_item.item());
            }
        }
    }

    /** Iterates over the items in the array (by const reference). */
    template <typename Action>
    void doForAll (Action&& action) const
    {
        for (const auto& bucket : all_buckets)
        {
            for (const auto [bucket_item, is_occupied] : zip (bucket->items, bucket->occupied))
            {
                if (! is_occupied)
                    continue;
                action (bucket_item.item());
            }
        }
    }

    /**
     * Removes an item from the array (by locator).
     *
     * Note that this method will call the destructor for the item being removed,
     * but it won't deallocate any memory on its own. So if the item's destructor
     * does not do any de-allocation, then you can safely remove items from the
     * array without ever deallocating memory!
     */
    void remove (BucketLocator locator)
    {
        if (locator.bucket_index >= all_buckets.size() || locator.slot_index >= items_per_bucket)
        {
            // Tried to remove an element from an invalid locator!
            jassertfalse;
            return;
        }

        auto& bucket = all_buckets[locator.bucket_index];
        jassert (bucket->occupied[locator.slot_index] == true);

        const auto was_full = (bucket->count == items_per_bucket);

        bucket->items[locator.slot_index].destruct();
        bucket->occupied[locator.slot_index] = false;

        if (locator.slot_index < bucket->lowest_maybe_not_occupied)
            bucket->lowest_maybe_not_occupied = locator.slot_index;

        bucket->count--;
        count--;

        if (was_full)
        {
            // if this bucket was full, it shouldn't be in unfull_buckets!
            jassert (std::find (unfull_buckets.begin(), unfull_buckets.end(), bucket.get()) == unfull_buckets.end());
            unfull_buckets.push_back (bucket.get());
        }
    }

private:
    template <typename... ConstructorArgs>
    std::pair<BucketLocator, T*> find_and_occupy_empty_slot (ConstructorArgs&&... args)
    {
        if (unfull_buckets.size() == 0)
            add_bucket();

        auto* bucket = unfull_buckets[0];

        size_t slot_index = BucketLocator::INVALID_SLOT_INDEX;
        for (size_t i = bucket->lowest_maybe_not_occupied; i < items_per_bucket; ++i)
        {
            if (bucket->occupied[i] == false)
            {
                slot_index = i;
                break;
            }
        }

        jassert (slot_index != BucketLocator::INVALID_SLOT_INDEX);
        bucket->occupied[slot_index] = true;
        bucket->count++;
        bucket->lowest_maybe_not_occupied = slot_index + 1;
        jassert (bucket->count <= items_per_bucket);

        auto* memory = bucket->items[slot_index].construct (std::forward<ConstructorArgs> (args)...);
        count++;

        if (bucket->count == items_per_bucket)
        {
            auto unfull_bucket_iter = std::find (unfull_buckets.begin(), unfull_buckets.end(), bucket);
            if (unfull_bucket_iter != unfull_buckets.end())
            {
                unfull_buckets.erase (unfull_bucket_iter);
            }
            else
            {
                // Apparently the bucket we've been adding to wasn't unfull?
                jassertfalse;
            }
        }

        BucketLocator locator {};
        locator.bucket_index = bucket->bucket_index;
        locator.slot_index = slot_index;

        return std::make_pair (locator, memory);
    }

    void add_bucket()
    {
        // we shouldn't be adding a bucket if there's some unfull buckets out there!
        jassert (unfull_buckets.size() == 0);

        const auto new_bucket_index = all_buckets.size();
        auto& new_bucket = all_buckets.emplace_back();
        new_bucket = std::make_unique<Bucket>();
        new_bucket->bucket_index = new_bucket_index;

        unfull_buckets.push_back (new_bucket.get());
    }

    size_t count = 0;

    std::vector<std::unique_ptr<Bucket>> all_buckets;
    std::vector<Bucket*> unfull_buckets;
};
} // namespace chowdsp
