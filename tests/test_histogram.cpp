#include <gtest/gtest.h>
#include "section.hpp"
#include "range_static.hpp"
#include "histogram.hpp"
#include "error.hpp"
#include "../structures/ArraySequence.hpp"


TEST(SelectionHistogramTest, UniformBasic) {
    auto h = selection_histogram<int>::uniform(0, 100, 10);
    EXPECT_EQ(h.get_bin_count(), 10);
    EXPECT_EQ(h.get_min(), 0);
    EXPECT_EQ(h.get_max(), 100);
}

TEST(SelectionHistogramTest, UniformNotDivisible) {
    auto h = selection_histogram<int>::uniform(0, 95, 10);
    EXPECT_EQ(h.get_bin_count(), 10);
    EXPECT_EQ(h.get_max(), 95);
}

TEST(SelectionHistogramTest, UniformInvalidRange) {
    EXPECT_THROW(selection_histogram<int>::uniform(100, 0, 10),
                 invalid_range);
}

TEST(SelectionHistogramTest, UniformInvalidStep) {
    EXPECT_THROW(selection_histogram<int>::uniform(0, 100, 0),
                 invalid_range);
}


TEST(SelectionHistogramTest, UnevenBasic) {
    ArraySequence<int> bounds;
    bounds.Append(0);
    bounds.Append(18);
    bounds.Append(30);
    bounds.Append(60);
    bounds.Append(100);

    auto h = selection_histogram<int>::uneven(bounds);
    EXPECT_EQ(h.get_bin_count(), 4);
    EXPECT_EQ(h.get_min(), 0);
    EXPECT_EQ(h.get_max(), 100);
}

TEST(SelectionHistogramTest, UnevenTooFewBounds) {
    ArraySequence<int> bounds;
    bounds.Append(0);
    EXPECT_THROW(selection_histogram<int>::uneven(bounds), length_arr);
}

TEST(SelectionHistogramTest, UnevenNotIncreasing) {
    ArraySequence<int> bounds;
    bounds.Append(0);
    bounds.Append(100);
    bounds.Append(50);
    EXPECT_THROW(selection_histogram<int>::uneven(bounds), length_arr);
}

TEST(SelectionHistogramTest, FindBinUniform) {
    auto h = selection_histogram<int>::uniform(0, 100, 10);

    EXPECT_EQ(h.find_bin(0), 0);
    EXPECT_EQ(h.find_bin(5), 0);
    EXPECT_EQ(h.find_bin(10), 1);
    EXPECT_EQ(h.find_bin(25), 2);
    EXPECT_EQ(h.find_bin(50), 5);
    EXPECT_EQ(h.find_bin(95), 9);
    EXPECT_EQ(h.find_bin(100), 9);
}

TEST(SelectionHistogramTest, FindBinOutOfRange) {
    auto h = selection_histogram<int>::uniform(0, 100, 10);

    EXPECT_EQ(h.find_bin(-5), selection_histogram<int>::no_position);
    EXPECT_EQ(h.find_bin(150), selection_histogram<int>::no_position);
}

TEST(SelectionHistogramTest, FindBinUneven) {
    ArraySequence<int> bounds;
    bounds.Append(0);
    bounds.Append(18);
    bounds.Append(30);
    bounds.Append(60);
    bounds.Append(100);

    auto h = selection_histogram<int>::uneven(bounds);

    EXPECT_EQ(h.find_bin(0), 0);
    EXPECT_EQ(h.find_bin(17), 0);
    EXPECT_EQ(h.find_bin(18), 1);
    EXPECT_EQ(h.find_bin(25), 1);
    EXPECT_EQ(h.find_bin(30), 2);
    EXPECT_EQ(h.find_bin(50), 2);
    EXPECT_EQ(h.find_bin(60), 3);
    EXPECT_EQ(h.find_bin(100), 3);
}

TEST(SelectionHistogramTest, GetLowerUpper) {
    auto h = selection_histogram<int>::uniform(0, 100, 10);

    EXPECT_EQ(h.get_lower(0), 0);
    EXPECT_EQ(h.get_upper(0), 10);
    EXPECT_EQ(h.get_lower(5), 50);
    EXPECT_EQ(h.get_upper(5), 60);
    EXPECT_EQ(h.get_lower(9), 90);
    EXPECT_EQ(h.get_upper(9), 100);
}

TEST(SelectionHistogramTest, CheckIndexThrows) {
    auto h = selection_histogram<int>::uniform(0, 100, 10);

    EXPECT_THROW(h.get_lower(10), std::out_of_range);
    EXPECT_THROW(h.get_upper(15), std::out_of_range);
}

TEST(SelectionHistogramTest, Contains) {
    auto h = selection_histogram<int>::uniform(0, 100, 10);

    EXPECT_TRUE(h.contains(0));
    EXPECT_TRUE(h.contains(50));
    EXPECT_TRUE(h.contains(100));
    EXPECT_FALSE(h.contains(-1));
    EXPECT_FALSE(h.contains(200));
}

TEST(RangeStatisticTest, Empty) {
    range_statistic<int> stats;

    EXPECT_EQ(stats.m_count, 0);
    EXPECT_DOUBLE_EQ(stats.mean(), 0.0);
    EXPECT_DOUBLE_EQ(stats.variance(), 0.0);
    EXPECT_DOUBLE_EQ(stats.median(), 0.0);
}

TEST(RangeStatisticTest, AddOne) {
    range_statistic<int> stats;
    stats.add(42);

    EXPECT_EQ(stats.m_count, 1);
    EXPECT_EQ(stats.m_min_elem, 42);
    EXPECT_EQ(stats.m_max_elem, 42);
    EXPECT_DOUBLE_EQ(stats.mean(), 42.0);
    EXPECT_DOUBLE_EQ(stats.median(), 42.0);
}

TEST(RangeStatisticTest, AddMultiple) {
    range_statistic<int> stats;
    stats.add(25);
    stats.add(27);
    stats.add(35);

    EXPECT_EQ(stats.m_count, 3);
    EXPECT_EQ(stats.m_min_elem, 25);
    EXPECT_EQ(stats.m_max_elem, 35);
    EXPECT_DOUBLE_EQ(stats.mean(), 29.0);
}

TEST(RangeStatisticTest, MedianOdd) {
    range_statistic<int> stats;
    stats.add(5);
    stats.add(2);
    stats.add(8);
    stats.add(1);
    stats.add(9);

    EXPECT_DOUBLE_EQ(stats.median(), 5.0);
}

TEST(RangeStatisticTest, MedianEven) {
    range_statistic<int> stats;
    stats.add(5);
    stats.add(2);
    stats.add(8);
    stats.add(1);

    EXPECT_DOUBLE_EQ(stats.median(), 3.5);
}

TEST(RangeStatisticTest, Variance) {
    range_statistic<int> stats;
    stats.add(20);
    stats.add(30);

    EXPECT_DOUBLE_EQ(stats.mean(), 25.0);
    EXPECT_DOUBLE_EQ(stats.variance(), 25.0);
}

TEST(RangeStatisticTest, MinMaxUpdate) {
    range_statistic<int> stats;
    stats.add(50);
    stats.add(10);
    stats.add(30);
    stats.add(90);
    stats.add(20);

    EXPECT_EQ(stats.m_min_elem, 10);
    EXPECT_EQ(stats.m_max_elem, 90);
}

TEST(HistogramTest, UniformEmpty) {
    histogram<int> hist(0, 100, 10);

    EXPECT_EQ(hist.get_bin_count(), 10);
    EXPECT_EQ(hist.get_min(), 0);
    EXPECT_EQ(hist.get_max(), 100);
}

TEST(HistogramTest, UniformAddOne) {
    histogram<int> hist(0, 100, 10);
    EXPECT_TRUE(hist.add(25));

    auto stats = hist.get_bin(2);
    EXPECT_EQ(stats.m_count, 1);
    EXPECT_EQ(stats.m_min_elem, 25);
    EXPECT_EQ(stats.m_max_elem, 25);
}

TEST(HistogramTest, UniformAddMultiple) {
    histogram<int> hist(0, 100, 10);
    hist.add(25);
    hist.add(27);
    hist.add(35);
    hist.add(95);

    EXPECT_EQ(hist.get_bin(2).m_count, 2);
    EXPECT_EQ(hist.get_bin(3).m_count, 1);
    EXPECT_EQ(hist.get_bin(9).m_count, 1);
}

TEST(HistogramTest, UniformBoundary) {
    histogram<int> hist(0, 100, 10);
    hist.add(20);
    hist.add(30);

    EXPECT_EQ(hist.get_bin(2).m_count, 1);
    EXPECT_EQ(hist.get_bin(3).m_count, 1);
}

TEST(HistogramTest, UnevenBasic) {
    ArraySequence<int> bounds;
    bounds.Append(0);
    bounds.Append(18);
    bounds.Append(30);
    bounds.Append(60);
    bounds.Append(100);

    histogram<int> hist(bounds);
    EXPECT_EQ(hist.get_bin_count(), 4);
}

TEST(HistogramTest, UnevenAdd) {
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

    EXPECT_EQ(hist.get_bin(0).m_count, 2);
    EXPECT_EQ(hist.get_bin(1).m_count, 1);
    EXPECT_EQ(hist.get_bin(2).m_count, 1);
    EXPECT_EQ(hist.get_bin(3).m_count, 1);
}

TEST(HistogramTest, AddOutOfRange) {
    histogram<int> hist(0, 100, 10);

    EXPECT_FALSE(hist.add(-5));
    EXPECT_FALSE(hist.add(150));
    EXPECT_TRUE(hist.add(50));

    EXPECT_EQ(hist.get_bin(5).m_count, 1);
}

TEST(HistogramTest, GetBinOutOfRange) {
    histogram<int> hist(0, 100, 10);
    EXPECT_THROW(hist.get_bin(10), no_find);
}

TEST(HistogramTest, GetBinEmpty) {
    histogram<int> hist(0, 100, 10);

    auto stats = hist.get_bin(2);
    EXPECT_EQ(stats.m_count, 0);
}

TEST(HistogramTest, HasData) {
    histogram<int> hist(0, 100, 10);
    hist.add(25);

    EXPECT_TRUE(hist.has_data(2));
    EXPECT_FALSE(hist.has_data(3));
}

TEST(HistogramTest, Statistics) {
    histogram<int> hist(0, 100, 10);
    hist.add(21);
    hist.add(22);
    hist.add(23);

    auto stats = hist.get_bin(2);
    EXPECT_EQ(stats.m_count, 3);
    EXPECT_DOUBLE_EQ(stats.mean(), 22.0);
    EXPECT_DOUBLE_EQ(stats.median(), 22.0);
}

TEST(HistogramTest, GetSelection) {
    histogram<int> hist(0, 100, 10);

    const selection_histogram<int>& sel = hist.get_selection();
    EXPECT_EQ(sel.get_bin_count(), 10);
    EXPECT_EQ(sel.get_min(), 0);
    EXPECT_EQ(sel.get_max(), 100);
}

TEST(HistogramTest, DoubleHistogram) {
    histogram<double> hist(0.0, 1.0, 0.1);
    hist.add(0.25);
    hist.add(0.27);

    EXPECT_EQ(hist.get_bin_count(), 10);
    EXPECT_EQ(hist.get_bin(2).m_count, 2);
}