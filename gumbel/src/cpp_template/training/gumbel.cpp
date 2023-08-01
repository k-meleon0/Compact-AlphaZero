#include <random>
#include "macro.cpp"

/* ----------------------------------------------------------------------------------------
----------------------------------------- GUMBEL ------------------------------------------
------------------------------------------------------------------------------------------- */

// Sample k variables from the Gumbel distribution
void sample_gumbel(float gumbel_samples[], int number_of_actions) {
    static std::random_device rd;
    static std::mt19937 gen(rd());

    std::extreme_value_distribution<> Gumbel_0 {0, 1};

    for (int i = 0; i < number_of_actions; ++i) {
        gumbel_samples[i] = Gumbel_0(gen);
    }
}