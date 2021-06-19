#include "fpga_api.h"
#include <cstdio>
#include <cstring>

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#define DATA_SIZE SIZE*(SIZE+1)*sizeof(float) // fpga bram data size

#define min(x,y) (((x)<(y))?(x):(y))

FPGA::FPGA(off_t data_addr, off_t api_addr)
{
    fd_ = open("/dev/mem", O_RDWR);
    data_ = static_cast<float*>(mmap(NULL, DATA_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd_, data_addr));
    api_ = static_cast<unsigned int*>(mmap(NULL, sizeof(unsigned int), PROT_READ|PROT_WRITE, MAP_SHARED,fd_, api_addr));
}

FPGA::~FPGA()
{
    munmap(data_, DATA_SIZE );
    munmap(api_, sizeof(unsigned int));
    close(fd_);
}

float* FPGA::matrix_M1(void)
{
	return data_ ;
}

float* FPGA::matrix_M2(void)
{
	return data_ + SIZE * SIZE;
}

const float* __attribute__((optimize("O0"))) FPGA::run()
{
    *api_ = 0x5555;
    while(*api_ == 0x5555);

    return data_;    
}

// Test code for bitstream
void FPGA::largeMM(const float* weight_mat, const float* input_mat, float* output, 
							int num_input, int num_output, int num_matrix2)
{
	float* m1 = this->matrix_M1();
	float* m2 = this->matrix_M2();
	for(int i = 0; i < num_output*num_matrix2; ++i)
    output[i] = 0;

  for(int i = 0; i < num_output; i += SIZE)
  {
    for(int j = 0; j < num_input; j += SIZE)
    {			
      for(int k = 0; k < num_matrix2; k += SIZE)
      {
         // 0) Initialize input vector
        int block_row = min(SIZE, num_output-i);
        int block_col_1 = min(SIZE, num_input-j);
        int block_col_2 = min(SIZE, num_matrix2-k);

        // 1) Assign a m1
        // Implement This
        printf("\tAssigning m1 for %d, %d, %d\n", i, j, k);
        memset(m1, 0, SIZE * SIZE * sizeof(float));

        for(int m=0; m<block_row; m++){ 
          printf("\tmemcpy %d\n", m);
          memcpy(m1 + m*SIZE, weight_mat + (i+m)*num_input + j, block_col_1 * sizeof(float));
        }

        // 2) Assign a m2
        // IMPLEMENT THIS
        printf("\n\tAssigning m2 for %d, %d, %d\n", i, j, k);
        memset(m2, 0, SIZE * SIZE *sizeof(float));

        for(int n=0; n<block_col_1; n++){
          printf("\tmemcpy %d\n", n);
          memcpy(m2 + n*SIZE, input_mat + (j+n)*num_matrix2 + k, block_col_2 * sizeof(float));
        }

		// 3) Call a function `blockMM() to execute Matrix matrix multiplication
    printf("Calling function blockMM\n");

		const float* rst = this->run();

    // 4) Accumulate intermediate results
    // It is slightly different from the code for the project.
        printf("Accumulating\n");

		for(int n = 0; n<block_row; ++n)
        {
          for(int m = 0; m<block_col_2; ++m)
          {
            output[n*SIZE + m] += rst[n*SIZE + m];
          }
        }
		// 4) Accumulate intermediate results
        printf("\tk: %d finished\n", k);

 	  } 
	}
  }
}