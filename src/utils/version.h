// Copyright 2009 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_UTILS_VERSION_H_
#define V8_UTILS_VERSION_H_

#include <cstdint>

#include "src/base/hashing.h"

namespace v8 {

namespace base {
template <typename T>
class Vector;
}  // namespace base

namespace internal {

class V8_EXPORT Version {
 public:
  // Return the various version components.
  static int GetMajor() { return major_; }
  static int GetMinor() { return minor_; }
  static int GetBuild() { return build_; }
  static int GetPatch() { return patch_; }
  static const char* GetEmbedder() { return embedder_; }
  static bool IsCandidate() { return candidate_; }
  // FlyWeb: V8 code caches (Code Cache/js, service workers) are only checked
  // against this hash and the non-default flags, so FlyWeb's own V8 changes
  // (new builtins, runtime functions, flags shipped by default) would accept
  // code cached by an earlier FlyWeb and crash. Raise kFlyWebCacheEpoch with
  // every change to FlyWeb's V8 patches. 2 = FlyWeb 1.1.1 (V8 11.6), 3 = V8
  // 11.6 with Promise.withResolvers. From engine level 118 each level has its
  // own V8 version, which changes the hash by itself; still raise it when the
  // patches change within the same V8 version. 4 = FlyWeb 1.10 (V8 12.6
  // with the gni patch), 5 = engine level 127 (V8 12.7), 6 = seg/v8-12.7,
  // 7 = FM.17 development branch with v8 aaa23fc87, 8 = reserved for the
  // next seg/v8-12.7, 9 = FM.17 on V8 12.8 (nube/v8-12.8-api), 10 = FlyWeb
  // 1.11.2 (V8 12.8 with SEGURIDAD's backports, seg/v8-12.8), 11 = the same
  // with v8 3349cb9fd (microtask completed callbacks), 12 = seg/v8-12.8,
  // 13 = seg/v8-12.9 (FlyWeb 1.13), 14 = reserved (never shipped),
  // 15 = seg/v8-13.0 (FlyWeb 1.14), 16 = seg/v8-13.1 (FlyWeb 1.15),
  // 17 = FlyWeb 2.0.1 (CVE-2026-11645), 18 = seg/v8-13.1-lle (FlyWeb 2.0.4),
  // 19 = seg/v8-13.2 (engine level 132), 20 = seg/fs29-v8 (new bytecode
  // DefineNamedOwnPropertyInLiteral), 21 = engine level 133 (V8 13.3 with
  // the M132-LTS backports it lacks), 22 = engine level 134 (V8 13.4 with
  // the same backports and the Maglev debt of seg/maglev-134, whose
  // af7d1483a5 changes bytecode), 23 = engine level 135 (V8 13.5 with the
  // same backports), 24 = seg/fs36-v8 (FS.36: post-135 merges, several of
  // which change bytecode and parsing, e.g. hole check elision on break and
  // continue, class member initializer reparse), 25 = engine level 136 (V8
  // 13.6 with the same backports), 26 = seg/fs37-v8 (FS.37: post-136
  // merges; 848d7f4122 changes the bytecode of `await using` in for loops),
  // 27 = engine level 137 (V8 13.7 with the same backports), 28 =
  // seg/fs38-v8 (FS.38: post-137 merges; no bytecode change, but new
  // builtins code and code cache checks), 29 = engine level 138 (V8 13.8
  // with the same backports), 30 = seg/fs39-v8 (FS.39: post-138 merges; no
  // bytecode change, but new Promise.any builtin code, the code cache FPU
  // state and the redone 13.7 patches).
  static constexpr int kFlyWebCacheEpoch = 30;
  static uint32_t Hash() {
    return static_cast<uint32_t>(base::hash_combine(
        major_, minor_, build_, patch_, kFlyWebCacheEpoch));
  }

  // Calculate the V8 version string.
  static void GetString(base::Vector<char> str);

  // Calculate the SONAME for the V8 shared library.
  static void GetSONAME(base::Vector<char> str);

  static const char* GetVersion() { return version_string_; }

 private:
  // NOTE: can't make these really const because of test-version.cc.
  static int major_;
  static int minor_;
  static int build_;
  static int patch_;
  static const char* embedder_;
  static bool candidate_;
  static const char* soname_;
  static const char* version_string_;

  // In test-version.cc.
  friend void SetVersion(int major, int minor, int build, int patch,
                         const char* embedder, bool candidate,
                         const char* soname);
};

}  // namespace internal
}  // namespace v8

#endif  // V8_UTILS_VERSION_H_
