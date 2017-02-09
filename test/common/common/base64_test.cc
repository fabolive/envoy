#include "common/buffer/buffer_impl.h"

#include "common/common/base64.h"

TEST(Base64Test, EmptyBufferEncode) {
  {
    Buffer::OwnedImpl buffer;
    EXPECT_EQ("", Base64::encode(buffer, 0));
  }

  {
    Buffer::OwnedImpl buffer;
    buffer.add("\0\0", 2);
    EXPECT_EQ("AAA=", Base64::encode(buffer, 2));
  }
}

TEST(Base64Test, SingleSliceBufferEncode) {
  Buffer::OwnedImpl buffer;
  buffer.add("foo", 3);
  EXPECT_EQ("Zm9v", Base64::encode(buffer, 3));
  EXPECT_EQ("Zm8=", Base64::encode(buffer, 2));
}

TEST(Base64Test, EncodeString) {
  EXPECT_EQ("", Base64::encode(""));

  std::string two_zero_bytes("\0\0", 2);
  EXPECT_EQ("AAA=", Base64::encode(two_zero_bytes));

  EXPECT_EQ("Zm9v", Base64::encode("foo"));
  EXPECT_EQ("Zm8=", Base64::encode("fo"));
}

TEST(Base64Test, Decode) {
  EXPECT_EQ("", Base64::decode(""));
  EXPECT_EQ("foo", Base64::decode("Zm9v"));
  EXPECT_EQ("fo", Base64::decode("Zm8="));
  EXPECT_EQ("foobar", Base64::decode("Zm9vYmFy"));
  EXPECT_EQ("foob", Base64::decode("Zm9vYg=="));
  EXPECT_EQ("", Base64::decode("123"));

  {
    const char* test_string = "\0\1\2\3\b\n\t";
    EXPECT_FALSE(memcmp(test_string, Base64::decode("AAECAwgKCQ==").data(), 7));
  }

  {
    const char* test_string = "\0\0\0\0als;jkopqitu[\0opbjlcxnb35g]b[\xaa\b\n";
    Buffer::OwnedImpl buffer;
    buffer.add(test_string, 36);
    EXPECT_FALSE(memcmp(test_string, Base64::decode(Base64::encode(buffer, 36)).data(), 36));
  }

  {
    std::string test_string("\0\0\0\0als;jkopqitu[\0opbjlcxnb35g]b[\xaa\b\n", 36);
    EXPECT_FALSE(
        memcmp(test_string.data(), Base64::decode(Base64::encode(test_string)).data(), 36));
  }

  {
    std::string test_string = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string decoded = Base64::decode(test_string);
    Buffer::OwnedImpl buffer(decoded);
    EXPECT_EQ(test_string, Base64::encode(buffer, decoded.length()));
  }

  {
    std::string test_string = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string decoded = Base64::decode(test_string);
    EXPECT_EQ(test_string, Base64::encode(decoded));
  }
}

TEST(Base64Test, MultiSlicesBufferEncode) {
  Buffer::OwnedImpl buffer;
  buffer.add("foob", 4);
  buffer.add("ar", 2);
  EXPECT_EQ("Zm9vYg==", Base64::encode(buffer, 4));
  EXPECT_EQ("Zm9vYmE=", Base64::encode(buffer, 5));
  EXPECT_EQ("Zm9vYmFy", Base64::encode(buffer, 6));
  EXPECT_EQ("Zm9vYmFy", Base64::encode(buffer, 7));
}

TEST(Base64Test, BinaryBufferEncode) {
  Buffer::OwnedImpl buffer;
  buffer.add("\0\1\2\3", 4);
  buffer.add("\b\n\t", 4);
  buffer.add("\xaa\xbc\xde", 3);
  EXPECT_EQ("AAECAwgKCQ==", Base64::encode(buffer, 7));
  EXPECT_EQ("AAECAwgKCQA=", Base64::encode(buffer, 8));
  EXPECT_EQ("AAECAwgKCQCq", Base64::encode(buffer, 9));
  EXPECT_EQ("AAECAwgKCQCqvA==", Base64::encode(buffer, 10));
  EXPECT_EQ("AAECAwgKCQCqvN4=", Base64::encode(buffer, 30));
}
