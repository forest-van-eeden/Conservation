// compile: clang e_prime_sequence.c -o e_prime_sequence -lm
// execute ./e_prime_sequence

#include <stdio.h>
#include <math.h>

// Function to check if a number is prime
// Returns 1 if prime, 0 otherwise
int isPrime(long long n) {
    if (n <= 1) return 0;
    if (n == 2) return 1;
    if (n % 2 == 0) return 0;
    
    for (long long i = 3; i * i <= n; i += 2) {
        if (n % i == 0) return 0;
    }
    
    return 1;
}

int main() {
    double e = M_E;
	
    printf("Calculating the sequence i * e for i from 1 to 530:\n\n");

	    for (int i = 1; i <= 530; ++i) {
	        double result = i * e;
	        long long result_floored = (long long)floor(result);
	        
	        if (isPrime(result_floored)) {
            printf("%d * e = %f -> Rounded down to %lld.\n", 
                   i, result, result_floored);
        	}
	    }
    return 0;
}
