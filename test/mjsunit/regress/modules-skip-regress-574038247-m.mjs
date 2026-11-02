// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Resource of regress-574038247.mjs (upstream bundle module m.mjs).
const a = 1;
export { a, a as b, a as c, a as d, a as e };
export * from "modules-skip-regress-574038247-outer.mjs";
export * from "modules-skip-regress-574038247-n.mjs";
