package(default_visibility = ["//visibility:public"])

licenses(["notice"])

cc_library(
    name = "file_reader",
    srcs = ["file_reader.cc"],
    hdrs = ["file_reader.h"],
    deps = [
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:span",
        "//riegeli/base",
        "//riegeli/base:chain",
        "//riegeli/base:status",
        "//riegeli/bytes:backward_writer",
        "//riegeli/bytes:reader",
        "//riegeli/bytes:writer",
        # Do not depend on @org_tensorflow//tensorflow/core:lib because
        # //python/riegeli/tensorflow:ops/_riegeli_dataset_ops.so
        # disallows such dependencies.
        "@org_tensorflow//tensorflow/core:framework_headers_lib",
        # Avoid including from /usr/include/google/protobuf.
        "@protobuf_archive//:protobuf_headers",
    ],
)

cc_library(
    name = "file_writer",
    srcs = ["file_writer.cc"],
    hdrs = ["file_writer.h"],
    deps = [
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/strings",
        "//riegeli/base",
        "//riegeli/base:buffer",
        "//riegeli/base:chain",
        "//riegeli/base:status",
        "//riegeli/bytes:writer",
        # Do not depend on @org_tensorflow//tensorflow/core:lib because
        # //python/riegeli/tensorflow:ops/_riegeli_dataset_ops.so
        # disallows such dependencies.
        "@org_tensorflow//tensorflow/core:framework_headers_lib",
        # Avoid including from /usr/include/google/protobuf.
        "@protobuf_archive//:protobuf_headers",
    ],
)
