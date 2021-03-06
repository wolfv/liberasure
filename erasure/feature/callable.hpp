/*
 * Copyright 2015, 2016 Gašper Ažman
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once
#include "erasure/erasure.hpp"

namespace erasure {
namespace features {
namespace callable_detail {

struct provides_call_operator {
  template <template <typename> class T, typename U>
  struct importer : T<U> {
    using T<U>::operator();
    using U::operator();
  };
};

} // namespace callable_detail

// provide the correct form for pattern matching
template <typename Signature>
struct callable;

#define ERASURE_CALLABLE_DEF(constness)                                        \
  template <typename Return, typename... Args>                                 \
  struct callable<Return(Args...) constness> : feature_support::feature {      \
    using provides = callable_detail::provides_call_operator;                  \
                                                                               \
    template <typename C>                                                      \
    struct vtbl : C {                                                          \
      using C::erase;                                                          \
      virtual auto erase(erasure::tag_t<callable>, Args... args) constness     \
          -> Return = 0;                                                       \
    };                                                                         \
                                                                               \
    template <typename M>                                                      \
    struct model : M {                                                         \
      using M::erase;                                                          \
      auto erase(erasure::tag_t<callable>, Args... args) constness             \
          -> Return final {                                                    \
        return erasure::value(std::forward<M constness>(*this))(args...);      \
      }                                                                        \
    };                                                                         \
                                                                               \
    template <typename I>                                                      \
    struct interface : I {                                                     \
      auto operator()(Args... args) constness -> Return {                      \
        namespace f = feature_support;                                         \
        return erasure::call<callable>(*this, (Args)args...);                  \
      }                                                                        \
    };                                                                         \
  }

ERASURE_CALLABLE_DEF();
ERASURE_CALLABLE_DEF(const);
ERASURE_CALLABLE_DEF(&);
ERASURE_CALLABLE_DEF(&&);
ERASURE_CALLABLE_DEF(const &);
ERASURE_CALLABLE_DEF(const &&);

#undef ERASURE_CALLABLE_DEF

/**
 * The default size for the buffer for function is vtable_ptr + function
 * pointer + this state.
 */
template <typename Signature, std::size_t BufferSize = 3 * sizeof(void *)>
using function =
    feature_support::typelist<buffer_size<BufferSize>, callable<Signature>,
                              move_constructible, copy_constructible>;

} // namespace features
} // namespace erasure
