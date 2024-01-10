/**
 *  @brief  StringZilla C++ wrapper improving over the performance of `std::string_view` and `std::string`,
 *          mostly for substring search, adding approximate matching functionality, and C++23 functionality
 *          to a C++11 compatible implementation.
 *
 *  This implementation is aiming to be compatible with C++11, while implementing the C++23 functionality.
 *  By default, it includes C++ STL headers, but that can be avoided to minimize compilation overhead.
 *  https://artificial-mind.net/projects/compile-health/
 */
#ifndef STRINGZILLA_HPP_
#define STRINGZILLA_HPP_

/**
 *  @brief  When set to 1, the library will include the C++ STL headers and implement
 *          automatic conversion from and to `std::stirng_view` and `std::basic_string<any_allocator>`.
 */
#ifndef SZ_INCLUDE_STL_CONVERSIONS
#define SZ_INCLUDE_STL_CONVERSIONS 1
#endif

/**
 *  @brief  When set to 1, the strings `+` will return an expression template rather than a temporary string.
 *          This will improve performance, but may break some STL-specific code, so it's disabled by default.
 */
#ifndef SZ_LAZY_CONCAT
#define SZ_LAZY_CONCAT 0
#endif

/**
 *  @brief  When set to 1, the library will change `substr` and several other member methods of `string`
 *          to return a view of its slice, rather than a copy, if the lifetime of the object is guaranteed.
 *          This will improve performance, but may break some STL-specific code, so it's disabled by default.
 */
#ifndef SZ_PREFER_VIEWS
#define SZ_PREFER_VIEWS 0
#endif

#if SZ_INCLUDE_STL_CONVERSIONS
#include <string>
#include <string_view>
#endif

#include <cassert> // `assert`
#include <cstddef> // `std::size_t`

#include <stringzilla/stringzilla.h>

namespace ashvardanian {
namespace stringzilla {

/**
 *  @brief  The concatenation of the `ascii_lowercase` and `ascii_uppercase`. This value is not locale-dependent.
 *          https://docs.python.org/3/library/string.html#string.ascii_letters
 */
inline constexpr static char ascii_letters[52] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
                                                  'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
                                                  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
                                                  'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};

/**
 *  @brief  The lowercase letters "abcdefghijklmnopqrstuvwxyz". This value is not locale-dependent.
 *          https://docs.python.org/3/library/string.html#string.ascii_lowercase
 */
inline constexpr static char ascii_lowercase[26] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
                                                    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};

/**
 *  @brief  The uppercase letters "ABCDEFGHIJKLMNOPQRSTUVWXYZ". This value is not locale-dependent.
 *          https://docs.python.org/3/library/string.html#string.ascii_uppercase
 */
inline constexpr static char ascii_uppercase[26] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
                                                    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};

/**
 *  @brief  ASCII characters which are considered printable.
 *          A combination of `digits`, `ascii_letters`, `punctuation`, and `whitespace`.
 *          https://docs.python.org/3/library/string.html#string.printable
 */
inline constexpr static char ascii_printables[100] = {
    '0', '1', '2', '3', '4', '5',  '6', '7', '8',  '9', 'a', 'b', 'c', 'd', 'e', 'f',  'g',  'h',  'i',  'j',
    'k', 'l', 'm', 'n', 'o', 'p',  'q', 'r', 's',  't', 'u', 'v', 'w', 'x', 'y', 'z',  'A',  'B',  'C',  'D',
    'E', 'F', 'G', 'H', 'I', 'J',  'K', 'L', 'M',  'N', 'O', 'P', 'Q', 'R', 'S', 'T',  'U',  'V',  'W',  'X',
    'Y', 'Z', '!', '"', '#', '$',  '%', '&', '\'', '(', ')', '*', '+', ',', '-', '.',  '/',  ':',  ';',  '<',
    '=', '>', '?', '@', '[', '\\', ']', '^', '_',  '`', '{', '|', '}', '~', ' ', '\t', '\n', '\r', '\f', '\v'};

/**
 *  @brief  Non-printable ASCII control characters.
 *          Includes all codes from 0 to 31 and 127.
 */
inline constexpr static char ascii_controls[33] = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16,
                                                   17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 127};

/**
 *  @brief  The digits "0123456789".
 *          https://docs.python.org/3/library/string.html#string.digits
 */
inline constexpr static char digits[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

/**
 *  @brief  The letters "0123456789abcdefABCDEF".
 *          https://docs.python.org/3/library/string.html#string.hexdigits
 */
inline constexpr static char hexdigits[22] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', //
                                              'a', 'b', 'c', 'd', 'e', 'f', 'A', 'B', 'C', 'D', 'E', 'F'};

/**
 *  @brief  The letters "01234567".
 *          https://docs.python.org/3/library/string.html#string.octdigits
 */
inline constexpr static char octdigits[8] = {'0', '1', '2', '3', '4', '5', '6', '7'};

/**
 *  @brief  ASCII characters considered punctuation characters in the C locale:
 *          !"#$%&'()*+,-./:;<=>?@[\]^_`{|}~.
 *          https://docs.python.org/3/library/string.html#string.punctuation
 */
inline constexpr static char punctuation[32] = { //
    '!', '"', '#', '$', '%', '&', '\'', '(',  ')', '*', '+', ',', '-', '.', '/', ':',
    ';', '<', '=', '>', '?', '@', '[',  '\\', ']', '^', '_', '`', '{', '|', '}', '~'};

/**
 *  @brief  ASCII characters that are considered whitespace.
 *          This includes space, tab, linefeed, return, formfeed, and vertical tab.
 *          https://docs.python.org/3/library/string.html#string.whitespace
 */
inline constexpr static char whitespaces[6] = {' ', '\t', '\n', '\r', '\f', '\v'};

/**
 *  @brief  ASCII characters that are considered line delimiters.
 *          https://docs.python.org/3/library/stdtypes.html#str.splitlines
 */
inline constexpr static char newlines[8] = {'\n', '\r', '\f', '\v', '\x1C', '\x1D', '\x1E', '\x85'};

/**
 *  @brief  ASCII characters forming the BASE64 encoding alphabet.
 */
inline constexpr static char base64[64] = { //
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R',
    'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};

/**
 *  @brief  A set of characters represented as a bitset with 256 slots.
 */
class character_set {
    sz_u8_set_t bitset_;

  public:
    constexpr character_set() noexcept {
        // ! Instead of relying on the `sz_u8_set_init`, we have to reimplement it to support `constexpr`.
        bitset_._u64s[0] = 0, bitset_._u64s[1] = 0, bitset_._u64s[2] = 0, bitset_._u64s[3] = 0;
    }
    constexpr explicit character_set(std::initializer_list<char> chars) noexcept : character_set() {
        // ! Instead of relying on the `sz_u8_set_add(&bitset_, c)`, we have to reimplement it to support `constexpr`.
        for (auto c : chars) bitset_._u64s[c >> 6] |= (1ull << (c & 63u));
    }
    template <std::size_t count_characters>
    constexpr explicit character_set(char const (&chars)[count_characters]) noexcept : character_set() {
        static_assert(count_characters > 0, "Character array cannot be empty");
        for (std::size_t i = 0; i < count_characters - 1; ++i) { // count_characters - 1 to exclude the null terminator
            char c = chars[i];
            bitset_._u64s[c >> 6] |= (1ull << (c & 63u));
        }
    }

    constexpr character_set(character_set const &other) noexcept : bitset_(other.bitset_) {}
    constexpr character_set &operator=(character_set const &other) noexcept {
        bitset_ = other.bitset_;
        return *this;
    }

    constexpr character_set operator|(character_set other) const noexcept {
        character_set result = *this;
        result.bitset_._u64s[0] |= other.bitset_._u64s[0], result.bitset_._u64s[1] |= other.bitset_._u64s[1],
            result.bitset_._u64s[2] |= other.bitset_._u64s[2], result.bitset_._u64s[3] |= other.bitset_._u64s[3];
        return *this;
    }

    inline character_set &add(char c) noexcept {
        sz_u8_set_add(&bitset_, c);
        return *this;
    }
    inline sz_u8_set_t &raw() noexcept { return bitset_; }
    inline sz_u8_set_t const &raw() const noexcept { return bitset_; }
    inline bool contains(char c) const noexcept { return sz_u8_set_contains(&bitset_, c); }
    inline character_set inverted() const noexcept {
        character_set result = *this;
        sz_u8_set_invert(&result.bitset_);
        return result;
    }
};

inline constexpr static character_set ascii_letters_set {ascii_letters};
inline constexpr static character_set ascii_lowercase_set {ascii_lowercase};
inline constexpr static character_set ascii_uppercase_set {ascii_uppercase};
inline constexpr static character_set ascii_printables_set {ascii_printables};
inline constexpr static character_set ascii_controls_set {ascii_controls};
inline constexpr static character_set digits_set {digits};
inline constexpr static character_set hexdigits_set {hexdigits};
inline constexpr static character_set octdigits_set {octdigits};
inline constexpr static character_set punctuation_set {punctuation};
inline constexpr static character_set whitespaces_set {whitespaces};
inline constexpr static character_set newlines_set {newlines};
inline constexpr static character_set base64_set {base64};

/**
 *  @brief  A result of split a string once, containing the string slice ::before,
 *          the ::match itself, and the slice ::after.
 */
template <typename string_>
struct string_partition_result {
    string_ before;
    string_ match;
    string_ after;
};

/**
 *  @brief  Zero-cost wrapper around the `.find` member function of string-like classes.
 *
 *  TODO: Apply Galil rule to match repetitive patterns in strictly linear time.
 */
template <typename string_view_>
struct matcher_find {
    using size_type = typename string_view_::size_type;
    string_view_ needle_;
    std::size_t skip_after_match_ = 1;

    matcher_find(string_view_ needle = {}, bool allow_overlaps = true) noexcept
        : needle_(needle), skip_after_match_(allow_overlaps ? 1 : needle_.length()) {}
    size_type needle_length() const noexcept { return needle_.length(); }
    size_type skip_length() const noexcept { return skip_after_match_; }
    size_type operator()(string_view_ haystack) const noexcept { return haystack.find(needle_); }
};

/**
 *  @brief  Zero-cost wrapper around the `.rfind` member function of string-like classes.
 *
 *  TODO: Apply Galil rule to match repetitive patterns in strictly linear time.
 */
template <typename string_view_>
struct matcher_rfind {
    using size_type = typename string_view_::size_type;
    string_view_ needle_;
    std::size_t skip_after_match_ = 1;

    matcher_rfind(string_view_ needle = {}, bool allow_overlaps = true) noexcept
        : needle_(needle), skip_after_match_(allow_overlaps ? 1 : needle_.length()) {}
    size_type needle_length() const noexcept { return needle_.length(); }
    size_type skip_length() const noexcept { return skip_after_match_; }
    size_type operator()(string_view_ haystack) const noexcept { return haystack.rfind(needle_); }
};

/**
 *  @brief  Zero-cost wrapper around the `.find_first_of` member function of string-like classes.
 */
template <typename string_view_>
struct matcher_find_first_of {
    using size_type = typename string_view_::size_type;
    string_view_ needles_;
    constexpr size_type needle_length() const noexcept { return 1; }
    constexpr size_type skip_length() const noexcept { return 1; }
    size_type operator()(string_view_ haystack) const noexcept { return haystack.find_first_of(needles_); }
};

/**
 *  @brief  Zero-cost wrapper around the `.find_last_of` member function of string-like classes.
 */
template <typename string_view_>
struct matcher_find_last_of {
    using size_type = typename string_view_::size_type;
    string_view_ needles_;
    constexpr size_type needle_length() const noexcept { return 1; }
    constexpr size_type skip_length() const noexcept { return 1; }
    size_type operator()(string_view_ haystack) const noexcept { return haystack.find_last_of(needles_); }
};

/**
 *  @brief  Zero-cost wrapper around the `.find_first_not_of` member function of string-like classes.
 */
template <typename string_view_>
struct matcher_find_first_not_of {
    using size_type = typename string_view_::size_type;
    string_view_ needles_;
    constexpr size_type needle_length() const noexcept { return 1; }
    constexpr size_type skip_length() const noexcept { return 1; }
    size_type operator()(string_view_ haystack) const noexcept { return haystack.find_first_not_of(needles_); }
};

/**
 *  @brief  Zero-cost wrapper around the `.find_last_not_of` member function of string-like classes.
 */
template <typename string_view_>
struct matcher_find_last_not_of {
    using size_type = typename string_view_::size_type;
    string_view_ needles_;
    constexpr size_type needle_length() const noexcept { return 1; }
    constexpr size_type skip_length() const noexcept { return 1; }
    size_type operator()(string_view_ haystack) const noexcept { return haystack.find_last_not_of(needles_); }
};

struct end_sentinel_type {};
inline static constexpr end_sentinel_type end_sentinel;

/**
 *  @brief  A range of string slices representing the matches of a substring search.
 *          Compatible with C++23 ranges, C++11 string views, and of course, StringZilla.
 */
template <typename string_view_, template <typename> typename matcher_template_>
class range_matches {
    using string_view = string_view_;
    using matcher = matcher_template_<string_view>;

    string_view haystack_;
    matcher matcher_;

  public:
    range_matches(string_view haystack, matcher needle) : haystack_(haystack), matcher_(needle) {}

    class iterator {
        matcher matcher_;
        string_view remaining_;

      public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = string_view;
        using pointer = string_view;   // Needed for compatibility with STL container constructors.
        using reference = string_view; // Needed for compatibility with STL container constructors.

        iterator(string_view haystack, matcher matcher) noexcept : matcher_(matcher), remaining_(haystack) {
            auto position = matcher_(remaining_);
            remaining_.remove_prefix(position != string_view::npos ? position : remaining_.size());
        }

        value_type operator*() const noexcept { return remaining_.substr(0, matcher_.needle_length()); }

        iterator &operator++() noexcept {
            remaining_.remove_prefix(matcher_.skip_length());
            auto position = matcher_(remaining_);
            remaining_.remove_prefix(position != string_view::npos ? position : remaining_.size());
            return *this;
        }

        iterator operator++(int) noexcept {
            iterator temp = *this;
            ++(*this);
            return temp;
        }

        bool operator!=(iterator const &other) const noexcept { return remaining_.begin() != other.remaining_.begin(); }
        bool operator==(iterator const &other) const noexcept { return remaining_.begin() == other.remaining_.begin(); }
        bool operator!=(end_sentinel_type) const noexcept { return !remaining_.empty(); }
        bool operator==(end_sentinel_type) const noexcept { return remaining_.empty(); }
    };

    iterator begin() const noexcept { return iterator(haystack_, matcher_); }
    iterator end() const noexcept { return iterator({haystack_.end(), 0}, matcher_); }
    typename iterator::difference_type size() const noexcept { return std::distance(begin(), end()); }
    bool empty() const noexcept { return begin() == end_sentinel; }
    bool allow_overlaps() const noexcept { return matcher_.skip_length() < matcher_.needle_length(); }

    /**
     *  @brief  Copies the matches into a container.
     */
    template <typename container_>
    void to(container_ &container) {
        for (auto match : *this) { container.push_back(match); }
    }

    /**
     *  @brief  Copies the matches into a consumed container, returning it at the end.
     */
    template <typename container_>
    container_ to() {
        return container_ {begin(), end()};
    }
};

/**
 *  @brief  A range of string slices representing the matches of a @b reverse-order substring search.
 *          Compatible with C++23 ranges, C++11 string views, and of course, StringZilla.
 */
template <typename string_view_, template <typename> typename matcher_template_>
class range_rmatches {
    using string_view = string_view_;
    using matcher = matcher_template_<string_view>;

    matcher matcher_;
    string_view haystack_;

  public:
    range_rmatches(string_view haystack, matcher needle) : haystack_(haystack), matcher_(needle) {}

    class iterator {
        string_view remaining_;
        matcher matcher_;

      public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = string_view;
        using pointer = string_view;   // Needed for compatibility with STL container constructors.
        using reference = string_view; // Needed for compatibility with STL container constructors.

        iterator(string_view haystack, matcher matcher) noexcept : matcher_(matcher), remaining_(haystack) {
            auto position = matcher_(remaining_);
            remaining_.remove_suffix(position != string_view::npos
                                         ? remaining_.size() - position - matcher_.needle_length()
                                         : remaining_.size());
        }

        value_type operator*() const noexcept {
            return remaining_.substr(remaining_.size() - matcher_.needle_length());
        }

        iterator &operator++() noexcept {
            remaining_.remove_suffix(matcher_.skip_length());
            auto position = matcher_(remaining_);
            remaining_.remove_suffix(position != string_view::npos
                                         ? remaining_.size() - position - matcher_.needle_length()
                                         : remaining_.size());
            return *this;
        }

        iterator operator++(int) noexcept {
            iterator temp = *this;
            ++(*this);
            return temp;
        }

        bool operator!=(iterator const &other) const noexcept { return remaining_.end() != other.remaining_.end(); }
        bool operator==(iterator const &other) const noexcept { return remaining_.end() == other.remaining_.end(); }
        bool operator!=(end_sentinel_type) const noexcept { return !remaining_.empty(); }
        bool operator==(end_sentinel_type) const noexcept { return remaining_.empty(); }
    };

    iterator begin() const noexcept { return iterator(haystack_, matcher_); }
    iterator end() const noexcept { return iterator({haystack_.begin(), 0}, matcher_); }
    typename iterator::difference_type size() const noexcept { return std::distance(begin(), end()); }
    bool empty() const noexcept { return begin() == end_sentinel; }
    bool allow_overlaps() const noexcept { return matcher_.skip_length() < matcher_.needle_length(); }

    /**
     *  @brief  Copies the matches into a container.
     */
    template <typename container_>
    void to(container_ &container) {
        for (auto match : *this) { container.push_back(match); }
    }

    /**
     *  @brief  Copies the matches into a consumed container, returning it at the end.
     */
    template <typename container_>
    container_ to() {
        return container_ {begin(), end()};
    }
};

/**
 *  @brief  A range of string slices for different splits of the data.
 *          Compatible with C++23 ranges, C++11 string views, and of course, StringZilla.
 *
 *  In some sense, represents the inverse operation to `range_matches`, as it reports not the search matches
 *  but the data between them. Meaning that for `N` search matches, there will be `N+1` elements in the range.
 *  Unlike ::range_matches, this range can't be empty. It also can't report overlapping intervals.
 */
template <typename string_view_, template <typename> typename matcher_template_>
class range_splits {
    using string_view = string_view_;
    using matcher = matcher_template_<string_view>;

    string_view haystack_;
    matcher matcher_;

  public:
    range_splits(string_view haystack, matcher needle) : haystack_(haystack), matcher_(needle) {}

    class iterator {
        matcher matcher_;
        string_view remaining_;
        std::size_t length_within_remaining_;
        bool reached_tail_;

      public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = string_view;
        using pointer = string_view;   // Needed for compatibility with STL container constructors.
        using reference = string_view; // Needed for compatibility with STL container constructors.

        iterator(string_view haystack, matcher matcher) noexcept : matcher_(matcher), remaining_(haystack) {
            auto position = matcher_(remaining_);
            length_within_remaining_ = position != string_view::npos ? position : remaining_.size();
            reached_tail_ = false;
        }

        iterator(string_view haystack, matcher matcher, end_sentinel_type) noexcept
            : matcher_(matcher), remaining_(haystack), reached_tail_(true) {}

        value_type operator*() const noexcept { return remaining_.substr(0, length_within_remaining_); }

        iterator &operator++() noexcept {
            remaining_.remove_prefix(length_within_remaining_);
            reached_tail_ = remaining_.empty();
            remaining_.remove_prefix(matcher_.needle_length() * !reached_tail_);
            auto position = matcher_(remaining_);
            length_within_remaining_ = position != string_view::npos ? position : remaining_.size();
            return *this;
        }

        iterator operator++(int) noexcept {
            iterator temp = *this;
            ++(*this);
            return temp;
        }

        bool operator!=(iterator const &other) const noexcept {
            return (remaining_.begin() != other.remaining_.begin()) || (reached_tail_ != other.reached_tail_);
        }
        bool operator==(iterator const &other) const noexcept {
            return (remaining_.begin() == other.remaining_.begin()) && (reached_tail_ == other.reached_tail_);
        }
        bool operator!=(end_sentinel_type) const noexcept { return !remaining_.empty() || !reached_tail_; }
        bool operator==(end_sentinel_type) const noexcept { return remaining_.empty() && reached_tail_; }
    };

    iterator begin() const noexcept { return iterator(haystack_, matcher_); }
    iterator end() const noexcept { return iterator({haystack_.end(), 0}, matcher_, end_sentinel); }
    typename iterator::difference_type size() const noexcept { return std::distance(begin(), end()); }
    constexpr bool empty() const noexcept { return false; }

    /**
     *  @brief  Copies the matches into a container.
     */
    template <typename container_>
    void to(container_ &container) {
        for (auto match : *this) { container.push_back(match); }
    }

    /**
     *  @brief  Copies the matches into a consumed container, returning it at the end.
     */
    template <typename container_>
    container_ to(container_ &&container = {}) {
        for (auto match : *this) { container.push_back(match); }
        return std::move(container);
    }
};

/**
 *  @brief  A range of string slices for different splits of the data in @b reverse-order.
 *          Compatible with C++23 ranges, C++11 string views, and of course, StringZilla.
 *
 *  In some sense, represents the inverse operation to `range_matches`, as it reports not the search matches
 *  but the data between them. Meaning that for `N` search matches, there will be `N+1` elements in the range.
 *  Unlike ::range_matches, this range can't be empty. It also can't report overlapping intervals.
 */
template <typename string_view_, template <typename> typename matcher_template_>
class range_rsplits {
    using string_view = string_view_;
    using matcher = matcher_template_<string_view>;

    string_view haystack_;
    matcher matcher_;

  public:
    range_rsplits(string_view haystack, matcher needle) : haystack_(haystack), matcher_(needle) {}

    class iterator {
        matcher matcher_;
        string_view remaining_;
        std::size_t length_within_remaining_;
        bool reached_tail_;

      public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = string_view;
        using pointer = string_view;   // Needed for compatibility with STL container constructors.
        using reference = string_view; // Needed for compatibility with STL container constructors.

        iterator(string_view haystack, matcher matcher) noexcept : matcher_(matcher), remaining_(haystack) {
            auto position = matcher_(remaining_);
            length_within_remaining_ = position != string_view::npos
                                           ? remaining_.size() - position - matcher_.needle_length()
                                           : remaining_.size();
            reached_tail_ = false;
        }

        iterator(string_view haystack, matcher matcher, end_sentinel_type) noexcept
            : matcher_(matcher), remaining_(haystack), reached_tail_(true) {}

        value_type operator*() const noexcept {
            return remaining_.substr(remaining_.size() - length_within_remaining_);
        }

        iterator &operator++() noexcept {
            remaining_.remove_suffix(length_within_remaining_);
            reached_tail_ = remaining_.empty();
            remaining_.remove_suffix(matcher_.needle_length() * !reached_tail_);
            auto position = matcher_(remaining_);
            length_within_remaining_ = position != string_view::npos
                                           ? remaining_.size() - position - matcher_.needle_length()
                                           : remaining_.size();
            return *this;
        }

        iterator operator++(int) noexcept {
            iterator temp = *this;
            ++(*this);
            return temp;
        }

        bool operator!=(iterator const &other) const noexcept {
            return (remaining_.end() != other.remaining_.end()) || (reached_tail_ != other.reached_tail_);
        }
        bool operator==(iterator const &other) const noexcept {
            return (remaining_.end() == other.remaining_.end()) && (reached_tail_ == other.reached_tail_);
        }
        bool operator!=(end_sentinel_type) const noexcept { return !remaining_.empty() || !reached_tail_; }
        bool operator==(end_sentinel_type) const noexcept { return remaining_.empty() && reached_tail_; }
    };

    iterator begin() const noexcept { return iterator(haystack_, matcher_); }
    iterator end() const noexcept { return iterator({haystack_.begin(), 0}, matcher_, end_sentinel); }
    typename iterator::difference_type size() const noexcept { return std::distance(begin(), end()); }
    constexpr bool empty() const noexcept { return false; }

    /**
     *  @brief  Copies the matches into a container.
     */
    template <typename container_>
    void to(container_ &container) {
        for (auto match : *this) { container.push_back(match); }
    }

    /**
     *  @brief  Copies the matches into a consumed container, returning it at the end.
     */
    template <typename container_>
    container_ to(container_ &&container = {}) {
        for (auto match : *this) { container.push_back(match); }
        return std::move(container);
    }
};

template <typename string>
range_matches<string, matcher_find> find_all(string h, string n, bool interleaving = true) noexcept {
    return {h, n};
}

template <typename string>
range_rmatches<string, matcher_rfind> rfind_all(string h, string n, bool interleaving = true) noexcept {
    return {h, n};
}

template <typename string>
range_matches<string, matcher_find_first_of> find_all_characters(string h, string n) noexcept {
    return {h, n};
}

template <typename string>
range_rmatches<string, matcher_find_last_of> rfind_all_characters(string h, string n) noexcept {
    return {h, n};
}

template <typename string>
range_matches<string, matcher_find_first_not_of> find_all_other_characters(string h, string n) noexcept {
    return {h, n};
}

template <typename string>
range_rmatches<string, matcher_find_last_not_of> rfind_all_other_characters(string h, string n) noexcept {
    return {h, n};
}

template <typename string>
range_splits<string, matcher_find> split(string h, string n, bool interleaving = true) noexcept {
    return {h, n};
}

template <typename string>
range_rmatches<string, matcher_rfind> rsplit(string h, string n, bool interleaving = true) noexcept {
    return {h, n};
}

template <typename string>
range_splits<string, matcher_find_first_of> split_characters(string h, string n) noexcept {
    return {h, n};
}

template <typename string>
range_rsplits<string, matcher_find_last_of> rsplit_characters(string h, string n) noexcept {
    return {h, n};
}

template <typename string>
range_splits<string, matcher_find_first_not_of> split_other_characters(string h, string n) noexcept {
    return {h, n};
}

template <typename string>
range_rsplits<string, matcher_find_last_not_of> rsplit_other_characters(string h, string n) noexcept {
    return {h, n};
}

/**
 *  @brief  A reverse iterator for mutable and immutable character buffers.
 *          Replaces `std::reverse_iterator` to avoid including `<iterator>`.
 */
template <typename value_type_>
class reversed_iterator_for {
  public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = value_type_;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type_ *;
    using reference = value_type_ &;

    reversed_iterator_for(pointer ptr) noexcept : ptr_(ptr) {}
    reference operator*() const noexcept { return *ptr_; }

    bool operator==(reversed_iterator_for const &other) const noexcept { return ptr_ == other.ptr_; }
    bool operator!=(reversed_iterator_for const &other) const noexcept { return ptr_ != other.ptr_; }
    reference operator[](difference_type n) const noexcept { return *(*this + n); }
    reversed_iterator_for operator+(difference_type n) const noexcept { return reversed_iterator_for(ptr_ - n); }
    reversed_iterator_for operator-(difference_type n) const noexcept { return reversed_iterator_for(ptr_ + n); }
    difference_type operator-(reversed_iterator_for const &other) const noexcept { return other.ptr_ - ptr_; }

    reversed_iterator_for &operator++() noexcept {
        --ptr_;
        return *this;
    }

    reversed_iterator_for operator++(int) const noexcept {
        reversed_iterator_for temp = *this;
        --ptr_;
        return temp;
    }

    reversed_iterator_for &operator--() const noexcept {
        ++ptr_;
        return *this;
    }

    reversed_iterator_for operator--(int) const noexcept {
        reversed_iterator_for temp = *this;
        ++ptr_;
        return temp;
    }

  private:
    value_type_ *ptr_;
};

/**
 *  @brief  A string view class implementing with the superset of C++23 functionality
 *          with much faster SIMD-accelerated substring search and approximate matching.
 *          Unlike STL, never raises exceptions. Constructors are `constexpr` enabling `_sz` literals.
 */
class string_view {
    sz_cptr_t start_;
    sz_size_t length_;

  public:
    // Member types
    using traits_type = std::char_traits<char>;
    using value_type = char;
    using pointer = char *;
    using const_pointer = char const *;
    using reference = char &;
    using const_reference = char const &;
    using const_iterator = char const *;
    using iterator = const_iterator;
    using const_reverse_iterator = reversed_iterator_for<char const>;
    using reverse_iterator = const_reverse_iterator;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    using partition_result = string_partition_result<string_view>;

    /** @brief  Special value for missing matches. */
    static constexpr size_type npos = size_type(-1);

    constexpr string_view() noexcept : start_(nullptr), length_(0) {}
    constexpr string_view(const_pointer c_string) noexcept
        : start_(c_string), length_(null_terminated_length(c_string)) {}
    constexpr string_view(const_pointer c_string, size_type length) noexcept : start_(c_string), length_(length) {}
    constexpr string_view(string_view const &other) noexcept : start_(other.start_), length_(other.length_) {}
    constexpr string_view &operator=(string_view const &other) noexcept { return assign(other); }
    string_view(std::nullptr_t) = delete;

#if SZ_INCLUDE_STL_CONVERSIONS
#if __cplusplus >= 202002L
#define sz_constexpr_if20 constexpr
#else
#define sz_constexpr_if20 inline
#endif

    sz_constexpr_if20 string_view(std::string const &other) noexcept : string_view(other.data(), other.size()) {}
    sz_constexpr_if20 string_view(std::string_view const &other) noexcept : string_view(other.data(), other.size()) {}
    sz_constexpr_if20 string_view &operator=(std::string const &other) noexcept {
        return assign({other.data(), other.size()});
    }
    sz_constexpr_if20 string_view &operator=(std::string_view const &other) noexcept {
        return assign({other.data(), other.size()});
    }

    inline operator std::string() const { return {data(), size()}; }
    inline operator std::string_view() const noexcept { return {data(), size()}; }
#endif

    inline const_iterator begin() const noexcept { return const_iterator(start_); }
    inline const_iterator end() const noexcept { return const_iterator(start_ + length_); }
    inline const_iterator cbegin() const noexcept { return const_iterator(start_); }
    inline const_iterator cend() const noexcept { return const_iterator(start_ + length_); }
    inline const_reverse_iterator rbegin() const noexcept;
    inline const_reverse_iterator rend() const noexcept;
    inline const_reverse_iterator crbegin() const noexcept;
    inline const_reverse_iterator crend() const noexcept;

    inline const_reference operator[](size_type pos) const noexcept { return start_[pos]; }
    inline const_reference at(size_type pos) const noexcept { return start_[pos]; }
    inline const_reference front() const noexcept { return start_[0]; }
    inline const_reference back() const noexcept { return start_[length_ - 1]; }
    inline const_pointer data() const noexcept { return start_; }

    inline size_type size() const noexcept { return length_; }
    inline size_type length() const noexcept { return length_; }
    inline size_type max_size() const noexcept { return sz_size_max; }
    inline bool empty() const noexcept { return length_ == 0; }

    /** @brief Removes the first `n` characters from the view. The behavior is undefined if `n > size()`. */
    inline void remove_prefix(size_type n) noexcept { assert(n <= size()), start_ += n, length_ -= n; }

    /** @brief Removes the last `n` characters from the view. The behavior is undefined if `n > size()`. */
    inline void remove_suffix(size_type n) noexcept { assert(n <= size()), length_ -= n; }

    /** @brief Exchanges the view with that of the `other`. */
    inline void swap(string_view &other) noexcept {
        std::swap(start_, other.start_), std::swap(length_, other.length_);
    }

    /** @brief  Added for STL compatibility. */
    inline string_view substr() const noexcept { return *this; }

    /** @brief  Equivalent of `remove_prefix(pos)`. The behavior is undefined if `pos > size()`. */
    inline string_view substr(size_type pos) const noexcept { return string_view(start_ + pos, length_ - pos); }

    /** @brief  Returns a sub-view [pos, pos + rlen), where `rlen` is the smaller of count and `size() - pos`.
     *          Equivalent to `substr(pos).substr(0, count)` or combining `remove_prefix` and `remove_suffix`.
     *          The behavior is undefined if `pos > size()`.  */
    inline string_view substr(size_type pos, size_type count) const noexcept {
        return string_view(start_ + pos, sz_min_of_two(count, length_ - pos));
    }

    /**
     *  @brief  Compares two strings lexicographically. If prefix matches, lengths are compared.
     *  @return 0 if equal, negative if `*this` is less than `other`, positive if `*this` is greater than `other`.
     */
    inline int compare(string_view other) const noexcept {
        return (int)sz_order(start_, length_, other.start_, other.length_);
    }

    /**
     *  @brief  Compares two strings lexicographically. If prefix matches, lengths are compared.
     *  @return 0 if equal, negative if `*this` is less than `other`, positive if `*this` is greater than `other`.
     */
    inline int compare(size_type pos1, size_type count1, string_view other) const noexcept {
        return substr(pos1, count1).compare(other);
    }

    /**
     *  @brief  Compares two strings lexicographically. If prefix matches, lengths are compared.
     *  @return 0 if equal, negative if `*this` is less than `other`, positive if `*this` is greater than `other`.
     */
    inline int compare(size_type pos1, size_type count1, string_view other, size_type pos2,
                       size_type count2) const noexcept {
        return substr(pos1, count1).compare(other.substr(pos2, count2));
    }

    /**
     *  @brief  Compares two strings lexicographically. If prefix matches, lengths are compared.
     *  @return 0 if equal, negative if `*this` is less than `other`, positive if `*this` is greater than `other`.
     */
    inline int compare(const_pointer other) const noexcept { return compare(string_view(other)); }

    /**
     *  @brief  Compares two strings lexicographically. If prefix matches, lengths are compared.
     *  @return 0 if equal, negative if `*this` is less than `other`, positive if `*this` is greater than `other`.
     */
    inline int compare(size_type pos1, size_type count1, const_pointer other) const noexcept {
        return substr(pos1, count1).compare(string_view(other));
    }

    /**
     *  @brief  Compares two strings lexicographically. If prefix matches, lengths are compared.
     *  @return 0 if equal, negative if `*this` is less than `other`, positive if `*this` is greater than `other`.
     */
    inline int compare(size_type pos1, size_type count1, const_pointer other, size_type count2) const noexcept {
        return substr(pos1, count1).compare(string_view(other, count2));
    }

    /** @brief  Checks if the string is equal to the other string. */
    inline bool operator==(string_view other) const noexcept {
        return length_ == other.length_ && sz_equal(start_, other.start_, other.length_) == sz_true_k;
    }

#if __cplusplus >= 201402L
#define sz_deprecate_compare [[deprecated("Use the three-way comparison operator (<=>) in C++20 and later")]]
#else
#define sz_deprecate_compare
#endif

    /** @brief  Checks if the string is not equal to the other string. */
    sz_deprecate_compare inline bool operator!=(string_view other) const noexcept {
        return length_ != other.length_ || sz_equal(start_, other.start_, other.length_) == sz_false_k;
    }

    /** @brief  Checks if the string is lexicographically smaller than the other string. */
    sz_deprecate_compare inline bool operator<(string_view other) const noexcept { return compare(other) == sz_less_k; }

    /** @brief  Checks if the string is lexicographically equal or smaller than the other string. */
    sz_deprecate_compare inline bool operator<=(string_view other) const noexcept {
        return compare(other) != sz_greater_k;
    }

    /** @brief  Checks if the string is lexicographically greater than the other string. */
    sz_deprecate_compare inline bool operator>(string_view other) const noexcept {
        return compare(other) == sz_greater_k;
    }

    /** @brief  Checks if the string is lexicographically equal or greater than the other string. */
    sz_deprecate_compare inline bool operator>=(string_view other) const noexcept {
        return compare(other) != sz_less_k;
    }

#if __cplusplus >= 202002L

    /** @brief  Checks if the string is not equal to the other string. */
    inline int operator<=>(string_view other) const noexcept { return compare(other); }
#endif

    /** @brief  Checks if the string starts with the other string. */
    inline bool starts_with(string_view other) const noexcept {
        return length_ >= other.length_ && sz_equal(start_, other.start_, other.length_) == sz_true_k;
    }

    /** @brief  Checks if the string starts with the other string. */
    inline bool starts_with(const_pointer other) const noexcept {
        auto other_length = null_terminated_length(other);
        return length_ >= other_length && sz_equal(start_, other, other_length) == sz_true_k;
    }

    /** @brief  Checks if the string starts with the other character. */
    inline bool starts_with(value_type other) const noexcept { return length_ && start_[0] == other; }

    /** @brief  Checks if the string ends with the other string. */
    inline bool ends_with(string_view other) const noexcept {
        return length_ >= other.length_ &&
               sz_equal(start_ + length_ - other.length_, other.start_, other.length_) == sz_true_k;
    }

    /** @brief  Checks if the string ends with the other string. */
    inline bool ends_with(const_pointer other) const noexcept {
        auto other_length = null_terminated_length(other);
        return length_ >= other_length && sz_equal(start_ + length_ - other_length, other, other_length) == sz_true_k;
    }

    /** @brief  Checks if the string ends with the other character. */
    inline bool ends_with(value_type other) const noexcept { return length_ && start_[length_ - 1] == other; }

    /** @brief  Find the first occurrence of a substring. */
    inline size_type find(string_view other) const noexcept {
        auto ptr = sz_find(start_, length_, other.start_, other.length_);
        return ptr ? ptr - start_ : npos;
    }

    /** @brief  Find the first occurrence of a substring. The behavior is undefined if `pos > size()`. */
    inline size_type find(string_view other, size_type pos) const noexcept { return substr(pos).find(other); }

    /** @brief  Find the first occurrence of a character. */
    inline size_type find(value_type character) const noexcept {
        auto ptr = sz_find_byte(start_, length_, &character);
        return ptr ? ptr - start_ : npos;
    }

    /** @brief  Find the first occurrence of a character. The behavior is undefined if `pos > size()`. */
    inline size_type find(value_type character, size_type pos) const noexcept { return substr(pos).find(character); }

    /** @brief  Find the first occurrence of a substring. The behavior is undefined if `pos > size()`. */
    inline size_type find(const_pointer other, size_type pos, size_type count) const noexcept {
        return substr(pos).find(string_view(other, count));
    }

    /** @brief  Find the first occurrence of a substring. The behavior is undefined if `pos > size()`. */
    inline size_type find(const_pointer other, size_type pos = 0) const noexcept {
        return substr(pos).find(string_view(other));
    }

    /** @brief  Find the first occurrence of a substring. */
    inline size_type rfind(string_view other) const noexcept {
        auto ptr = sz_find_last(start_, length_, other.start_, other.length_);
        return ptr ? ptr - start_ : npos;
    }

    /** @brief  Find the first occurrence of a substring. The behavior is undefined if `pos > size()`. */
    inline size_type rfind(string_view other, size_type pos) const noexcept { return substr(pos).rfind(other); }

    /** @brief  Find the first occurrence of a character. */
    inline size_type rfind(value_type character) const noexcept {
        auto ptr = sz_find_last_byte(start_, length_, &character);
        return ptr ? ptr - start_ : npos;
    }

    /** @brief  Find the first occurrence of a character. The behavior is undefined if `pos > size()`. */
    inline size_type rfind(value_type character, size_type pos) const noexcept { return substr(pos).rfind(character); }

    /** @brief  Find the first occurrence of a substring. The behavior is undefined if `pos > size()`. */
    inline size_type rfind(const_pointer other, size_type pos, size_type count) const noexcept {
        return substr(pos).rfind(string_view(other, count));
    }

    /** @brief  Find the first occurrence of a substring. The behavior is undefined if `pos > size()`. */
    inline size_type rfind(const_pointer other, size_type pos = 0) const noexcept {
        return substr(pos).rfind(string_view(other));
    }

    inline bool contains(string_view other) const noexcept { return find(other) != npos; }
    inline bool contains(value_type character) const noexcept { return find(character) != npos; }
    inline bool contains(const_pointer other) const noexcept { return find(other) != npos; }

    /** @brief  Find the first occurrence of a character from a set. */
    inline size_type find_first_of(string_view other) const noexcept { return find_first_of(other.as_set()); }

    /** @brief  Find the first occurrence of a character outside of the set. */
    inline size_type find_first_not_of(string_view other) const noexcept { return find_first_not_of(other.as_set()); }

    /** @brief  Find the last occurrence of a character from a set. */
    inline size_type find_last_of(string_view other) const noexcept { return find_last_of(other.as_set()); }

    /** @brief  Find the last occurrence of a character outside of the set. */
    inline size_type find_last_not_of(string_view other) const noexcept { return find_last_not_of(other.as_set()); }

    /** @brief  Find the first occurrence of a character from a set. */
    inline size_type find_first_of(character_set set) const noexcept {
        auto ptr = sz_find_from_set(start_, length_, &set.raw());
        return ptr ? ptr - start_ : npos;
    }

    /** @brief  Find the first occurrence of a character from a set. */
    inline size_type find(character_set set) const noexcept { return find_first_of(set); }

    /** @brief  Find the first occurrence of a character outside of the set. */
    inline size_type find_first_not_of(character_set set) const noexcept { return find_first_of(set.inverted()); }

    /** @brief  Find the last occurrence of a character from a set. */
    inline size_type find_last_of(character_set set) const noexcept {
        auto ptr = sz_find_last_from_set(start_, length_, &set.raw());
        return ptr ? ptr - start_ : npos;
    }

    /** @brief  Find the last occurrence of a character from a set. */
    inline size_type rfind(character_set set) const noexcept { return find_last_of(set); }

    /** @brief  Find the last occurrence of a character outside of the set. */
    inline size_type find_last_not_of(character_set set) const noexcept { return find_last_of(set.inverted()); }

    /** @brief  Python-like convinience function, dropping the matching prefix. */
    inline string_view remove_prefix(string_view other) const noexcept {
        return starts_with(other) ? string_view {start_ + other.length_, length_ - other.length_} : *this;
    }

    /** @brief  Python-like convinience function, dropping the matching suffix. */
    inline string_view remove_suffix(string_view other) const noexcept {
        return ends_with(other) ? string_view {start_, length_ - other.length_} : *this;
    }

    /** @brief  Python-like convinience function, dropping prefix formed of given characters. */
    inline string_view lstrip(character_set set) const noexcept {
        set = set.inverted();
        auto new_start = sz_find_from_set(start_, length_, &set.raw());
        return new_start ? string_view {new_start, length_ - static_cast<size_type>(new_start - start_)}
                         : string_view();
    }

    /** @brief  Python-like convinience function, dropping suffix formed of given characters. */
    inline string_view rstrip(character_set set) const noexcept {
        set = set.inverted();
        auto new_end = sz_find_last_from_set(start_, length_, &set.raw());
        return new_end ? string_view {start_, static_cast<size_type>(new_end - start_ + 1)} : string_view();
    }

    /** @brief  Python-like convinience function, dropping both the prefix & the suffix formed of given characters. */
    inline string_view strip(character_set set) const noexcept {
        set = set.inverted();
        auto new_start = sz_find_from_set(start_, length_, &set.raw());
        return new_start
                   ? string_view {new_start,
                                  static_cast<size_type>(
                                      sz_find_last_from_set(new_start, length_ - (new_start - start_), &set.raw()) -
                                      new_start + 1)}
                   : string_view();
    }

    /** @brief  Find all occurrences of a given string.
     *  @param  interleave  If true, interleaving offsets are returned as well. */
    inline range_matches<string_view, matcher_find> find_all(string_view, bool interleave = true) const noexcept;

    /** @brief  Find all occurrences of a given string in @b reverse order.
     *  @param  interleave  If true, interleaving offsets are returned as well. */
    inline range_rmatches<string_view, matcher_rfind> rfind_all(string_view, bool interleave = true) const noexcept;

    /** @brief  Find all occurrences of given characters. */
    inline range_matches<string_view, matcher_find_first_of> find_all(character_set) const noexcept;

    /** @brief  Find all occurrences of given characters in @b reverse order. */
    inline range_rmatches<string_view, matcher_find_last_of> rfind_all(character_set) const noexcept;

    /** @brief  Split the string into three parts, before the match, the match itself, and after it. */
    inline partition_result partition(string_view pattern) const noexcept { return split_(pattern, pattern.length()); }

    /** @brief  Split the string into three parts, before the match, the match itself, and after it. */
    inline partition_result partition(character_set pattern) const noexcept { return split_(pattern, 1); }

    /** @brief  Split the string into three parts, before the @b last match, the last match itself, and after it. */
    inline partition_result rpartition(string_view pattern) const noexcept { return split_(pattern, pattern.length()); }

    /** @brief  Split the string into three parts, before the @b last match, the last match itself, and after it. */
    inline partition_result rpartition(character_set pattern) const noexcept { return split_(pattern, 1); }

    /** @brief  Find all occurrences of a given string.
     *  @param  interleave  If true, interleaving offsets are returned as well. */
    inline range_splits<string_view, matcher_find> split(string_view) const noexcept;

    /** @brief  Find all occurrences of a given string in @b reverse order.
     *  @param  interleave  If true, interleaving offsets are returned as well. */
    inline range_rsplits<string_view, matcher_rfind> rsplit(string_view) const noexcept;

    /** @brief  Find all occurrences of given characters. */
    inline range_splits<string_view, matcher_find_first_of> split(character_set = whitespaces_set) const noexcept;

    /** @brief  Find all occurrences of given characters in @b reverse order. */
    inline range_rsplits<string_view, matcher_find_last_of> rsplit(character_set = whitespaces_set) const noexcept;

    inline size_type copy(pointer destination, size_type count, size_type pos = 0) const noexcept = delete;

    /** @brief  Hashes the string, equivalent to `std::hash<string_view>{}(str)`. */
    inline size_type hash() const noexcept { return static_cast<size_type>(sz_hash(start_, length_)); }

    inline bool contains_only(character_set set) const noexcept { return find_first_not_of(set) == npos; }
    inline bool isalpha() const noexcept { return !empty() && contains_only(ascii_letters_set); }
    inline bool isalnum() const noexcept { return !empty() && contains_only(ascii_letters_set | digits_set); }
    inline bool isascii() const noexcept { return empty() || contains_only(ascii_controls_set | ascii_printables_set); }
    inline bool isdigit() const noexcept { return !empty() && contains_only(digits_set); }
    inline bool islower() const noexcept { return !empty() && contains_only(ascii_lowercase_set); }
    inline bool isprintable() const noexcept { return empty() || contains_only(ascii_printables_set); }
    inline bool isspace() const noexcept { return !empty() && contains_only(whitespaces_set); }
    inline bool isupper() const noexcept { return !empty() && contains_only(ascii_uppercase_set); }
    inline range_splits<string_view, matcher_find_first_of> splitlines() const noexcept;

    inline character_set as_set() const noexcept {
        character_set set;
        for (auto c : *this) set.add(c);
        return set;
    }

  private:
    constexpr string_view &assign(string_view const &other) noexcept {
        start_ = other.start_;
        length_ = other.length_;
        return *this;
    }
    constexpr static size_type null_terminated_length(const_pointer s) noexcept {
        const_pointer p = s;
        while (*p) ++p;
        return p - s;
    }

    template <typename pattern_>
    partition_result split_(pattern_ &&pattern, std::size_t pattern_length) const noexcept {
        size_type pos = find(pattern);
        if (pos == npos) return {substr(), string_view(), string_view()};
        return {substr(0, pos), substr(pos, pattern_length), substr(pos + pattern_length)};
    }

    template <typename pattern_>
    partition_result rsplit_(pattern_ &&pattern, std::size_t pattern_length) const noexcept {
        size_type pos = rfind(pattern);
        if (pos == npos) return {substr(), string_view(), string_view()};
        return {substr(0, pos), substr(pos, pattern_length), substr(pos + pattern_length)};
    }
};

/**
 *  @brief  Memory-owning string class with a Small String Optimization.
 *
 *  @section Exceptions
 *
 *  Default constructor is `constexpr`. Move constructor and move assignment operator are `noexcept`.
 *  Copy constructor and copy assignment operator are not! They may throw `std::bad_alloc` if the memory
 *  allocation fails. Alternatively, if exceptions are disabled, they may call `std::terminate`.
 */
template <typename allocator_type_ = std::allocator<char>>
class basic_string {

    using calloc_type = sz_memory_allocator_t;

    sz_string_t string_;

    /**
     *  Stateful allocators and their support in C++ strings is extremely error-prone by design.
     *  Depending on traits like `propagate_on_container_copy_assignment` and `propagate_on_container_move_assignment`,
     *  its state will be copied from one string to another. It goes against the design of most string constructors,
     *  as they also receive allocator as the last argument!
     */
    static_assert(std::is_empty<allocator_type_>::value, "We currently only support stateless allocators");

    static void *call_allocate(sz_size_t n, void *allocator_state) noexcept {
        return reinterpret_cast<allocator_type_ *>(allocator_state)->allocate(n);
    }

    static void call_free(void *ptr, sz_size_t n, void *allocator_state) noexcept {
        return reinterpret_cast<allocator_type_ *>(allocator_state)->deallocate(reinterpret_cast<char *>(ptr), n);
    }

    template <typename allocator_callback>
    bool with_alloc(allocator_callback &&callback) const noexcept {
        allocator_type_ allocator;
        sz_memory_allocator_t alloc;
        alloc.allocate = &call_allocate;
        alloc.free = &call_free;
        alloc.handle = &allocator;
        return callback(alloc);
    }

    bool is_internal() const noexcept { return sz_string_is_on_stack(&string_); }

    void init(std::size_t length, char value) noexcept(false) {
        sz_ptr_t start;
        if (!with_alloc([&](calloc_type &alloc) { return (start = sz_string_init_length(&string_, length, &alloc)); }))
            throw std::bad_alloc();
        sz_fill(start, length, value);
    }

    void init(string_view other) noexcept(false) {
        sz_ptr_t start;
        if (!with_alloc(
                [&](calloc_type &alloc) { return (start = sz_string_init_length(&string_, other.size(), &alloc)); }))
            throw std::bad_alloc();
        sz_copy(start, other.data(), other.size());
    }

    void move(basic_string &other) noexcept {
        // We can't just assign the other string state, as its start address may be somewhere else on the stack.
        sz_ptr_t string_start;
        sz_size_t string_length;
        sz_size_t string_space;
        sz_bool_t string_is_external;
        sz_string_unpack(&other.string_, &string_start, &string_length, &string_space, &string_is_external);

        // Acquire the old string's value bitwise
        *(&string_) = *(&other.string_);
        // Reposition the string start pointer to the stack if it fits.
        // Ternary condition may be optimized to a branchless version.
        string_.internal.start = string_is_external ? string_.internal.start : &string_.internal.chars[0];
        sz_string_init(&other.string_); // Discard the other string.
    }

  public:
    // Member types
    using traits_type = std::char_traits<char>;
    using value_type = char;
    using pointer = char *;
    using const_pointer = char const *;
    using reference = char &;
    using const_reference = char const &;
    using const_iterator = char const *;
    using iterator = const_iterator;
    using const_reverse_iterator = reversed_iterator_for<char const>;
    using reverse_iterator = const_reverse_iterator;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    using allocator_type = allocator_type_;
    using partition_result = string_partition_result<string_view>;

    /** @brief  Special value for missing matches. */
    static constexpr size_type npos = size_type(-1);

    constexpr basic_string() noexcept {
        // ! Instead of relying on the `sz_string_init`, we have to reimplement it to support `constexpr`.
        string_.internal.start = &string_.internal.chars[0];
        string_.u64s[1] = 0;
        string_.u64s[2] = 0;
        string_.u64s[3] = 0;
    }

    ~basic_string() noexcept {
        with_alloc([&](calloc_type &alloc) {
            sz_string_free(&string_, &alloc);
            return true;
        });
    }

    basic_string(basic_string &&other) noexcept { move(other); }
    basic_string &operator=(basic_string &&other) noexcept {
        if (!is_internal()) {
            with_alloc([&](calloc_type &alloc) {
                sz_string_free(&string_, &alloc);
                return true;
            });
        }
        move(other);
        return *this;
    }

    basic_string(basic_string const &other) noexcept(false) { init(other); }
    basic_string &operator=(basic_string const &other) noexcept(false) { return assign(other); }
    basic_string(string_view view) noexcept(false) { init(view); }
    basic_string &operator=(string_view view) noexcept(false) { return assign(view); }

    basic_string(const_pointer c_string) noexcept(false) : basic_string(string_view(c_string)) {}
    basic_string(const_pointer c_string, size_type length) noexcept(false)
        : basic_string(string_view(c_string, length)) {}
    basic_string(std::nullptr_t) = delete;

    /** @brief  Construct a string by repeating a certain ::character ::count times. */
    basic_string(size_type count, value_type character) noexcept(false) { init(count, character); }

    basic_string(basic_string const &other, size_type pos) noexcept(false) { init(string_view(other).substr(pos)); }
    basic_string(basic_string const &other, size_type pos, size_type count) noexcept(false) {
        init(string_view(other).substr(pos, count));
    }

    basic_string(std::initializer_list<value_type> list) noexcept(false) {
        init(string_view(list.begin(), list.size()));
    }

    operator string_view() const noexcept { return view(); }
    string_view view() const noexcept {
        sz_ptr_t string_start;
        sz_size_t string_length;
        sz_string_range(&string_, &string_start, &string_length);
        return {string_start, string_length};
    }

#if SZ_INCLUDE_STL_CONVERSIONS

    basic_string(std::string const &other) noexcept(false) : basic_string(other.data(), other.size()) {}
    basic_string(std::string_view other) noexcept(false) : basic_string(other.data(), other.size()) {}
    basic_string &operator=(std::string const &other) noexcept(false) { return assign({other.data(), other.size()}); }
    basic_string &operator=(std::string_view other) noexcept(false) { return assign({other.data(), other.size()}); }

    // As we are need both `data()` and `size()`, going through `operator string_view()`
    // and `sz_string_unpack` is faster than separate invokations.
    operator std::string() const { return view(); }
    operator std::string_view() const noexcept { return view(); }
#endif

    inline const_iterator begin() const noexcept { return const_iterator(data()); }
    inline const_iterator cbegin() const noexcept { return const_iterator(data()); }

    // As we are need both `data()` and `size()`, going through `operator string_view()`
    // and `sz_string_unpack` is faster than separate invokations.
    inline const_iterator end() const noexcept { return view().end(); }
    inline const_iterator cend() const noexcept { return view().end(); }
    inline const_reverse_iterator rbegin() const noexcept { return view().rbegin(); }
    inline const_reverse_iterator rend() const noexcept { return view().rend(); }
    inline const_reverse_iterator crbegin() const noexcept { return view().crbegin(); }
    inline const_reverse_iterator crend() const noexcept { return view().crend(); }

    inline const_reference operator[](size_type pos) const noexcept { return string_.internal.start[pos]; }
    inline const_reference at(size_type pos) const noexcept { return string_.internal.start[pos]; }
    inline const_reference front() const noexcept { return string_.internal.start[0]; }
    inline const_reference back() const noexcept { return string_.internal.start[size() - 1]; }
    inline const_pointer data() const noexcept { return string_.internal.start; }
    inline const_pointer c_str() const noexcept { return string_.internal.start; }

    inline bool empty() const noexcept { return string_.external.length == 0; }
    inline size_type size() const noexcept { return view().size(); }

    inline size_type length() const noexcept { return size(); }
    inline size_type max_size() const noexcept { return sz_size_max; }

    basic_string &assign(string_view other) noexcept(false) {
        if (!try_assign(other)) throw std::bad_alloc();
        return *this;
    }

    basic_string &append(string_view other) noexcept(false) {
        if (!try_append(other)) throw std::bad_alloc();
        return *this;
    }

    void push_back(char c) noexcept(false) {
        if (!try_push_back(c)) throw std::bad_alloc();
    }

    void resize(size_type count, value_type character = '\0') noexcept(false) {
        if (!try_resize(count, character)) throw std::bad_alloc();
    }

    void clear() noexcept { sz_string_erase(&string_, 0, sz_size_max); }

    basic_string &erase(std::size_t pos = 0, std::size_t count = sz_size_max) noexcept {
        sz_string_erase(&string_, pos, count);
        return *this;
    }

    bool try_resize(size_type count, value_type character = '\0') noexcept;

    bool try_assign(string_view other) noexcept;

    bool try_push_back(char c) noexcept;

    bool try_append(const_pointer str, size_type length) noexcept;

    bool try_append(string_view str) noexcept { return try_append(str.data(), str.size()); }

    size_type edit_distance(string_view other, size_type bound = npos) const noexcept {
        size_type distance;
        with_alloc([&](calloc_type &alloc) {
            distance = sz_edit_distance(data(), size(), other.data(), other.size(), bound, &alloc);
            return true;
        });
        return distance;
    }

    /** @brief Exchanges the view with that of the `other`. */
    inline void swap(basic_string &other) noexcept { std::swap(string_, other.string_); }

    /** @brief  Added for STL compatibility. */
    inline basic_string substr() const noexcept(false) { return *this; }

    /** @brief  Equivalent of `remove_prefix(pos)`. The behavior is undefined if `pos > size()`. */
    inline basic_string substr(size_type pos) const noexcept(false) { return view().substr(pos); }

    /** @brief  Returns a sub-view [pos, pos + rlen), where `rlen` is the smaller of count and `size() - pos`.
     *          Equivalent to `substr(pos).substr(0, count)` or combining `remove_prefix` and `remove_suffix`.
     *          The behavior is undefined if `pos > size()`.  */
    inline basic_string substr(size_type pos, size_type count) const noexcept(false) {
        return view().substr(pos, count);
    }

    /**
     *  @brief  Compares two strings lexicographically. If prefix matches, lengths are compared.
     *  @return 0 if equal, negative if `*this` is less than `other`, positive if `*this` is greater than `other`.
     */
    inline int compare(basic_string const &other) const noexcept { return sz_string_order(&string_, &other.string_); }

    /**
     *  @brief  Compares two strings lexicographically. If prefix matches, lengths are compared.
     *  @return 0 if equal, negative if `*this` is less than `other`, positive if `*this` is greater than `other`.
     */
    inline int compare(string_view other) const noexcept { return view().compare(other); }

    /**
     *  @brief  Compares two strings lexicographically. If prefix matches, lengths are compared.
     *  @return 0 if equal, negative if `*this` is less than `other`, positive if `*this` is greater than `other`.
     */
    inline int compare(size_type pos1, size_type count1, string_view other) const noexcept {
        return view().compare(pos1, count1, other);
    }

    /**
     *  @brief  Compares two strings lexicographically. If prefix matches, lengths are compared.
     *  @return 0 if equal, negative if `*this` is less than `other`, positive if `*this` is greater than `other`.
     */
    inline int compare(size_type pos1, size_type count1, string_view other, size_type pos2,
                       size_type count2) const noexcept {
        return view().compare(pos1, count1, other, pos2, count2);
    }

    /**
     *  @brief  Compares two strings lexicographically. If prefix matches, lengths are compared.
     *  @return 0 if equal, negative if `*this` is less than `other`, positive if `*this` is greater than `other`.
     */
    inline int compare(const_pointer other) const noexcept { return view().compare(other); }

    /**
     *  @brief  Compares two strings lexicographically. If prefix matches, lengths are compared.
     *  @return 0 if equal, negative if `*this` is less than `other`, positive if `*this` is greater than `other`.
     */
    inline int compare(size_type pos1, size_type count1, const_pointer other) const noexcept {
        return view().compare(pos1, count1, other);
    }

    /**
     *  @brief  Compares two strings lexicographically. If prefix matches, lengths are compared.
     *  @return 0 if equal, negative if `*this` is less than `other`, positive if `*this` is greater than `other`.
     */
    inline int compare(size_type pos1, size_type count1, const_pointer other, size_type count2) const noexcept {
        return view().compare(pos1, count1, other, count2);
    }

    /** @brief  Checks if the string is equal to the other string. */
    inline bool operator==(string_view other) const noexcept { return view() == other; }
    inline bool operator==(const_pointer other) const noexcept { return view() == other; }

    /** @brief  Checks if the string is equal to the other string. */
    inline bool operator==(basic_string const &other) const noexcept {
        return sz_string_equal(&string_, &other.string_);
    }

#if __cplusplus >= 201402L
#define sz_deprecate_compare [[deprecated("Use the three-way comparison operator (<=>) in C++20 and later")]]
#else
#define sz_deprecate_compare
#endif

    /** @brief  Checks if the string is not equal to the other string. */
    sz_deprecate_compare inline bool operator!=(string_view other) const noexcept { return !(operator==(other)); }
    sz_deprecate_compare inline bool operator!=(const_pointer other) const noexcept { return !(operator==(other)); }

    /** @brief  Checks if the string is not equal to the other string. */
    sz_deprecate_compare inline bool operator!=(basic_string const &other) const noexcept {
        return !(operator==(other));
    }

    /** @brief  Checks if the string is lexicographically smaller than the other string. */
    sz_deprecate_compare inline bool operator<(string_view other) const noexcept { return compare(other) == sz_less_k; }

    /** @brief  Checks if the string is lexicographically equal or smaller than the other string. */
    sz_deprecate_compare inline bool operator<=(string_view other) const noexcept {
        return compare(other) != sz_greater_k;
    }

    /** @brief  Checks if the string is lexicographically greater than the other string. */
    sz_deprecate_compare inline bool operator>(string_view other) const noexcept {
        return compare(other) == sz_greater_k;
    }

    /** @brief  Checks if the string is lexicographically equal or greater than the other string. */
    sz_deprecate_compare inline bool operator>=(string_view other) const noexcept {
        return compare(other) != sz_less_k;
    }

    /** @brief  Checks if the string is lexicographically smaller than the other string. */
    sz_deprecate_compare inline bool operator<(basic_string const &other) const noexcept {
        return compare(other) == sz_less_k;
    }

    /** @brief  Checks if the string is lexicographically equal or smaller than the other string. */
    sz_deprecate_compare inline bool operator<=(basic_string const &other) const noexcept {
        return compare(other) != sz_greater_k;
    }

    /** @brief  Checks if the string is lexicographically greater than the other string. */
    sz_deprecate_compare inline bool operator>(basic_string const &other) const noexcept {
        return compare(other) == sz_greater_k;
    }

    /** @brief  Checks if the string is lexicographically equal or greater than the other string. */
    sz_deprecate_compare inline bool operator>=(basic_string const &other) const noexcept {
        return compare(other) != sz_less_k;
    }

#if __cplusplus >= 202002L

    /** @brief  Checks if the string is not equal to the other string. */
    inline int operator<=>(string_view other) const noexcept { return compare(other); }

    /** @brief  Checks if the string is not equal to the other string. */
    inline int operator<=>(basic_string const &other) const noexcept { return compare(other); }
#endif

    /** @brief  Checks if the string starts with the other string. */
    inline bool starts_with(string_view other) const noexcept { return view().starts_with(other); }

    /** @brief  Checks if the string starts with the other string. */
    inline bool starts_with(const_pointer other) const noexcept { return view().starts_with(other); }

    /** @brief  Checks if the string starts with the other character. */
    inline bool starts_with(value_type other) const noexcept { return empty() ? false : at(0) == other; }

    /** @brief  Checks if the string ends with the other string. */
    inline bool ends_with(string_view other) const noexcept { return view().ends_with(other); }

    /** @brief  Checks if the string ends with the other string. */
    inline bool ends_with(const_pointer other) const noexcept { return view().ends_with(other); }

    /** @brief  Checks if the string ends with the other character. */
    inline bool ends_with(value_type other) const noexcept { return view().ends_with(other); }

    /** @brief  Find the first occurrence of a substring. */
    inline size_type find(string_view other) const noexcept { return view().find(other); }

    /** @brief  Find the first occurrence of a substring. The behavior is undefined if `pos > size()`. */
    inline size_type find(string_view other, size_type pos) const noexcept { return view().find(other, pos); }

    /** @brief  Find the first occurrence of a character. */
    inline size_type find(value_type character) const noexcept { return view().find(character); }

    /** @brief  Find the first occurrence of a character. The behavior is undefined if `pos > size()`. */
    inline size_type find(value_type character, size_type pos) const noexcept { return view().find(character, pos); }

    /** @brief  Find the first occurrence of a substring. The behavior is undefined if `pos > size()`. */
    inline size_type find(const_pointer other, size_type pos, size_type count) const noexcept {
        return view().find(other, pos, count);
    }

    /** @brief  Find the first occurrence of a substring. The behavior is undefined if `pos > size()`. */
    inline size_type find(const_pointer other, size_type pos = 0) const noexcept { return view().find(other, pos); }

    /** @brief  Find the first occurrence of a substring. */
    inline size_type rfind(string_view other) const noexcept { return view().rfind(other); }

    /** @brief  Find the first occurrence of a substring. The behavior is undefined if `pos > size()`. */
    inline size_type rfind(string_view other, size_type pos) const noexcept { return view().rfind(other, pos); }

    /** @brief  Find the first occurrence of a character. */
    inline size_type rfind(value_type character) const noexcept { return view().rfind(character); }

    /** @brief  Find the first occurrence of a character. The behavior is undefined if `pos > size()`. */
    inline size_type rfind(value_type character, size_type pos) const noexcept { return view().rfind(character, pos); }

    /** @brief  Find the first occurrence of a substring. The behavior is undefined if `pos > size()`. */
    inline size_type rfind(const_pointer other, size_type pos, size_type count) const noexcept {
        return view().rfind(other, pos, count);
    }

    /** @brief  Find the first occurrence of a substring. The behavior is undefined if `pos > size()`. */
    inline size_type rfind(const_pointer other, size_type pos = 0) const noexcept { return view().rfind(other, pos); }

    inline bool contains(string_view other) const noexcept { return find(other) != npos; }
    inline bool contains(value_type character) const noexcept { return find(character) != npos; }
    inline bool contains(const_pointer other) const noexcept { return find(other) != npos; }

    /** @brief  Find the first occurrence of a character from a set. */
    inline size_type find_first_of(string_view other) const noexcept { return find_first_of(other.as_set()); }

    /** @brief  Find the first occurrence of a character outside of the set. */
    inline size_type find_first_not_of(string_view other) const noexcept { return find_first_not_of(other.as_set()); }

    /** @brief  Find the last occurrence of a character from a set. */
    inline size_type find_last_of(string_view other) const noexcept { return find_last_of(other.as_set()); }

    /** @brief  Find the last occurrence of a character outside of the set. */
    inline size_type find_last_not_of(string_view other) const noexcept { return find_last_not_of(other.as_set()); }

    /** @brief  Find the first occurrence of a character from a set. */
    inline size_type find_first_of(character_set set) const noexcept { return view().find_first_of(set); }

    /** @brief  Find the first occurrence of a character from a set. */
    inline size_type find(character_set set) const noexcept { return find_first_of(set); }

    /** @brief  Find the first occurrence of a character outside of the set. */
    inline size_type find_first_not_of(character_set set) const noexcept { return find_first_of(set.inverted()); }

    /** @brief  Find the last occurrence of a character from a set. */
    inline size_type find_last_of(character_set set) const noexcept { return view().find_last_of(set); }

    /** @brief  Find the last occurrence of a character from a set. */
    inline size_type rfind(character_set set) const noexcept { return find_last_of(set); }

    /** @brief  Find the last occurrence of a character outside of the set. */
    inline size_type find_last_not_of(character_set set) const noexcept { return find_last_of(set.inverted()); }

    /** @brief  Find all occurrences of a given string.
     *  @param  interleave  If true, interleaving offsets are returned as well. */
    inline range_matches<string_view, matcher_find> find_all(string_view other, bool interleave = true) const noexcept;

    /** @brief  Find all occurrences of a given string in @b reverse order.
     *  @param  interleave  If true, interleaving offsets are returned as well. */
    inline range_rmatches<string_view, matcher_rfind> rfind_all(string_view other,
                                                                bool interleave = true) const noexcept;

    /** @brief  Find all occurrences of given characters. */
    inline range_matches<string_view, matcher_find_first_of> find_all(character_set set) const noexcept;

    /** @brief  Find all occurrences of given characters in @b reverse order. */
    inline range_rmatches<string_view, matcher_find_last_of> rfind_all(character_set set) const noexcept;

    /** @brief  Split the string into three parts, before the match, the match itself, and after it. */
    inline partition_result partition(string_view pattern) const noexcept { return view().partition(pattern); }

    /** @brief  Split the string into three parts, before the match, the match itself, and after it. */
    inline partition_result partition(character_set pattern) const noexcept { return view().partition(pattern); }

    /** @brief  Split the string into three parts, before the @b last match, the last match itself, and after it. */
    inline partition_result rpartition(string_view pattern) const noexcept { return view().partition(pattern); }

    /** @brief  Split the string into three parts, before the @b last match, the last match itself, and after it. */
    inline partition_result rpartition(character_set pattern) const noexcept { return view().partition(pattern); }

    /** @brief  Find all occurrences of a given string.
     *  @param  interleave  If true, interleaving offsets are returned as well. */
    inline range_splits<string_view, matcher_find> split(string_view pattern, bool interleave = true) const noexcept;

    /** @brief  Find all occurrences of a given string in @b reverse order.
     *  @param  interleave  If true, interleaving offsets are returned as well. */
    inline range_rsplits<string_view, matcher_rfind> rsplit(string_view pattern, bool interleave = true) const noexcept;

    /** @brief  Find all occurrences of given characters. */
    inline range_splits<string_view, matcher_find_first_of> split(character_set = whitespaces_set) const noexcept;

    /** @brief  Find all occurrences of given characters in @b reverse order. */
    inline range_rsplits<string_view, matcher_find_last_of> rsplit(character_set = whitespaces_set) const noexcept;

    /** @brief  Hashes the string, equivalent to `std::hash<string_view>{}(str)`. */
    inline size_type hash() const noexcept { return view().hash(); }

    /**
     *  @brief  Overwrites the string with random characters from the given alphabet using the random generator.
     *
     *  @param  generator  A random generator function object that returns a random number in the range [0, 2^64).
     *  @param  alphabet   A string of characters to choose from.
     */
    template <typename generator_type>
    basic_string &randomize(generator_type &&generator, string_view alphabet = "abcdefghijklmnopqrstuvwxyz") noexcept {
        sz_ptr_t start;
        sz_size_t length;
        sz_string_range(&string_, &start, &length);
        sz_random_generator_t generator_callback = &random_generator<generator_type>;
        sz_generate(alphabet.data(), alphabet.size(), start, length, generator_callback, &generator);
        return *this;
    }

    /**
     *  @brief  Overwrites the string with random characters from the given alphabet
     *          using `std::rand` as the random generator.
     *
     *  @param  alphabet   A string of characters to choose from.
     */
    basic_string &randomize(string_view alphabet = "abcdefghijklmnopqrstuvwxyz") noexcept {
        return randomize(&std::rand, alphabet);
    }

    /**
     *  @brief  Generate a new random string of given length using `std::rand` as the random generator.
     *          May throw exceptions if the memory allocation fails.
     *
     *  @param  length     The length of the generated string.
     *  @param  alphabet   A string of characters to choose from.
     */
    static basic_string random(size_type length, string_view alphabet = "abcdefghijklmnopqrstuvwxyz") noexcept(false) {
        return basic_string(length, '\0').randomize(alphabet);
    }

    inline bool contains_only(character_set set) const noexcept { return find_first_not_of(set) == npos; }
    inline bool isalpha() const noexcept { return !empty() && contains_only(ascii_letters_set); }
    inline bool isalnum() const noexcept { return !empty() && contains_only(ascii_letters_set | digits_set); }
    inline bool isascii() const noexcept { return empty() || contains_only(ascii_controls_set | ascii_printables_set); }
    inline bool isdigit() const noexcept { return !empty() && contains_only(digits_set); }
    inline bool islower() const noexcept { return !empty() && contains_only(ascii_lowercase_set); }
    inline bool isprintable() const noexcept { return empty() || contains_only(ascii_printables_set); }
    inline bool isspace() const noexcept { return !empty() && contains_only(whitespaces_set); }
    inline bool isupper() const noexcept { return !empty() && contains_only(ascii_uppercase_set); }
    inline range_splits<string_view, matcher_find_first_of> splitlines() const noexcept;

  private:
    template <typename generator_type>
    static sz_u64_t random_generator(void *state) noexcept {
        generator_type &generator = *reinterpret_cast<generator_type *>(state);
        return generator();
    }
};

using string = basic_string<>;

static_assert(sizeof(string) == 4 * sizeof(void *), "String size must be 4 pointers.");

namespace literals {
constexpr string_view operator""_sz(char const *str, std::size_t length) noexcept { return {str, length}; }
} // namespace literals

template <>
struct matcher_find_first_of<string_view> {
    using size_type = typename string_view::size_type;
    character_set needles_set_;
    matcher_find_first_of() noexcept {}
    matcher_find_first_of(character_set set) noexcept : needles_set_(set) {}
    matcher_find_first_of(string_view needle) noexcept : needles_set_(needle.as_set()) {}
    constexpr size_type needle_length() const noexcept { return 1; }
    constexpr size_type skip_length() const noexcept { return 1; }
    size_type operator()(string_view haystack) const noexcept { return haystack.find_first_of(needles_set_); }
};

template <>
struct matcher_find_last_of<string_view> {
    using size_type = typename string_view::size_type;
    character_set needles_set_;
    matcher_find_last_of() noexcept {}
    matcher_find_last_of(character_set set) noexcept : needles_set_(set) {}
    matcher_find_last_of(string_view needle) noexcept : needles_set_(needle.as_set()) {}
    constexpr size_type needle_length() const noexcept { return 1; }
    constexpr size_type skip_length() const noexcept { return 1; }
    size_type operator()(string_view haystack) const noexcept { return haystack.find_last_of(needles_set_); }
};

template <>
struct matcher_find_first_not_of<string_view> {
    using size_type = typename string_view::size_type;
    character_set needles_set_;
    matcher_find_first_not_of() noexcept {}
    matcher_find_first_not_of(character_set set) noexcept : needles_set_(set) {}
    matcher_find_first_not_of(string_view needle) noexcept : needles_set_(needle.as_set()) {}
    constexpr size_type needle_length() const noexcept { return 1; }
    constexpr size_type skip_length() const noexcept { return 1; }
    size_type operator()(string_view haystack) const noexcept { return haystack.find_first_not_of(needles_set_); }
};

template <>
struct matcher_find_last_not_of<string_view> {
    using size_type = typename string_view::size_type;
    character_set needles_set_;
    matcher_find_last_not_of() noexcept {}
    matcher_find_last_not_of(character_set set) noexcept : needles_set_(set) {}
    matcher_find_last_not_of(string_view needle) noexcept : needles_set_(needle.as_set()) {}
    constexpr size_type needle_length() const noexcept { return 1; }
    constexpr size_type skip_length() const noexcept { return 1; }
    size_type operator()(string_view haystack) const noexcept { return haystack.find_last_not_of(needles_set_); }
};

inline range_matches<string_view, matcher_find> string_view::find_all(string_view n, bool i) const noexcept {
    return {*this, {n, i}};
}

inline range_rmatches<string_view, matcher_rfind> string_view::rfind_all(string_view n, bool i) const noexcept {
    return {*this, {n, i}};
}

inline range_matches<string_view, matcher_find_first_of> string_view::find_all(character_set set) const noexcept {
    return {*this, {set}};
}

inline range_rmatches<string_view, matcher_find_last_of> string_view::rfind_all(character_set set) const noexcept {
    return {*this, {set}};
}

inline range_splits<string_view, matcher_find> string_view::split(string_view n) const noexcept { return {*this, {n}}; }

inline range_rsplits<string_view, matcher_rfind> string_view::rsplit(string_view n) const noexcept {
    return {*this, {n}};
}

inline range_splits<string_view, matcher_find_first_of> string_view::split(character_set set) const noexcept {
    return {*this, {set}};
}

inline range_rsplits<string_view, matcher_find_last_of> string_view::rsplit(character_set set) const noexcept {
    return {*this, {set}};
}

inline range_splits<string_view, matcher_find_first_of> string_view::splitlines() const noexcept {
    return split(newlines_set);
}

template <typename allocator_>
inline range_splits<string_view, matcher_find_first_of> basic_string<allocator_>::splitlines() const noexcept {
    return split(newlines_set);
}

template <typename allocator_>
inline range_matches<string_view, matcher_find> basic_string<allocator_>::find_all(string_view other,
                                                                                   bool interleave) const noexcept {
    return view().find_all(other, interleave);
}

template <typename allocator_>
inline range_rmatches<string_view, matcher_rfind> basic_string<allocator_>::rfind_all(string_view other,
                                                                                      bool interleave) const noexcept {
    return view().rfind_all(other, interleave);
}

template <typename allocator_>
inline range_matches<string_view, matcher_find_first_of> basic_string<allocator_>::find_all(
    character_set set) const noexcept {
    return view().find_all(set);
}

template <typename allocator_>
inline range_rmatches<string_view, matcher_find_last_of> basic_string<allocator_>::rfind_all(
    character_set set) const noexcept {
    return view().rfind_all(set);
}

template <typename allocator_>
inline range_splits<string_view, matcher_find> basic_string<allocator_>::split(string_view pattern,
                                                                               bool interleave) const noexcept {
    return view().split(pattern, interleave);
}

template <typename allocator_>
inline range_rsplits<string_view, matcher_rfind> basic_string<allocator_>::rsplit(string_view pattern,
                                                                                  bool interleave) const noexcept {
    return view().rsplit(pattern, interleave);
}

template <typename allocator_>
inline range_splits<string_view, matcher_find_first_of> basic_string<allocator_>::split(
    character_set set) const noexcept {
    return view().split(set);
}

template <typename allocator_>
inline range_rsplits<string_view, matcher_find_last_of> basic_string<allocator_>::rsplit(
    character_set set) const noexcept {
    return view().rsplit(set);
}

template <typename allocator_>
bool basic_string<allocator_>::try_resize(size_type count, value_type character) noexcept {
    sz_ptr_t string_start;
    sz_size_t string_length;
    sz_size_t string_space;
    sz_bool_t string_is_external;
    sz_string_unpack(&string_, &string_start, &string_length, &string_space, &string_is_external);

    // Allocate more space if needed.
    if (count >= string_space) {
        if (!with_alloc(
                [&](calloc_type &alloc) { return sz_string_expand(&string_, sz_size_max, count, &alloc) != NULL; }))
            return false;
        sz_string_unpack(&string_, &string_start, &string_length, &string_space, &string_is_external);
    }

    // Fill the trailing characters.
    if (count > string_length) {
        sz_fill(string_start + string_length, count - string_length, character);
        string_start[count] = '\0';
        // Knowing the layout of the string, we can perform this operation safely,
        // even if its located on stack.
        string_.external.length += count - string_length;
    }
    else { sz_string_erase(&string_, count, sz_size_max); }
    return true;
}

template <typename allocator_>
bool basic_string<allocator_>::try_assign(string_view other) noexcept {
    // We can't just assign the other string state, as its start address may be somewhere else on the stack.
    sz_ptr_t string_start;
    sz_size_t string_length;
    sz_string_range(&string_, &string_start, &string_length);

    if (string_length >= other.length()) {
        sz_string_erase(&string_, other.length(), sz_size_max);
        sz_copy(string_start, other.data(), other.length());
    }
    else {
        if (!with_alloc([&](calloc_type &alloc) {
                string_start = sz_string_expand(&string_, sz_size_max, other.length(), &alloc);
                if (!string_start) return false;
                sz_copy(string_start, other.data(), other.length());
                return true;
            }))
            return false;
    }
    return true;
}

template <typename allocator_>
bool basic_string<allocator_>::try_push_back(char c) noexcept {
    return with_alloc([&](calloc_type &alloc) {
        sz_ptr_t start = sz_string_expand(&string_, sz_size_max, 1, &alloc);
        if (!start) return false;
        start[size() - 1] = c;
        return true;
    });
}

template <typename allocator_>
bool basic_string<allocator_>::try_append(const_pointer str, size_type length) noexcept {
    return with_alloc([&](calloc_type &alloc) {
        sz_ptr_t start = sz_string_expand(&string_, sz_size_max, 1, &alloc);
        if (!start) return false;
        sz_copy(start + size() - 1, str, length);
        return true;
    });
}

} // namespace stringzilla
} // namespace ashvardanian

#pragma region STL Specializations

namespace std {

template <>
struct hash<ashvardanian::stringzilla::string_view> {
    size_t operator()(ashvardanian::stringzilla::string_view str) const noexcept { return str.hash(); }
};

template <>
struct hash<ashvardanian::stringzilla::string> {
    size_t operator()(ashvardanian::stringzilla::string const &str) const noexcept { return str.hash(); }
};

} // namespace std

#pragma endregion

#endif // STRINGZILLA_HPP_
