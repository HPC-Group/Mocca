#include "gtest/gtest.h"

#include "mocca/base/ByteArray.h"
#include "mocca/base/Error.h"

using namespace mocca;

class ByteArrayTest : public ::testing::Test {
protected:
    ByteArrayTest() {
        // You can do set-up work for each test here.
    }

    virtual ~ByteArrayTest() {
        // You can do clean-up work that doesn't throw exceptions here.
    }
};

TEST_F(ByteArrayTest, CreateFixedSize) {
    ByteArray target(sizeof(int32_t));
    int32_t value = 42;
    memcpy(target.data(), &value, sizeof(int32_t));
    ASSERT_TRUE(memcmp(target.data(), &value, sizeof(int32_t)) == 0);
}

TEST_F(ByteArrayTest, SetSize) {
    ByteArray target(10);
    ASSERT_EQ(0, target.size());
    target.setSize(10);
    ASSERT_EQ(10, target.size());
}

TEST_F(ByteArrayTest, SetSizeExceedingCapacity) {
    ByteArray target(10);
    ASSERT_THROW(target.setSize(11), Error);
}

TEST_F(ByteArrayTest, Capacity) {
    ByteArray target(42);
    ASSERT_EQ(target.capacity(), 42);
}

TEST_F(ByteArrayTest, CreateFromRaw) {
    const char* str = "123456789";
    ByteArray target = ByteArray::createFromRaw(str, 9);
    ASSERT_EQ(9, target.size());
    ASSERT_TRUE(memcmp(target.data(), str, 9) == 0);
}

TEST_F(ByteArrayTest, AppendDoesNotExceedCapacity) {
    ByteArray target(10);
    memset(target.data(), 0, 10);
    memset(target.data(), 1, 3);
    target.setSize(3);

    ByteArray other(10);
    memset(other.data(), 0, 10);
    memset(other.data(), 2, 4);
    other.setSize(4);

    target.append(other);

    ASSERT_EQ(10, target.capacity());

    char expected[10] = {0x1, 0x1, 0x1, 0x2, 0x2, 0x2, 0x2, 0x0, 0x0, 0x0};
    ASSERT_TRUE(memcmp(expected, target.data(), 10) == 0);
    ASSERT_EQ(7, target.size());
}

TEST_F(ByteArrayTest, AppendExceedsCapacity) {
    ByteArray target(5);
    memset(target.data(), 0, 5);
    memset(target.data(), 1, 3);
    target.setSize(3);

    ByteArray other(5);
    memset(other.data(), 0, 5);
    memset(other.data(), 2, 4);
    other.setSize(4);

    target.append(other);

    ASSERT_GT(target.capacity(), size_t{5});

    char expected[10] = {0x1, 0x1, 0x1, 0x2, 0x2, 0x2, 0x2};
    ASSERT_TRUE(memcmp(expected, target.data(), 7) == 0);
    ASSERT_EQ(7, target.size());
}

TEST_F(ByteArrayTest, AppendReservesEnoughSpace) {
    ByteArray target(5);
    ByteArray other(2048);
    other.setSize(2048);
    target.append(other);
    ASSERT_TRUE(target.capacity() >= 2048+5);
}

TEST_F(ByteArrayTest, Move) {
    ByteArray target(1);
    memset(target.data(), 1, 1);
    target.setSize(1);
    auto targetData = target.data();
    ByteArray moved(std::move(target));
    memset(moved.data(), 2, 1);
    ASSERT_EQ(nullptr, target.data());
    ASSERT_EQ(1, moved.size());
    ASSERT_EQ(targetData, moved.data());
}

TEST_F(ByteArrayTest, Int) {
    {
        ByteArray target;
        int32_t val = 42;
        target << val;
        int32_t result = 0;
        target >> result;
        ASSERT_EQ(val, result);
    }
    {
        ByteArray target;
        int64_t val = (1ll << 33);
        target << val;
        int64_t result = 0;
        target >> result;
        ASSERT_EQ(val, result);
    }
}

TEST_F(ByteArrayTest, UInt) {
    ByteArray target;
    uint32_t val = (1 << 31);
    target << val;
    uint32_t result = 0;
    target >> result;
    double promoted(result);
    ASSERT_FALSE(promoted < val);
    ASSERT_FALSE(promoted > val);
}

TEST_F(ByteArrayTest, Float) {
    ByteArray target;
    target << 42.0f;
    float result = 0.0f;
    target >> result;
    ASSERT_EQ(42.0f, result);
}

TEST_F(ByteArrayTest, Double) {
    ByteArray target;
    target << 42.0;
    double result = 0.0;
    target >> result;
    ASSERT_EQ(42.0, result);
}

TEST_F(ByteArrayTest, Bool) {
    {
        ByteArray target;
        target << true;
        bool result = false;
        target >> result;
        ASSERT_TRUE(result);
    }
    {
        ByteArray target;
        target << false;
        bool result = true;
        target >> result;
        ASSERT_FALSE(result);
    }
}

TEST_F(ByteArrayTest, String) {
    ByteArray target;
    std::string str("Hello World");
    target << str;
    std::string result = target.read(str.size());
    ASSERT_EQ(str, result);
}

TEST_F(ByteArrayTest, NestedByteArray) {
    ByteArray outer;
    outer << 17;
    outer << 18;
    ByteArray inner;
    inner << 41;
    inner << 42;
    outer << inner;
    outer << 19;
    ASSERT_EQ(17, outer.read<int32_t>());
    ASSERT_EQ(18, outer.read<int32_t>());
    ByteArray innerResult;
    outer >> innerResult;
    ASSERT_EQ(41, innerResult.read<int32_t>());
    ASSERT_EQ(42, innerResult.read<int32_t>());
    ASSERT_EQ(19, outer.read<int32_t>());
}

TEST_F(ByteArrayTest, MixedTypes) {
    ByteArray target;
    target << 42 << 17.0f << true;
    int32_t i = 0;
    float f = 0.0f;
    bool b = false;
    target >> i >> f >> b;
    ASSERT_EQ(42, i);
    ASSERT_EQ(17.0f, f);
    ASSERT_EQ(true, b);
}

TEST_F(ByteArrayTest, Get) {
    ByteArray target;
    target << 42 << 17.0f << true;
    ASSERT_EQ(42, target.read<int32_t>());
    ASSERT_EQ(17.0f, target.read<float>());
    ASSERT_EQ(true, target.read<bool>());
}

TEST_F(ByteArrayTest, ReadOutOfBounds) {
    {
        ByteArray target;
        target << 42;
        double d;
        ASSERT_THROW(target >> d, Error);
    }
    {
        ByteArray target;
        target << 42;
        int32_t i1, i2;
        target >> i1;
        ASSERT_THROW(target >> i2, Error);
    }
    {
        ByteArray target;
        target << 42 << 17;
        double d;
        int32_t i;
        target >> d;
        ASSERT_THROW(target >> i, Error);
    }
    {
        ByteArray outer;
        ByteArray inner;
        inner << 42 << 17;
        outer << 23 << inner << 7;
        ASSERT_NO_THROW(outer.read<int32_t>());
        ASSERT_NO_THROW(outer.read<ByteArray>());
        ASSERT_NO_THROW(outer.read<int32_t>());
        ASSERT_THROW(outer.read<int32_t>(), Error);
    }
}

TEST_F(ByteArrayTest, SubscriptOperator) {
    ByteArray target;
    std::string data = "abc";
    target.append(data.c_str(), data.size());
    ASSERT_EQ('a', target[0]);
    ASSERT_EQ('b', target[1]);
    ASSERT_EQ('c', target[2]);
#ifdef MOCCA_BYTEARRAY_CHECKS
    ASSERT_THROW(target[-1], Error);
    ASSERT_THROW(target[3], Error);
#endif
}

TEST_F(ByteArrayTest, BuildByteArray) {
    ByteArray target = makeFormattedByteArray(23, 17.0f, "blubb", false, 20.0, std::string("blubb2"));
    auto x = parseFormattedByteArray<int, float, std::string, bool, double, std::string>(target);
    ASSERT_EQ(23, std::get<0>(x));
    ASSERT_EQ(17.0f, std::get<1>(x));
    ASSERT_EQ("blubb", std::get<2>(x));
    ASSERT_EQ(false, std::get<3>(x));
    ASSERT_EQ(20.0, std::get<4>(x));
    ASSERT_EQ("blubb2", std::get<5>(x));
}