// Jerrod Eanes Project #1 EEE 4334 CAD for VLSI
// Implementation of QM 
// Can handle multiple outputs
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_TERMS 256
#define MAX_BITS 32

typedef struct {
    char bits[MAX_BITS + 1];
    int combined;
} Term;

//Reads the input file of the program
void read_pla(const char *filename, int *num_inputs, int *num_outputs, int *num_terms, Term terms[], char outputs[MAX_TERMS][MAX_BITS]);
//Writes the output file of the program
void write_pla(const char *filename, int num_inputs, int num_outputs, int num_primes, Term prime_implicants[], char outputs[MAX_TERMS][MAX_BITS]);
//Calculates how many bits are different between 2 terms
int hamming_distance(const char *a, const char *b);
//combines terms with hamming distance of 1 
void combine_terms(Term *a, Term *b, Term *result);
//Main prime implicant generation function which does the comparing of 2 terms for reduction
void find_prime_implicants(int num_terms, Term terms[], int num_inputs, Term prime_implicants[], int *num_primes);
//check to see if all the inputed terms are coverd
int covers(const char *term, const char *implicant);
//implements the 2nd part of QM by seeing if terms are better than others after generation
void minimize_terms(int num_terms, Term terms[], int num_primes, Term prime_implicants[], int *num_final_primes);

int main() {
    int num_inputs, num_outputs, num_terms;
    Term terms[MAX_TERMS];
    char outputs[MAX_TERMS][MAX_BITS];

    read_pla("input.pla", &num_inputs, &num_outputs, &num_terms, terms, outputs);

    Term prime_implicants[MAX_TERMS];
    int num_primes = 0;

    find_prime_implicants(num_terms, terms, num_inputs, prime_implicants, &num_primes);

    write_pla("output.pla", num_inputs, num_outputs, num_primes, prime_implicants, outputs);

    printf("Minimized PLA written to ouput\n");
    return 0;
}

void read_pla(const char *filename, int *num_inputs, int *num_outputs, int *num_terms, Term terms[], char outputs[MAX_TERMS][MAX_BITS]) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char line[256];
    *num_terms = 0;

    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '.') {
            if (strncmp(line, ".i", 2) == 0) {
                *num_inputs = atoi(line + 3);
            } else if (strncmp(line, ".o", 2) == 0) {
                *num_outputs = atoi(line + 3);
            }
        } else if (line[0] != '\n' && line[0] != '#') {
            sscanf(line, "%s %s", terms[*num_terms].bits, outputs[*num_terms]);
            terms[*num_terms].combined = 0;
            (*num_terms)++;
        }
    }

    fclose(file);
}

void write_pla(const char *filename, int num_inputs, int num_outputs, int num_primes, Term prime_implicants[], char outputs[MAX_TERMS][MAX_BITS]) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    fprintf(file, ".i %d\n", num_inputs);
    fprintf(file, ".o %d\n", num_outputs);
    fprintf(file, ".p %d\n", num_primes);

    for (int i = 0; i < num_primes; i++) {
        fprintf(file, "%s %s\n", prime_implicants[i].bits, outputs[i]);
    }

    fprintf(file, ".e\n");
    fclose(file);
}
int hamming_distance(const char *a, const char *b) {
    int diff = 0;
    while (*a && *b) {
        if (*a != *b && (*a != '-' || *b != '-')) diff++;
        a++;
        b++;
    }
    return diff;
}

void combine_terms(Term *a, Term *b, Term *result) {
    for (int i = 0; a->bits[i] && b->bits[i]; i++) {
        result->bits[i] = (a->bits[i] == b->bits[i]) ? a->bits[i] : '-';
    }
    result->bits[strlen(a->bits)] = '\0';
    a->combined = b->combined = 1;
    result->combined = 0;
}

void find_prime_implicants(int num_terms, Term terms[], int num_inputs, Term prime_implicants[], int *num_primes) {
    Term next_terms[MAX_TERMS];
    int next_count;

    do {
        next_count = 0;
        for (int i = 0; i < num_terms; i++) {
            for (int j = i + 1; j < num_terms; j++) {
                if (hamming_distance(terms[i].bits, terms[j].bits) == 1) {
                    combine_terms(&terms[i], &terms[j], &next_terms[next_count++]);
                }
            }
        }

        // Add uncombined terms to the prime implicants
        for (int i = 0; i < num_terms; i++) {
            if (!terms[i].combined) {
                prime_implicants[*num_primes] = terms[i];
                (*num_primes)++;
            }
        }

        // Move next_terms to terms for the next iteration
        num_terms = next_count;
        for (int i = 0; i < num_terms; i++) {
            terms[i] = next_terms[i];
        }

    } while (num_terms > 0);
}

int covers(const char *term, const char *implicant) {
    // Check if the term is covered by the implicant
    for (int i = 0; term[i]; i++) {
        if (implicant[i] != '-' && term[i] != implicant[i]) {
            return 0;  // Term is not covered
        }
    }
    return 1;  // Term is covered
}

void minimize_terms(int num_terms, Term terms[], int num_primes, Term prime_implicants[], int *num_final_primes) {
    // Remove redundant terms that are covered by another implicant
    for (int i = 0; i < num_primes; i++) {
        int is_redundant = 0;
        for (int j = 0; j < num_terms; j++) {
            if (covers(terms[j].bits, prime_implicants[i].bits)) {
                is_redundant = 1;
                break;
            }
        }
        if (!is_redundant) {
            prime_implicants[*num_final_primes] = prime_implicants[i];
            (*num_final_primes)++;
        }
    }
}
