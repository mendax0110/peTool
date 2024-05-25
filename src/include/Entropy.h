#include <iostream>
#include <vector>

namespace EntropyInternals
{
    class Entropy
    {
    public:
        Entropy();
        ~Entropy();

        double calculateEntropy(const std::vector<uint8_t>& data);
        static std::vector<int> createHistogram(const std::vector<uint8_t>& data);
        static std::vector<double> calculateProbabilities(const std::vector<int>& histogram, size_t dataSize);
        void printEntropy(const std::vector<uint8_t>& data, size_t offset, size_t size);
    };
}