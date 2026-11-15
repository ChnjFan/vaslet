// Copyright (c) 2026 ChnjFan
// Licensed under the MIT License. See LICENSE in the project root.

#ifndef VASLET_SRC_NET_CONNECTION_H_
#define VASLET_SRC_NET_CONNECTION_H_

namespace vaslet {
namespace net {

// Owns a client connection and its receive and send buffers.
// Template only; module behavior is not implemented yet.
class Connection {
   public:
    Connection();
    ~Connection();

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;
    Connection(Connection&&) = delete;
    Connection& operator=(Connection&&) = delete;
};

}  // namespace net
}  // namespace vaslet

#endif  // VASLET_SRC_NET_CONNECTION_H_
