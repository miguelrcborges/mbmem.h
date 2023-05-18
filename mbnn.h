#ifndef MBNN_H
#define MBNN_H

#include <immintrin.h>

#ifdef STATIC_NN_H
#define MBNN_DEF static
#else
#define MBNN_DEF extern
#endif

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct NNLayer NNLayer;
typedef struct NNOutput NNOutput;
typedef union GenericTarget GenericTarget;

static inline float _mm256_reduce_add_ps(__m256 x);
MBNN_DEF NNLayer *NN_Layer_create(unsigned int size);
MBNN_DEF NNOutput *NN_Output_create(unsigned int size);
MBNN_DEF void NN_Initialize_Neurons(NNLayer *base, float *values, unsigned int nElems);
MBNN_DEF void NN_Connect(NNLayer *base, GenericTarget target);
MBNN_DEF void NN_Compute(NNLayer *l);

union GenericTarget {
	NNLayer *nnlayer;
	NNOutput *nnoutput;
};

struct NNLayer {
	unsigned int nNeurons;
	unsigned int isOutput;
	__m256 *neurons;
	__m256 *weights;
	float *bias;
	GenericTarget target;
};

struct NNOutput {
	unsigned int nNeurons;
	unsigned int isOutput;
	__m256 *neurons;
};

#ifdef __cplusplus
}
#endif /* __cplusplus */


#ifdef MBNN_H_IMPLEMENTATIONS

#ifndef ALLOC
#define ALLOC(ALIGNMENT, AMOUNT) aligned_alloc(ALIGNMENT, AMOUNT)
#endif /* ALLOC */

inline float _mm256_reduce_add_ps(__m256 x) {
    const __m128 x128 = _mm_add_ps(_mm256_extractf128_ps(x, 1), _mm256_castps256_ps128(x));
    const __m128 x64 = _mm_add_ps(x128, _mm_movehl_ps(x128, x128));
    const __m128 x32 = _mm_add_ss(x64, _mm_shuffle_ps(x64, x64, 0x55));
    return _mm_cvtss_f32(x32);
}

NNLayer *NN_Layer_create(unsigned int size) {
	NNLayer *l = (NNLayer *)ALLOC(64, sizeof(NNLayer));
	if (size % 8 != 0) {
		size += 8 - size % 8;
	}
	l->neurons = (__m256 *)ALLOC(sizeof(__m256), (sizeof(__m256) / 8) * size);
	l->nNeurons = size;
	l->isOutput = 0;
	return l;
}

NNOutput *NN_Output_create(unsigned int size) {
	NNOutput *o = (NNOutput *)ALLOC(1, sizeof(NNLayer));
	if (size % 8 != 0) {
		size += 8 - size % 8;
	}
	o->neurons = (__m256 *)ALLOC(sizeof(__m256), (sizeof(__m256) / 8) * size);
	o->nNeurons = size;
	o->isOutput = 1;
	return o;
}

void NN_Connect(NNLayer *base, GenericTarget target) {
	base->target = target;
	unsigned int n_weights = base->nNeurons * target.nnoutput->nNeurons;
	base->weights = (__m256 *)ALLOC(sizeof(__m256), (sizeof(__m256) / 8) * n_weights);
	base->bias = (float *)ALLOC(sizeof(float), sizeof(float) * target.nnoutput->nNeurons);
}

void NN_Initialize_Neurons(NNLayer *base, float *values, unsigned int nElems) {
	for (size_t i = 0; i < nElems; i += 1) {
		base->neurons[i / 8][i % 8] = values[i];
	}
}

void NN_Compute(NNLayer *l) {
	const int avx_ops_per_neuron = l->nNeurons / 8;
	while (!l->isOutput) {
		for (size_t i = 0; i < l->target.nnlayer->nNeurons; ++i) {
			float result = l->bias[i];
			for (size_t ii = 0; ii < avx_ops_per_neuron; ii += 1) {
				result += _mm256_reduce_add_ps(_mm256_mul_ps(l->neurons[ii], l->weights[avx_ops_per_neuron * i + ii]));
			}
			l->target.nnlayer->neurons[i / 8][i % 8] = result;
		}
		l = l->target.nnlayer;
	}
}

#endif /* MBNN_H_IMPLEMENTATIONS */

#endif /* MBNN_H */
