//===- lib/Core/TextStubCommon.cpp - Common TBD Mappings --------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements common TBD mappings
///
//===----------------------------------------------------------------------===//

#include "tapi/Core/TextStubCommon.h"

using namespace llvm::MachO;

namespace llvm {
namespace yaml {

using Impl = ScalarTraits<StringRef>;
void ScalarTraits<FlowStringRef>::output(const FlowStringRef &value, void *ctx,
                                         raw_ostream &os) {
  Impl::output(value, ctx, os);
}
StringRef ScalarTraits<FlowStringRef>::input(StringRef value, void *ctx,
                                             FlowStringRef &out) {
  return Impl::input(value, ctx, out.value);
}
QuotingType ScalarTraits<FlowStringRef>::mustQuote(StringRef name) {
  return Impl::mustQuote(name);
}

using tapi::ObjCConstraint;
void ScalarEnumerationTraits<ObjCConstraint>::enumeration(
    IO &io, ObjCConstraint &constraint) {
  io.enumCase(constraint, "none", ObjCConstraint::None);
  io.enumCase(constraint, "retain_release", ObjCConstraint::Retain_Release);
  io.enumCase(constraint, "retain_release_for_simulator",
              ObjCConstraint::Retain_Release_For_Simulator);
  io.enumCase(constraint, "retain_release_or_gc",
              ObjCConstraint::Retain_Release_Or_GC);
  io.enumCase(constraint, "gc", ObjCConstraint::GC);
}

void ScalarEnumerationTraits<PlatformKind>::enumeration(
    IO &io, PlatformKind &platform) {
  io.enumCase(platform, "unknown", PlatformKind::unknown);
  io.enumCase(platform, "macosx", PlatformKind::macOS);
  io.enumCase(platform, "ios", PlatformKind::iOS);
  io.enumCase(platform, "ios", PlatformKind::iOSSimulator);
  io.enumCase(platform, "watchos", PlatformKind::watchOS);
  io.enumCase(platform, "watchos", PlatformKind::watchOSSimulator);
  io.enumCase(platform, "tvos", PlatformKind::tvOS);
  io.enumCase(platform, "tvos", PlatformKind::tvOSSimulator);
  io.enumCase(platform, "bridgeos", PlatformKind::bridgeOS);
}

void ScalarBitSetTraits<ArchitectureSet>::bitset(IO &io,
                                                 ArchitectureSet &archs) {
#define ARCHINFO(arch, type, subtype, numbits)                                 \
  io.bitSetCase(archs, #arch, 1U << static_cast<int>(AK_##arch));
#include "llvm/TextAPI/Architecture.def"
#undef ARCHINFO
}

void ScalarTraits<Architecture>::output(const Architecture &value, void *,
                                        raw_ostream &os) {
  os << value;
}
StringRef ScalarTraits<Architecture>::input(StringRef scalar, void *,
                                            Architecture &value) {
  value = getArchitectureFromName(scalar);
  return {};
}
QuotingType ScalarTraits<Architecture>::mustQuote(StringRef) {
  return QuotingType::None;
}

using TAPI_INTERNAL::PackedVersion;
void ScalarTraits<PackedVersion>::output(const PackedVersion &value, void *,
                                         raw_ostream &os) {
  os << value;
}
StringRef ScalarTraits<PackedVersion>::input(StringRef scalar, void *,
                                             PackedVersion &value) {
  if (!value.parse32(scalar))
    return "invalid packed version string.";
  return {};
}
QuotingType ScalarTraits<PackedVersion>::mustQuote(StringRef) {
  return QuotingType::None;
}

void ScalarTraits<SwiftVersion>::output(const SwiftVersion &value, void *,
                                        raw_ostream &os) {
  switch (value) {
  case 1:
    os << "1.0";
    break;
  case 2:
    os << "1.1";
    break;
  case 3:
    os << "2.0";
    break;
  case 4:
    os << "3.0";
    break;
  default:
    os << (unsigned)value;
    break;
  }
}
StringRef ScalarTraits<SwiftVersion>::input(StringRef scalar, void *,
                                            SwiftVersion &value) {
  value = StringSwitch<SwiftVersion>(scalar)
              .Case("1.0", 1)
              .Case("1.1", 2)
              .Case("2.0", 3)
              .Case("3.0", 4)
              .Default(0);
  if (value != SwiftVersion(0))
    return {};

  if (scalar.getAsInteger(10, value))
    return "invalid Swift ABI version.";

  return StringRef();
}
QuotingType ScalarTraits<SwiftVersion>::mustQuote(StringRef) {
  return QuotingType::None;
}

using TAPI_INTERNAL::AvailabilityInfo;
void ScalarTraits<AvailabilityInfo>::output(const AvailabilityInfo &value,
                                            void *, raw_ostream &os) {
  if (value._unavailable) {
    os << "n/a";
    return;
  }

  os << value._introduced;
  if (!value._obsoleted.empty())
    os << ".." << value._obsoleted;
}
StringRef ScalarTraits<AvailabilityInfo>::input(StringRef scalar, void *,
                                                AvailabilityInfo &value) {
  if (scalar == "n/a") {
    value._unavailable = true;
    return {};
  }

  auto split = scalar.split("..");
  auto introduced = split.first.trim();
  auto obsoleted = split.second.trim();

  if (!value._introduced.parse32(introduced))
    return "invalid packed version string.";

  if (obsoleted.empty())
    return StringRef();

  if (!value._obsoleted.parse32(obsoleted))
    return "invalid packed version string.";

  return StringRef();
}
QuotingType ScalarTraits<AvailabilityInfo>::mustQuote(StringRef) {
  return QuotingType::None;
}

void ScalarTraits<UUID>::output(const UUID &value, void *c, raw_ostream &os) {
  auto *ctx = reinterpret_cast<YAMLContext *>(c);
  assert(ctx);

  if (ctx->fileType < TBDv4)
    os << value.first.Arch << ": " << value.second;
  else
    os << value.first << ": " << value.second;
}
StringRef ScalarTraits<UUID>::input(StringRef scalar, void *c, UUID &value) {
  auto split = scalar.split(':');
  auto arch = split.first.trim();
  auto uuid = split.second.trim();
  if (uuid.empty())
    return "invalid uuid string pair";

  value.first = Target{getArchitectureFromName(arch), PlatformKind::unknown};
  value.second = uuid.str();
  return {};
}
QuotingType ScalarTraits<UUID>::mustQuote(StringRef) {
  return QuotingType::Single;
}

using clang::Language;
void ScalarEnumerationTraits<Language>::enumeration(IO &io, Language &kind) {
  io.enumCase(kind, "c", Language::C);
  io.enumCase(kind, "cxx", Language::CXX);
  io.enumCase(kind, "objective-c", Language::ObjC);
  io.enumCase(kind, "objc", Language::ObjC); // to keep old snapshots working.
  io.enumCase(kind, "objective-cxx", Language::ObjCXX);
  io.enumCase(kind, "objcxx",
              Language::ObjCXX); // to keep old snapshots working.
  io.enumCase(kind, "unknown", Language::Unknown);
}

} // end namespace yaml.
} // end namespace llvm.
