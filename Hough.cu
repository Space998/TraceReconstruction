#include "Hough.h"
#include <vector>
#include <string>
#include <iostream>
#include "Rivelatore.h"
#include <math.h>

#include <thrust/device_vector.h>
#include <thrust/host_vector.h>
#include <thrust/copy.h>
#include <thrust/iterator/constant_iterator.h>
#include <thrust/transform.h>

void VecToTrust(std::vector<float> &v)
{
  std::cout << "--Mucca" << std::endl;
  thrust::device_vector<float> D(v.begin(), v.end());
  print_vector("Test",D);
}

// sparse histogram using reduce_by_key
template <typename Vector1,
          typename Vector2,
          typename Vector3>
void sparse_histogram(Vector1& data,
                            Vector2& histogram_values,
                            Vector3& histogram_counts)
{
  typedef typename Vector1::value_type ValueType; // input value type
  typedef typename Vector3::value_type IndexType; // histogram index type

  /*
  // copy input data (could be skipped if input is allowed to be modified)
  thrust::device_vector<ValueType> data(input);
    
  // print the initial data
  print_vector("initial data", data);
  */

  // sort data to bring equal elements together
  thrust::sort(data.begin(), data.end());
  
  // print the sorted data
  //print_vector("sorted data", data);

  // number of histogram bins is equal to number of unique values (assumes data.size() > 0)
  IndexType num_bins = thrust::inner_product(data.begin(), data.end() - 1,
                                             data.begin() + 1,
                                             IndexType(1),
                                             thrust::plus<IndexType>(),
                                             thrust::not_equal_to<ValueType>());

  // resize histogram storage
  histogram_values.resize(num_bins);
  histogram_counts.resize(num_bins);
  
  // compact find the end of each bin of values
  thrust::reduce_by_key(data.begin(), data.end(),
                        thrust::constant_iterator<IndexType>(1),
                        histogram_values.begin(),
                        histogram_counts.begin());
  
  // print the sparse histogram
  //print_vector("histogram values", histogram_values);
  //print_vector("histogram counts", histogram_counts);
}


void calculateRho(std::vector<std::vector<std::vector<int>>> &values, std::vector<int> &max, std::vector<float> &yValueFloat, std::vector<float> &xValueFloat, const float thetaPrecision, const float rhoPrecision, const float ymax, bool costrain)
{
  thrust::device_vector<float> xValueFloatTrust(xValueFloat.begin(), xValueFloat.end());
  thrust::device_vector<float> yValueFloatTrust(yValueFloat.begin(), yValueFloat.end());

  //thrust::device_vector<float> cosValue(xValueFloatTrust.size()); //Creatin of vector for cos value
  //thrust::device_vector<float> sinValue(xValueFloatTrust.size()); //Creatin of vector for sin value

  //Termporari vectors to store x*cos and y*sin
  thrust::device_vector<float> xTemp(xValueFloatTrust.size());
  thrust::device_vector<float> yTemp(xValueFloatTrust.size());

  //Vector temporary containin non dicreta values of rho
  thrust::device_vector<float> rhoTemp(xValueFloatTrust.size());

  //Vector containing fila rho discrete values
  thrust::device_vector<int> rho(xValueFloatTrust.size());

  //Detector for histogram
  thrust::device_vector<int> histogram_values;
  thrust::device_vector<int> histogram_counts;

  float angle = 0;

  for (int i = 0; i < int(values.size()); i++)
  {
    //std::cout << i+1 << std::endl;
    /*
    //Creatin of vector for cos value 
    cosValue.clear();
    thrust::fill(cosValue.begin(), cosValue.end(), cos(((i+1)*thetaPrecision*M_PI)/180));
    */

    angle = ((i+1)*thetaPrecision*M_PI)/180;

    //Calculation of cos(theta)*x
    thrust::transform(xValueFloatTrust.begin(), xValueFloatTrust.end(), xTemp.begin(), floatMultiplication(cos(angle)));
    //thrust::transform(xValueFloatTrust.begin(), xValueFloatTrust.end(), cosValue.begin(), xTemp.begin(), thrust::multiplies<float>());

    /*
    //Creatin of vector for sin value
    sinValue.clear();
    //thrust::fill(sinValue.begin(), sinValue.end(), sin(((i+1)*thetaPrecision*M_PI)/180));
    */

    //Calculation of sin(theta)*y
    thrust::transform(yValueFloatTrust.begin(), yValueFloatTrust.end(), yTemp.begin(), floatMultiplication(sin(angle)));
    //thrust::transform(yValueFloatTrust.begin(), yValueFloatTrust.end(), sinValue.begin(), yTemp.begin(), thrust::multiplies<float>());

    //Calulate sum
    thrust::transform(xTemp.begin(), xTemp.end(), yTemp.begin(), rhoTemp.begin(), thrust::plus<float>());

    //Calculate rho discrete
    thrust::transform(rhoTemp.begin(), rhoTemp.end(), rho.begin(), intDivision(rhoPrecision));

    /*
    print_vector("X", xValueFloatTrust);
    print_vector("COS", xTemp);
    print_vector("Y",yValueFloatTrust);
    print_vector("SIN",yTemp);
    print_vector("RHO",rhoTemp);
    print_vector("RHOD",rho);
    */

    sparse_histogram(rho, histogram_values, histogram_counts);

    std::vector<int> histoValue(histogram_values.size());
    thrust::copy(histogram_values.begin(), histogram_values.end(), histoValue.begin());
    
    std::vector<int> histoCount(histogram_values.size());
    thrust::copy(histogram_counts.begin(), histogram_counts.end(), histoCount.begin());
    
    values.at(i).push_back(histoValue);
    values.at(i).push_back(histoCount);
    
    for (int j = 0; j < int(histogram_values.size()); j++)
    {
        //Calculate value for fit (angle, rho, significance)
        if (histogram_counts[j] > max.at(2))
        {
            if (costrain)
            {
                float y0 = (histogram_values[j]*rhoPrecision)/(sin(((i+1)*thetaPrecision*M_PI)/180));
                if (y0 > 0 && y0 <ymax)
                {
                    max.at(0) = i;
                    max.at(1) = histogram_values[j];
                    max.at(2) = histogram_counts[j];
                }
            }
            else
            {
                max.at(0) = i;
                max.at(1) = histogram_values[j];
                max.at(2) = histogram_counts[j];
            }
        }

        //Calculate max rho
        if (histogram_values[j] > max.at(3))
            max.at(3) = histogram_values[j];
    }

  }
}
