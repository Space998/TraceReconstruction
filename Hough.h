#ifndef HUOGH_H
#define HUOGH_H

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <iterator>

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
#include <thrust/transform.h>
#include <thrust/transform_reduce.h>

template <typename Vector>
void print_vector(const std::string& name, const Vector &v)
{
  typedef typename Vector::value_type T;
  std::cout << "  " << std::setw(20) << name << "  ";
  thrust::copy(v.begin(), v.end(), std::ostream_iterator<T>(std::cout << std::dec, " "));
  std::cout << std::endl;
}

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

struct floatMultiplicationSum
{
    const float a;

    floatMultiplicationSum(float _a) : a(_a) {}

    __host__ __device__
        float operator()(const float& x) const 
        { 
            return (a * x) + a;
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

struct sum
{
    const float a;

    sum(float _a) : a(_a) {}

    __host__ __device__
        float operator()(const float& x) const 
        { 
            return x+a;
        }
};

// sparse histogram using reduce_by_key
template <typename Vector1,
          typename Vector2,
          typename Vector3>
void sparse_histogram(const Vector1& data,
                            Vector2& histogram_values,
                            Vector3& histogram_counts);

__global__ void rho(float* ang, float* y, float *x, int* r, const float rhoPrecision, const int bigSize, const int smallSize);

void Hough(std::vector<std::vector<std::vector<int>>> &values, std::vector<int> &max, std::vector<float> &yValueFloat, std::vector<float> &xValueFloat, const float thetaPrecision, const float rhoPrecision, const float ymax, bool costrain, const float limitAngle);

void calculateRho(std::vector<std::vector<std::vector<int>>> &values, std::vector<int> &max, std::vector<float> &yValueFloat, std::vector<float> &xValueFloat, const float thetaPrecision, const float rhoPrecision, const float ymax, bool costrain, const float limitAngle);

#endif