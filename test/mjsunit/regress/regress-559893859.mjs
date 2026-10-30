// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --stack-size=100
// Resources: test/mjsunit/regress/modules-skip-regress-559893859-cycle-root.mjs
// Resources: test/mjsunit/regress/modules-skip-regress-559893859-cycle-member.mjs
// Resources: test/mjsunit/regress/modules-skip-regress-559893859-named-import.mjs
// Resources: test/mjsunit/regress/modules-skip-regress-559893859-namespace-import.mjs
// Resources: test/mjsunit/regress/modules-skip-regress-559893859-indirect-export.mjs

// FlyWeb: upstream regress-559893859.js rewritten for V8 13.6, which has no
// d8 --bundle and no import defer (the deferred-import case is dropped).
//
// modules-skip-regress-559893859-cycle-root.mjs and -cycle-member.mjs form one strongly connected
// component. The body of cycle-root is a call with 16000 arguments, so its
// register file is larger than the whole --stack-size=100 stack and
// RunInitializationCode fails halfway through the component. cycle-member
// used to be left LINKED while cycle-root was reset to UNLINKED; the
// importers below reach it through different traversals.

for (const specifier of ["modules-skip-regress-559893859-cycle-root.mjs",
                         "modules-skip-regress-559893859-named-import.mjs",
                         "modules-skip-regress-559893859-namespace-import.mjs",
                         "modules-skip-regress-559893859-indirect-export.mjs",
                         "modules-skip-regress-559893859-cycle-root.mjs",
                         "modules-skip-regress-559893859-cycle-member.mjs"]) {
  await assertThrowsAsync(import(specifier), RangeError);
}
