#include "trigger.hpp"

Trigger::Trigger(boost::lockfree::queue<buffer*, boost::lockfree::fixed_sized<false>> *inputQ,
                 boost::lockfree::queue<buffer*, boost::lockfree::fixed_sized<false>> *outputQ,
                 int8_t level)
{
    if (inputQ == NULL) {
        ERROR << "trigger inputQ is null";
    } else {
        inputQueue = inputQ;
    }

    if (outputQ == NULL) {
        ERROR << "trigger outputQ is null";
    } else {
        outputQueue = outputQ;
    }

    triggerLevel = level;
    clearCount();

    stopTrigger.store(false); 
    pauseTrigger.store(true); 
    threadExists.store(false);
    triggerMet.store(false);
}

Trigger::~Trigger(void)
{
    INFO << "Trigger Destructor Called";
    destroyThread();
}

#ifdef DBG
uint32_t temp = 0;
#endif

void Trigger::checkTrigger(buffer* currentBuffer)
{
#ifdef DBG
    INFO << "Checking a Trigger";
    INFO << "size of uint64_t: " << sizeof(uint64_t);
#endif
    // Compute the trigger
    for (int i = 0; i < BUFFER_SIZE/64; i++) {
        currentBuffer->trigger[i] = 
                     ((uint64_t)((currentBuffer->data[i * 64 + 0] < triggerLevel) &&
                       (currentBuffer->data[i * 64 +  0 + 1] >= triggerLevel)) << 63) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 1] < triggerLevel) &&
                       (currentBuffer->data[i * 64 +  1 + 1] >= triggerLevel)) << 62) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 2] < triggerLevel) &&
                       (currentBuffer->data[i * 64 +  2 + 1] >= triggerLevel)) << 61) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 3] < triggerLevel) &&
                       (currentBuffer->data[i * 64 +  3 + 1] >= triggerLevel)) << 60) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 4] < triggerLevel) &&
                       (currentBuffer->data[i * 64 +  4 + 1] >= triggerLevel)) << 59) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 5] < triggerLevel) &&
                       (currentBuffer->data[i * 64 +  5 + 1] >= triggerLevel)) << 58) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 6] < triggerLevel) &&
                       (currentBuffer->data[i * 64 +  6 + 1] >= triggerLevel)) << 57) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 7] < triggerLevel) &&
                       (currentBuffer->data[i * 64 +  7 + 1] >= triggerLevel)) << 56) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 8] < triggerLevel) &&
                       (currentBuffer->data[i * 64 +  8 + 1] >= triggerLevel)) << 55) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 9] < triggerLevel) &&
                       (currentBuffer->data[i * 64 +  9 + 1] >= triggerLevel)) << 54) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 10] < triggerLevel) &&
                       (currentBuffer->data[i * 64 + 10 + 1] >= triggerLevel)) << 53) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 11] < triggerLevel) &&
                       (currentBuffer->data[i * 64 + 11 + 1] >= triggerLevel)) << 52) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 12] < triggerLevel) &&
                       (currentBuffer->data[i * 64 + 12 + 1] >= triggerLevel)) << 51) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 13] < triggerLevel) &&
                       (currentBuffer->data[i * 64 + 13 + 1] >= triggerLevel)) << 50) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 14] < triggerLevel) &&
                       (currentBuffer->data[i * 64 + 14 + 1] >= triggerLevel)) << 49) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 15] < triggerLevel) &&
                       (currentBuffer->data[i * 64 + 15 + 1] >= triggerLevel)) << 48) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 16] < triggerLevel) &&
                       (currentBuffer->data[i * 64 + 16 + 1] >= triggerLevel)) << 47) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 17] < triggerLevel) &&
                       (currentBuffer->data[i * 64 + 17 + 1] >= triggerLevel)) << 46) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 18] < triggerLevel) &&
                       (currentBuffer->data[i * 64 + 18 + 1] >= triggerLevel)) << 45) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 19] < triggerLevel) &&
                       (currentBuffer->data[i * 64 + 19 + 1] >= triggerLevel)) << 44) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 20] < triggerLevel) &&
                       (currentBuffer->data[i * 64 + 20 + 1] >= triggerLevel)) << 43) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 21] < triggerLevel) &&
                       (currentBuffer->data[i * 64 + 21 + 1] >= triggerLevel)) << 42) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 22] < triggerLevel) &&
                       (currentBuffer->data[i * 64 + 22 + 1] >= triggerLevel)) << 41) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 23] < triggerLevel) &&
                       (currentBuffer->data[i * 64 + 23 + 1] >= triggerLevel)) << 40) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 24] < triggerLevel) &&
                       (currentBuffer->data[i * 64 + 24 + 1] >= triggerLevel)) << 39) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 25] < triggerLevel) &&
                       (currentBuffer->data[i * 64 + 25 + 1] >= triggerLevel)) << 38) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 26] < triggerLevel) &&
                       (currentBuffer->data[i * 64 + 26 + 1] >= triggerLevel)) << 37) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 27] < triggerLevel) &&
                       (currentBuffer->data[i * 64 + 27 + 1] >= triggerLevel)) << 36) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 28] < triggerLevel) &&
                       (currentBuffer->data[i * 64 + 28 + 1] >= triggerLevel)) << 35) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 29] < triggerLevel) &&
                       (currentBuffer->data[i * 64 + 29 + 1] >= triggerLevel)) << 34) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 30] < triggerLevel) &&
                       (currentBuffer->data[i * 64 + 30 + 1] >= triggerLevel)) << 33) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 31] < triggerLevel) &&
                       (currentBuffer->data[i * 64 + 31 + 1] >= triggerLevel)) << 32) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 32] < triggerLevel) &&
                       (currentBuffer->data[i * 64 + 32 + 1] >= triggerLevel)) << 31) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 33] < triggerLevel) &&
                       (currentBuffer->data[i * 64 + 33 + 1] >= triggerLevel)) << 30) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 34] < triggerLevel) &&
                       (currentBuffer->data[i * 64 + 34 + 1] >= triggerLevel)) << 29) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 35] < triggerLevel) &&
                       (currentBuffer->data[i * 64 + 35 + 1] >= triggerLevel)) << 28) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 36] < triggerLevel) &&
                       (currentBuffer->data[i * 64 + 36 + 1] >= triggerLevel)) << 27) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 37] < triggerLevel) &&
                       (currentBuffer->data[i * 64 + 37 + 1] >= triggerLevel)) << 26) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 38] < triggerLevel) &&
                       (currentBuffer->data[i * 64 + 38 + 1] >= triggerLevel)) << 25) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 39] < triggerLevel) &&
                       (currentBuffer->data[i * 64 + 39 + 1] >= triggerLevel)) << 24) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 40] < triggerLevel) &&
                       (currentBuffer->data[i * 64 + 40 + 1] >= triggerLevel)) << 23) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 41] < triggerLevel) &&
                       (currentBuffer->data[i * 64 + 41 + 1] >= triggerLevel)) << 22) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 42] < triggerLevel) &&
                       (currentBuffer->data[i * 64 + 42 + 1] >= triggerLevel)) << 21) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 43] < triggerLevel) &&
                       (currentBuffer->data[i * 64 + 43 + 1] >= triggerLevel)) << 20) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 44] < triggerLevel) &&
                       (currentBuffer->data[i * 64 + 44 + 1] >= triggerLevel)) << 19) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 45] < triggerLevel) &&
                       (currentBuffer->data[i * 64 + 45 + 1] >= triggerLevel)) << 18) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 46] < triggerLevel) &&
                       (currentBuffer->data[i * 64 + 46 + 1] >= triggerLevel)) << 17) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 47] < triggerLevel) &&
                       (currentBuffer->data[i * 64 + 47 + 1] >= triggerLevel)) << 16) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 48] < triggerLevel) &&
                       (currentBuffer->data[i * 64 + 48 + 1] >= triggerLevel)) << 15) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 49] < triggerLevel) &&
                       (currentBuffer->data[i * 64 + 49 + 1] >= triggerLevel)) << 14) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 50] < triggerLevel) &&
                       (currentBuffer->data[i * 64 + 50 + 1] >= triggerLevel)) << 13) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 51] < triggerLevel) &&
                       (currentBuffer->data[i * 64 + 51 + 1] >= triggerLevel)) << 12) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 52] < triggerLevel) &&
                       (currentBuffer->data[i * 64 + 52 + 1] >= triggerLevel)) << 11) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 53] < triggerLevel) &&
                       (currentBuffer->data[i * 64 + 53 + 1] >= triggerLevel)) << 10) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 54] < triggerLevel) &&
                       (currentBuffer->data[i * 64 + 54 + 1] >= triggerLevel)) << 9) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 55] < triggerLevel) &&
                       (currentBuffer->data[i * 64 + 55 + 1] >= triggerLevel)) << 8) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 56] < triggerLevel) &&
                       (currentBuffer->data[i * 64 + 56 + 1] >= triggerLevel)) << 7) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 57] < triggerLevel) &&
                       (currentBuffer->data[i * 64 + 57 + 1] >= triggerLevel)) << 6) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 58] < triggerLevel) &&
                       (currentBuffer->data[i * 64 + 58 + 1] >= triggerLevel)) << 5) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 59] < triggerLevel) &&
                       (currentBuffer->data[i * 64 + 59 + 1] >= triggerLevel)) << 4) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 60] < triggerLevel) &&
                       (currentBuffer->data[i * 64 + 60 + 1] >= triggerLevel)) << 3) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 61] < triggerLevel) &&
                       (currentBuffer->data[i * 64 + 61 + 1] >= triggerLevel)) << 2) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 62] < triggerLevel) &&
                       (currentBuffer->data[i * 64 + 62 + 1] >= triggerLevel)) << 1) |
                     ((uint64_t)((currentBuffer->data[i * 64 + 63] < triggerLevel) &&
                       (currentBuffer->data[i * 64 + 63 + 1] >= triggerLevel)) << 0);

#ifdef DBG
        if (temp < 5) {
            INFO << "Trigger index: " << i << " value: " << currentBuffer->trigger[i];
            temp++;
        }
#endif
    }
}

void Trigger::coreLoop()
{
    buffer *currentBuffer;
    buffer *nextBuffer;

    // Get the first buffer into currentBuffer
    if (inputQueue == NULL) {
        ERROR << "Input Queue null in core loop";
    } else {
        while (inputQueue->pop(currentBuffer) == false && stopTrigger.load() == false) {
//            INFO << "Waiting for first data element";
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        };
        INFO << "Core Loop Entered";

        // Outer loop
        while (stopTrigger.load() == false) {

            // Inner Loop
            while (pauseTrigger.load() == false) {
                // Attempt to pop from the pueue

                if (inputQueue->pop(nextBuffer)) {
                    count++;

                    INFO << "trigger next buffer";

                    // copy first value from next buffer to current buffer
                    currentBuffer->data[BUFFER_SIZE] = nextBuffer->data[0];

                    // generate triggers on new data
                    checkTrigger(currentBuffer);

                    // push triggers and buffer onto post processor thread
                    outputQueue->push(currentBuffer);

                    // swap next to current
                    currentBuffer = nextBuffer;

                } else {

                    // TODO: clean this up for when not running tests
                    triggerMet.store(true);
                    // Queue empty, Sleep for a bit
                    std::this_thread::sleep_for(std::chrono::microseconds(100));
                }
            }
        }
    }
}

bool Trigger::getTriggerStatus()
{
    return triggerMet.load();
}

void Trigger::createThread()
{
    const std::lock_guard<std::mutex> lock(lockThread);

    // Check it thread created
    if (threadExists.load() == false) {
        // start thread paused
        pauseTrigger.store(true);
        stopTrigger.store(false);

        // create new thread
        triggerThread = std::thread(&Trigger::coreLoop, this);

        // set thread exists flag
        threadExists.store(true);
    } else {
        // Thread already created
        throw EVException(10, "Trigger::createThread(): Thread already created");
    }
    INFO << "Created Trigger Thread";
}

void Trigger::destroyThread()
{
    const std::lock_guard<std::mutex> lock(lockThread);

    if (threadExists.load() == true) {
        // Stop the transer and join thread
        stopTrigger.store(true);
        pauseTrigger.store(true);
        triggerThread.join();

        // clear thread exists flag
        threadExists.store(false);
    } else {
        // Thread does not exist
        throw EVException(10, "destroyThread(): thread does not exist");
    }
    INFO << "Destroyed Trigger Thread";
}

void Trigger::triggerStop()
{
    stopTrigger.store(true);
    INFO << "Stopping Trigger";
}

void Trigger::triggerStart()
{
    stopTrigger.store(false);
    INFO << "Starting Trigger";
}

void Trigger::triggerPause()
{
    pauseTrigger.store(true);
    INFO << "Pausing Trigger";
}

void Trigger::triggerUnpause()
{
    pauseTrigger.store(false);
    INFO << "Unpausing Trigger";
}

uint32_t Trigger::getCount()
{
    return count;
}

uint32_t Trigger::getCountBytes()
{
    return getCount() * BUFFER_SIZE;
}

void Trigger::setCount(uint32_t newCount)
{
    count = newCount;
}

void Trigger::clearCount()
{
    setCount(0);
}
