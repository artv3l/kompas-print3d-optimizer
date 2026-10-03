#pragma once

class IProgressBar
{
  public:
    virtual ~IProgressBar() = default;

    virtual void start(size_t min, size_t max) = 0;
    virtual void update(size_t value) = 0;
    virtual void stop() = 0;
};
