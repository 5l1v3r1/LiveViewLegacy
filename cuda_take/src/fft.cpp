//Written by Noah
#include "fft.hpp"
#include <assert.h>
#include <iostream>
#define printComp(a) printf("%f+i%f,",creal(a),cimag(a))
#define SWAPC(a,b) tempc = a; a = b; b = tempc;
#define IS_POW2(n) (n & (n - 1)) == 0
#define BIT_REVERSE(v,BASE) ( (unsigned int)( (BitReverseTable256[v & 0xff] << 24) | \
		(BitReverseTable256[(v >> 8) & 0xff] << 16) | \
		(BitReverseTable256[(v >> 16) & 0xff] << 8) | \
		(BitReverseTable256[(v >> 24) & 0xff])  )) >> (32-BASE)

std::complex<float> I(0,1);
std::complex<float> tempc;

static const unsigned char BitReverseTable256[] =
{
		0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
		0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
		0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
		0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
		0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
		0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
		0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
		0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
		0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
		0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
		0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
		0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
		0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
		0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
		0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
		0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF
};
fft::fft()
{
    /*! \brief Allocates memory for the complex array copy of the input series. */
	CFFT = new std::complex<float>[MAX_FFT_SIZE];
}
fft::~fft() {
	delete CFFT;
}
void fft::bitReverseOrder(std::complex<float> * arr, unsigned int len) //Overloaded for
{
    /*! \brief Standard method for reversing the bits of a 2-byte number.
     */
	unsigned int base = (int)ceil(log10((float)len)/log10(2.0));
	unsigned int i = 0;
	//printf("base %u\n",base);
	for(i = 0; i < len; i++)
	{
		unsigned int j = BIT_REVERSE(i,base);
		if(i < j)
		{
			// printf("b4 swapc");
			SWAPC(arr[i],arr[j]);
		}
	}
}

void fft::doRealFFT(float * real_arr, unsigned int ring_head,float *fft_real_result)
{
    /*! \brief Topmost function for calculating the FFT of the time series.
     * \param real_arr The input series to the function.
     * \param ring_head The current position in the ring buffer, if applicable.
     * \param fft_real_result The output of real FFT magnitudes.
     */
	CFFT  =  doFFT(real_arr, ring_head);
	double max = 0;
	
	//std::cout<<"Ring head late= "<< ring_head<< std::endl;
	//realFFT[len/2] = (float) std::real(CFFT[len/2]);
	for(unsigned int i = 0; i < FFT_INPUT_LENGTH/2; i++)
	{
		fft_real_result[i] = std::abs(CFFT[i]);
		if(fft_real_result[i] > max && i !=0)
		{
            max = fft_real_result[i];
		}
	}
	//printf("max nonconst in fft:%f\n",max);


}
std::complex<float> * fft::doFFT(float * real_arr, unsigned int ring_head)
{
    /*! \brief Middle layer function which converts the series to a complex-valued array.
     *
     * Also passes the array to the method which actually calculates the FFT of the complex input
     * array.
     */
	for(unsigned int i = 0; i < FFT_INPUT_LENGTH; i++)
	{
        CFFT[i] = std::complex<float>(real_arr[(ring_head+i) % FFT_MEAN_BUFFER_LENGTH],0);
	}
	return doFFT(CFFT,FFT_INPUT_LENGTH);
}
std::complex<float> * fft::doFFT(std::complex<float> * arr, unsigned int len)
{
    /*! \brief Calculate the FFT on the complex input array
     *
     * This is a textbook method for calculating the FFT. The function asserts that the input array has a length that
     * is a power of 2 and the length is less than the maximum allowed length.
     * \param arr Complex form of the input time series.
     * \param len Number of elements in the array parameter, here set to the FFT_INPUT_LENGTH macro.
     */
    assert(IS_POW2(len));
    assert(len <= MAX_FFT_SIZE);
	bitReverseOrder(arr,len);
	//printf("doing fft\n");
	unsigned int base = (int)ceil(log10((float)len)/log10(2.0));

	std::complex<float> Wn = std::exp(std::complex<float>(0,-1.0f*2*M_PI/len)); //For twiddle factors

	for(unsigned int stage = 1; stage <= base; stage++)
	{
		unsigned int N = pow(2,stage);
		//float complex Wn = cexp(((float complex)-2*M_PI)*I/N);
		//printf("on stage:%u N=%u Wn=%f+i%f\n",stage,N,creal(Wn),cimag(Wn));
		for(unsigned int index = 0; index < len; index+=N)
		{
			for(unsigned int r = 0; r < N/2; r++)
			{
				std::complex<float> a = arr[index+r]; //Save temp values to avoid data availability problem
				std::complex<float> b = arr[index+r+N/2];
				std::complex<float> exponent =(len/N)*r;
				std::complex<float> twiddle = std::pow(Wn,exponent); //This twiddle is equivalent to global N twiddle seen in some books (i.e. W_8^2 = W_4^1)
				arr[index+r] = a + b*twiddle;
				arr[index+r+N/2] = a - b*twiddle;
			}

		}
	}
	return arr;
}
