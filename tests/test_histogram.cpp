#include <gtest/gtest.h>
#include "histogram.hpp"
#include "range_static.hpp"
#include "AVL_tree.hpp"


TEST(RangeStaticTest, Empty) {
    range_static<int> stats;
    EXPECT_EQ(stats.m_count, 0);
    EXPECT_DOUBLE_EQ(stats.mean(), 0.0);
    EXPECT_DOUBLE_EQ(stats.variance(), 0.0);
    EXPECT_DOUBLE_EQ(stats.median(), 0.0);
}

TEST(RangeStaticTest, AddOne) {
    range_static<int> stats;
    stats.add(42);

    EXPECT_EQ(stats.m_count, 1);
    EXPECT_EQ(stats.m_max_elem, 42);
    EXPECT_EQ(stats.m_min_elem, 42);
    EXPECT_DOUBLE_EQ(stats.mean(), 42.0);
}

TEST(RangeStaticTest, AddMultiple) {
    range_static<int> stats;
    stats.add(25);
    stats.add(27);
    stats.add(35);

    EXPECT_EQ(stats.m_count, 3);
    EXPECT_EQ(stats.m_max_elem, 35);
    EXPECT_EQ(stats.m_min_elem, 25);
    EXPECT_DOUBLE_EQ(stats.mean(), 29.0);
}

TEST(RangeStaticTest, MedianOdd) {
    range_static<int> stats;
    stats.add(5);
    stats.add(2);
    stats.add(8);
    stats.add(1);
    stats.add(9);

    EXPECT_DOUBLE_EQ(stats.median(), 5.0);
}

TEST(RangeStaticTest, MedianEven) {
    range_static<int> stats;
    stats.add(5);
    stats.add(2);
    stats.add(8);
    stats.add(1);

    EXPECT_DOUBLE_EQ(stats.median(), 3.5);
}

TEST(HistogramTest, UniformEmpty) {
    histogram<int> hist(0, 100, 10);
    EXPECT_EQ(hist.get_bin_count(), 0);
    EXPECT_EQ(hist.get_min(), 0);
    EXPECT_EQ(hist.get_max(), 100);
    EXPECT_EQ(hist.get_step(), 10);
    EXPECT_TRUE(hist.is_uniform());
}


TEST(HistogramTest, UniformMultipleValues) {
    histogram<int> hist(0, 100, 10);
    hist.add(25);
    hist.add(27);
    hist.add(35);
    hist.add(95);

    EXPECT_EQ(hist.get_bin_count(), 3);

    auto stats20 = hist.get_bin(20);
    EXPECT_EQ(stats20.m_count, 2);

    auto stats30 = hist.get_bin(30);
    EXPECT_EQ(stats30.m_count, 1);

    auto stats90 = hist.get_bin(90);
    EXPECT_EQ(stats90.m_count, 1);
}


TEST(HistogramTest, NonUniformEmpty) {
    ArraySequence<int> bounds;
    bounds.Append(0);
    bounds.Append(18);
    bounds.Append(30);
    bounds.Append(60);
    bounds.Append(100);

    histogram<int> hist(bounds);
    EXPECT_EQ(hist.get_bin_count(), 0);
    EXPECT_FALSE(hist.is_uniform());
}

TEST(HistogramTest, NonUniformValues) {
    ArraySequence<int> bounds;
    bounds.Append(0);
    bounds.Append(18);
    bounds.Append(30);
    bounds.Append(60);
    bounds.Append(100);

    histogram<int> hist(bounds);
    hist.add(5);
    hist.add(12);
    hist.add(25);
    hist.add(35);
    hist.add(70);

    EXPECT_EQ(hist.get_bin_count(), 4);

    EXPECT_EQ(hist.get_bin(0).m_count, 2);
    EXPECT_EQ(hist.get_bin(18).m_count, 1);
    EXPECT_EQ(hist.get_bin(30).m_count, 1);
    EXPECT_EQ(hist.get_bin(60).m_count, 1);
}
