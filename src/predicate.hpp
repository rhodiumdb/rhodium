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

#ifndef RDSS_PREDICATE_H_
#define RDSS_PREDICATE_H_

#include <cstdint>
#include <string>
#include <vector>

#include <absl/memory/memory.h>
#include <absl/strings/str_cat.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_join.h>

#include "attr.hpp"

namespace rdss {

////////////////////////////////////////////////////////////////////////////////

struct Predicate {
    virtual std::string ToString() const = 0;
    virtual ~Predicate() = default;
};

struct PredicateAnd : public Predicate {
    std::vector<Predicate*> children;

    explicit PredicateAnd(absl::Span<Predicate* const> children_)
        : children(children_.begin(), children_.end()) {}

    std::string ToString() const override {
        std::vector<std::string> child_strings;
        for (Predicate* child : children) {
            child_strings.push_back(child->ToString());
        }
        return absl::StrCat("(", absl::StrJoin(child_strings, " && "), ")");
    }
};

struct PredicateOr : public Predicate {
    std::vector<Predicate*> children;

    explicit PredicateOr(absl::Span<Predicate* const> children_)
        : children(children_.begin(), children_.end()) {}

    std::string ToString() const override {
        std::vector<std::string> child_strings;
        for (Predicate* child : children) {
            child_strings.push_back(child->ToString());
        }
        return absl::StrCat("(", absl::StrJoin(child_strings, " || "), ")");
    }
};

struct PredicateNot : public Predicate {
    Predicate* pred;

    explicit PredicateNot(Predicate* pred_)
        : pred(pred_) {}

    std::string ToString() const override {
        return absl::StrCat("!", pred->ToString());
    }
};

struct PredicateLike : public Predicate {
    Attr attr;
    std::string string;

    PredicateLike(Attr attr_, const std::string& string_)
        : attr(attr_), string(string_) {}

    std::string ToString() const override {
        return absl::StrFormat("(attr%d LIKE \"%s\")", attr, string);
    }
};

struct PredicateLessThan : public Predicate {
    Attr attr;
    int32_t integer;

    PredicateLessThan(Attr attr_, int32_t integer_)
        : attr(attr_), integer(integer_) {}

    std::string ToString() const override {
        return absl::StrFormat("(attr%d < %d)", attr, integer);
    }
};

struct PredicateEquals : public Predicate {
    Attr attr;
    int32_t integer;

    PredicateEquals(Attr attr_, int32_t integer_)
        : attr(attr_), integer(integer_) {}

    std::string ToString() const override {
        return absl::StrFormat("(attr%d ≡ %d)", attr, integer);
    }
};

////////////////////////////////////////////////////////////////////////////////

struct PredicateFactory {
    std::vector<std::unique_ptr<Predicate>> predicates;

    template<typename T, typename... Args>
    T* Make(Args&&... args) {
        std::unique_ptr<T> value =
            absl::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = value.get();
        predicates.push_back(std::move(value));
        return ptr;
    }
};

////////////////////////////////////////////////////////////////////////////////

}  // namespace rdss

#endif  // RDSS_PREDICATE_H_
