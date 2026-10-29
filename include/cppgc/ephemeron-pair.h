// Copyright 2020 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef INCLUDE_CPPGC_EPHEMERON_PAIR_H_
#define INCLUDE_CPPGC_EPHEMERON_PAIR_H_

#include "cppgc/liveness-broker.h"
#include "cppgc/member.h"

namespace cppgc {

/**
 * An ephemeron pair is used to conditionally retain an object.
 * The `value` will be kept alive only if the `key` is alive.
 */
template <typename K, typename V>
struct EphemeronPair {
  EphemeronPair(K* k, V* v) : key(k), value(v) {}
  WeakMember<K> key;
  Member<V> value;

  void ClearValueIfKeyIsDead(const LivenessBroker& broker) {
    if (!broker.IsHeapObjectAlive(key)) value = nullptr;
  }

  // FlyWeb (upstream 5a113ffdf2): also clear the dead key, so that no dangling
  // pointer stays behind once the weak key is collected.
  void ClearKeyAndValueIfKeyIsDead(const LivenessBroker& broker) {
    if (!broker.IsHeapObjectAlive(key)) {
      key = nullptr;
      value = nullptr;
    }
  }
};

}  // namespace cppgc

#endif  // INCLUDE_CPPGC_EPHEMERON_PAIR_H_
