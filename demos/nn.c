#define MBMEM_STATIC
#define MBNN_STATIC

#define MBMEM_H_IMPLEMENTATIONS
#include "../mbmem.h"

Arena arena;

#define ALLOC(ALIGNMENT, AMOUNT) Arena_nozero_alloc(&arena, AMOUNT, ALIGNMENT)
#define MBNN_H_IMPLEMENTATIONS
#include "../mbnn.h"

#include <stdio.h>

enum {
	OUTPUT_NEURONS_AMOUNT = 8,
	HIDDEN_LAYERS = 100,
	START_NEURONS_AMOUNT = 512,
} Neurons_Amount; 

enum {
	ARENA_SIZE = 1000000000,
};

int main(void) {
	char *arena_buf = malloc(ARENA_SIZE);

	if (arena_buf == NULL) {
	 	return 1;
	}

	Arena_init(&arena, arena_buf, ARENA_SIZE);
	NNLayer *start = NN_Layer_create(START_NEURONS_AMOUNT);
	NNLayer *cursor = start;

	for (size_t i = 0; i < HIDDEN_LAYERS; ++i) {
		NNLayer *tmp = NN_Layer_create(START_NEURONS_AMOUNT);
		NN_Connect(cursor, (GenericTarget) {.nnlayer = tmp});
		for (size_t i = 0; i < cursor->target.nnlayer->nNeurons; ++i) {
			cursor->bias[i] = 1.0f;
		}
		cursor = cursor->target.nnlayer;
	}

	NNOutput *output = NN_Output_create(OUTPUT_NEURONS_AMOUNT);
	NN_Connect(cursor, (GenericTarget) {.nnoutput = output});
	for (size_t i = 0; i < output->nNeurons; ++i) {
		cursor->bias[i] = 1.0f;
	}

	for (size_t i = 0; i < 100; ++i)
	NN_Compute(start);

	printf("Results:\n");
	for (size_t i = 0; i < OUTPUT_NEURONS_AMOUNT; ++i) {
		printf("Neuron %zu: %f\n", i, output->neurons[i/8][i%8]);
	}
	printf("\n");
}
