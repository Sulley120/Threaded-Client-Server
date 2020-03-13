struct args {
	struct Stack *new;
	int connection;
	long long array[100];
};

struct Stack {
	long long top;
	unsigned int capacity;
	long long *array;
};

void push(struct Stack *stack, long long item);

int isFull(struct Stack *stack);

int isEmpty(struct Stack *stack);

long long pop(struct Stack *stack);

struct Stack *createStack(unsigned int capacity);

void *peon(void *args);

void handle(int arg);