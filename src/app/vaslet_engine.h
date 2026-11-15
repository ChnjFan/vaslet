// Copyright (c) 2026 ChnjFan
// Licensed under the MIT License. See LICENSE in the project root.

#ifndef VASLET_SRC_APP_VASLET_ENGINE_H_
#define VASLET_SRC_APP_VASLET_ENGINE_H_

#include <memory>

namespace vaslet {
namespace app {

struct StartupConfig;

// Coordinates server module ownership and lifecycle.
// Module initialization, execution, and shutdown are not implemented yet.
class VasletEngine {
public:
    // Keeps a copy of config for the engine's lifetime.
    explicit VasletEngine(const StartupConfig& config);
    ~VasletEngine();

    VasletEngine(const VasletEngine&) = delete;
    VasletEngine& operator=(const VasletEngine&) = delete;
    VasletEngine(VasletEngine&&) = delete;
    VasletEngine& operator=(VasletEngine&&) = delete;

    // Placeholder for module initialization; currently performs no work.
    void Initialize();

    // Placeholder for the server run loop; currently returns immediately.
    void Run();

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace app
}  // namespace vaslet

#endif  // VASLET_SRC_APP_VASLET_ENGINE_H_
