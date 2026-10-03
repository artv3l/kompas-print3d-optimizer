#pragma once

#include <KsAPI.h>

#include "generic/IProgressBar.hpp"

class ProgressBar : public IProgressBar
{
  public:
    ProgressBar(ksapi::IApplication & application);

    void start(size_t min, size_t max) override;
    void update(size_t value) override;
    void stop() override;

  private:
    ksapi::IProgressBarPtr m_progressBar;
};
