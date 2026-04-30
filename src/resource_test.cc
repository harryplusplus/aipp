#include "src/resource.h"

#include <gtest/gtest.h>

#include <memory>
#include <utility>

template <typename T, typename D, typename = void>
struct ResourceValid : std::false_type {};
template <typename T, typename D>
struct ResourceValid<T, D, std::void_t<decltype(sizeof(Resource<T, D>))>>
    : std::true_type {};

static_assert(ResourceDeleter<void (*)(int), int>);
static_assert(ResourceDeleter<void (*)(int&&), int>);
static_assert(ResourceDeleter<void (*)(const int&), int>);

static_assert(!ResourceDeleter<void (*)(int&), int>);
static_assert(!ResourceDeleter<int, int>);

static_assert(ResourceValid<int, void (*)(int)>::value);
static_assert(ResourceValid<int, void (*)(int&&)>::value);
static_assert(ResourceValid<int, void (*)(const int&)>::value);

static_assert(ResourceValid<int*, void (*)(int*)>::value);
static_assert(ResourceValid<int*, void (*)(int*&&)>::value);

static_assert(
    ResourceValid<std::unique_ptr<int>, void (*)(std::unique_ptr<int>)>::value);
static_assert(ResourceValid<std::unique_ptr<int>,
                            void (*)(std::unique_ptr<int>&&)>::value);
static_assert(ResourceValid<std::unique_ptr<int>,
                            void (*)(const std::unique_ptr<int>&)>::value);

static_assert(!ResourceValid<int, void (*)(int&)>::value);
static_assert(!ResourceValid<int, int>::value);

struct NotInvocable {};
static_assert(!ResourceValid<int, NotInvocable>::value);

namespace {

struct MoveOnlyInt {  // NOLINT: test helper, no resource ownership
  int value{};
  MoveOnlyInt() = default;
  explicit MoveOnlyInt(int v) : value(v) {}
  MoveOnlyInt(const MoveOnlyInt&) = delete;
  MoveOnlyInt& operator=(const MoveOnlyInt&) = delete;
  MoveOnlyInt(MoveOnlyInt&&) = default;
  MoveOnlyInt& operator=(MoveOnlyInt&&) = default;
  explicit operator bool() const { return value != 0; }
};

struct DeleterLog {
  int* count;
  void operator()(MoveOnlyInt&& obj) const {
    obj.value = 0;
    *count += 1;
  }
};

struct DeleterTakesUniquePtr {
  void operator()(std::unique_ptr<int>&& p) { *p = 42; }
};

struct FileDesc {
  int fd;
  explicit FileDesc(int f) : fd(f) {}
};

static_assert(ResourceValid<MoveOnlyInt, void (*)(MoveOnlyInt)>::value);
static_assert(ResourceValid<MoveOnlyInt, void (*)(MoveOnlyInt&&)>::value);
static_assert(ResourceValid<MoveOnlyInt, void (*)(const MoveOnlyInt&)>::value);

static_assert(!ResourceValid<MoveOnlyInt, void (*)(MoveOnlyInt&)>::value);

}  // namespace

TEST(ResourceTest, MoveConstruct) {
  int delete_count = 0;
  {
    Resource r(MoveOnlyInt(7), DeleterLog{&delete_count});
    EXPECT_TRUE(r);
    EXPECT_EQ(r.Get().value, 7);
  }
  EXPECT_EQ(delete_count, 1);
}

TEST(ResourceTest, MoveAssign) {
  int delete_count1 = 0, delete_count2 = 0;
  {
    Resource r1(MoveOnlyInt(1), DeleterLog{&delete_count1});
    Resource r2(MoveOnlyInt(2), DeleterLog{&delete_count2});
    r1 = std::move(r2);
    EXPECT_EQ(delete_count1, 1);
    EXPECT_EQ(delete_count2, 0);
  }
  EXPECT_EQ(delete_count2, 1);
}

TEST(ResourceTest, MoveOnlyType) {
  Resource<std::unique_ptr<int>, DeleterTakesUniquePtr> r(
      std::make_unique<int>(10), DeleterTakesUniquePtr{});
  EXPECT_TRUE(r);
  EXPECT_EQ(*r.Get(), 10);
}

TEST(ResourceTest, BoolConversion) {
  int delete_count = 0;
  Resource r(MoveOnlyInt(5), DeleterLog{&delete_count});
  EXPECT_TRUE(r.HasValue());

  auto empty = std::move(r);
  EXPECT_FALSE(r.HasValue());  // NOLINT: intentional — check moved-from state
}

TEST(ResourceTest, EmptyStateAfterMove) {
  int del1 = 0, del2 = 0;
  {
    Resource<MoveOnlyInt, DeleterLog> r1(MoveOnlyInt(1), DeleterLog{&del1});
    Resource<MoveOnlyInt, DeleterLog> r2(MoveOnlyInt(2), DeleterLog{&del2});
    r2 = std::move(r1);
    EXPECT_FALSE(
        r1.HasValue());  // NOLINT: intentional — check moved-from state
  }
  EXPECT_EQ(del1, 1);
  EXPECT_EQ(del2, 1);
}

TEST(ResourceTest, NonDefaultConstructible) {
  bool deleted = false;
  {
    auto del = [&](FileDesc&& /*f*/) { deleted = true; };
    Resource r(FileDesc{42}, std::move(del));
    EXPECT_TRUE(r);
    EXPECT_EQ(r.Get().fd, 42);
  }
  EXPECT_TRUE(deleted);
}
