// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/builtins/builtins-wasm-gen.h"

#include "src/builtins/builtins-utils-gen.h"
#include "src/codegen/code-stub-assembler-inl.h"
#include "src/codegen/interface-descriptors.h"
#include "src/objects/map-inl.h"
#include "src/objects/objects-inl.h"
#include "src/wasm/wasm-objects.h"

namespace v8::internal {

#include "src/codegen/define-code-stub-assembler-macros.inc"

TNode<WasmTrustedInstanceData>
WasmBuiltinsAssembler::LoadInstanceDataFromFrame() {
  return CAST(LoadFromParentFrame(WasmFrameConstants::kWasmInstanceDataOffset));
}

TNode<WasmTrustedInstanceData>
WasmBuiltinsAssembler::LoadTrustedDataFromInstance(
    TNode<WasmInstanceObject> instance_object) {
  return CAST(LoadTrustedPointerFromObject(
      instance_object, WasmInstanceObject::kTrustedDataOffset,
      kWasmTrustedInstanceDataIndirectPointerTag));
}

TNode<NativeContext> WasmBuiltinsAssembler::LoadContextFromWasmOrJsFrame() {
  static_assert(BuiltinFrameConstants::kFunctionOffset ==
                WasmFrameConstants::kWasmInstanceDataOffset);
  TVARIABLE(NativeContext, context_result);
  TNode<HeapObject> function_or_instance =
      CAST(LoadFromParentFrame(WasmFrameConstants::kWasmInstanceDataOffset));
  Label is_js_function(this);
  Label is_import_data(this);
  Label done(this);
  TNode<Uint16T> instance_type =
      LoadMapInstanceType(LoadMap(function_or_instance));
  GotoIf(IsJSFunctionInstanceType(instance_type), &is_js_function);
  GotoIf(Word32Equal(instance_type, Int32Constant(WASM_IMPORT_DATA_TYPE)),
         &is_import_data);
  context_result = LoadContextFromInstanceData(CAST(function_or_instance));
  Goto(&done);

  BIND(&is_js_function);
  TNode<JSFunction> function = CAST(function_or_instance);
  TNode<Context> context =
      LoadObjectField<Context>(function, JSFunction::kContextOffset);
  context_result = LoadNativeContext(context);
  Goto(&done);

  BIND(&is_import_data);
  TNode<WasmImportData> import_data = CAST(function_or_instance);
  context_result = LoadObjectField<NativeContext>(
      import_data, WasmImportData::kNativeContextOffset);
  Goto(&done);

  BIND(&done);
  return context_result.value();
}

TNode<NativeContext> WasmBuiltinsAssembler::LoadContextFromInstanceData(
    TNode<WasmTrustedInstanceData> trusted_data) {
  return CAST(
      Load(MachineType::AnyTagged(), trusted_data,
           IntPtrConstant(WasmTrustedInstanceData::kNativeContextOffset -
                          kHeapObjectTag)));
}

TNode<WasmTrustedInstanceData>
WasmBuiltinsAssembler::LoadSharedPartFromInstanceData(
    TNode<WasmTrustedInstanceData> trusted_data) {
  return CAST(LoadProtectedPointerFromObject(
      trusted_data,
      IntPtrConstant(WasmTrustedInstanceData::kProtectedSharedPartOffset -
                     kHeapObjectTag)));
}

TNode<FixedArray> WasmBuiltinsAssembler::LoadTablesFromInstanceData(
    TNode<WasmTrustedInstanceData> trusted_data) {
  return LoadObjectField<FixedArray>(trusted_data,
                                     WasmTrustedInstanceData::kTablesOffset);
}

TNode<FixedArray> WasmBuiltinsAssembler::LoadFuncRefsFromInstanceData(
    TNode<WasmTrustedInstanceData> trusted_data) {
  return LoadObjectField<FixedArray>(trusted_data,
                                     WasmTrustedInstanceData::kFuncRefsOffset);
}

TNode<FixedArray> WasmBuiltinsAssembler::LoadManagedObjectMapsFromInstanceData(
    TNode<WasmTrustedInstanceData> trusted_data) {
  return LoadObjectField<FixedArray>(
      trusted_data, WasmTrustedInstanceData::kManagedObjectMapsOffset);
}

TNode<Float64T> WasmBuiltinsAssembler::StringToFloat64(TNode<String> input) {
#ifdef V8_ENABLE_FP_PARAMS_IN_C_LINKAGE
  TNode<ExternalReference> string_to_float64 =
      ExternalConstant(ExternalReference::wasm_string_to_f64());
  return TNode<Float64T>::UncheckedCast(
      CallCFunction(string_to_float64, MachineType::Float64(),
                    std::make_pair(MachineType::AnyTagged(), input)));
#else
  // We could support the fast path by passing the float via a stackslot, see
  // MachineOperatorBuilder::StackSlot.
  TNode<Object> result =
      CallRuntime(Runtime::kStringParseFloat, NoContextConstant(), input);
  return ChangeNumberToFloat64(CAST(result));
#endif
}

TNode<Smi> WasmBuiltinsAssembler::SignatureCheckFail(
    TNode<WasmInternalFunction> internal_function,
    TNode<UintPtrT> expected_hash) {
  TNode<ExternalReference> function =
      ExternalConstant(ExternalReference::wasm_signature_check_fail());
  // The C-side return type is "void", but "None()" isn't supported here.
  // Since we ignore the result anyway, it doesn't matter to pretend there's
  // a pointer in the return register.
  CallCFunction(function, MachineType::Pointer(),
                std::make_pair(MachineType::AnyTagged(), internal_function),
                std::make_pair(MachineType::UintPtr(), expected_hash));
  return SmiConstant(0);
}

TF_BUILTIN(WasmFloat32ToNumber, WasmBuiltinsAssembler) {
  auto val = UncheckedParameter<Float32T>(Descriptor::kValue);
  Return(ChangeFloat32ToTagged(val));
}

TF_BUILTIN(WasmFloat64ToNumber, WasmBuiltinsAssembler) {
  auto val = UncheckedParameter<Float64T>(Descriptor::kValue);
  Return(ChangeFloat64ToTagged(val));
}

TF_BUILTIN(WasmFloat64ToString, WasmBuiltinsAssembler) {
  TNode<Float64T> val = UncheckedParameter<Float64T>(Descriptor::kValue);
  // Having to allocate a HeapNumber is a bit unfortunate, but the subsequent
  // runtime call will have to allocate a string anyway, which probably
  // dwarfs the cost of one more small allocation here.
  TNode<Number> tagged = ChangeFloat64ToTagged(val);
  Return(NumberToString(tagged));
}

TF_BUILTIN(JSToWasmLazyDeoptContinuation, WasmBuiltinsAssembler) {
  // Reset thread_in_wasm_flag.
  TNode<ExternalReference> thread_in_wasm_flag_address_address =
      ExternalConstant(
          ExternalReference::thread_in_wasm_flag_address_address(isolate()));
  auto thread_in_wasm_flag_address =
      Load<RawPtrT>(thread_in_wasm_flag_address_address);
  StoreNoWriteBarrier(MachineRepresentation::kWord32,
                      thread_in_wasm_flag_address, Int32Constant(0));

  // Return the argument.
  auto value = Parameter<Object>(Descriptor::kArgument);
  Return(value);
}

TF_BUILTIN(WasmToJsWrapperCSA, WasmBuiltinsAssembler) {
  TorqueStructWasmToJSResult result = WasmToJSWrapper(
      UncheckedParameter<WasmImportData>(Descriptor::kWasmImportData));
  PopAndReturn(result.popCount, result.result0, result.result1, result.result2,
               result.result3);
}

TF_BUILTIN(WasmToJsWrapperInvalidSig, WasmBuiltinsAssembler) {
  TNode<WasmImportData> data =
      UncheckedParameter<WasmImportData>(Descriptor::kWasmImportData);
  TNode<Context> context =
      LoadObjectField<Context>(data, WasmImportData::kNativeContextOffset);

  CallRuntime(Runtime::kWasmThrowJSTypeError, context);
  Unreachable();
}

// FlyWeb: CVE-2026-87491 (v8 36079c36, crbug 543557673), same interface as
// the old Torque builtin (it keeps the context parameter, so callers are
// unchanged). Wasm uses private symbols (e.g. wasm_exception_tag_symbol) to
// stamp certain objects. This builtin checks for presence of such symbols; it
// is a specialization of "GetOwnProperty" behavior: if the object is not a
// plain JS object or the property is not an own data property, return
// undefined. To allow compilers to assume no side effects from this builtin,
// it's particularly important to not invoke any getters.
TF_BUILTIN(WasmGetOwnProperty, WasmBuiltinsAssembler) {
  TNode<Object> object = Parameter<Object>(Descriptor::kObject);
  TNode<Name> name = Parameter<Name>(Descriptor::kUniqueName);

  Label return_undefined(this, Label::kDeferred);

  GotoIf(TaggedIsSmi(object), &return_undefined);
  TNode<HeapObject> heap_object = CAST(object);

  TNode<Map> map = LoadMap(heap_object);
  TNode<Int32T> instance_type = LoadMapInstanceType(map);

  GotoIfNot(IsJSReceiverInstanceType(instance_type), &return_undefined);
  GotoIf(IsSpecialReceiverInstanceType(instance_type), &return_undefined);

  TNode<JSObject> js_object = CAST(heap_object);

  {
    TVARIABLE(HeapObject, var_meta_storage);
    TVARIABLE(IntPtrT, var_entry);
    TVARIABLE(Object, var_value);
    TVARIABLE(Uint32T, var_details);
    Label if_found_fast(this), if_found_dict(this), got_value(this);

    TryLookupPropertyInSimpleObject(js_object, map, name, &if_found_fast,
                                    &if_found_dict, &var_meta_storage,
                                    &var_entry, &return_undefined,
                                    &return_undefined);

    BIND(&if_found_fast);
    {
      TNode<DescriptorArray> descriptors = CAST(var_meta_storage.value());
      LoadPropertyFromFastObject(js_object, map, descriptors, var_entry.value(),
                                 &var_details, &var_value);
      Goto(&got_value);
    }

    BIND(&if_found_dict);
    {
      TNode<PropertyDictionary> dictionary = CAST(var_meta_storage.value());
      LoadPropertyFromDictionary(dictionary, var_entry.value(), &var_details,
                                 &var_value);
      Goto(&got_value);
    }

    BIND(&got_value);
    {
      TNode<Uint32T> kind =
          DecodeWord32<PropertyDetails::KindField>(var_details.value());
      constexpr int kData = static_cast<int>(PropertyKind::kData);
      GotoIfNot(Word32Equal(kind, Int32Constant(kData)), &return_undefined);
      Return(var_value.value());
    }
  }

  BIND(&return_undefined);
  Return(UndefinedConstant());
}

#include "src/codegen/undef-code-stub-assembler-macros.inc"

}  // namespace v8::internal
