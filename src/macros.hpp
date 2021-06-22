// Copyright 2021 The RDSS Authors
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

#ifndef RDSS_MACROS_H_
#define RDSS_MACROS_H_

#define CONCAT(a, b) CONCAT_INNER(a, b)
#define CONCAT_INNER(a, b) a ## b

#define ASSIGN_OR_RETURN_HELPER(lhs, rhs, temp) \
    auto temp = (rhs);                          \
    if(!temp.ok()) { return temp.status(); }    \
    lhs = std::move(*temp);

#define ASSIGN_OR_RETURN(lhs, rhs)                                      \
    ASSIGN_OR_RETURN_HELPER(lhs, rhs, CONCAT(temporary, __COUNTER__))

#define RETURN_IF_ERROR_HELPER(value, temp)     \
    auto temp = (value);                        \
    if(!temp.ok()) { return temp; }

#define RETURN_IF_ERROR(value)                                      \
    RETURN_IF_ERROR_HELPER(value, CONCAT(temporary, __COUNTER__))

#endif  // RDSS_MACROS_H_
