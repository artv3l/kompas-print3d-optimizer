#include "kapiwrap/ProgressBar.hpp"

ProgressBar::ProgressBar(ksapi::IApplication & application)
    : m_progressBar(application.GetProgressBar())
{
}

void ProgressBar::start(size_t min, size_t max)
{
    m_progressBar->Start(static_cast<int32_t>(min), static_cast<int32_t>(max));
}

void ProgressBar::update(size_t value)
{
    m_progressBar->SetProgress(static_cast<int32_t>(value));
}

void ProgressBar::stop()
{
    m_progressBar->Stop();
}
