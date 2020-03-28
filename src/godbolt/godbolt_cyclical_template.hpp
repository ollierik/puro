#include <iostream>
#include <cstdlib>
#include <math.h>

template <class ElementType>
class EngineInterface
{
public:
    virtual ElementType getNumber() = 0;
};

template <class ElementType>
class SchedulerTemplate
{
public:

    int tick()
    {
        return engine->getNumber();
    }

    EngineInterface<ElementType>* engine;
};

template <class ElementType, class SchedulerType>
class EngineTemplate : public EngineInterface<ElementType>
{
public:

    EngineTemplate(SchedulerType& s) : scheduler(s)
    {
        scheduler.engine = this;
    }

    int advance()
    {
        return scheduler.tick();
    }

    ElementType getNumber() override
    {
        return static_cast<ElementType> (std::rand());
    }

    SchedulerType& scheduler;
};

int main()
{
    using Element = int;
    using Scheduler = SchedulerTemplate<Element>;
    using Engine = EngineTemplate<Element, Scheduler>;

    Scheduler scheduler;
    Engine engine(scheduler);

    return engine.advance();
}
