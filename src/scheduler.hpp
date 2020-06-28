#pragma once

struct SimpleScheduler
{
    SimpleScheduler(int interval) : interval(interval) {}

    int tick(int numSamples)
    {
        counter += numSamples;

        if (counter <= interval)
            return 0;

        numSamples = counter - interval;
        counter = 0;

        return numSamples;
    }

    int interval = 10;
    int counter = 0;
};
