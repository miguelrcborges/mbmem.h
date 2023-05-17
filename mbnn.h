#ifndef NN_H
#define NN_H

#include <immintrin.h>

#ifdef STATIC_NN_H
#define MBNN_DEF static
#else
#define MBNN_DEF extern
#endif

typedef struct NNLayer NNLayer;
typedef struct NNOutput NNOutput;
typedef union GenericTarget GenericTarget;

MBNN_DEF inline float _mm256_reduce_add_ps(__m256 x);
MBNN_DEF NNLayer *NNLayer_create(unsigned int size, float weights[], float bias[]);
MBNN_DEF void NNConnect(NNLayer *base, GenericTarget target);

union GenericTarget {
	NNLayer *nnlayer;
	NNOutput *nnoutput;
};

struct NNLayer {
	unsigned int nNeurons;
	__m256 *neurons;
	__m256 *weights;
	__m256 *bias;
	GenericTarget target;
};

struct NNOutput {
	unsigned nNeurons;
	__m256 *neurons;
};


#ifdef NN_H_IMPLEMENTATIONS

inline float _mm256_reduce_add_ps(__m256 x) {
    const __m128 x128 = _mm_add_ps(_mm256_extractf128_ps(x, 1), _mm256_castps256_ps128(x));
    const __m128 x64 = _mm_add_ps(x128, _mm_movehl_ps(x128, x128));
    const __m128 x32 = _mm_add_ss(x64, _mm_shuffle_ps(x64, x64, 0x55));
    return _mm_cvtss_f32(x32);
}


NNLayer *NNLayer_create(unsigned int size, float weights[], float bias[]) {
	NNLayer *l = (NNLayer *)malloc(sizeof(NNLayer));
	size += 8 - size % 8;
	l->neurons = (__m256 *)malloc(size / 8);
	l->nNeurons = size;
	return l;
}

void NNConnect(NNLayer *base, GenericTarget target) {
	base->target = target;
	unsigned int n = sizeof(__m256) * (target.nnoutput->nNeurons / 8 + (target.nnoutput->nNeurons % 8 != 0));
	base->weights = (__m256 *)malloc(n);
	base->bias = (__m256 *)malloc(n);
}

#endif /* NN_H_IMPLEMENTATIONS */

#endif /* NN_H */
