#include <iostream>
#include <vector>

/// @brief EntropyInternals namespace, which contains the Entropy class for calculating entropy of data \namespace EntropyInternals
namespace EntropyInternals
{
    /// @brief The Entropy class, a class for calculating the entropy of data \class Entropy
    class Entropy
    {
    public:

        /**
         * @brief Construct a new Entropy object
         */
        Entropy();

        /**
         * @brief Destroy the Entropy object
         */
        ~Entropy();

        /**
         * @brief Calculates the entropy of the given data.
         * @param data A vector of bytes representing the data.
         * @return A double representing the calculated entropy.
         */
        static double calculateEntropy(const std::vector<uint8_t>& data);

        /**
         * @brief Creates a histogram of byte frequencies from the given data.
         * @param data A vector of bytes representing the data.
         * @return A vector of integers representing the histogram of byte frequencies.
         */
        static std::vector<int> createHistogram(const std::vector<uint8_t>& data);

        /**
         * @brief Calculates the probabilities of each byte in the histogram.
         * @param histogram A vector of integers representing the histogram of byte frequencies.
         * @param dataSize The size of the data used to create the histogram.
         * @return A vector of doubles representing the probabilities of each byte.
         */
        static std::vector<double> calculateProbabilities(const std::vector<int>& histogram, size_t dataSize);

        /**
         * @brief Prints the entropy of the data at a specific offset and size.
         * @param data A vector of bytes representing the data.
         * @param offset The offset in the data where to start calculating entropy.
         * @param size The size of the data segment to calculate entropy for.
         */
        static void printEntropy(const std::vector<uint8_t>& data, size_t offset, size_t size);
    };
}