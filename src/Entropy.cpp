#include "../include/VIEW/Entropy.h"

#include <iostream>
#include <vector>
#include <cmath>

using namespace EntropyInternals;

Entropy::Entropy() = default;

Entropy::~Entropy() = default;

/**
 * @brief Calculate the entropy of a given data
 * @param data The data to calculate the entropy
 * @return The entropy of the data
 */
double Entropy::calculateEntropy(const std::vector<uint8_t>& data)
{
    auto histogram = createHistogram(data);
    auto dataSize = data.size();
    auto probabilities = calculateProbabilities(histogram, dataSize);
    double entropy = 0.0;
    for (const auto& probability : probabilities)
    {
        if (probability > 0.0)
        {
            entropy -= probability * std::log2(probability);
        }
    }
    return entropy;
}

/**
 * @brief Create a histogram of the data
 * @param data The data to create the histogram
 * @return The histogram of the data
 */
std::vector<int> Entropy::createHistogram(const std::vector<uint8_t>& data)
{
    std::vector<int> histogram(256, 0);
    for (uint8_t byte : data)
    {
        histogram[byte]++;
    }
    return histogram;
}

/**
 * @brief Calculate the probabilities of the histogram
 * @param histogram The histogram to calculate the probabilities
 * @param dataSize The size of the data
 * @return The probabilities of the histogram
 */
std::vector<double> Entropy::calculateProbabilities(const std::vector<int>& histogram, size_t dataSize)
{
    std::vector<double> probabilities;
    probabilities.reserve(histogram.size());
    for (int count : histogram)
    {
        probabilities.push_back(static_cast<double>(count) / dataSize);
    }
    return probabilities;
}

/**
 * @brief Print the entropy of the data
 * @param data The data to print the entropy
 */
void Entropy::printEntropy(const std::vector<uint8_t>& data, size_t offset, size_t size)
{
    if (offset + size > data.size())
    {
        std::cerr << "Invalid offset and size." << std::endl;
        return;
    }

    std::cout << "Name\tOffset\tSize\tEntropy\tStatus" << std::endl;
    std::cout << "----\t------\t----\t-------\t------" << std::endl;
    for (size_t i = 0; i < size; i++)
    {
        std::vector<uint8_t> subData(data.begin() + offset + i, data.begin() + offset + i + 1);
        double entropy = calculateEntropy(subData);
        std::string status = entropy < 7.0 ? "Low" : "High";
        std::cout << "Byte " << i << "\t" << offset + i << "\t1\t" << entropy << "\t" << status << std::endl;
    }
}