//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <algorithm>
#include <cstddef>
#include <vector>

void test_serialization(phylanx::ir::node_data<double> const& array_value1)
{
    phylanx::ir::node_data<double> array_value2;

    std::vector<char> buffer = phylanx::util::serialize(array_value1);
    phylanx::util::detail::unserialize(buffer, array_value2);

    HPX_TEST_EQ(array_value1, array_value2);
}

int main(int argc, char* argv[])
{
    {
        phylanx::ir::node_data<double> single_value(42.0);
        HPX_TEST_EQ(single_value[0], 42.0);

        HPX_TEST_EQ(single_value.num_dimensions(), std::size_t(0UL));
        HPX_TEST(single_value.dimensions() ==
            phylanx::ir::node_data<double>::dimensions_type({1, 1}));

        test_serialization(single_value);
    }

    {
        blaze::Rand<blaze::DynamicVector<double>> gen{};
        blaze::DynamicVector<double> v = gen.generate(1007UL);

        phylanx::ir::node_data<double> array_value(v);

        HPX_TEST_EQ(array_value.num_dimensions(), std::size_t(1UL));
        HPX_TEST(array_value.dimensions() ==
            phylanx::ir::node_data<double>::dimensions_type({v.size(), 1UL}));

        test_serialization(array_value);
    }

    {
        std::vector<double> v(1007);
        std::generate(v.begin(), v.end(), std::rand);

        phylanx::ir::node_data<double> array_value(v);

        HPX_TEST_EQ(array_value.num_dimensions(), std::size_t(1UL));
        HPX_TEST(array_value.dimensions() ==
            phylanx::ir::node_data<double>::dimensions_type({v.size(), 1UL}));

        test_serialization(array_value);
    }

    {
        blaze::Rand<blaze::DynamicMatrix<double>> gen{};
        blaze::DynamicMatrix<double> m = gen.generate(42UL, 101UL);

        phylanx::ir::node_data<double> array_value(m);

        HPX_TEST_EQ(array_value.num_dimensions(), std::size_t(2UL));
        HPX_TEST(array_value.dimensions() ==
            phylanx::ir::node_data<double>::dimensions_type(
                {m.rows(), m.columns()}));

        test_serialization(array_value);
    }

    return hpx::util::report_errors();
}
