// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: google/protobuf/util/internal/testdata/default_value_test.proto

#ifndef GOOGLE_PROTOBUF_INCLUDED_google_2fprotobuf_2futil_2finternal_2ftestdata_2fdefault_5fvalue_5ftest_2eproto
#define GOOGLE_PROTOBUF_INCLUDED_google_2fprotobuf_2futil_2finternal_2ftestdata_2fdefault_5fvalue_5ftest_2eproto

#include <limits>
#include <string>

#include <google/protobuf/port_def.inc>
#if PROTOBUF_VERSION < 3011000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers. Please update
#error your headers.
#endif
#if 3011004 < PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers. Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/port_undef.inc>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_table_driven.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/inlined_string_field.h>
#include <google/protobuf/metadata.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/generated_enum_reflection.h>
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>
#define PROTOBUF_INTERNAL_EXPORT_google_2fprotobuf_2futil_2finternal_2ftestdata_2fdefault_5fvalue_5ftest_2eproto
PROTOBUF_NAMESPACE_OPEN
namespace internal {
class AnyMetadata;
}  // namespace internal
PROTOBUF_NAMESPACE_CLOSE

// Internal implementation detail -- do not use these members.
struct TableStruct_google_2fprotobuf_2futil_2finternal_2ftestdata_2fdefault_5fvalue_5ftest_2eproto {
  static const ::PROTOBUF_NAMESPACE_ID::internal::ParseTableField entries[]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::AuxillaryParseTableField aux[]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::ParseTable schema[1]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::FieldMetadata field_metadata[];
  static const ::PROTOBUF_NAMESPACE_ID::internal::SerializationTable serialization_table[];
  static const ::PROTOBUF_NAMESPACE_ID::uint32 offsets[];
};
extern const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable descriptor_table_google_2fprotobuf_2futil_2finternal_2ftestdata_2fdefault_5fvalue_5ftest_2eproto;
namespace proto_util_converter {
namespace testing {
class DefaultValueTest;
class DefaultValueTestDefaultTypeInternal;
extern DefaultValueTestDefaultTypeInternal _DefaultValueTest_default_instance_;
}  // namespace testing
}  // namespace proto_util_converter
PROTOBUF_NAMESPACE_OPEN
template<> ::proto_util_converter::testing::DefaultValueTest* Arena::CreateMaybeMessage<::proto_util_converter::testing::DefaultValueTest>(Arena*);
PROTOBUF_NAMESPACE_CLOSE
namespace proto_util_converter {
namespace testing {

enum DefaultValueTest_EnumDefault : int {
  DefaultValueTest_EnumDefault_ENUM_FIRST = 0,
  DefaultValueTest_EnumDefault_ENUM_SECOND = 1,
  DefaultValueTest_EnumDefault_ENUM_THIRD = 2,
  DefaultValueTest_EnumDefault_DefaultValueTest_EnumDefault_INT_MIN_SENTINEL_DO_NOT_USE_ = std::numeric_limits<::PROTOBUF_NAMESPACE_ID::int32>::min(),
  DefaultValueTest_EnumDefault_DefaultValueTest_EnumDefault_INT_MAX_SENTINEL_DO_NOT_USE_ = std::numeric_limits<::PROTOBUF_NAMESPACE_ID::int32>::max()
};
bool DefaultValueTest_EnumDefault_IsValid(int value);
constexpr DefaultValueTest_EnumDefault DefaultValueTest_EnumDefault_EnumDefault_MIN = DefaultValueTest_EnumDefault_ENUM_FIRST;
constexpr DefaultValueTest_EnumDefault DefaultValueTest_EnumDefault_EnumDefault_MAX = DefaultValueTest_EnumDefault_ENUM_THIRD;
constexpr int DefaultValueTest_EnumDefault_EnumDefault_ARRAYSIZE = DefaultValueTest_EnumDefault_EnumDefault_MAX + 1;

const ::PROTOBUF_NAMESPACE_ID::EnumDescriptor* DefaultValueTest_EnumDefault_descriptor();
template<typename T>
inline const std::string& DefaultValueTest_EnumDefault_Name(T enum_t_value) {
  static_assert(::std::is_same<T, DefaultValueTest_EnumDefault>::value ||
    ::std::is_integral<T>::value,
    "Incorrect type passed to function DefaultValueTest_EnumDefault_Name.");
  return ::PROTOBUF_NAMESPACE_ID::internal::NameOfEnum(
    DefaultValueTest_EnumDefault_descriptor(), enum_t_value);
}
inline bool DefaultValueTest_EnumDefault_Parse(
    const std::string& name, DefaultValueTest_EnumDefault* value) {
  return ::PROTOBUF_NAMESPACE_ID::internal::ParseNamedEnum<DefaultValueTest_EnumDefault>(
    DefaultValueTest_EnumDefault_descriptor(), name, value);
}
// ===================================================================

class DefaultValueTest :
    public ::PROTOBUF_NAMESPACE_ID::Message /* @@protoc_insertion_point(class_definition:proto_util_converter.testing.DefaultValueTest) */ {
 public:
  DefaultValueTest();
  virtual ~DefaultValueTest();

  DefaultValueTest(const DefaultValueTest& from);
  DefaultValueTest(DefaultValueTest&& from) noexcept
    : DefaultValueTest() {
    *this = ::std::move(from);
  }

  inline DefaultValueTest& operator=(const DefaultValueTest& from) {
    CopyFrom(from);
    return *this;
  }
  inline DefaultValueTest& operator=(DefaultValueTest&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }

  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* descriptor() {
    return GetDescriptor();
  }
  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* GetDescriptor() {
    return GetMetadataStatic().descriptor;
  }
  static const ::PROTOBUF_NAMESPACE_ID::Reflection* GetReflection() {
    return GetMetadataStatic().reflection;
  }
  static const DefaultValueTest& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const DefaultValueTest* internal_default_instance() {
    return reinterpret_cast<const DefaultValueTest*>(
               &_DefaultValueTest_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    0;

  friend void swap(DefaultValueTest& a, DefaultValueTest& b) {
    a.Swap(&b);
  }
  inline void Swap(DefaultValueTest* other) {
    if (other == this) return;
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  inline DefaultValueTest* New() const final {
    return CreateMaybeMessage<DefaultValueTest>(nullptr);
  }

  DefaultValueTest* New(::PROTOBUF_NAMESPACE_ID::Arena* arena) const final {
    return CreateMaybeMessage<DefaultValueTest>(arena);
  }
  void CopyFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
  void MergeFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
  void CopyFrom(const DefaultValueTest& from);
  void MergeFrom(const DefaultValueTest& from);
  PROTOBUF_ATTRIBUTE_REINITIALIZES void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  const char* _InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) final;
  ::PROTOBUF_NAMESPACE_ID::uint8* _InternalSerialize(
      ::PROTOBUF_NAMESPACE_ID::uint8* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const final;
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  inline void SharedCtor();
  inline void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(DefaultValueTest* other);
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "proto_util_converter.testing.DefaultValueTest";
  }
  private:
  inline ::PROTOBUF_NAMESPACE_ID::Arena* GetArenaNoVirtual() const {
    return nullptr;
  }
  inline void* MaybeArenaPtr() const {
    return nullptr;
  }
  public:

  ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadata() const final;
  private:
  static ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadataStatic() {
    ::PROTOBUF_NAMESPACE_ID::internal::AssignDescriptors(&::descriptor_table_google_2fprotobuf_2futil_2finternal_2ftestdata_2fdefault_5fvalue_5ftest_2eproto);
    return ::descriptor_table_google_2fprotobuf_2futil_2finternal_2ftestdata_2fdefault_5fvalue_5ftest_2eproto.file_level_metadata[kIndexInFileMessages];
  }

  public:

  // nested types ----------------------------------------------------

  typedef DefaultValueTest_EnumDefault EnumDefault;
  static constexpr EnumDefault ENUM_FIRST =
    DefaultValueTest_EnumDefault_ENUM_FIRST;
  static constexpr EnumDefault ENUM_SECOND =
    DefaultValueTest_EnumDefault_ENUM_SECOND;
  static constexpr EnumDefault ENUM_THIRD =
    DefaultValueTest_EnumDefault_ENUM_THIRD;
  static inline bool EnumDefault_IsValid(int value) {
    return DefaultValueTest_EnumDefault_IsValid(value);
  }
  static constexpr EnumDefault EnumDefault_MIN =
    DefaultValueTest_EnumDefault_EnumDefault_MIN;
  static constexpr EnumDefault EnumDefault_MAX =
    DefaultValueTest_EnumDefault_EnumDefault_MAX;
  static constexpr int EnumDefault_ARRAYSIZE =
    DefaultValueTest_EnumDefault_EnumDefault_ARRAYSIZE;
  static inline const ::PROTOBUF_NAMESPACE_ID::EnumDescriptor*
  EnumDefault_descriptor() {
    return DefaultValueTest_EnumDefault_descriptor();
  }
  template<typename T>
  static inline const std::string& EnumDefault_Name(T enum_t_value) {
    static_assert(::std::is_same<T, EnumDefault>::value ||
      ::std::is_integral<T>::value,
      "Incorrect type passed to function EnumDefault_Name.");
    return DefaultValueTest_EnumDefault_Name(enum_t_value);
  }
  static inline bool EnumDefault_Parse(const std::string& name,
      EnumDefault* value) {
    return DefaultValueTest_EnumDefault_Parse(name, value);
  }

  // accessors -------------------------------------------------------

  enum : int {
    kRepeatedDoubleFieldNumber = 2,
    kStringValueFieldNumber = 15,
    kBytesValueFieldNumber = 17,
    kDoubleValueFieldNumber = 1,
    kInt64ValueFieldNumber = 5,
    kFloatValueFieldNumber = 3,
    kInt32ValueFieldNumber = 9,
    kUint64ValueFieldNumber = 7,
    kUint32ValueFieldNumber = 11,
    kBoolValueFieldNumber = 13,
    kEnumValueFieldNumber = 18,
  };
  // repeated double repeated_double = 2;
  int repeated_double_size() const;
  private:
  int _internal_repeated_double_size() const;
  public:
  void clear_repeated_double();
  private:
  double _internal_repeated_double(int index) const;
  const ::PROTOBUF_NAMESPACE_ID::RepeatedField< double >&
      _internal_repeated_double() const;
  void _internal_add_repeated_double(double value);
  ::PROTOBUF_NAMESPACE_ID::RepeatedField< double >*
      _internal_mutable_repeated_double();
  public:
  double repeated_double(int index) const;
  void set_repeated_double(int index, double value);
  void add_repeated_double(double value);
  const ::PROTOBUF_NAMESPACE_ID::RepeatedField< double >&
      repeated_double() const;
  ::PROTOBUF_NAMESPACE_ID::RepeatedField< double >*
      mutable_repeated_double();

  // string string_value = 15;
  void clear_string_value();
  const std::string& string_value() const;
  void set_string_value(const std::string& value);
  void set_string_value(std::string&& value);
  void set_string_value(const char* value);
  void set_string_value(const char* value, size_t size);
  std::string* mutable_string_value();
  std::string* release_string_value();
  void set_allocated_string_value(std::string* string_value);
  private:
  const std::string& _internal_string_value() const;
  void _internal_set_string_value(const std::string& value);
  std::string* _internal_mutable_string_value();
  public:

  // bytes bytes_value = 17 [ctype = CORD];
  void clear_bytes_value();
 private:
  // Hidden due to unknown ctype option.
  const std::string& bytes_value() const;
  void set_bytes_value(const std::string& value);
  void set_bytes_value(std::string&& value);
  void set_bytes_value(const char* value);
  void set_bytes_value(const void* value, size_t size);
  std::string* mutable_bytes_value();
  std::string* release_bytes_value();
  void set_allocated_bytes_value(std::string* bytes_value);
  private:
  const std::string& _internal_bytes_value() const;
  void _internal_set_bytes_value(const std::string& value);
  std::string* _internal_mutable_bytes_value();
  public:
 public:

  // double double_value = 1;
  void clear_double_value();
  double double_value() const;
  void set_double_value(double value);
  private:
  double _internal_double_value() const;
  void _internal_set_double_value(double value);
  public:

  // int64 int64_value = 5;
  void clear_int64_value();
  ::PROTOBUF_NAMESPACE_ID::int64 int64_value() const;
  void set_int64_value(::PROTOBUF_NAMESPACE_ID::int64 value);
  private:
  ::PROTOBUF_NAMESPACE_ID::int64 _internal_int64_value() const;
  void _internal_set_int64_value(::PROTOBUF_NAMESPACE_ID::int64 value);
  public:

  // float float_value = 3;
  void clear_float_value();
  float float_value() const;
  void set_float_value(float value);
  private:
  float _internal_float_value() const;
  void _internal_set_float_value(float value);
  public:

  // int32 int32_value = 9;
  void clear_int32_value();
  ::PROTOBUF_NAMESPACE_ID::int32 int32_value() const;
  void set_int32_value(::PROTOBUF_NAMESPACE_ID::int32 value);
  private:
  ::PROTOBUF_NAMESPACE_ID::int32 _internal_int32_value() const;
  void _internal_set_int32_value(::PROTOBUF_NAMESPACE_ID::int32 value);
  public:

  // uint64 uint64_value = 7;
  void clear_uint64_value();
  ::PROTOBUF_NAMESPACE_ID::uint64 uint64_value() const;
  void set_uint64_value(::PROTOBUF_NAMESPACE_ID::uint64 value);
  private:
  ::PROTOBUF_NAMESPACE_ID::uint64 _internal_uint64_value() const;
  void _internal_set_uint64_value(::PROTOBUF_NAMESPACE_ID::uint64 value);
  public:

  // uint32 uint32_value = 11;
  void clear_uint32_value();
  ::PROTOBUF_NAMESPACE_ID::uint32 uint32_value() const;
  void set_uint32_value(::PROTOBUF_NAMESPACE_ID::uint32 value);
  private:
  ::PROTOBUF_NAMESPACE_ID::uint32 _internal_uint32_value() const;
  void _internal_set_uint32_value(::PROTOBUF_NAMESPACE_ID::uint32 value);
  public:

  // bool bool_value = 13;
  void clear_bool_value();
  bool bool_value() const;
  void set_bool_value(bool value);
  private:
  bool _internal_bool_value() const;
  void _internal_set_bool_value(bool value);
  public:

  // .proto_util_converter.testing.DefaultValueTest.EnumDefault enum_value = 18;
  void clear_enum_value();
  ::proto_util_converter::testing::DefaultValueTest_EnumDefault enum_value() const;
  void set_enum_value(::proto_util_converter::testing::DefaultValueTest_EnumDefault value);
  private:
  ::proto_util_converter::testing::DefaultValueTest_EnumDefault _internal_enum_value() const;
  void _internal_set_enum_value(::proto_util_converter::testing::DefaultValueTest_EnumDefault value);
  public:

  // @@protoc_insertion_point(class_scope:proto_util_converter.testing.DefaultValueTest)
 private:
  class _Internal;

  ::PROTOBUF_NAMESPACE_ID::internal::InternalMetadataWithArena _internal_metadata_;
  ::PROTOBUF_NAMESPACE_ID::RepeatedField< double > repeated_double_;
  mutable std::atomic<int> _repeated_double_cached_byte_size_;
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr string_value_;
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr bytes_value_;
  double double_value_;
  ::PROTOBUF_NAMESPACE_ID::int64 int64_value_;
  float float_value_;
  ::PROTOBUF_NAMESPACE_ID::int32 int32_value_;
  ::PROTOBUF_NAMESPACE_ID::uint64 uint64_value_;
  ::PROTOBUF_NAMESPACE_ID::uint32 uint32_value_;
  bool bool_value_;
  int enum_value_;
  mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
  friend struct ::TableStruct_google_2fprotobuf_2futil_2finternal_2ftestdata_2fdefault_5fvalue_5ftest_2eproto;
};
// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// DefaultValueTest

// double double_value = 1;
inline void DefaultValueTest::clear_double_value() {
  double_value_ = 0;
}
inline double DefaultValueTest::_internal_double_value() const {
  return double_value_;
}
inline double DefaultValueTest::double_value() const {
  // @@protoc_insertion_point(field_get:proto_util_converter.testing.DefaultValueTest.double_value)
  return _internal_double_value();
}
inline void DefaultValueTest::_internal_set_double_value(double value) {
  
  double_value_ = value;
}
inline void DefaultValueTest::set_double_value(double value) {
  _internal_set_double_value(value);
  // @@protoc_insertion_point(field_set:proto_util_converter.testing.DefaultValueTest.double_value)
}

// repeated double repeated_double = 2;
inline int DefaultValueTest::_internal_repeated_double_size() const {
  return repeated_double_.size();
}
inline int DefaultValueTest::repeated_double_size() const {
  return _internal_repeated_double_size();
}
inline void DefaultValueTest::clear_repeated_double() {
  repeated_double_.Clear();
}
inline double DefaultValueTest::_internal_repeated_double(int index) const {
  return repeated_double_.Get(index);
}
inline double DefaultValueTest::repeated_double(int index) const {
  // @@protoc_insertion_point(field_get:proto_util_converter.testing.DefaultValueTest.repeated_double)
  return _internal_repeated_double(index);
}
inline void DefaultValueTest::set_repeated_double(int index, double value) {
  repeated_double_.Set(index, value);
  // @@protoc_insertion_point(field_set:proto_util_converter.testing.DefaultValueTest.repeated_double)
}
inline void DefaultValueTest::_internal_add_repeated_double(double value) {
  repeated_double_.Add(value);
}
inline void DefaultValueTest::add_repeated_double(double value) {
  _internal_add_repeated_double(value);
  // @@protoc_insertion_point(field_add:proto_util_converter.testing.DefaultValueTest.repeated_double)
}
inline const ::PROTOBUF_NAMESPACE_ID::RepeatedField< double >&
DefaultValueTest::_internal_repeated_double() const {
  return repeated_double_;
}
inline const ::PROTOBUF_NAMESPACE_ID::RepeatedField< double >&
DefaultValueTest::repeated_double() const {
  // @@protoc_insertion_point(field_list:proto_util_converter.testing.DefaultValueTest.repeated_double)
  return _internal_repeated_double();
}
inline ::PROTOBUF_NAMESPACE_ID::RepeatedField< double >*
DefaultValueTest::_internal_mutable_repeated_double() {
  return &repeated_double_;
}
inline ::PROTOBUF_NAMESPACE_ID::RepeatedField< double >*
DefaultValueTest::mutable_repeated_double() {
  // @@protoc_insertion_point(field_mutable_list:proto_util_converter.testing.DefaultValueTest.repeated_double)
  return _internal_mutable_repeated_double();
}

// float float_value = 3;
inline void DefaultValueTest::clear_float_value() {
  float_value_ = 0;
}
inline float DefaultValueTest::_internal_float_value() const {
  return float_value_;
}
inline float DefaultValueTest::float_value() const {
  // @@protoc_insertion_point(field_get:proto_util_converter.testing.DefaultValueTest.float_value)
  return _internal_float_value();
}
inline void DefaultValueTest::_internal_set_float_value(float value) {
  
  float_value_ = value;
}
inline void DefaultValueTest::set_float_value(float value) {
  _internal_set_float_value(value);
  // @@protoc_insertion_point(field_set:proto_util_converter.testing.DefaultValueTest.float_value)
}

// int64 int64_value = 5;
inline void DefaultValueTest::clear_int64_value() {
  int64_value_ = PROTOBUF_LONGLONG(0);
}
inline ::PROTOBUF_NAMESPACE_ID::int64 DefaultValueTest::_internal_int64_value() const {
  return int64_value_;
}
inline ::PROTOBUF_NAMESPACE_ID::int64 DefaultValueTest::int64_value() const {
  // @@protoc_insertion_point(field_get:proto_util_converter.testing.DefaultValueTest.int64_value)
  return _internal_int64_value();
}
inline void DefaultValueTest::_internal_set_int64_value(::PROTOBUF_NAMESPACE_ID::int64 value) {
  
  int64_value_ = value;
}
inline void DefaultValueTest::set_int64_value(::PROTOBUF_NAMESPACE_ID::int64 value) {
  _internal_set_int64_value(value);
  // @@protoc_insertion_point(field_set:proto_util_converter.testing.DefaultValueTest.int64_value)
}

// uint64 uint64_value = 7;
inline void DefaultValueTest::clear_uint64_value() {
  uint64_value_ = PROTOBUF_ULONGLONG(0);
}
inline ::PROTOBUF_NAMESPACE_ID::uint64 DefaultValueTest::_internal_uint64_value() const {
  return uint64_value_;
}
inline ::PROTOBUF_NAMESPACE_ID::uint64 DefaultValueTest::uint64_value() const {
  // @@protoc_insertion_point(field_get:proto_util_converter.testing.DefaultValueTest.uint64_value)
  return _internal_uint64_value();
}
inline void DefaultValueTest::_internal_set_uint64_value(::PROTOBUF_NAMESPACE_ID::uint64 value) {
  
  uint64_value_ = value;
}
inline void DefaultValueTest::set_uint64_value(::PROTOBUF_NAMESPACE_ID::uint64 value) {
  _internal_set_uint64_value(value);
  // @@protoc_insertion_point(field_set:proto_util_converter.testing.DefaultValueTest.uint64_value)
}

// int32 int32_value = 9;
inline void DefaultValueTest::clear_int32_value() {
  int32_value_ = 0;
}
inline ::PROTOBUF_NAMESPACE_ID::int32 DefaultValueTest::_internal_int32_value() const {
  return int32_value_;
}
inline ::PROTOBUF_NAMESPACE_ID::int32 DefaultValueTest::int32_value() const {
  // @@protoc_insertion_point(field_get:proto_util_converter.testing.DefaultValueTest.int32_value)
  return _internal_int32_value();
}
inline void DefaultValueTest::_internal_set_int32_value(::PROTOBUF_NAMESPACE_ID::int32 value) {
  
  int32_value_ = value;
}
inline void DefaultValueTest::set_int32_value(::PROTOBUF_NAMESPACE_ID::int32 value) {
  _internal_set_int32_value(value);
  // @@protoc_insertion_point(field_set:proto_util_converter.testing.DefaultValueTest.int32_value)
}

// uint32 uint32_value = 11;
inline void DefaultValueTest::clear_uint32_value() {
  uint32_value_ = 0u;
}
inline ::PROTOBUF_NAMESPACE_ID::uint32 DefaultValueTest::_internal_uint32_value() const {
  return uint32_value_;
}
inline ::PROTOBUF_NAMESPACE_ID::uint32 DefaultValueTest::uint32_value() const {
  // @@protoc_insertion_point(field_get:proto_util_converter.testing.DefaultValueTest.uint32_value)
  return _internal_uint32_value();
}
inline void DefaultValueTest::_internal_set_uint32_value(::PROTOBUF_NAMESPACE_ID::uint32 value) {
  
  uint32_value_ = value;
}
inline void DefaultValueTest::set_uint32_value(::PROTOBUF_NAMESPACE_ID::uint32 value) {
  _internal_set_uint32_value(value);
  // @@protoc_insertion_point(field_set:proto_util_converter.testing.DefaultValueTest.uint32_value)
}

// bool bool_value = 13;
inline void DefaultValueTest::clear_bool_value() {
  bool_value_ = false;
}
inline bool DefaultValueTest::_internal_bool_value() const {
  return bool_value_;
}
inline bool DefaultValueTest::bool_value() const {
  // @@protoc_insertion_point(field_get:proto_util_converter.testing.DefaultValueTest.bool_value)
  return _internal_bool_value();
}
inline void DefaultValueTest::_internal_set_bool_value(bool value) {
  
  bool_value_ = value;
}
inline void DefaultValueTest::set_bool_value(bool value) {
  _internal_set_bool_value(value);
  // @@protoc_insertion_point(field_set:proto_util_converter.testing.DefaultValueTest.bool_value)
}

// string string_value = 15;
inline void DefaultValueTest::clear_string_value() {
  string_value_.ClearToEmptyNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
}
inline const std::string& DefaultValueTest::string_value() const {
  // @@protoc_insertion_point(field_get:proto_util_converter.testing.DefaultValueTest.string_value)
  return _internal_string_value();
}
inline void DefaultValueTest::set_string_value(const std::string& value) {
  _internal_set_string_value(value);
  // @@protoc_insertion_point(field_set:proto_util_converter.testing.DefaultValueTest.string_value)
}
inline std::string* DefaultValueTest::mutable_string_value() {
  // @@protoc_insertion_point(field_mutable:proto_util_converter.testing.DefaultValueTest.string_value)
  return _internal_mutable_string_value();
}
inline const std::string& DefaultValueTest::_internal_string_value() const {
  return string_value_.GetNoArena();
}
inline void DefaultValueTest::_internal_set_string_value(const std::string& value) {
  
  string_value_.SetNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), value);
}
inline void DefaultValueTest::set_string_value(std::string&& value) {
  
  string_value_.SetNoArena(
    &::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:proto_util_converter.testing.DefaultValueTest.string_value)
}
inline void DefaultValueTest::set_string_value(const char* value) {
  GOOGLE_DCHECK(value != nullptr);
  
  string_value_.SetNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:proto_util_converter.testing.DefaultValueTest.string_value)
}
inline void DefaultValueTest::set_string_value(const char* value, size_t size) {
  
  string_value_.SetNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:proto_util_converter.testing.DefaultValueTest.string_value)
}
inline std::string* DefaultValueTest::_internal_mutable_string_value() {
  
  return string_value_.MutableNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
}
inline std::string* DefaultValueTest::release_string_value() {
  // @@protoc_insertion_point(field_release:proto_util_converter.testing.DefaultValueTest.string_value)
  
  return string_value_.ReleaseNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
}
inline void DefaultValueTest::set_allocated_string_value(std::string* string_value) {
  if (string_value != nullptr) {
    
  } else {
    
  }
  string_value_.SetAllocatedNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), string_value);
  // @@protoc_insertion_point(field_set_allocated:proto_util_converter.testing.DefaultValueTest.string_value)
}

// bytes bytes_value = 17 [ctype = CORD];
inline void DefaultValueTest::clear_bytes_value() {
  bytes_value_.ClearToEmptyNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
}
inline const std::string& DefaultValueTest::bytes_value() const {
  // @@protoc_insertion_point(field_get:proto_util_converter.testing.DefaultValueTest.bytes_value)
  return _internal_bytes_value();
}
inline void DefaultValueTest::set_bytes_value(const std::string& value) {
  _internal_set_bytes_value(value);
  // @@protoc_insertion_point(field_set:proto_util_converter.testing.DefaultValueTest.bytes_value)
}
inline std::string* DefaultValueTest::mutable_bytes_value() {
  // @@protoc_insertion_point(field_mutable:proto_util_converter.testing.DefaultValueTest.bytes_value)
  return _internal_mutable_bytes_value();
}
inline const std::string& DefaultValueTest::_internal_bytes_value() const {
  return bytes_value_.GetNoArena();
}
inline void DefaultValueTest::_internal_set_bytes_value(const std::string& value) {
  
  bytes_value_.SetNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), value);
}
inline void DefaultValueTest::set_bytes_value(std::string&& value) {
  
  bytes_value_.SetNoArena(
    &::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:proto_util_converter.testing.DefaultValueTest.bytes_value)
}
inline void DefaultValueTest::set_bytes_value(const char* value) {
  GOOGLE_DCHECK(value != nullptr);
  
  bytes_value_.SetNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:proto_util_converter.testing.DefaultValueTest.bytes_value)
}
inline void DefaultValueTest::set_bytes_value(const void* value, size_t size) {
  
  bytes_value_.SetNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:proto_util_converter.testing.DefaultValueTest.bytes_value)
}
inline std::string* DefaultValueTest::_internal_mutable_bytes_value() {
  
  return bytes_value_.MutableNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
}
inline std::string* DefaultValueTest::release_bytes_value() {
  // @@protoc_insertion_point(field_release:proto_util_converter.testing.DefaultValueTest.bytes_value)
  
  return bytes_value_.ReleaseNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
}
inline void DefaultValueTest::set_allocated_bytes_value(std::string* bytes_value) {
  if (bytes_value != nullptr) {
    
  } else {
    
  }
  bytes_value_.SetAllocatedNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), bytes_value);
  // @@protoc_insertion_point(field_set_allocated:proto_util_converter.testing.DefaultValueTest.bytes_value)
}

// .proto_util_converter.testing.DefaultValueTest.EnumDefault enum_value = 18;
inline void DefaultValueTest::clear_enum_value() {
  enum_value_ = 0;
}
inline ::proto_util_converter::testing::DefaultValueTest_EnumDefault DefaultValueTest::_internal_enum_value() const {
  return static_cast< ::proto_util_converter::testing::DefaultValueTest_EnumDefault >(enum_value_);
}
inline ::proto_util_converter::testing::DefaultValueTest_EnumDefault DefaultValueTest::enum_value() const {
  // @@protoc_insertion_point(field_get:proto_util_converter.testing.DefaultValueTest.enum_value)
  return _internal_enum_value();
}
inline void DefaultValueTest::_internal_set_enum_value(::proto_util_converter::testing::DefaultValueTest_EnumDefault value) {
  
  enum_value_ = value;
}
inline void DefaultValueTest::set_enum_value(::proto_util_converter::testing::DefaultValueTest_EnumDefault value) {
  _internal_set_enum_value(value);
  // @@protoc_insertion_point(field_set:proto_util_converter.testing.DefaultValueTest.enum_value)
}

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__

// @@protoc_insertion_point(namespace_scope)

}  // namespace testing
}  // namespace proto_util_converter

PROTOBUF_NAMESPACE_OPEN

template <> struct is_proto_enum< ::proto_util_converter::testing::DefaultValueTest_EnumDefault> : ::std::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor< ::proto_util_converter::testing::DefaultValueTest_EnumDefault>() {
  return ::proto_util_converter::testing::DefaultValueTest_EnumDefault_descriptor();
}

PROTOBUF_NAMESPACE_CLOSE

// @@protoc_insertion_point(global_scope)

#include <google/protobuf/port_undef.inc>
#endif  // GOOGLE_PROTOBUF_INCLUDED_GOOGLE_PROTOBUF_INCLUDED_google_2fprotobuf_2futil_2finternal_2ftestdata_2fdefault_5fvalue_5ftest_2eproto
