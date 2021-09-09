// Copyright (c) 2020 Oxford-Hainan Blockchain Research Institute
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "abi/abicoder.h"

#include "abi/types/array.h"
#include "abi/types/type.h"
#include "app/utils.h"
#include "doctest/doctest.h"
#include "eEVM/rlp.h"
#include "fmt/core.h"
#include "jsonrpc.h"
#include "string"

#include <eEVM/util.h>
#include <vector>

using namespace std;
using namespace eevm;

namespace abicoder {

eevm::Address to_address(const vector<uint8_t>& inputs) { return eevm::from_big_endian(inputs.data()); }

template <typename T>
void test_basic(Type* pd, const T&& correct) {
    CHECK(pd->encode() == correct);
}

TEST_CASE("Test Address") {
    string src = "0xde0B295669a9FD93d5F28D9Ec85E40f4cb697BAe";
    auto correct = to_bytes(src, 12u);
    auto addr = Address(src);
    test_basic(&addr, move(correct));

    auto de_addr = Address();
    de_addr.decode(correct);
    CHECK(eevm::to_checksum_address(to_address(de_addr.encode())) == src);
}

TEST_CASE("Test Bool") {
    SUBCASE("when paramters value is true") {
        string src = "0x1";
        auto correct = to_bytes(src, 31u);
        auto boolean = Boolean(true);
        test_basic(&boolean, move(correct));

        auto boolean1 = Boolean(src);
        test_basic(&boolean1, move(correct));

        auto de_boolean = Boolean();
        de_boolean.decode(correct);
        // CHECK(de_boolean.get_value());
    }

    SUBCASE("When paramters value is false") {
        string src = "0x0";
        auto correct = to_bytes(src, 31u);
        auto boolean = Boolean(false);
        test_basic(&boolean, move(correct));

        auto boolean1 = Boolean(src);
        test_basic(&boolean1, move(correct));

        auto de_boolean = Boolean();
        de_boolean.decode(correct);
        // CHECK(de_boolean.get_value() == false);
    }

    SUBCASE("When paramters value is 0x10") {
        string src = "0x10";
        auto correct = to_bytes(src, 31u);
        try {
            auto boolean = Boolean(src);
            test_basic(&boolean, move(correct));
        } catch (const std::exception& e) {
            CHECK(true);
        }
    }
}

TEST_CASE("Test String") {
    string src = "hello, world!";
    auto correct = eevm::to_bytes(
        "0x000000000000000000000000000000000000000000000000000000000000000d"
        "68656c6c6f2c20776f726c642100000000000000000000000000000000000000");
    auto utf8 = Utf8String(src);
    test_basic(&utf8, move(correct));

    auto de_utf8 = Utf8String();
    de_utf8.decode(correct);
    CHECK(de_utf8.get_value() == std::vector<uint8_t>(src.begin(), src.end()));
}

TEST_CASE("Test dynamic bytes") {
    string src = "hello, world!";
    auto correct = eevm::to_bytes(
        "0x000000000000000000000000000000000000000000000000000000000000000d"
        "68656c6c6f2c20776f726c642100000000000000000000000000000000000000");

    auto bytes = DynamicBytes(src);
    test_basic(&bytes, move(correct));

    auto de_bytes = DynamicBytes();
    de_bytes.decode(correct);
    // cout << eevm::to_hex_string(de_bytes.get_value()) << endl;
    CHECK(de_bytes.get_value() == std::vector<uint8_t>(src.begin(), src.end()));
}

TEST_CASE("Test static bytes") {
    string src = "1234567890";
    auto correct = eevm::to_bytes("0x3132333435363738393000000000000000000000000000000000000000000000");

    auto bytes = Bytes(10, src);
    test_basic(&bytes, move(correct));

    auto de_bytes = Bytes(10);
    de_bytes.decode(correct);
    CHECK(de_bytes.get_value() == std::vector<uint8_t>(src.begin(), src.end()));
}

TEST_CASE("Test uint") {
    auto src = eevm::to_uint256("69");
    auto correct = eevm::to_bytes("0x0000000000000000000000000000000000000000000000000000000000000045");

    // uint256
    auto uint_ = Uint(src);
    test_basic(&uint_, move(correct));

    // string
    auto uint_1 = Uint("0x45");
    test_basic(&uint_1, move(correct));

    // vector<uint8_t>
    std::vector<uint8_t> src1({0x45});
    auto uint_2 = Uint(src1);
    test_basic(&uint_2, move(correct));

    // to_uint64
    CHECK(NumericType(correct).to_uint64() == 69);  // NOLINT
}

TEST_CASE("Test dynamic array") {
    auto src =
        vector<string>({"0xde0B295669a9FD93d5F28D9Ec85E40f4cb697BAe", "0xde0B295669a9FD93d5F28D9Ec85E40f4cb697BAe"});

    auto correct = eevm::to_bytes(
        "0x0000000000000000000000000000000000000000000000000000000000000002"
        "000000000000000000000000de0b295669a9fd93d5f28d9ec85e40f4cb697bae"
        "000000000000000000000000de0b295669a9fd93d5f28d9ec85e40f4cb697bae");

    auto array = DynamicArray("address", src);
    test_basic(&array, move(correct));
}

TEST_CASE("Test static array") {
    auto src =
        vector<string>({"0xde0B295669a9FD93d5F28D9Ec85E40f4cb697BAe", "0xde0B295669a9FD93d5F28D9Ec85E40f4cb697BAe"});

    auto correct = eevm::to_bytes(
        "0x000000000000000000000000de0b295669a9fd93d5f28d9ec85e40f4cb697bae"
        "000000000000000000000000de0b295669a9fd93d5f28d9ec85e40f4cb697bae");

    auto array = StaticArray("address", src);
    test_basic(&array, move(correct));
}

TEST_CASE("Test encode") {
    auto encoder = Encoder("test");
    std::vector<std::string> arrs = {"0xde0B295669a9FD93d5F28D9Ec85E40f4cb697BAe",
                                     "0xde0B295669a9FD93d5F28D9Ec85E40f4cb697BAe"};

    encoder.add_inputs("a", "uint", "0x123");
    encoder.add_inputs("b", "address[2]", arrs);
    encoder.add_inputs("c", "bytes10", "1234567890");
    encoder.add_inputs("d", "string", "Hello, world!");

    auto correct = eevm::to_bytes(
        "0x0000000000000000000000000000000000000000000000000000000000000123"
        "000000000000000000000000de0b295669a9fd93d5f28d9ec85e40f4cb697bae"
        "000000000000000000000000de0b295669a9fd93d5f28d9ec85e40f4cb697bae"
        "3132333435363738393000000000000000000000000000000000000000000000"
        "00000000000000000000000000000000000000000000000000000000000000a0"
        "000000000000000000000000000000000000000000000000000000000000000d"
        "48656c6c6f2c20776f726c642100000000000000000000000000000000000000");

    CHECK(encoder.encode() == correct);
}

TEST_CASE("Test function") {
    auto func = Decoder();
    SUBCASE("Include static array") {
        func.add_params("a", "uint256");
        func.add_params("address", "address[2]");
        func.add_params("c", "bytes");
        func.add_params("d", "uint");
        auto correct = eevm::to_bytes(
            "0x0000000000000000000000000000000000000000000000000000000000000002"
            "000000000000000000000000de0b295669a9fd93d5f28d9ec85e40f4cb697bae"
            "000000000000000000000000de0b295669a9fd93d5f28d9ec85e40f4cb697bae"
            "00000000000000000000000000000000000000000000000000000000000000a0"
            "0000000000000000000000000000000000000000000000000000000000006981"
            "000000000000000000000000000000000000000000000000000000000000000d"
            "68656c6c6f2c20776f726c642100000000000000000000000000000000000000");
        func.decode(correct);
    }

    SUBCASE("Include dynamic array") {
        func.add_params("a", "uint256");
        func.add_params("address", "address[]");
        func.add_params("c", "bytes");
        func.add_params("d", "uint");
        auto correct = eevm::to_bytes(
            "0x0000000000000000000000000000000000000000000000000000000000000002"
            "0000000000000000000000000000000000000000000000000000000000000080"
            "00000000000000000000000000000000000000000000000000000000000000e0"
            "0000000000000000000000000000000000000000000000000000000000006981"
            "0000000000000000000000000000000000000000000000000000000000000002"
            "000000000000000000000000de0b295669a9fd93d5f28d9ec85e40f4cb697bae"
            "000000000000000000000000de0b295669a9fd93d5f28d9ec85e40f4cb697bae"
            "000000000000000000000000000000000000000000000000000000000000000d"
            "68656c6c6f2c20776f726c642100000000000000000000000000000000000000");
        func.decode(correct);
    }
}

}  // namespace abicoder
