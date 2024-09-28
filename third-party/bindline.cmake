# Copyright (C) 2024 Fred Emmott <fred@fredemmott.com>
# SPDX-License-Identifier: MIT

include(FetchContent)

FetchContent_Declare(
    bindline
    URL "https://github.com/fredemmott/bindline/archive/refs/tags/v0.1.zip"
    URL_HASH "SHA256=765a1a5251d7901a99cc6663fe08cb751c09deca6740ed99b2d964d0af8ccbc6"
    EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(bindline)

add_library(bindline_unified INTERFACE)
target_link_libraries(
    bindline_unified
    INTERFACE
    FredEmmott::bindline
    FredEmmott::cppwinrt
    FredEmmott::weak_refs
    ThirdParty::CppWinRT
    ThirdParty::WIL
)

add_library(ThirdParty::bindline ALIAS bindline_unified)

install(
    FILES
    "${bindline_SOURCE_DIR}/LICENSE"
    TYPE DOC
    RENAME "LICENSE-ThirdParty-bindline.txt"
)