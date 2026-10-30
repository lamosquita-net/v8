// Copyright 2026 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// FlyWeb (FS.38): JSPI stacks return through the parent's own return address
// (v8 3137a74400) and WasmResume/WasmReject check their argument count
// (v8 49a1dbeb1b). Exercise completion and rejection from the initial wrapper
// and from resumed stacks, many times, so that a wrong stack cleanup shows up
// as a crash or a wrong result.

d8.file.execute('test/mjsunit/wasm/wasm-module-builder.js');

let builder = new WasmModuleBuilder();
let suspend_idx = builder.addImport('m', 'suspend', kSig_i_i);
let throw_idx = builder.addImport('m', 'maybe_throw', kSig_v_i);
builder.addFunction('twice', kSig_i_i)
    .addBody([
      kExprLocalGet, 0,
      kExprCallFunction, suspend_idx,
      kExprCallFunction, suspend_idx,
      kExprLocalGet, 0,
      kExprCallFunction, throw_idx,
    ])
    .exportFunc();
builder.addFunction('no_suspend', kSig_i_i)
    .addBody([
      kExprLocalGet, 0,
      kExprI32Const, 1,
      kExprI32Add,
    ])
    .exportFunc();

let instance = builder.instantiate({
  m: {
    suspend: new WebAssembly.Suspending(x => Promise.resolve(x + 1)),
    maybe_throw: x => { if (x % 3 == 0) throw new Error('boom ' + x); },
  }
});
let twice = WebAssembly.promising(instance.exports.twice);
let no_suspend = WebAssembly.promising(instance.exports.no_suspend);

async function run() {
  for (let i = 1; i < 200; i++) {
    assertEquals(i + 1, await no_suspend(i));
    if (i % 3 == 0) {
      await assertThrowsAsync(twice(i), Error, 'boom ' + i);
    } else {
      assertEquals(i + 2, await twice(i));
    }
  }
  // Several stacks in flight at once.
  let results = await Promise.allSettled(
      [1, 2, 3, 4, 5, 6].map(x => twice(x)));
  for (let k = 0; k < results.length; k++) {
    let x = k + 1;
    if (x % 3 == 0) {
      assertEquals('rejected', results[k].status);
    } else {
      assertEquals(x + 2, results[k].value);
    }
  }
}

assertPromiseResult(run());
