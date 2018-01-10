// Copyright 2017 Google LLC
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

#include "riegeli/bytes/zstd_reader.h"

#include <stddef.h>
#include <memory>
#include <string>
#include <utility>

#include "riegeli/base/assert.h"
#include "riegeli/base/base.h"
#include "riegeli/bytes/buffered_reader.h"
#include "riegeli/bytes/reader.h"
#include "zstd.h"

namespace riegeli {

ZstdReader::ZstdReader() : src_(nullptr), decompressor_(nullptr) {
  MarkCancelled();
}

ZstdReader::ZstdReader(std::unique_ptr<Reader> src, Options options)
    : ZstdReader(src.get(), options) {
  owned_src_ = std::move(src);
}

ZstdReader::ZstdReader(Reader* src, Options options)
    : BufferedReader(options.buffer_size_),
      src_(RIEGELI_ASSERT_NOTNULL(src)),
      decompressor_(ZSTD_createDStream()) {
  if (RIEGELI_UNLIKELY(decompressor_ == nullptr)) {
    Fail("ZSTD_createDStream() failed");
    return;
  }
  const size_t result = ZSTD_initDStream(decompressor_);
  if (RIEGELI_UNLIKELY(ZSTD_isError(result))) {
    Fail(std::string("ZSTD_initDStream() failed: ") + ZSTD_getErrorName(result));
  }
}

ZstdReader::ZstdReader(ZstdReader&& src) noexcept
    : BufferedReader(std::move(src)),
      src_(riegeli::exchange(src.src_, nullptr)),
      owned_src_(std::move(src.owned_src_)),
      decompressor_(riegeli::exchange(src.decompressor_, nullptr)) {}

ZstdReader& ZstdReader::operator=(ZstdReader&& src) noexcept {
  if (&src != this) {
    BufferedReader::operator=(std::move(src));
    src_ = riegeli::exchange(src.src_, nullptr);
    owned_src_ = std::move(src.owned_src_);
    decompressor_ = riegeli::exchange(src.decompressor_, nullptr);
  }
  return *this;
}

ZstdReader::~ZstdReader() { Cancel(); }

void ZstdReader::Done() {
  if (RIEGELI_UNLIKELY(!Pull() && healthy() && decompressor_ != nullptr)) {
    Fail("Truncated Zstd-compressed stream");
  } else if (RIEGELI_LIKELY(healthy()) && owned_src_ != nullptr) {
    if (RIEGELI_UNLIKELY(!owned_src_->Close())) Fail(owned_src_->Message());
  }
  src_ = nullptr;
  owned_src_.reset();
  ZSTD_freeDStream(decompressor_);
  decompressor_ = nullptr;
  BufferedReader::Done();
}

bool ZstdReader::PullSlow() {
  RIEGELI_ASSERT_EQ(available(), 0u);
  // After all data have been decompressed, skip BufferedReader::PullSlow()
  // to avoid allocating the buffer in case it was not allocated yet.
  if (RIEGELI_UNLIKELY(decompressor_ == nullptr)) return false;
  return BufferedReader::PullSlow();
}

bool ZstdReader::ReadInternal(char* dest, size_t min_length,
                              size_t max_length) {
  RIEGELI_ASSERT_GT(min_length, 0u);
  RIEGELI_ASSERT_GE(max_length, min_length);
  RIEGELI_ASSERT(healthy());
  if (RIEGELI_UNLIKELY(decompressor_ == nullptr)) return false;
  ZSTD_outBuffer output = {dest, max_length, 0};
  for (;;) {
    ZSTD_inBuffer input = {src_->cursor(), src_->available(), 0};
    const size_t result = ZSTD_decompressStream(decompressor_, &output, &input);
    src_->set_cursor(static_cast<const char*>(input.src) + input.pos);
    if (RIEGELI_UNLIKELY(result == 0)) {
      ZSTD_freeDStream(decompressor_);
      decompressor_ = nullptr;
      limit_pos_ += output.pos;
      return output.pos >= min_length;
    }
    if (RIEGELI_UNLIKELY(ZSTD_isError(result))) {
      Fail(std::string("ZSTD_decompressStream() failed: ") +
           ZSTD_getErrorName(result));
      limit_pos_ += output.pos;
      return output.pos >= min_length;
    }
    if (output.pos >= min_length) {
      limit_pos_ += output.pos;
      return true;
    }
    RIEGELI_ASSERT_EQ(input.pos, input.size);
    if (RIEGELI_UNLIKELY(!src_->Pull())) {
      limit_pos_ += output.pos;
      if (RIEGELI_LIKELY(src_->HopeForMore())) return false;
      if (src_->healthy()) return Fail("Truncated Zstd-compressed stream");
      return Fail(src_->Message());
    }
  }
}

bool ZstdReader::HopeForMoreSlow() const {
  RIEGELI_ASSERT_EQ(available(), 0u);
  if (RIEGELI_UNLIKELY(!healthy())) return false;
  return decompressor_ != nullptr;
}

}  // namespace riegeli