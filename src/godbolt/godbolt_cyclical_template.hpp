#include <iostream>
#include <cstdlib>
#include <math.h>


class SchedulerInterface
{
    virtual void tick() = 0;
    virtual void setRunner(RunnerInterface* r) = 0;
};

template <class RunnerType>
class Scheduler : public SchedulerInterface
{
    void tick() override
    {
        runner->action();
    }

    void setRunner(RunnerType* r) override { runner = r; };

    RunnerInterface* runner;
};


class RunnerInterface
{
    virtual void action() = 0;
    virtual void setScheduler(SchedulerInterface* r) = 0;
};

template <class SchedulerType>
class Runner : public RunnerInterface
{
    void advance()
    {
        scheduler->tick();
    }

    void action() override
    {
        std::cout << "foobar\n";
    }

    void setScheduler(SchedulerType* s) override { scheduler = s; };

    SchedulerType* scheduler;
};

int main()
{
    Scheduler<RunnerInterface> scheduler;
    Runner<SchedulerInterface> runner;

    scheduler.setRunner(&runner);
    runner.setScheduler(&scheduler);

    runner.advance();
    return 0;
}
