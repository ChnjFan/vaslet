// Copyright (c) 2026 ChnjFan
// Licensed under the MIT License. See LICENSE in the project root.

#ifndef VASLET_SRC_CONFIG_CONFIG_SERVICE_H_
#define VASLET_SRC_CONFIG_CONFIG_SERVICE_H_

namespace vaslet {
namespace config {

// Coordinates configuration operations and business validation.
// Template only; module behavior is not implemented yet.
class ConfigService {
   public:
    ConfigService();
    ~ConfigService();

    ConfigService(const ConfigService&) = delete;
    ConfigService& operator=(const ConfigService&) = delete;
    ConfigService(ConfigService&&) = delete;
    ConfigService& operator=(ConfigService&&) = delete;
};

}  // namespace config
}  // namespace vaslet

#endif  // VASLET_SRC_CONFIG_CONFIG_SERVICE_H_
