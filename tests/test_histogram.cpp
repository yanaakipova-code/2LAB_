#include <gtest/gtest.h>
#include <string>
#include "histogram.hpp"
#include "person.hpp"
#include "../structures/ArraySequence.hpp"

static std::size_t fill_by_age(histogram<std::size_t>& h,
                               const ArraySequence<person>& people) {
    std::size_t added = 0;
    for (std::size_t i = 0; i < people.GetLength(); ++i) {
        if (h.add(people.Get(i).get_age())) {
            ++added;
        }
    }
    return added;
}

TEST(HistogramPerson, ByAgeUniqueBins) {
    ArraySequence<person> people;
    people.Append(person({1, 1}, "Ivan", "Ivanov",   "Ivanovich", 20));
    people.Append(person({2, 2}, "Petr", "Petrov",   "Petrovich", 25));
    people.Append(person({3, 3}, "Anna", "Sidorova", "Petrovna",  30));

    auto h = histogram<std::size_t>::make_histogram(
        static_cast<std::size_t>(0),
        static_cast<std::size_t>(100),
        static_cast<std::size_t>(10));

    const std::size_t added = fill_by_age(h, people);
    EXPECT_EQ(added, people.GetLength());

    const std::size_t idx20 = h.get_selection().find_bin(20);
    ASSERT_NE(idx20, selection_histogram<std::size_t>::no_position);
    ASSERT_TRUE(h.has_data(idx20));

    const std::size_t idx25 = h.get_selection().find_bin(25);
    ASSERT_NE(idx25, selection_histogram<std::size_t>::no_position);
    ASSERT_TRUE(h.has_data(idx25));

    const std::size_t idx30 = h.get_selection().find_bin(30);
    ASSERT_NE(idx30, selection_histogram<std::size_t>::no_position);
    ASSERT_TRUE(h.has_data(idx30));
}

TEST(HistogramPerson, ByAgeCustomBoundsSkipsOutOfRange) {
    ArraySequence<person> people;
    people.Append(person({1, 1}, "A", "A", "A", 17));
    people.Append(person({2, 2}, "B", "B", "B", 25));
    people.Append(person({3, 3}, "C", "C", "C", 70));

    ArraySequence<std::size_t> bounds;
    bounds.Append(static_cast<std::size_t>(18));
    bounds.Append(static_cast<std::size_t>(65));

    auto h = histogram<std::size_t>::make_histogram(bounds);

    EXPECT_EQ(h.get_bin_count(), 1u);

    const std::size_t added = fill_by_age(h, people);
    EXPECT_EQ(added, 1u); 
}


TEST(HistogramNumber, BasicAddAndCount) {
    auto h = histogram<double>::make_histogram(0.0, 100.0, 10.0);

    EXPECT_EQ(h.get_bin_count(), 10u);

    EXPECT_TRUE(h.add(5.5));
    EXPECT_TRUE(h.add(7.2));
    EXPECT_TRUE(h.add(25.0));
    EXPECT_TRUE(h.add(99.9));
    
    EXPECT_FALSE(h.add(-5.0)); 
    EXPECT_FALSE(h.add(100.1)); 

    const std::size_t idx1 = h.get_selection().find_bin(5.5);
    ASSERT_NE(idx1, selection_histogram<double>::no_position);
    ASSERT_TRUE(h.has_data(idx1));
    EXPECT_EQ(h.get_bin(idx1).m_count, 2u);

    const std::size_t idx2 = h.get_selection().find_bin(25.0);
    ASSERT_NE(idx2, selection_histogram<double>::no_position);
    ASSERT_TRUE(h.has_data(idx2));
    EXPECT_EQ(h.get_bin(idx2).m_count, 1u);

    const std::size_t idx3 = h.get_selection().find_bin(99.9);
    ASSERT_NE(idx3, selection_histogram<double>::no_position);
    ASSERT_TRUE(h.has_data(idx3));
    EXPECT_EQ(h.get_bin(idx3).m_count, 1u);
}