#include <gtest/gtest.h>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <string>
#include <filesystem>
#include "../src/output/array.cpp"

class ArrayOutputTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create temporary test files
        createTestFile("test_empty.json", "[]");
        createTestFile("test_single.json", "[1.5]");
        createTestFile("test_multiple.json", "[1.0, 2.5, 3.75, -4.2]");
        createTestFile("test_with_spaces.json", "[ 1.0 , 2.5 , 3.75 ]");
        createTestFile("test_invalid.json", "not an array");
        createTestFile("test_invalid_format.json", "[1.0, 2.5, invalid, 4.0]");
    }

    void TearDown() override {
        // Clean up test files
        remove("test_empty.json");
        remove("test_single.json");
        remove("test_multiple.json");
        remove("test_with_spaces.json");
        remove("test_invalid.json");
        remove("test_invalid_format.json");
    }

    void createTestFile(const std::string& filename, const std::string& content) {
        std::ofstream file(filename);
        file << content;
        file.close();
    }
};

// Test reading empty array
TEST_F(ArrayOutputTest, ReadEmptyArray) {
    ArrayOutput output("test_empty.json");
    EXPECT_EQ(output.content.size(), 0);
}

// Test reading single element array
TEST_F(ArrayOutputTest, ReadSingleElement) {
    ArrayOutput output("test_single.json");
    EXPECT_EQ(output.content.size(), 1);
    EXPECT_FLOAT_EQ(output.content[0], 1.5f);
}

// Test reading multiple elements
TEST_F(ArrayOutputTest, ReadMultipleElements) {
    ArrayOutput output("test_multiple.json");
    EXPECT_EQ(output.content.size(), 4);
    EXPECT_FLOAT_EQ(output.content[0], 1.0f);
    EXPECT_FLOAT_EQ(output.content[1], 2.5f);
    EXPECT_FLOAT_EQ(output.content[2], 3.75f);
    EXPECT_FLOAT_EQ(output.content[3], -4.2f);
}

// Test reading array with extra spaces
TEST_F(ArrayOutputTest, ReadWithSpaces) {
    ArrayOutput output("test_with_spaces.json");
    EXPECT_EQ(output.content.size(), 3);
    EXPECT_FLOAT_EQ(output.content[0], 1.0f);
    EXPECT_FLOAT_EQ(output.content[1], 2.5f);
    EXPECT_FLOAT_EQ(output.content[2], 3.75f);
}

// Test reading invalid file (not an array)
TEST_F(ArrayOutputTest, ReadInvalidFile) {
    EXPECT_THROW(ArrayOutput output("test_invalid.json"), std::runtime_error);
}

// Test reading file with invalid format
TEST_F(ArrayOutputTest, ReadInvalidFormat) {
    EXPECT_THROW(ArrayOutput output("test_invalid_format.json"), std::runtime_error);
}

// Test reading non-existent file
TEST_F(ArrayOutputTest, ReadNonExistentFile) {
    EXPECT_THROW(ArrayOutput output("nonexistent.json"), std::runtime_error);
}

// Test distance calculation between identical arrays
TEST_F(ArrayOutputTest, DistanceIdenticalArrays) {
    ArrayOutput output1("test_multiple.json");
    ArrayOutput output2("test_multiple.json");
    
    double distance = output1.get_distance(output2);
    EXPECT_DOUBLE_EQ(distance, 0.0);
}

// Test distance calculation between different arrays
TEST_F(ArrayOutputTest, DistanceDifferentArrays) {
    createTestFile("test_array1.json", "[1.0, 2.0, 3.0]");
    createTestFile("test_array2.json", "[1.0, 2.0, 4.0]");
    
    ArrayOutput output1("test_array1.json");
    ArrayOutput output2("test_array2.json");
    
    double distance = output1.get_distance(output2);
    EXPECT_DOUBLE_EQ(distance, 1.0); // |3.0 - 4.0| = 1.0
    
    remove("test_array1.json");
    remove("test_array2.json");
}

// Test distance calculation between arrays of different lengths
TEST_F(ArrayOutputTest, DistanceDifferentLengths) {
    createTestFile("test_short.json", "[1.0, 2.0]");
    createTestFile("test_long.json", "[1.0, 2.0, 3.0, 4.0]");
    
    ArrayOutput output1("test_short.json");
    ArrayOutput output2("test_long.json");
    
    double distance = output1.get_distance(output2);
    // |1.0-1.0| + |2.0-2.0| + |0.0-3.0| + |0.0-4.0| = 0 + 0 + 3 + 4 = 7.0
    EXPECT_DOUBLE_EQ(distance, 7.0);
    
    remove("test_short.json");
    remove("test_long.json");
}

TEST_F(ArrayOutputTest, Dvec1Content) {
    std::filesystem::path current_path = std::filesystem::current_path();
    std::string output_path = current_path / "tests" / "dvec1.json";
    ArrayOutput output(output_path);
    EXPECT_EQ(output.content.size(), 5);
    EXPECT_FLOAT_EQ(output.content[0], 0.94973442515074025f);
    EXPECT_FLOAT_EQ(output.content[1], 0.24971252450443293f);
    EXPECT_FLOAT_EQ(output.content[2], -0.67988283591499377f);
    EXPECT_FLOAT_EQ(output.content[3], -0.98440677539916488f);
    EXPECT_FLOAT_EQ(output.content[4], -0.38386932183718619f);
}
// Test distance calculation with negative values
TEST_F(ArrayOutputTest, DistanceWithNegatives) {
    createTestFile("test_neg1.json", "[1.0, -2.0, 3.0]");
    createTestFile("test_neg2.json", "[1.0, 2.0, -3.0]");
    
    ArrayOutput output1("test_neg1.json");
    ArrayOutput output2("test_neg2.json");
    
    double distance = output1.get_distance(output2);
    // |1.0-1.0| + |-2.0-2.0| + |3.0-(-3.0)| = 0 + 4 + 6 = 10.0
    EXPECT_DOUBLE_EQ(distance, 10.0);
    
    remove("test_neg1.json");
    remove("test_neg2.json");
}

// Test distance calculation with empty array
TEST_F(ArrayOutputTest, DistanceWithEmptyArray) {
    createTestFile("test_nonempty.json", "[1.0, 2.0, 3.0]");
    
    ArrayOutput output1("test_empty.json");
    ArrayOutput output2("test_nonempty.json");
    
    double distance = output1.get_distance(output2);
    // |0.0-1.0| + |0.0-2.0| + |0.0-3.0| = 1 + 2 + 3 = 6.0
    EXPECT_DOUBLE_EQ(distance, 6.0);
    
    remove("test_nonempty.json");
}

// Test error when comparing with different output type
TEST_F(ArrayOutputTest, DistanceDifferentType) {
    // Create a mock output that's not ArrayOutput
    class MockOutput : public OutputBase {
    public:
        double get_distance(OutputBase& other) override {
            return 0.0;
        }
    };
    
    ArrayOutput arrayOutput("test_single.json");
    MockOutput mockOutput;
    
    EXPECT_THROW(arrayOutput.get_distance(mockOutput), std::runtime_error);
}

// Test reading array with trailing comma
TEST_F(ArrayOutputTest, ReadWithTrailingComma) {
    createTestFile("test_trailing_comma.json", "[1.0, 2.5, 3.75,]");
    
    ArrayOutput output("test_trailing_comma.json");
    EXPECT_EQ(output.content.size(), 3);
    EXPECT_FLOAT_EQ(output.content[0], 1.0f);
    EXPECT_FLOAT_EQ(output.content[1], 2.5f);
    EXPECT_FLOAT_EQ(output.content[2], 3.75f);
    
    remove("test_trailing_comma.json");
}

// Test reading array with only whitespace
TEST_F(ArrayOutputTest, ReadWithOnlyWhitespace) {
    createTestFile("test_whitespace_only.json", "[   ]");
    
    ArrayOutput output("test_whitespace_only.json");
    EXPECT_EQ(output.content.size(), 0);
    
    remove("test_whitespace_only.json");
}
