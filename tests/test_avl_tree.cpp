#include <gtest/gtest.h>
#include "AVL_tree.hpp"
#include "error.hpp"
#include "avl_iterator.hpp"

TEST(AVLTreeTest, EmptyTree) {
    avl_tree<int, int> tree;
    EXPECT_EQ(tree.get_count(), 0);
    EXPECT_FALSE(tree.contains_key(42));
}

TEST(AVLTreeTest, AddOne) {
    avl_tree<int, int> tree;
    tree.add(10, 100);
    EXPECT_EQ(tree.get_count(), 1);
    EXPECT_TRUE(tree.contains_key(10));
    EXPECT_EQ(tree.get_elem_by_key(10), 100);
}

TEST(AVLTreeTest, AddDuplicateThrows) {
    avl_tree<int, int> tree;
    tree.add(10, 100);
    EXPECT_THROW(tree.add(10, 200), invalid_key);
}

TEST(AVLTreeTest, AddAscending) {
    avl_tree<int, int> tree;
    for (int i = 1; i <= 100; ++i) {
        tree.add(i, i * 10);
    }
    EXPECT_EQ(tree.get_count(), 100);
    for (int i = 1; i <= 100; ++i) {
        EXPECT_TRUE(tree.contains_key(i));
    }
}

TEST(AVLTreeTest, RemoveLeaf) {
    avl_tree<int, int> tree;
    tree.add(50, 500);
    tree.add(30, 300);
    tree.add(70, 700);

    tree.remove_by_key(30);
    EXPECT_EQ(tree.get_count(), 2);
    EXPECT_FALSE(tree.contains_key(30));
}

TEST(AVLTreeTest, RemoveWithTwoChildren) {
    avl_tree<int, int> tree;
    tree.add(50, 500);
    tree.add(30, 300);
    tree.add(70, 700);

    tree.remove_by_key(50);
    EXPECT_EQ(tree.get_count(), 2);
    EXPECT_FALSE(tree.contains_key(50));
}

TEST(AVLTreeTest, GetRefByKey) {
    avl_tree<int, int> tree;
    tree.add(10, 100);

    tree.get_ref_by_key(10) = 999;
    EXPECT_EQ(tree.get_elem_by_key(10), 999);
}

TEST(AVLIteratorTest, EmptyTree) {
    avl_tree<int, int> tree;
    auto it = tree.begin();
    EXPECT_EQ(it, tree.end());
}

TEST(AVLIteratorTest, OneElement) {
    avl_tree<int, int> tree;
    tree.add(10, 100);

    auto it = tree.begin();
    EXPECT_EQ(*it, 100);
    ++it;
    EXPECT_EQ(it, tree.end());
}
