#include "Hough.h"
#include "Rivelatore.h"
#include "Simulazione.h"

constexpr int thread = 1024;

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

__global__ void rho(float* ang, float* y, float *x, int* r, const float rhoPrecision, const int bigSize, const int smallSize)
{
  int idx = blockIdx.x * blockDim.x + threadIdx.x;
  int idy = blockIdx.y * blockDim.y + threadIdx.y;
  
  if(idx < bigSize && idy < smallSize)
  {
    r[(idx*smallSize) + idy] = int(  ((cos(ang[idx]*M_PI/180)*x[idy]) + (sin(ang[idx]*M_PI/180)*y[idy]))    /rhoPrecision);
  }
}

void Hough(std::vector<std::vector<std::vector<int>>> &values, std::vector<int> &max, std::vector<float> &yValueFloat, std::vector<float> &xValueFloat, const float thetaPrecision, const float rhoPrecision, const float ymax, bool costrain, const float limitAngle)
{
  float a[values.size()];
  std::vector<int> r(xValueFloat.size()*values.size(),0);

  //Fill a with angle values
  for(int i = 0; i < sizeof(a)/sizeof(*a); i++)
  {
    a[i] = (i*thetaPrecision)+limitAngle;
  }

  float* dev_a = nullptr;  
  float* dev_x = nullptr;
  float* dev_y = nullptr;
  int* dev_r = nullptr;
  cudaMalloc((void**)&dev_a, sizeof(a)/sizeof(*a) * sizeof(float));
  cudaMalloc((void**)&dev_x, xValueFloat.size() * sizeof(float));
  cudaMalloc((void**)&dev_y, xValueFloat.size() * sizeof(float));
  cudaMalloc((void**)&dev_r, xValueFloat.size() * values.size() * sizeof(float));
  cudaMemcpy(dev_a, a, sizeof(a)/sizeof(*a) * sizeof(float), cudaMemcpyHostToDevice);
  cudaMemcpy(dev_x, &(xValueFloat[0]), xValueFloat.size() * sizeof(float), cudaMemcpyHostToDevice);
  cudaMemcpy(dev_y, &(yValueFloat[0]), xValueFloat.size() * sizeof(float), cudaMemcpyHostToDevice);

  dim3 block(thread,1);
  dim3 grid((int(values.size())/thread)+1,int(xValueFloat.size()));

  rho<<<grid,block>>>(dev_a, dev_y, dev_x, dev_r, rhoPrecision, int(xValueFloat.size() * values.size()), int(xValueFloat.size()));

  /* 
  //Function to check cuda errors
  cudaError_t kErr=cudaGetLastError();
  if(kErr) // Se kErr==0, non ci sono errori
      printf("Errore %s\n",cudaGetErrorString(kErr));
  */

  // Copy output vector from GPU buffer to host memory.
  cudaMemcpy(&(r[0]), dev_r, xValueFloat.size() * values.size() * sizeof(int), cudaMemcpyDeviceToHost);

  cudaDeviceSynchronize();
  cudaFree(dev_a);
  cudaFree(dev_x);
  cudaFree(dev_y);
  cudaFree(dev_r);

  //std::chrono::high_resolution_clock::time_point time1 = std::chrono::high_resolution_clock::now(); 
  for(int i = 0; i < int(values.size()); i++)
  {
    thrust::device_vector<int> rhoVec(r.begin()+(i*int(xValueFloat.size())),r.begin()+(((i+1)*int(xValueFloat.size()))));

    //Detector for histogram
    thrust::device_vector<int> histogram_values;
    thrust::device_vector<int> histogram_counts;

    sparse_histogram(rhoVec, histogram_values, histogram_counts);

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
              float y0 = (((histogram_values[j]*rhoPrecision)+(rhoPrecision/2))/(sin(a[i]*M_PI/180)));
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
  //std::cout << duration(time1)*1e-6 << std::endl;
}

void calculateRho(std::vector<std::vector<std::vector<int>>> &values, std::vector<int> &max, std::vector<float> &yValueFloat, std::vector<float> &xValueFloat, const float thetaPrecision, const float rhoPrecision, const float ymax, bool costrain, const float limitAngle)
{
  thrust::device_vector<float> xValueFloatTrust(xValueFloat.begin(), xValueFloat.end());
  thrust::device_vector<float> yValueFloatTrust(yValueFloat.begin(), yValueFloat.end());

  //Termporari vectors to store x*cos and y*sin
  thrust::device_vector<float> xTemp(xValueFloatTrust.size());
  thrust::device_vector<float> yTemp(xValueFloatTrust.size());

  //Vector temporary containing non discreat values of rho
  thrust::device_vector<float> rhoTemp(xValueFloatTrust.size());

  //Vector containing fila rho discrete values
  thrust::device_vector<int> rho(xValueFloatTrust.size());

  float angle = 0;
  float angleRad = 0;

  for (int i = 0; i < int(values.size()); i++)
  {
  
    angle =  (i*thetaPrecision)+limitAngle;
    angleRad = (angle*M_PI)/180;

    //Calculation of cos(theta)*x
    thrust::transform(xValueFloatTrust.begin(), xValueFloatTrust.end(), xTemp.begin(), floatMultiplication(cos(angleRad)));

    //Calculation of sin(theta)*y
    thrust::transform(yValueFloatTrust.begin(), yValueFloatTrust.end(), yTemp.begin(), floatMultiplication(sin(angleRad)));

    //Calulate sum
    thrust::transform(xTemp.begin(), xTemp.end(), yTemp.begin(), rhoTemp.begin(), thrust::plus<float>());
    //print_vector("rho float", rhoTemp);

    //Calculate rho discrete
    thrust::transform(rhoTemp.begin(), rhoTemp.end(), rho.begin(), intDivision(rhoPrecision));
    //print_vector("rho", rho);

    //Detector for histogram
    thrust::device_vector<int> histogram_values;
    thrust::device_vector<int> histogram_counts;
    
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
                float y0 = (((histogram_values[j]*rhoPrecision)+(rhoPrecision/2))/(sin(angleRad)));
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
