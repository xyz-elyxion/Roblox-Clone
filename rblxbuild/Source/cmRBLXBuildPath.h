/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://rblxbuild.org/licensing for details.  */

#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cstddef>
#include <string>
#include <utility>

#include <cm/filesystem>
#include <cm/string_view>
#include <cm/type_traits>

namespace cm {
class static_string_view;
}

namespace detail {
#if defined(__SUNPRO_CC) && defined(__sparc)
// Oracle DeveloperStudio C++ compiler on Solaris/Sparc fails to compile
// the full 'is_pathable' and 'is_move_pathable' checks.  We use it only to
// improve error messages via 'enable_if' when calling methods with incorrect
// types. Just pretend all types are allowed so we can at least compile valid
// code.
template <typename T>
struct is_pathable : std::true_type
{
};

template <typename T>
struct is_move_pathable : std::true_type
{
};

#else
template <typename T, typename = void>
struct is_pathable : std::false_type
{
};

template <>
struct is_pathable<cm::filesystem::path> : std::true_type
{
};
template <>
struct is_pathable<std::string> : std::true_type
{
};
template <>
struct is_pathable<cm::string_view> : std::true_type
{
};
template <>
struct is_pathable<cm::static_string_view> : std::true_type
{
};
template <typename T>
struct is_pathable<
  T,
  cm::enable_if_t<std::is_same<char*, typename std::decay<T>::type>::value,
                  void>>
  : cm::bool_constant<std::is_same<char*, typename std::decay<T>::type>::value>
{
};

template <typename T>
struct is_move_pathable : std::false_type
{
};

template <>
struct is_move_pathable<cm::filesystem::path> : std::true_type
{
};
template <>
struct is_move_pathable<std::string> : std::true_type
{
};
#endif
}

class cmRBLXBuildPath
{
private:
  template <typename Source>
  using enable_if_move_pathable =
    cm::enable_if_t<detail::is_move_pathable<Source>::value, cmRBLXBuildPath&>;

  template <typename Source>
  using enable_if_pathable =
    cm::enable_if_t<detail::is_pathable<Source>::value, cmRBLXBuildPath&>;

public:
  using value_type = cm::filesystem::path::value_type;
  using string_type = cm::filesystem::path::string_type;

  enum format : unsigned char
  {
    auto_format =
      static_cast<unsigned char>(cm::filesystem::path::format::auto_format),
    native_format =
      static_cast<unsigned char>(cm::filesystem::path::format::native_format),
    generic_format =
      static_cast<unsigned char>(cm::filesystem::path::format::generic_format)
  };

  class iterator;
  using const_iterator = iterator;

  cmRBLXBuildPath() noexcept = default;

  cmRBLXBuildPath(cmRBLXBuildPath const&) = default;

  cmRBLXBuildPath(cmRBLXBuildPath&& path) noexcept
    : Path(std::forward<cm::filesystem::path>(path.Path))
  {
  }

  cmRBLXBuildPath(cm::filesystem::path path) noexcept
    : Path(std::move(path))
  {
  }
  cmRBLXBuildPath(cm::string_view source, format fmt = generic_format) noexcept
    : Path(FormatPath(source, fmt))
  {
  }
  cmRBLXBuildPath(char const* source, format fmt = generic_format) noexcept
    : Path(FormatPath(cm::string_view{ source }, fmt))
  {
  }
#if defined(__SUNPRO_CC) && defined(__sparc)
  // Oracle DeveloperStudio C++ compiler on Solaris/Sparc is confused when
  // standard methods and templates use the same name. The template is selected
  // rather than the standard one regardless the arguments of the method.
  cmRBLXBuildPath(std::string const& source, format fmt = generic_format)
    : Path(FormatPath(source, fmt))
  {
  }
  cmRBLXBuildPath(std::string&& source, format fmt = generic_format)
    : Path(FormatPath(std::move(source), fmt))
  {
  }
#else
  template <typename Source, typename = enable_if_move_pathable<Source>>
  cmRBLXBuildPath(Source source, format fmt = generic_format)
    : Path(FormatPath(std::move(source), fmt))
  {
  }
#endif

  template <typename Source, typename = enable_if_move_pathable<Source>>
  cmRBLXBuildPath& Assign(Source&& source)
  {
    this->Path = std::forward<Source>(source);
    return *this;
  }
  template <typename Source, typename = enable_if_pathable<Source>>
  cmRBLXBuildPath& Assign(Source const& source)
  {
    this->Path = source;
    return *this;
  }

  cmRBLXBuildPath& operator=(cmRBLXBuildPath const& path)
  {
    if (this != &path) {
      this->Path = path.Path;
    }
    return *this;
  }
  cmRBLXBuildPath& operator=(cmRBLXBuildPath&& path) noexcept
  {
    if (this != &path) {
      this->Path = std::move(path.Path);
    }
    return *this;
  }
#if defined(__SUNPRO_CC) && defined(__sparc)
  // Oracle DeveloperStudio C++ compiler on Solaris/Sparc is confused when
  // standard methods and templates use the same name. The template is selected
  // rather than the standard one regardless the arguments of the method.
  cmRBLXBuildPath& operator=(cm::filesystem::path&& source)
  {
    this->Assign(std::forward<cm::filesystem::path>(source));
    return *this;
  }
  cmRBLXBuildPath& operator=(std::string&& source)
  {
    this->Assign(std::forward<std::string>(source));
    return *this;
  }
  cmRBLXBuildPath& operator=(cm::filesystem::path const& source)
  {
    this->Assign(source);
    return *this;
  }
  cmRBLXBuildPath& operator=(std::string const& source)
  {
    this->Assign(source);
    return *this;
  }
  cmRBLXBuildPath& operator=(cm::string_view const source)
  {
    this->Assign(source);
    return *this;
  }
  cmRBLXBuildPath& operator=(char const* source)
  {
    this->Assign(cm::string_view{ source });
    return *this;
  }
#else
  template <typename Source, typename = enable_if_move_pathable<Source>>
  cmRBLXBuildPath& operator=(Source&& source)
  {
    this->Assign(std::forward<Source>(source));
    return *this;
  }
  template <typename Source, typename = enable_if_pathable<Source>>
  cmRBLXBuildPath& operator=(Source const& source)
  {
    this->Assign(source);
    return *this;
  }
#endif

  // Concatenation
  cmRBLXBuildPath& Append(cmRBLXBuildPath const& path)
  {
    return this->Append(path.Path);
  }
  cmRBLXBuildPath& Append(cm::filesystem::path const& path)
  {
    this->Path /= path;
    // filesystem::path::append use preferred_separator ('\' on Windows)
    // so convert back to '/'
    this->Path = this->Path.generic_string();
    return *this;
  }
#if defined(__SUNPRO_CC) && defined(__sparc)
  // Oracle DeveloperStudio C++ compiler on Solaris/Sparc is confused when
  // standard methods and templates use the same name. The template is selected
  // rather than the standard one regardless the arguments of the method.
  cmRBLXBuildPath& Append(std::string const& source)
  {
    return this->Append(cm::filesystem::path(source));
  }
  cmRBLXBuildPath& Append(cm::string_view source)
  {
    return this->Append(cm::filesystem::path(source));
  }
  cmRBLXBuildPath& Append(char const* source)
  {
    return this->Append(cm::filesystem::path(cm::string_view{ source }));
  }
#else
  template <typename Source, typename = enable_if_pathable<Source>>
  cmRBLXBuildPath& Append(Source const& source)
  {
    return this->Append(cm::filesystem::path(source));
  }
#endif

  cmRBLXBuildPath& operator/=(cmRBLXBuildPath const& path)
  {
    return this->Append(path);
  }
  template <typename Source, typename = enable_if_pathable<Source>>
  cmRBLXBuildPath& operator/=(Source const& source)
  {
    return this->Append(source);
  }

  cmRBLXBuildPath& Concat(cmRBLXBuildPath const& path)
  {
    this->Path += path.Path;
    return *this;
  }
  cmRBLXBuildPath& Concat(cm::string_view source)
  {
    this->Path.operator+=(std::string(source));
    return *this;
  }
#if defined(__SUNPRO_CC) && defined(__sparc)
  // Oracle DeveloperStudio C++ compiler on Solaris/Sparc is confused when
  // standard methods and templates use the same name. The template is selected
  // rather than the standard one regardless the arguments of the method.
  cmRBLXBuildPath& Concat(cm::filesystem::path const& source)
  {
    this->Path.operator+=(source);
    return *this;
  }
  cmRBLXBuildPath& Concat(std::string const& source)
  {
    this->Path.operator+=(source);
    return *this;
  }
  cmRBLXBuildPath& Concat(char const* source)
  {
    this->Path.operator+=(source);
    return *this;
  }
#else
  template <typename Source, typename = enable_if_pathable<Source>>
  cmRBLXBuildPath& Concat(Source const& source)
  {
    this->Path.operator+=(source);
    return *this;
  }
#endif

  cmRBLXBuildPath& operator+=(cmRBLXBuildPath const& path)
  {
    return this->Concat(path);
  }
  template <typename Source, typename = enable_if_pathable<Source>>
  cmRBLXBuildPath& operator+=(Source const& source)
  {
    return this->Concat(source);
  }

  // Manipulation
  void Clear() noexcept { this->Path.clear(); }

  cmRBLXBuildPath& RemoveFileName()
  {
    this->Path.remove_filename();
    return *this;
  }

  cmRBLXBuildPath& ReplaceFileName(cmRBLXBuildPath const& filename)
  {
    if (this->Path.has_filename()) {
      this->Path.replace_filename(filename.Path);
    }
    return *this;
  }
#if defined(__SUNPRO_CC) && defined(__sparc)
  // Oracle DeveloperStudio C++ compiler on Solaris/Sparc is confused when
  // standard methods and templates use the same name. The template is selected
  // rather than the standard one regardless the arguments of the method.
  cmRBLXBuildPath& ReplaceFileName(cm::filesystem::path const& filename)
  {
    if (this->Path.has_filename()) {
      this->Path.replace_filename(filename);
    }
    return *this;
  }
  cmRBLXBuildPath& ReplaceFileName(std::string const& filename)
  {
    if (this->Path.has_filename()) {
      this->Path.replace_filename(filename);
    }
    return *this;
  }
  cmRBLXBuildPath& ReplaceFileName(cm::string_view filename)
  {
    if (this->Path.has_filename()) {
      this->Path.replace_filename(filename);
    }
    return *this;
  }
#else
  template <typename Source, typename = enable_if_pathable<Source>>
  cmRBLXBuildPath& ReplaceFileName(Source const& filename)
  {
    if (this->Path.has_filename()) {
      this->Path.replace_filename(filename);
    }
    return *this;
  }
#endif

  cmRBLXBuildPath& ReplaceExtension(cmRBLXBuildPath const& extension = cmRBLXBuildPath())
  {
    this->Path.replace_extension(extension.Path);
    return *this;
  }
#if defined(__SUNPRO_CC) && defined(__sparc)
  // Oracle DeveloperStudio C++ compiler on Solaris/Sparc is confused when
  // standard methods and templates use the same name. The template is selected
  // rather than the standard one regardless the arguments of the method.
  cmRBLXBuildPath& ReplaceExtension(cm::filesystem::path const& extension)
  {
    this->Path.replace_extension(extension);
    return *this;
  }
  cmRBLXBuildPath& ReplaceExtension(std::string const& extension)
  {
    this->Path.replace_extension(extension);
    return *this;
  }
  cmRBLXBuildPath& ReplaceExtension(cm::string_view const extension)
  {
    this->Path.replace_extension(extension);
    return *this;
  }
#else
  template <typename Source, typename = enable_if_pathable<Source>>
  cmRBLXBuildPath& ReplaceExtension(Source const& extension)
  {
    this->Path.replace_extension(extension);
    return *this;
  }
#endif

  cmRBLXBuildPath& ReplaceWideExtension(
    cmRBLXBuildPath const& extension = cmRBLXBuildPath())
  {
    return this->ReplaceWideExtension(
      static_cast<cm::string_view>(extension.Path.string()));
  }
  cmRBLXBuildPath& ReplaceWideExtension(cm::filesystem::path const& extension)
  {
    return this->ReplaceWideExtension(
      static_cast<cm::string_view>(extension.string()));
  }
#if defined(__SUNPRO_CC) && defined(__sparc)
  // Oracle DeveloperStudio C++ compiler on Solaris/Sparc is confused when
  // standard methods and templates use the same name. The template is selected
  // rather than the standard one regardless the arguments of the method.
  cmRBLXBuildPath& ReplaceWideExtension(std::string const& extension)
  {
    return this->ReplaceWideExtension(cm::string_view{ extension });
  }
#else
  template <typename Source, typename = enable_if_pathable<Source>>
  cmRBLXBuildPath& ReplaceWideExtension(Source const& extension)
  {
    return this->ReplaceWideExtension(extension);
  }
#endif
  cmRBLXBuildPath& ReplaceWideExtension(cm::string_view extension);

  cmRBLXBuildPath& RemoveExtension()
  {
    if (this->Path.has_extension()) {
      this->ReplaceExtension(cm::string_view(""));
    }
    return *this;
  }

  cmRBLXBuildPath& RemoveWideExtension()
  {
    if (this->Path.has_extension()) {
      this->ReplaceWideExtension(cm::string_view(""));
    }
    return *this;
  }

  void swap(cmRBLXBuildPath& other) noexcept { this->Path.swap(other.Path); }

  // Observers
  std::string String() const { return this->Path.string(); }
  std::wstring WString() const { return this->Path.wstring(); }

  string_type Native() const
  {
    string_type path;
    this->GetNativePath(path);

    return path;
  }
  std::string NativeString() const
  {
    std::string path;
    this->GetNativePath(path);

    return path;
  }
  std::wstring NativeWString() const
  {
    std::wstring path;
    this->GetNativePath(path);

    return path;
  }
  std::string GenericString() const { return this->Path.generic_string(); }
  std::wstring GenericWString() const { return this->Path.generic_wstring(); }

  // Decomposition
  cmRBLXBuildPath GetRootName() const { return this->Path.root_name(); }
  cmRBLXBuildPath GetRootDirectory() const { return this->Path.root_directory(); }
  cmRBLXBuildPath GetRootPath() const { return this->Path.root_path(); }
  cmRBLXBuildPath GetFileName() const { return this->Path.filename(); }
  cmRBLXBuildPath GetExtension() const { return this->Path.extension(); }
  cmRBLXBuildPath GetWideExtension() const;
  cmRBLXBuildPath GetStem() const { return this->Path.stem(); }
  cmRBLXBuildPath GetNarrowStem() const;

  cmRBLXBuildPath GetRelativePath() const { return this->Path.relative_path(); }
  cmRBLXBuildPath GetParentPath() const { return this->Path.parent_path(); }

  // Generation
  cmRBLXBuildPath Normal() const
  {
    auto path = this->Path.lexically_normal();
    // filesystem::path:lexically_normal use preferred_separator ('\') on
    // Windows) so convert back to '/'
    return path.generic_string();
  }

  cmRBLXBuildPath Relative(cmRBLXBuildPath const& base) const
  {
    return this->Relative(base.Path);
  }
  cmRBLXBuildPath Relative(cm::filesystem::path const& base) const
  {
    auto path = this->Path.lexically_relative(base);
    // filesystem::path:lexically_relative use preferred_separator ('\') on
    // Windows) so convert back to '/'
    return path.generic_string();
  }
#if defined(__SUNPRO_CC) && defined(__sparc)
  // Oracle DeveloperStudio C++ compiler on Solaris/Sparc is confused when
  // standard methods and templates use the same name. The template is selected
  // rather than the standard one regardless the arguments of the method.
  cmRBLXBuildPath Relative(std::string const& base) const
  {
    return this->Relative(cm::filesystem::path(base));
  }
  cmRBLXBuildPath Relative(cm::string_view base) const
  {
    return this->Relative(cm::filesystem::path(base));
  }
#else
  template <typename Source, typename = enable_if_pathable<Source>>
  cmRBLXBuildPath Relative(Source const& base) const
  {
    return this->Relative(cm::filesystem::path(base));
  }
#endif
  cmRBLXBuildPath Proximate(cmRBLXBuildPath const& base) const
  {
    return this->Proximate(base.Path);
  }
  cmRBLXBuildPath Proximate(cm::filesystem::path const& base) const
  {
    auto path = this->Path.lexically_proximate(base);
    // filesystem::path::lexically_proximate use preferred_separator ('\') on
    // Windows) so convert back to '/'
    return path.generic_string();
  }
#if defined(__SUNPRO_CC) && defined(__sparc)
  // Oracle DeveloperStudio C++ compiler on Solaris/Sparc is confused when
  // standard methods and templates use the same name. The template is selected
  // rather than the standard one regardless the arguments of the method.
  cmRBLXBuildPath Proximate(std::string const& base) const
  {
    return this->Proximate(cm::filesystem::path(base));
  }
  cmRBLXBuildPath Proximate(cm::string_view base) const
  {
    return this->Proximate(cm::filesystem::path(base));
  }
#else
  template <typename Source, typename = enable_if_pathable<Source>>
  cmRBLXBuildPath Proximate(Source const& base) const
  {
    return this->Proximate(cm::filesystem::path(base));
  }
#endif

  cmRBLXBuildPath Absolute(cmRBLXBuildPath const& base) const
  {
    return this->Absolute(base.Path);
  }
#if defined(__SUNPRO_CC) && defined(__sparc)
  // Oracle DeveloperStudio C++ compiler on Solaris/Sparc is confused when
  // standard methods and templates use the same name. The template is selected
  // rather than the standard one regardless the arguments of the method.
  cmRBLXBuildPath Absolute(std::string const& base) const
  {
    return this->Absolute(cm::filesystem::path(base));
  }
  cmRBLXBuildPath Absolute(cm::string_view base) const
  {
    return this->Absolute(cm::filesystem::path(base));
  }
#else
  template <typename Source, typename = enable_if_pathable<Source>>
  cmRBLXBuildPath Absolute(Source const& base) const
  {
    return this->Absolute(cm::filesystem::path(base));
  }
#endif
  cmRBLXBuildPath Absolute(cm::filesystem::path const& base) const;

  // Comparison
  int Compare(cmRBLXBuildPath const& path) const noexcept
  {
    return this->Path.compare(path.Path);
  }

  // Query
  bool IsEmpty() const noexcept { return this->Path.empty(); }

  bool HasRootPath() const { return this->Path.has_root_path(); }
  bool HasRootName() const { return this->Path.has_root_name(); }
  bool HasRootDirectory() const { return this->Path.has_root_directory(); }
  bool HasRelativePath() const { return this->Path.has_relative_path(); }
  bool HasParentPath() const { return this->Path.has_parent_path(); }
  bool HasFileName() const { return this->Path.has_filename(); }
  bool HasStem() const { return this->Path.has_stem(); }
  bool HasExtension() const { return this->Path.has_extension(); }

  bool IsAbsolute() const { return this->Path.is_absolute(); }
  bool IsRelative() const { return this->Path.is_relative(); }
  bool IsPrefix(cmRBLXBuildPath const& path) const;

  // Iterators
  // =========
  inline iterator begin() const;
  inline iterator end() const;

  // Non-members
  // ===========
  friend bool operator==(cmRBLXBuildPath const& lhs,
                         cmRBLXBuildPath const& rhs) noexcept
  {
    return lhs.Compare(rhs) == 0;
  }
  friend bool operator!=(cmRBLXBuildPath const& lhs,
                         cmRBLXBuildPath const& rhs) noexcept
  {
    return lhs.Compare(rhs) != 0;
  }

  friend cmRBLXBuildPath operator/(cmRBLXBuildPath const& lhs, cmRBLXBuildPath const& rhs)
  {
    cmRBLXBuildPath result(lhs);
    result /= rhs;

    return result;
  }

private:
  friend std::size_t hash_value(cmRBLXBuildPath const& path) noexcept;

  static std::string FormatPath(std::string path, format fmt = generic_format);
  static std::string FormatPath(cm::string_view path,
                                format fmt = generic_format)
  {
    return FormatPath(std::string(path), fmt);
  }

  void GetNativePath(std::string& path) const;
  void GetNativePath(std::wstring& path) const;

  cm::filesystem::path Path;
};

class cmRBLXBuildPath::iterator
{
public:
  using iterator_category = cm::filesystem::path::iterator::iterator_category;

  using value_type = cmRBLXBuildPath;
  using difference_type = cm::filesystem::path::iterator::difference_type;
  using pointer = cmRBLXBuildPath const*;
  using reference = cmRBLXBuildPath const&;

  iterator() = default;

  iterator(iterator const& other) = default;

  ~iterator() = default;

  iterator& operator=(iterator const& other)
  {
    if (this != &other) {
      this->Iterator = other.Iterator;
      this->Path = other.Path;
      this->PathElement = other.PathElement;
    }

    return *this;
  }

  reference operator*() const { return this->PathElement; }

  pointer operator->() const { return &this->PathElement; }

  iterator& operator++()
  {
    ++this->Iterator;
    if (this->Path && this->Iterator != this->Path->Path.end()) {
      this->PathElement = *this->Iterator;
    }

    return *this;
  }

  iterator operator++(int)
  {
    iterator it(*this);
    this->operator++();
    return it;
  }

  iterator& operator--()
  {
    --this->Iterator;
    if (this->Path && this->Iterator != this->Path->Path.end()) {
      this->PathElement = *this->Iterator;
    }

    return *this;
  }

  iterator operator--(int)
  {
    iterator it(*this);
    this->operator--();
    return it;
  }

private:
  friend class cmRBLXBuildPath;
  friend bool operator==(iterator const&, iterator const&);

  iterator(cmRBLXBuildPath const* path, cm::filesystem::path::iterator const& it)
    : Iterator(it)
    , Path(path)
  {
    if (this->Path && this->Iterator != this->Path->Path.end()) {
      this->PathElement = *this->Iterator;
    }
  }

  cm::filesystem::path::iterator Iterator;
  cmRBLXBuildPath const* Path = nullptr;
  cmRBLXBuildPath PathElement;
};

inline cmRBLXBuildPath::iterator cmRBLXBuildPath::begin() const
{
  return iterator(this, this->Path.begin());
}
inline cmRBLXBuildPath::iterator cmRBLXBuildPath::end() const
{
  return iterator(this, this->Path.end());
}

// Non-member functions
// ====================
inline bool operator==(cmRBLXBuildPath::iterator const& lhs,
                       cmRBLXBuildPath::iterator const& rhs)
{
  return lhs.Path == rhs.Path && lhs.Path && lhs.Iterator == rhs.Iterator;
}

inline bool operator!=(cmRBLXBuildPath::iterator const& lhs,
                       cmRBLXBuildPath::iterator const& rhs)
{
  return !(lhs == rhs);
}

inline void swap(cmRBLXBuildPath& lhs, cmRBLXBuildPath& rhs) noexcept
{
  lhs.swap(rhs);
}

inline std::size_t hash_value(cmRBLXBuildPath const& path) noexcept
{
  return cm::filesystem::hash_value(path.Path);
}
