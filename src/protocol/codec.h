// Copyright (c) 2026 ChnjFan
// Licensed under the MIT License. See LICENSE in the project root.

#ifndef VASLET_SRC_PROTOCOL_CODEC_H_
#define VASLET_SRC_PROTOCOL_CODEC_H_

namespace vaslet {
namespace protocol {

// Decodes framed requests and encodes responses.
// Template only; module behavior is not implemented yet.
class Codec {
   public:
    Codec();
    ~Codec();
};

}  // namespace protocol
}  // namespace vaslet

#endif  // VASLET_SRC_PROTOCOL_CODEC_H_
