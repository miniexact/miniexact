const memory = new WebAssembly.Memory({
  initial: 50,
  maximum: 2000,
});

WebAssembly.instantiateStreaming(fetch("../build-wasi/xccsolve.wasm"), memory).then(
  (obj) => {
    // Call an exported function:
    obj.instance.exports.exported_func();

    // or access the buffer contents of an exported memory:
    const i32 = new Uint32Array(obj.instance.exports.memory.buffer);

    // or access the elements of an exported table:
    const table = obj.instance.exports.table;
    console.log(table.get(0)());
  },
);
