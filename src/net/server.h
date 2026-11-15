// Copyright (c) 2026 ChnjFan
// Licensed under the MIT License. See LICENSE in the project root.

#ifndef VASLET_SRC_NET_SERVER_H_
#define VASLET_SRC_NET_SERVER_H_

namespace vaslet {
namespace net {

// Owns the listener and coordinates client connections.
// Template only; module behavior is not implemented yet.
class Server {
   public:
    Server();
    ~Server();

    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;
    Server(Server&&) = delete;
    Server& operator=(Server&&) = delete;
};

}  // namespace net
}  // namespace vaslet

#endif  // VASLET_SRC_NET_SERVER_H_
