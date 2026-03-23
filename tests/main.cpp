#include <cstring>
#include <gtest/gtest.h>
extern "C" {
#include <packr/utils.h>
#include <packr/entry.h>
#include <packr/types.h>
}
#include <string>

// "join_to_path" tests
TEST(join_to_path, normal) {
    // First test
    std::string filename1{"file.txt"};
    std::string directory{"/home/user/desktop/directory"};
    std::string full_path{directory + '/' + filename1};
    std::string joined1{join_to_path(filename1.data(), directory.data())};
    EXPECT_EQ(joined1, full_path);

    // Second test
    std::string filename2{"somerandomfile"};
    std::string directory2{"/home/user/desktop/directory/"};
    std::string full_path2{join_to_path(filename2.data(), directory2.data())};
    std::string joined2{directory2 + '/' + filename2};
    EXPECT_EQ(joined2, full_path2);

    // Slightly abnormal
    std::string filename3{"somerandomfile/"};
    std::string directory3{"/home/user/desktop/directory"};
    std::string full_path3{join_to_path(filename3.data(), directory3.data())};
    std::string joined3{directory3 + '/' + filename3};
    EXPECT_EQ(joined3, full_path3);
}

TEST(join_to_path, null_inputs) {
    char* path = nullptr;
    char* filename = nullptr;
    EXPECT_EQ(nullptr, join_to_path(filename, path));
}

// "add_dirname" function tests
TEST(add_dirname, with_named_as) {
    dir_entry dir_ent{};
    std::string src_path{"/home/desktop/some_directory"};
    add_dirname(&dir_ent, nullptr, src_path.data());
    std::string test_str{"some_directory"};
    EXPECT_EQ(test_str, std::string{dir_ent.dirname});
    EXPECT_EQ(test_str.size(), dir_ent.dirname_length);
}

TEST(add_dirname, no_named_as) {
    dir_entry dir_ent{};
    std::string src_path{"/home/desktop/some_directory"};
    const char* named_as = "bla bla bla";
    add_dirname(&dir_ent, const_cast<char*>(named_as), src_path.data());
    EXPECT_EQ(std::string{named_as}, std::string{dir_ent.dirname});
    EXPECT_EQ(std::strlen(named_as), dir_ent.dirname_length);
}

// Special marker values' tests
TEST(special_markers, main_test) {
    EXPECT_EQ(ENT_DIR_START, 0x01);
    EXPECT_EQ(ENT_DIR_END, 0x02);
    EXPECT_EQ(ENT_FILE, 0x04);
    EXPECT_EQ(PACK_START, 0x08);
    EXPECT_EQ(PACK_END, 0x10);
}

// Testing other macros and constant values
TEST(other_macros_and_constants, main_tests) {
    EXPECT_EQ(NAME_LEN_MAX, 4096);
    Bool True{TRUE};
    EXPECT_EQ(True, 1);
    Bool False{FALSE};
    EXPECT_EQ(False, 0);
    EXPECT_EQ(DEFAULT_ROOT_DIR, 0);
    EXPECT_EQ(P_NOMETADATA, 0B00000001);
}
