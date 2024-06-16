#include <iostream>
#include <vector>

/**
 * @brief Namespace for the Entropy class
 * @namespace EntropyInternals
 */
namespace EntropyInternals
{
    /**
     * @brief A class to calculate the entropy of data
     * @class Entropy
     */
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