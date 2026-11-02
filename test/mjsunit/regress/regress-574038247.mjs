// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Resources: test/mjsunit/regress/modules-skip-regress-574038247-c1.mjs
// Resources: test/mjsunit/regress/modules-skip-regress-574038247-c2.mjs
// Resources: test/mjsunit/regress/modules-skip-regress-574038247-p.mjs
// Resources: test/mjsunit/regress/modules-skip-regress-574038247-n.mjs
// Resources: test/mjsunit/regress/modules-skip-regress-574038247-outer.mjs
// Resources: test/mjsunit/regress/modules-skip-regress-574038247-m.mjs
// Resources: test/mjsunit/regress/modules-skip-regress-574038247-trigger.mjs
// Resources: test/mjsunit/regress/modules-skip-regress-574038247-outer_no_ns.mjs
// Resources: test/mjsunit/regress/modules-skip-regress-574038247-m_no_ns.mjs
// Resources: test/mjsunit/regress/modules-skip-regress-574038247-trigger_no_ns.mjs
// Resources: test/mjsunit/regress/modules-skip-regress-574038247-import_x_from_m_no_ns.mjs

// FlyWeb: upstream regress-574038247.js rewritten for V8 13.8, which has no
// d8 --bundle: each bundled module is a modules-skip-regress-574038247-*.mjs
// resource (same names as upstream after the prefix).

import * as ns from "modules-skip-regress-574038247-m.mjs";
import { a } from "modules-skip-regress-574038247-m_no_ns.mjs";

assertEquals(1, a);

function read(o) {
  return o.a + o.b + o.c + o.d + o.e;
}

for (let i = 0; i < 12; i++) {
  assertEquals(5, read(ns));
}

await assertThrowsAsync(import("modules-skip-regress-574038247-trigger.mjs"), SyntaxError);
assertEquals(5, read(ns));

await assertThrowsAsync(import("modules-skip-regress-574038247-trigger_no_ns.mjs"), SyntaxError);
await assertThrowsAsync(import("modules-skip-regress-574038247-import_x_from_m_no_ns.mjs"), SyntaxError);
