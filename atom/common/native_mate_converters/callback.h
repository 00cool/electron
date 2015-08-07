// Copyright (c) 2015 GitHub, Inc. All rights reserved.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#include "base/bind.h"
#include "base/callback.h"
#include "native_mate/function_template.h"
#include "native_mate/locker.h"
#include "native_mate/scoped_persistent.h"

namespace mate {

namespace internal {

typedef scoped_refptr<RefCountedPersistent<v8::Function> > SafeV8Function;

// This set of templates invokes a V8::Function by converting the C++ types.
template<typename Sig>
struct V8FunctionInvoker;

template<typename V>
struct V8FunctionInvoker<v8::Local<V>()> {
  static v8::Local<V> Go(v8::Isolate* isolate, SafeV8Function function) {
    Locker locker(isolate);
    v8::EscapableHandleScope handle_scope(isolate);
    v8::Local<v8::Function> holder = function->NewHandle();
    v8::Local<v8::Context> context = holder->CreationContext();
    v8::Context::Scope context_scope(context);
    v8::Local<v8::Value> val(holder->Call(holder, 0, NULL));
        return handle_scope.Escape(val);
  }
};

template<typename R>
struct V8FunctionInvoker<R()> {
  static R Go(v8::Isolate* isolate, SafeV8Function function) {
    R ret;
    Locker locker(isolate);
    MATE_HANDLE_SCOPE(isolate);
    v8::Local<v8::Function> holder = function->NewHandle();
    v8::Local<v8::Context> context = holder->CreationContext();
    v8::Context::Scope context_scope(context);
    v8::Local<v8::Value> val(holder->Call(holder, 0, NULL));
    Converter<R>::FromV8(isolate, val, &ret);
    return ret;
  }
};

template<>
struct V8FunctionInvoker<void()> {
  static void Go(v8::Isolate* isolate, SafeV8Function function) {
    Locker locker(isolate);
    MATE_HANDLE_SCOPE(isolate);
    v8::Local<v8::Function> holder = function->NewHandle();
    v8::Local<v8::Context> context = holder->CreationContext();
    v8::Context::Scope context_scope(context);
    holder->Call(holder, 0, NULL);
  }
};

template<typename V, typename P1>
struct V8FunctionInvoker<v8::Local<V>(P1)> {
  static v8::Local<V> Go(v8::Isolate* isolate, SafeV8Function function, P1 a1) {
    Locker locker(isolate);
    v8::EscapableHandleScope handle_scope(isolate);
    v8::Local<v8::Function> holder = function->NewHandle();
    v8::Local<v8::Context> context = holder->CreationContext();
    v8::Context::Scope context_scope(context);
    v8::Local<v8::Value> args[] = {
        ConvertToV8(isolate, a1),
    };
    v8::Local<v8::Value> val(holder->Call(holder, arraysize(args), args));
        return handle_scope.Escape(val);
  }
};

template<typename R, typename P1>
struct V8FunctionInvoker<R(P1)> {
  static R Go(v8::Isolate* isolate, SafeV8Function function, P1 a1) {
    R ret;
    Locker locker(isolate);
    MATE_HANDLE_SCOPE(isolate);
    v8::Local<v8::Function> holder = function->NewHandle();
    v8::Local<v8::Context> context = holder->CreationContext();
    v8::Context::Scope context_scope(context);
    v8::Local<v8::Value> args[] = {
        ConvertToV8(isolate, a1),
    };
    v8::Local<v8::Value> val(holder->Call(holder, arraysize(args), args));
    Converter<R>::FromV8(isolate, val, &ret);
    return ret;
  }
};

template<typename P1>
struct V8FunctionInvoker<void(P1)> {
  static void Go(v8::Isolate* isolate, SafeV8Function function, P1 a1) {
    Locker locker(isolate);
    MATE_HANDLE_SCOPE(isolate);
    v8::Local<v8::Function> holder = function->NewHandle();
    v8::Local<v8::Context> context = holder->CreationContext();
    v8::Context::Scope context_scope(context);
    v8::Local<v8::Value> args[] = {
        ConvertToV8(isolate, a1),
    };
    holder->Call(holder, arraysize(args), args);
  }
};

template<typename V, typename P1, typename P2>
struct V8FunctionInvoker<v8::Local<V>(P1, P2)> {
  static v8::Local<V> Go(v8::Isolate* isolate, SafeV8Function function, P1 a1,
      P2 a2) {
    Locker locker(isolate);
    v8::EscapableHandleScope handle_scope(isolate);
    v8::Local<v8::Function> holder = function->NewHandle();
    v8::Local<v8::Context> context = holder->CreationContext();
    v8::Context::Scope context_scope(context);
    v8::Local<v8::Value> args[] = {
        ConvertToV8(isolate, a1),
        ConvertToV8(isolate, a2),
    };
    v8::Local<v8::Value> val(holder->Call(holder, arraysize(args), args));
        return handle_scope.Escape(val);
  }
};

template<typename R, typename P1, typename P2>
struct V8FunctionInvoker<R(P1, P2)> {
  static R Go(v8::Isolate* isolate, SafeV8Function function, P1 a1, P2 a2) {
    R ret;
    Locker locker(isolate);
    MATE_HANDLE_SCOPE(isolate);
    v8::Local<v8::Function> holder = function->NewHandle();
    v8::Local<v8::Context> context = holder->CreationContext();
    v8::Context::Scope context_scope(context);
    v8::Local<v8::Value> args[] = {
        ConvertToV8(isolate, a1),
        ConvertToV8(isolate, a2),
    };
    v8::Local<v8::Value> val(holder->Call(holder, arraysize(args), args));
    Converter<R>::FromV8(isolate, val, &ret);
    return ret;
  }
};

template<typename P1, typename P2>
struct V8FunctionInvoker<void(P1, P2)> {
  static void Go(v8::Isolate* isolate, SafeV8Function function, P1 a1, P2 a2) {
    Locker locker(isolate);
    MATE_HANDLE_SCOPE(isolate);
    v8::Local<v8::Function> holder = function->NewHandle();
    v8::Local<v8::Context> context = holder->CreationContext();
    v8::Context::Scope context_scope(context);
    v8::Local<v8::Value> args[] = {
        ConvertToV8(isolate, a1),
        ConvertToV8(isolate, a2),
    };
    holder->Call(holder, arraysize(args), args);
  }
};

template<typename V, typename P1, typename P2, typename P3>
struct V8FunctionInvoker<v8::Local<V>(P1, P2, P3)> {
  static v8::Local<V> Go(v8::Isolate* isolate, SafeV8Function function, P1 a1,
      P2 a2, P3 a3) {
    Locker locker(isolate);
    v8::EscapableHandleScope handle_scope(isolate);
    v8::Local<v8::Function> holder = function->NewHandle();
    v8::Local<v8::Context> context = holder->CreationContext();
    v8::Context::Scope context_scope(context);
    v8::Local<v8::Value> args[] = {
        ConvertToV8(isolate, a1),
        ConvertToV8(isolate, a2),
        ConvertToV8(isolate, a3),
    };
    v8::Local<v8::Value> val(holder->Call(holder, arraysize(args), args));
        return handle_scope.Escape(val);
  }
};

template<typename R, typename P1, typename P2, typename P3>
struct V8FunctionInvoker<R(P1, P2, P3)> {
  static R Go(v8::Isolate* isolate, SafeV8Function function, P1 a1, P2 a2,
      P3 a3) {
    R ret;
    Locker locker(isolate);
    MATE_HANDLE_SCOPE(isolate);
    v8::Local<v8::Function> holder = function->NewHandle();
    v8::Local<v8::Context> context = holder->CreationContext();
    v8::Context::Scope context_scope(context);
    v8::Local<v8::Value> args[] = {
        ConvertToV8(isolate, a1),
        ConvertToV8(isolate, a2),
        ConvertToV8(isolate, a3),
    };
    v8::Local<v8::Value> val(holder->Call(holder, arraysize(args), args));
    Converter<R>::FromV8(isolate, val, &ret);
    return ret;
  }
};

template<typename P1, typename P2, typename P3>
struct V8FunctionInvoker<void(P1, P2, P3)> {
  static void Go(v8::Isolate* isolate, SafeV8Function function, P1 a1, P2 a2,
      P3 a3) {
    Locker locker(isolate);
    MATE_HANDLE_SCOPE(isolate);
    v8::Local<v8::Function> holder = function->NewHandle();
    v8::Local<v8::Context> context = holder->CreationContext();
    v8::Context::Scope context_scope(context);
    v8::Local<v8::Value> args[] = {
        ConvertToV8(isolate, a1),
        ConvertToV8(isolate, a2),
        ConvertToV8(isolate, a3),
    };
    holder->Call(holder, arraysize(args), args);
  }
};

template<typename V, typename P1, typename P2, typename P3, typename P4>
struct V8FunctionInvoker<v8::Local<V>(P1, P2, P3, P4)> {
  static v8::Local<V> Go(v8::Isolate* isolate, SafeV8Function function, P1 a1,
      P2 a2, P3 a3, P4 a4) {
    Locker locker(isolate);
    v8::EscapableHandleScope handle_scope(isolate);
    v8::Local<v8::Function> holder = function->NewHandle();
    v8::Local<v8::Context> context = holder->CreationContext();
    v8::Context::Scope context_scope(context);
    v8::Local<v8::Value> args[] = {
        ConvertToV8(isolate, a1),
        ConvertToV8(isolate, a2),
        ConvertToV8(isolate, a3),
        ConvertToV8(isolate, a4),
    };
    v8::Local<v8::Value> val(holder->Call(holder, arraysize(args), args));
        return handle_scope.Escape(val);
  }
};

template<typename R, typename P1, typename P2, typename P3, typename P4>
struct V8FunctionInvoker<R(P1, P2, P3, P4)> {
  static R Go(v8::Isolate* isolate, SafeV8Function function, P1 a1, P2 a2,
      P3 a3, P4 a4) {
    R ret;
    Locker locker(isolate);
    MATE_HANDLE_SCOPE(isolate);
    v8::Local<v8::Function> holder = function->NewHandle();
    v8::Local<v8::Context> context = holder->CreationContext();
    v8::Context::Scope context_scope(context);
    v8::Local<v8::Value> args[] = {
        ConvertToV8(isolate, a1),
        ConvertToV8(isolate, a2),
        ConvertToV8(isolate, a3),
        ConvertToV8(isolate, a4),
    };
    v8::Local<v8::Value> val(holder->Call(holder, arraysize(args), args));
    Converter<R>::FromV8(isolate, val, &ret);
    return ret;
  }
};

template<typename P1, typename P2, typename P3, typename P4>
struct V8FunctionInvoker<void(P1, P2, P3, P4)> {
  static void Go(v8::Isolate* isolate, SafeV8Function function, P1 a1, P2 a2,
      P3 a3, P4 a4) {
    Locker locker(isolate);
    MATE_HANDLE_SCOPE(isolate);
    v8::Local<v8::Function> holder = function->NewHandle();
    v8::Local<v8::Context> context = holder->CreationContext();
    v8::Context::Scope context_scope(context);
    v8::Local<v8::Value> args[] = {
        ConvertToV8(isolate, a1),
        ConvertToV8(isolate, a2),
        ConvertToV8(isolate, a3),
        ConvertToV8(isolate, a4),
    };
    holder->Call(holder, arraysize(args), args);
  }
};

template<typename V, typename P1, typename P2, typename P3, typename P4,
    typename P5>
struct V8FunctionInvoker<v8::Local<V>(P1, P2, P3, P4, P5)> {
  static v8::Local<V> Go(v8::Isolate* isolate, SafeV8Function function, P1 a1,
      P2 a2, P3 a3, P4 a4, P5 a5) {
    Locker locker(isolate);
    v8::EscapableHandleScope handle_scope(isolate);
    v8::Local<v8::Function> holder = function->NewHandle();
    v8::Local<v8::Context> context = holder->CreationContext();
    v8::Context::Scope context_scope(context);
    v8::Local<v8::Value> args[] = {
        ConvertToV8(isolate, a1),
        ConvertToV8(isolate, a2),
        ConvertToV8(isolate, a3),
        ConvertToV8(isolate, a4),
        ConvertToV8(isolate, a5),
    };
    v8::Local<v8::Value> val(holder->Call(holder, arraysize(args), args));
        return handle_scope.Escape(val);
  }
};

template<typename R, typename P1, typename P2, typename P3, typename P4,
    typename P5>
struct V8FunctionInvoker<R(P1, P2, P3, P4, P5)> {
  static R Go(v8::Isolate* isolate, SafeV8Function function, P1 a1, P2 a2,
      P3 a3, P4 a4, P5 a5) {
    R ret;
    Locker locker(isolate);
    MATE_HANDLE_SCOPE(isolate);
    v8::Local<v8::Function> holder = function->NewHandle();
    v8::Local<v8::Context> context = holder->CreationContext();
    v8::Context::Scope context_scope(context);
    v8::Local<v8::Value> args[] = {
        ConvertToV8(isolate, a1),
        ConvertToV8(isolate, a2),
        ConvertToV8(isolate, a3),
        ConvertToV8(isolate, a4),
        ConvertToV8(isolate, a5),
    };
    v8::Local<v8::Value> val(holder->Call(holder, arraysize(args), args));
    Converter<R>::FromV8(isolate, val, &ret);
    return ret;
  }
};

template<typename P1, typename P2, typename P3, typename P4, typename P5>
struct V8FunctionInvoker<void(P1, P2, P3, P4, P5)> {
  static void Go(v8::Isolate* isolate, SafeV8Function function, P1 a1, P2 a2,
      P3 a3, P4 a4, P5 a5) {
    Locker locker(isolate);
    MATE_HANDLE_SCOPE(isolate);
    v8::Local<v8::Function> holder = function->NewHandle();
    v8::Local<v8::Context> context = holder->CreationContext();
    v8::Context::Scope context_scope(context);
    v8::Local<v8::Value> args[] = {
        ConvertToV8(isolate, a1),
        ConvertToV8(isolate, a2),
        ConvertToV8(isolate, a3),
        ConvertToV8(isolate, a4),
        ConvertToV8(isolate, a5),
    };
    holder->Call(holder, arraysize(args), args);
  }
};

template<typename V, typename P1, typename P2, typename P3, typename P4,
    typename P5, typename P6>
struct V8FunctionInvoker<v8::Local<V>(P1, P2, P3, P4, P5, P6)> {
  static v8::Local<V> Go(v8::Isolate* isolate, SafeV8Function function, P1 a1,
      P2 a2, P3 a3, P4 a4, P5 a5, P6 a6) {
    Locker locker(isolate);
    v8::EscapableHandleScope handle_scope(isolate);
    v8::Local<v8::Function> holder = function->NewHandle();
    v8::Local<v8::Context> context = holder->CreationContext();
    v8::Context::Scope context_scope(context);
    v8::Local<v8::Value> args[] = {
        ConvertToV8(isolate, a1),
        ConvertToV8(isolate, a2),
        ConvertToV8(isolate, a3),
        ConvertToV8(isolate, a4),
        ConvertToV8(isolate, a5),
        ConvertToV8(isolate, a6),
    };
    v8::Local<v8::Value> val(holder->Call(holder, arraysize(args), args));
        return handle_scope.Escape(val);
  }
};

template<typename R, typename P1, typename P2, typename P3, typename P4,
    typename P5, typename P6>
struct V8FunctionInvoker<R(P1, P2, P3, P4, P5, P6)> {
  static R Go(v8::Isolate* isolate, SafeV8Function function, P1 a1, P2 a2,
      P3 a3, P4 a4, P5 a5, P6 a6) {
    R ret;
    Locker locker(isolate);
    MATE_HANDLE_SCOPE(isolate);
    v8::Local<v8::Function> holder = function->NewHandle();
    v8::Local<v8::Context> context = holder->CreationContext();
    v8::Context::Scope context_scope(context);
    v8::Local<v8::Value> args[] = {
        ConvertToV8(isolate, a1),
        ConvertToV8(isolate, a2),
        ConvertToV8(isolate, a3),
        ConvertToV8(isolate, a4),
        ConvertToV8(isolate, a5),
        ConvertToV8(isolate, a6),
    };
    v8::Local<v8::Value> val(holder->Call(holder, arraysize(args), args));
    Converter<R>::FromV8(isolate, val, &ret);
    return ret;
  }
};

template<typename P1, typename P2, typename P3, typename P4, typename P5,
    typename P6>
struct V8FunctionInvoker<void(P1, P2, P3, P4, P5, P6)> {
  static void Go(v8::Isolate* isolate, SafeV8Function function, P1 a1, P2 a2,
      P3 a3, P4 a4, P5 a5, P6 a6) {
    Locker locker(isolate);
    MATE_HANDLE_SCOPE(isolate);
    v8::Local<v8::Function> holder = function->NewHandle();
    v8::Local<v8::Context> context = holder->CreationContext();
    v8::Context::Scope context_scope(context);
    v8::Local<v8::Value> args[] = {
        ConvertToV8(isolate, a1),
        ConvertToV8(isolate, a2),
        ConvertToV8(isolate, a3),
        ConvertToV8(isolate, a4),
        ConvertToV8(isolate, a5),
        ConvertToV8(isolate, a6),
    };
    holder->Call(holder, arraysize(args), args);
  }
};

}  // namespace internal

template<typename Sig>
struct Converter<base::Callback<Sig> > {
  static v8::Local<v8::Value> ToV8(v8::Isolate* isolate,
                                    const base::Callback<Sig>& val) {
    return CreateFunctionTemplate(isolate, val)->GetFunction();
  }
  static bool FromV8(v8::Isolate* isolate,
                     v8::Local<v8::Value> val,
                     base::Callback<Sig>* out) {
    if (!val->IsFunction())
      return false;

    internal::SafeV8Function function(
        new RefCountedPersistent<v8::Function>(isolate, val));
    *out = base::Bind(&internal::V8FunctionInvoker<Sig>::Go, isolate, function);
    return true;
  }
};

}  // namespace mate
