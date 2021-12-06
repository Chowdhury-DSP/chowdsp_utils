#include "chowdsp_FileListener.h"

namespace chowdsp
{
FileListener::FileListener (const juce::File& file, int timerSeconds) : fileToListenTo (file)
{
    fileModificationTime = fileToListenTo.getLastModificationTime().toMilliseconds();
    startTimer (timerSeconds * 1000);
}

void FileListener::timerCallback()
{
    auto newModificationTime = fileToListenTo.getLastModificationTime().toMilliseconds();

    if (newModificationTime < fileModificationTime)
    {
        jassertfalse; // file modification time moved backwards?
        return;
    }

    if (newModificationTime == fileModificationTime)
        return; // everything up-to-date

    // needs update!
    fileModificationTime = newModificationTime;
    fileChanged (fileToListenTo);
}

//void FileListener::addListener (Listener* l)
//{
//    l->fileChanged (fileToListenTo);
//    listeners.add (l);
//}
//
//void FileListener::removeListener (Listener* l)
//{
//    listeners.remove (l);
//}
}

