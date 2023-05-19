#define MBMEM_STATIC
#define MBNN_STATIC

#define MBMEM_H_IMPLEMENTATIONS
#include "../mbmem.h"

Arena arena;

#define ALLOC(ALIGNMENT, AMOUNT) Arena_alloc(&arena, AMOUNT, ALIGNMENT)
#define MBNN_H_IMPLEMENTATIONS
#include "../mbnn.h"

#include <stdio.h>

enum {
	OUTPUT_NEURONS_AMOUNT = 8,
	START_NEURONS_AMOUNT = 1024,
} Neurons_Amount; 

enum {
	ARENA_SIZE = 1048576,
};

int main(void) {
	char *arena_buf = malloc(ARENA_SIZE);
	if (arena_buf == NULL) {
		return 1;
	}
	Arena_init(&arena, arena_buf, ARENA_SIZE);

	NNLayer *start = NN_Layer_create(START_NEURONS_AMOUNT);
	NNOutput *output = NN_Output_create(OUTPUT_NEURONS_AMOUNT);
	NN_Connect(start, (GenericTarget) {.nnoutput = output});

	for (size_t i = 0; i < START_NEURONS_AMOUNT; ++i) {
		start->bias[i] = (float) i;
	}

	for (size_t i = 0; i < START_NEURONS_AMOUNT * OUTPUT_NEURONS_AMOUNT; ++i) {
		start->weights[i / 8][i % 8] = 0.1f;
	}

	NN_Compute(start);

	printf("Results:\n");
	for (size_t i = 0; i < OUTPUT_NEURONS_AMOUNT; ++i) {
		printf("Neuron %zu: %f\n", i, output->neurons[i/8][i%8]);
	}
	printf("\n");
}
