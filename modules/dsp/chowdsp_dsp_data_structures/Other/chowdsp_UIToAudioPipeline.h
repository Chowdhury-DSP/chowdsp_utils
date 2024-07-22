#pragma once

JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4324)

namespace chowdsp
{
/**
 * Object to use for sending a stream of objects from the UI thread to the
 * audio thread. The audio thread will always have access to the most recent
 * available instance of the object.
 *
 * Note that this class is intended to be used when the object is being updated
 * from the UI thread very quickly (i.e. many times per second). If the object
 * is not being updated very often, an atomic smart pointer or RCU system might
 * work better.
 */
template <typename ObjectType>
class UIToAudioPipeline
{
public:
    UIToAudioPipeline() = default;

    ~UIToAudioPipeline()
    {
        killLeftoverObjects();
    }

    /** Write an object into the pipeline from the UI thread. */
    void write (std::unique_ptr<ObjectType>&& object)
    {
        killLeftoverObjects();

        ObjectPtr uiObject;
        uiObject.ptr = object.release();
        uiToLiveQueue.enqueue (uiToLiveProducerToken, uiObject);
    }

    /** Reads the latest object from the pipeline from the audio thread. */
    ObjectType* read()
    {
        ObjectPtr objectFromUI {};
        if (uiToLiveQueue.try_dequeue_from_producer (uiToLiveProducerToken, objectFromUI))
        {
            ObjectPtr deadObject {};
            deadObject.ptr = liveObject.release();
            liveObject.reset (objectFromUI.ptr);
            liveToDeadQueue.try_enqueue (deadObject);
        }
        return liveObject.get();
    }

private:
    void killLeftoverObjects()
    {
        ObjectPtr deadObject {};
        while (liveToDeadQueue.try_dequeue (deadObject))
            deadObject.kill();
        while (uiToLiveQueue.try_dequeue_from_producer (uiToLiveProducerToken, deadObject))
            deadObject.kill();
    }

    struct ObjectPtr
    {
        void kill()
        {
            if (ptr != nullptr)
            {
                delete ptr;
                ptr = nullptr;
            }
        }
        ObjectType* ptr = nullptr;
    };

    std::unique_ptr<ObjectType> liveObject; // object to use on the audio thread
    moodycamel::ConcurrentQueue<ObjectPtr> uiToLiveQueue { 4 }; // pipeline from UI to audio thread
    moodycamel::ProducerToken uiToLiveProducerToken { uiToLiveQueue };
    moodycamel::ReaderWriterQueue<ObjectPtr, 4> liveToDeadQueue; // pipeline from audio thread to destruction

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UIToAudioPipeline)
};
} // namespace chowdsp

JUCE_END_IGNORE_WARNINGS_MSVC
