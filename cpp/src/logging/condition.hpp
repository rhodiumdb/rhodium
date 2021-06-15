// Copyright 2021 The RDSS Authors
// Copyright 2020 The XLS Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef RDSS_LOGGING_CONDITION_H_
#define RDSS_LOGGING_CONDITION_H_

#include "log_message.hpp"
#include "null_stream.hpp"

// `RDSS_LOGGING_INTERNAL_CONDITION` prefixes another macro that expands to a
// temporary `LogMessage` instantiation followed by zero or more streamed
// expressions. This definition is tricky to read correctly. It evaluates to
// either
//
//   (void)0;
//
// or
//
//   ::rdss::logging_internal::LogMessageVoidify() &&
//       ::rdss::logging_internal::LogMessage(...) << "the user's message";
//
// If the condition is evaluable at compile time, as is often the case, it
// compiles away to just one side or the other.
// `RDSS_LOGGING_INTERNAL_CONDITION` can be used consecutively e.g. if a macro's
// expansion produces more than one condition at different levels of expansion.
// In other words:
//
//   RDSS_LOGGING_INTERNAL_CONDITION(x) RDSS_LOGGING_INTERNAL_CONDITION(y)
//
// is equivalent to
//
//   RDSS_LOGGING_INTERNAL_CONDITION(x && y).
#define RDSS_LOGGING_INTERNAL_CONDITION(condition) \
  !(condition) ? (void)0 : ::rdss::logging_internal::LogMessageVoidify()&&

namespace rdss {
namespace logging_internal {

// This class is used to explicitly ignore values in the conditional logging
// macros. This avoids compiler warnings like "value computed is not used" and
// "statement has no effect".
class LogMessageVoidify {
 public:
  // This has to be an operator with a precedence lower than << but higher than
  // ?:
  void operator&&(const rdss::logging_internal::LogMessage&) {}
  void operator&&(const std::ostream&) {}
  void operator&&(const rdss::logging_internal::NullStream&) {}

  // This overload allows `RDSS_LOGGING_INTERNAL_CONDITION` to be used
  // consecutively.
  //
  //   RDSS_LOGGING_INTERNAL_CONDITION(x)
  //   RDSS_LOGGING_INTERNAL_CONDITION(y)
  //   LOG(INFO) << "Hi!";
  //
  // becomes this:
  //
  //   !x ? void(0) : LogMessageVoidify() &
  //   !y ? void(0) : LogMessageVoidify() &
  //   LogMessage(__FILE__, __LINE__) << "Hi!";
  //
  // `LogMessageVoidify() & !y` evaluates to just `!y`, so the final part (with
  // the `LogMessage`) is evaluated only if `x && y`.
  bool operator&&(bool b) { return b; }
};

}  // namespace logging_internal
}  // namespace rdss

#endif  // RDSS_LOGGING_CONDITION_H_
