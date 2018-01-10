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

#ifndef RIEGELI_BYTES_ZSTD_WRITER_H_
#define RIEGELI_BYTES_ZSTD_WRITER_H_

#include <stddef.h>
#include <memory>
#include <utility>

#include "riegeli/base/assert.h"
#include "riegeli/base/base.h"
#include "riegeli/base/string_view.h"
#include "riegeli/bytes/buffered_writer.h"
#include "riegeli/bytes/writer.h"
#include "zstd.h"

namespace riegeli {

// ZstdWriter::Options.
class ZstdWriterOptions {
 public:
  // Tune compression level vs. compression speed tradeoff.
  //
  // Level must be between 1 and 22. Default: 9.
  ZstdWriterOptions& set_compression_level(int level) & {
    RIEGELI_ASSERT_GE(level, 1);
    RIEGELI_ASSERT_LE(level, 22);
    compression_level_ = level;
    return *this;
  }
  ZstdWriterOptions&& set_compression_level(int level) && {
    return std::move(set_compression_level(level));
  }

  ZstdWriterOptions& set_buffer_size(size_t buffer_size) & {
    RIEGELI_ASSERT_GT(buffer_size, 0u);
    buffer_size_ = buffer_size;
    return *this;
  }
  ZstdWriterOptions&& set_buffer_size(size_t buffer_size) && {
    return std::move(set_buffer_size(buffer_size));
  }

  // Announce in advance the destination size. This may improve compression
  // density, and this causes the size to be stored in the compressed stream
  // header.
  //
  // If the size hint turns out to not match reality, nothing breaks, except
  // that ZSTD_getDecompressedSize() will report a wrong size, which may be
  // confusing (zstd.h explicitly states that this value cannot be trusted).
  ZstdWriterOptions& set_size_hint(Position size_hint) & {
    size_hint_ = size_hint;
    return *this;
  }
  ZstdWriterOptions&& set_size_hint(Position size_hint) && {
    return std::move(set_size_hint(size_hint));
  }

 private:
  friend class ZstdWriter;

  int compression_level_ = 9;
  size_t buffer_size_ = kDefaultBufferSize();
  Position size_hint_ = 0;
};

// A Writer which compresses data with Zstd before passing it to another Writer.
class ZstdWriter final : public BufferedWriter {
 public:
  using Options = ZstdWriterOptions;

  // Creates a cancelled ZstdWriter.
  ZstdWriter();

  // Will write Zstd-compressed stream to the byte Writer which is owned by this
  // ZstdWriter and will be closed and deleted when the ZstdWriter is closed.
  explicit ZstdWriter(std::unique_ptr<Writer> dest,
                      Options options = Options());

  // Will write Zstd-compressed stream to the byte Writer which is not owned by
  // this ZstdWriter and must be kept alive but not accessed until closing the
  // ZstdWriter.
  explicit ZstdWriter(Writer* dest, Options options = Options());

  ZstdWriter(ZstdWriter&& src) noexcept;
  ZstdWriter& operator=(ZstdWriter&& src) noexcept;

  ~ZstdWriter();

  bool Flush(FlushType flush_type) override;

 protected:
  void Done() override;
  bool WriteInternal(string_view src) override;

 private:
  template <typename Function>
  bool FlushInternal(Function function, const char* function_name);

  Writer* dest_;
  std::unique_ptr<Writer> owned_dest_;
  ZSTD_CStream* compressor_;
};

}  // namespace riegeli

#endif  // RIEGELI_BYTES_ZSTD_WRITER_H_