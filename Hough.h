#ifndef HUOGH_H
#define HUOGH_H

#include <thrust/device_vector.h>
#include <thrust/host_vector.h>
#include <thrust/sort.h>
#include <thrust/copy.h>
#include <thrust/random.h>
#include <thrust/inner_product.h>
#include <thrust/binary_search.h>
#include <thrust/adjacent_difference.h>
#include <thrust/iterator/constant_iterator.h>
#include <thrust/iterator/counting_iterator.h>

#include <iostream>
#include <iomanip>
#include <iterator>
#include <vector>

template <typename Vector>
void print_vector(const std::string& name, const Vector &v)
{
  typedef typename Vector::value_type T;
  std::cout << "  " << std::setw(20) << name << "  ";
  thrust::copy(v.begin(), v.end(), std::ostream_iterator<T>(std::cout, " "));
  std::cout << std::endl;
}

void VecToTrust(std::vector<float> &v);

struct floatMultiplication
{
    const float a;

    floatMultiplication(float _a) : a(_a) {}

    __host__ __device__
        float operator()(const float& x) const 
        { 
            return a * x;
        }
};

struct intDivision
{
    const float a;

    intDivision(float _a) : a(_a) {}

    __host__ __device__
        int operator()(const float& x) const 
        { 
            return int(x/a);
        }
};

// sparse histogram using reduce_by_key
template <typename Vector1,
          typename Vector2,
          typename Vector3>
void sparse_histogram(const Vector1& data,
                            Vector2& histogram_values,
                            Vector3& histogram_counts);

void calculateRho(std::vector<std::vector<std::vector<int>>> &values, std::vector<int> &max, std::vector<float> &yValueFloat, std::vector<float> &xValueFloat, const float thetaPrecision, const float rhoPrecision, const float ymax, bool costrain);

#endif