workspace(name = "com_google_gematria")

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository", "new_git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

SKYLIB_VERSION = "1.3.0"

http_archive(
    name = "bazel_skylib",
    sha256 = "74d544d96f4a5bb630d465ca8bbcfe231e3594e5aae57e1edbf17a6eb3ca2506",
    urls = [
        "https://github.com/bazelbuild/bazel-skylib/releases/download/{version}/bazel-skylib-{version}.tar.gz".format(version = SKYLIB_VERSION),
    ],
)

git_repository(
    name = "com_google_protobuf",
    remote = "https://github.com/protocolbuffers/protobuf.git",
    tag = "v23.2",
)

load("@com_google_protobuf//:protobuf_deps.bzl", "protobuf_deps")

protobuf_deps()

# Required for protobuf to build.
git_repository(
    name = "com_google_absl",
    remote = "https://github.com/abseil/abseil-cpp.git",
    tag = "20230125.3",
)

git_repository(
    name = "org_mizux_bazelpybind11",
    commit = "27da411499fe62f7c0969ac2665d343ce162b6a9",
    remote = "https://github.com/Mizux/bazel-pybind11.git",
)

git_repository(
    name = "com_google_googletest",
    remote = "https://github.com/google/googletest.git",
    tag = "release-1.12.1",
)

git_repository(
    name = "rules_proto",
    remote = "https://github.com/bazelbuild/rules_proto.git",
    tag = "5.3.0-21.7",
)

# Python
load("@upb//bazel:system_python.bzl", "system_python")
system_python(
    name = "system_python",
    minimum_python_version = "3.10",
)

bind(
    name = "python_headers",
    actual = "@system_python//:python_headers",
)

git_repository(
    name = "rules_python",
    remote = "https://github.com/bazelbuild/rules_python.git",
    tag = "0.19.0",
)

git_repository(
    name = "pybind11_bazel",
    commit = "b162c7c88a253e3f6b673df0c621aca27596ce6b",
    patch_args = ["-p1"],
    patches = ["@org_mizux_bazelpybind11//patches:pybind11_bazel.patch"],
    remote = "https://github.com/pybind/pybind11_bazel.git",
)

new_git_repository(
    name = "pybind11",
    build_file = "@pybind11_bazel//:pybind11.BUILD",
    remote = "https://github.com/pybind/pybind11.git",
    tag = "v2.10.3",
)

git_repository(
    # We can't use the name "pybind11_abseil" for the repository, as this would
    # break the `sys.path` hack that we use for importing third-party Python
    # modules. See the comment under "Python libraries" below for a detailed
    # explanation.
    name = "pybind11_abseil_repo",
    commit = "1caf1890443e8e303bf88850d3c27d5422903168",
    remote = "https://github.com/pybind/pybind11_abseil.git",
)

# TODO(ondrasej): Revert back to the official repository, once
# https://github.com/pybind/pybind11_protobuf/pull/117 is merged.
git_repository(
    name = "com_google_pybind11_protobuf",
    commit = "3e4d063c56156d752ad5cf63cf8338ef960c3269",
    remote = "https://github.com/ondrasej/pybind11_protobuf.git",
)

load("@pybind11_bazel//:python_configure.bzl", _pybind11_python_configure = "python_configure")

_pybind11_python_configure(
    name = "local_config_python",
    python_version = "3",
)

# Python libraries

# We need to manipulate sys.path to make these libraries work as if they were imported
# through PIP or the system package manager. When adding a new Python repository here,
# 1. the name of the repository should be different from the name used when importing it
#    in Python, to avoid confusing the module loader; by convention, we add `_repo` at
#    the end of the name.
# 2. the names of the repositories must be added to the list of third-party repositories
#    in `gematria/__init__.py` to make sure that they are added to sys.path.

git_repository(
    name = "sonnet_repo",
    commit = "cd5b5fa48e15e4d020f744968f5209949ebe750f",
    patch_args = ["-p1"],
    patches = ["//:sonnet.patch"],
    remote = "https://github.com/deepmind/sonnet.git",
)

new_git_repository(
    name = "graph_nets_repo",
    build_file = "@//:graph_nets.BUILD",
    commit = "adf25162ba21bb0ae176c35483a74fb0c9dff576",
    remote = "https://github.com/deepmind/graph_nets.git",
)

# LLVM and its dependencies

LLVM_COMMIT = "868591501c61bc1aa56da537bab85cc67439f16d"

LLVM_SHA256 = "7790344f8245cb6a60ec8cbdadd46640ef8cc217c8cadc6b4481da473292f77b"

http_archive(
    name = "llvm-raw",
    build_file_content = "# empty",
    sha256 = LLVM_SHA256,
    strip_prefix = "llvm-project-" + LLVM_COMMIT,
    urls = ["https://github.com/llvm/llvm-project/archive/{commit}.zip".format(commit = LLVM_COMMIT)],
)

load("@llvm-raw//utils/bazel:configure.bzl", "llvm_configure", "llvm_disable_optional_support_deps")

llvm_configure(name = "llvm-project")

llvm_disable_optional_support_deps()

http_archive(
    name = "llvm_zlib",
    build_file = "@llvm-raw//utils/bazel/third_party_build:zlib.BUILD",
    sha256 = "91844808532e5ce316b3c010929493c0244f3d37593afd6de04f71821d5136d9",
    strip_prefix = "zlib-1.2.12",
    urls = [
        "https://storage.googleapis.com/mirror.tensorflow.org/zlib.net/zlib-1.2.12.tar.gz",
        "https://zlib.net/zlib-1.2.12.tar.gz",
    ],
)

http_archive(
    name = "llvm_zstd",
    build_file = "@llvm-raw//utils/bazel/third_party_build:zstd.BUILD",
    sha256 = "7c42d56fac126929a6a85dbc73ff1db2411d04f104fae9bdea51305663a83fd0",
    strip_prefix = "zstd-1.5.2",
    urls = [
        "https://github.com/facebook/zstd/releases/download/v1.5.2/zstd-1.5.2.tar.gz",
    ],
)
