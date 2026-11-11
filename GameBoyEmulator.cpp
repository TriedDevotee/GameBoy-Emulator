#include <iostream>
#include <vector>

int findPrimes() {
    int count = 2;
    std::vector<int> primes;

    while (true){
        bool isPrime = true;
        for (int i = 0; i < primes.size(); i++) {
            if (count % primes[i] == 0) {
                isPrime = false;
            }
        }

        if (isPrime == true) {
            primes.push_back(count);
            std::cout << count << "\n";
        }

        count++;
    }
}

int main()
{
    std::cout << "Hello World!\n";

    findPrimes();
}

