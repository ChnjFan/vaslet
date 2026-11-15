// Copyright (c) 2026 ChnjFan
// Licensed under the MIT License. See LICENSE in the project root.

#ifndef VASLET_SRC_CONFIG_CONFIG_STORE_H_
#define VASLET_SRC_CONFIG_CONFIG_STORE_H_

namespace vaslet {
namespace config {

// Owns configuration state and manages persistence.
// Template only; module behavior is not implemented yet.
class ConfigStore {
   public:
    ConfigStore();
    ~ConfigStore();

    ConfigStore(const ConfigStore&) = delete;
    ConfigStore& operator=(const ConfigStore&) = delete;
    ConfigStore(ConfigStore&&) = delete;
    ConfigStore& operator=(ConfigStore&&) = delete;
};

}  // namespace config
}  // namespace vaslet

#endif  // VASLET_SRC_CONFIG_CONFIG_STORE_H_
