#include <gtest/gtest.h>
extern "C" {
#include <packr/utils.h>
}
#include <string>

TEST(join_to_path, normal) {
    std::string filename{"file.txt"};
    std::string directory{"/home/user/desktop/directory"};
    std::string full_path{directory + '/' + filename};
    std::string joined{join_to_path(filename.data(), directory.data())};
    EXPECT_EQ(joined, full_path);
}

TEST(join_to_path, null_inputs) {
    char* path = nullptr;
    char* filename = nullptr;
    EXPECT_EQ(nullptr, join_to_path(filename, path));
}
