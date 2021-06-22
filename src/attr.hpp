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

#ifndef RDSS_ATTR_H_
#define RDSS_ATTR_H_

#include <cstdint>

namespace rdss {

////////////////////////////////////////////////////////////////////////////////

using Attr = int32_t;
using AttrPermutation = std::vector<Attr>;
using AttrPartialPermutation = std::vector<absl::optional<Attr>>;

////////////////////////////////////////////////////////////////////////////////

}  // namespace rdss

#endif  // RDSS_ATTR_H_
