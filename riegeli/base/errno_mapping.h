// Copyright 2018 Google LLC
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

#ifndef RIEGELI_BASE_ERRNO_MAPPING_H_
#define RIEGELI_BASE_ERRNO_MAPPING_H_

#include "absl/strings/string_view.h"
#include "riegeli/base/status.h"

namespace riegeli {

// Converts errno value to Status.
Status ErrnoToCanonicalStatus(int error_number, absl::string_view message);

}  // namespace riegeli

#endif  // RIEGELI_BASE_ERRNO_MAPPING_H_
