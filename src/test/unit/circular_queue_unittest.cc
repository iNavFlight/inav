#include "unittest_macros.h"
#include "gtest/gtest.h"
#include <stdint.h>

#include <stdlib.h>
#include <time.h>

extern "C" {
    #include "common/circular_queue.h"
}
#include <queue>
#include <climits>


TEST(CircularQueue, ElementsAreCorrect){
    srand (time(NULL));

    circularBuffer_t buffer;

    const size_t bufferSize = 16;
    uint8_t buff[bufferSize];

    circularBufferInit(&buffer, buff, bufferSize, sizeof(char));

    const uint8_t testBurst = 3;

    const uint8_t dataToInsertSize[testBurst] = {5, 10, 3};
    const uint8_t dataToRemoveSize[testBurst] = {3, 7, 2};

    std::queue <uint8_t>queue;

    for(uint8_t j=0; j<testBurst; j++){

        for(uint8_t i = 0;i < dataToInsertSize[j]; i++)
        {
            uint8_t value = rand() % 255;
            queue.push(value);
            circularBufferPushElement(&buffer, &value);
        }
        for(uint8_t i = 0;i < dataToRemoveSize[j]; i++)
        {
            uint8_t value;
            circularBufferPopHead(&buffer, &value);
            EXPECT_EQ(queue.front(),value);
            queue.pop();
        }
        EXPECT_EQ(circularBufferCountElements(&buffer),queue.size());
    }

    EXPECT_EQ(circularBufferIsEmpty(&buffer),queue.empty());

}

TEST(CircularQueue, CheckIsEmptyAndIsFull16){ //check with uint16_t
    srand (time(NULL));

    circularBuffer_t buffer;

    typedef uint16_t tested_type;

    const size_t bufferSize = 16 * sizeof(tested_type);
    uint8_t buff[bufferSize];

    circularBufferInit(&buffer, buff, bufferSize, sizeof(tested_type));

    const int testBurst = 3;

    const int dataToInsertSize[testBurst] = {10, 8, 4};
    const int dataToRemoveSize[testBurst] = {3, 3, 0};
    //At the end we have 0 elements

    std::queue <tested_type>queue;

    EXPECT_EQ(circularBufferIsEmpty(&buffer),true);
    EXPECT_EQ(circularBufferIsFull(&buffer),false);

    for(uint8_t j=0; j<testBurst; j++){

        for(uint8_t i = 0;i < dataToInsertSize[j]; i++)
        {
            tested_type value = rand() % SHRT_MAX;
            queue.push(value);
            circularBufferPushElement(&buffer, (uint8_t*)&value);
        }
        for(uint8_t i = 0;i < dataToRemoveSize[j]; i++)
        {
            tested_type value;
            circularBufferPopHead(&buffer, (uint8_t*)&value);
            EXPECT_EQ(queue.front(),value);
            queue.pop();
        }
        EXPECT_EQ(circularBufferCountElements(&buffer),queue.size());
    }

    EXPECT_EQ(circularBufferIsFull(&buffer),true);

    //Remove all elements
    while(!circularBufferIsEmpty(&buffer))
    {
        tested_type value;
        circularBufferPopHead(&buffer, (uint8_t*)&value);
        EXPECT_EQ(queue.front(),value);
        queue.pop();
    }

    EXPECT_EQ(circularBufferIsFull(&buffer),false);
    EXPECT_EQ(circularBufferIsEmpty(&buffer),true);
    EXPECT_EQ(circularBufferIsEmpty(&buffer),queue.empty());
}

TEST(CircularQueue, CheckIsEmptyAndIsFull32){  //check with uint32_t
    srand (time(NULL));

    circularBuffer_t buffer;

    typedef uint32_t tested_type;

    const size_t bufferSize =  16 * sizeof(tested_type);
    uint8_t buff[bufferSize];

    circularBufferInit(&buffer, buff, bufferSize, sizeof(tested_type));

    const int testBurst = 3;

    const int dataToInsertSize[testBurst] = {10, 8, 4};
    const int dataToRemoveSize[testBurst] = {3, 3, 0};
    //At the end we have 0 elements

    std::queue <tested_type>queue;

    EXPECT_EQ(circularBufferIsEmpty(&buffer),true);
    EXPECT_EQ(circularBufferIsFull(&buffer),false);

    for(uint8_t j=0; j<testBurst; j++){

        for(uint8_t i = 0;i < dataToInsertSize[j]; i++)
        {
            tested_type value = rand() % INT_MAX;
            queue.push(value);
            circularBufferPushElement(&buffer, (uint8_t*)&value);
        }
        for(uint8_t i = 0;i < dataToRemoveSize[j]; i++)
        {
            tested_type value;
            circularBufferPopHead(&buffer, (uint8_t*)&value);
            EXPECT_EQ(queue.front(),value);
            queue.pop();
        }
    EXPECT_EQ(circularBufferCountElements(&buffer),queue.size());
    }

    EXPECT_EQ(circularBufferIsFull(&buffer),true);

    //Remove all elements
    while(!circularBufferIsEmpty(&buffer))
    {
        tested_type value;
        circularBufferPopHead(&buffer, (uint8_t*)&value);
        EXPECT_EQ(queue.front(),value);
        queue.pop();
    }

    EXPECT_EQ(circularBufferIsFull(&buffer),false);
    EXPECT_EQ(circularBufferIsEmpty(&buffer),true);
    EXPECT_EQ(circularBufferIsEmpty(&buffer),queue.empty());
}