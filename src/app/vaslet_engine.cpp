// Copyright (c) 2026 ChnjFan
// Licensed under the MIT License. See LICENSE in the project root.

#include "app/vaslet_engine.h"

#include <memory>

#include "app/startup_config.h"
#include "net/server.h"

namespace vaslet {
namespace app {

class VasletEngine::Impl {
   public:
    explicit Impl(const StartupConfig& config);
    ~Impl();

    void Initialize();

    void Run();

   private:
    StartupConfig config_;
    std::unique_ptr<net::Server> server_;
};

VasletEngine::Impl::Impl(const StartupConfig& config)
    : config_(config), server_(std::make_unique<net::Server>()) {}

VasletEngine::Impl::~Impl() = default;

void VasletEngine::Impl::Initialize() {
    // Module initialization will be implemented here.
}

void VasletEngine::Impl::Run() {
    // The server run loop will be implemented here.
}

VasletEngine::VasletEngine(const StartupConfig& config)
    : impl_(std::make_unique<Impl>(config)) {}

VasletEngine::~VasletEngine() = default;

void VasletEngine::Initialize() { impl_->Initialize(); }

void VasletEngine::Run() { impl_->Run(); }

}  // namespace app
}  // namespace vaslet
