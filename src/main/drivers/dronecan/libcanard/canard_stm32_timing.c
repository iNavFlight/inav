#include "canard_stm32_timing.h"

bool canardComputeCanTimingSolution(uint32_t pclk, uint32_t target_bitrate, CanardCanTimingSolution *out_solution)
{
    if (target_bitrate < 1) {
        return false;
    }

    static const int MaxBS1 = 16;
    static const int MaxBS2 = 8;

    /*
     * Ref. "Automatic Baudrate Detection in CANopen Networks", U. Koppe, MicroControl GmbH & Co. KG
     *      CAN in Automation, 2003
     *
     * According to the source, optimal quanta per bit are:
     *   Bitrate        Optimal Maximum
     *   1000 kbps      8       10
     *   500  kbps      16      17
     *   250  kbps      16      17
     *   125  kbps      16      17
     */
    const int max_quanta_per_bit = (target_bitrate >= 1000000) ? 10 : 17;
    static const int MaxSamplePointLocation = 900;

    /*
     * Computing (prescaler * BS):
     *   BITRATE = 1 / (PRESCALER * (1 / PCLK) * (1 + BS1 + BS2))       -- See the Reference Manual
     *   BITRATE = PCLK / (PRESCALER * (1 + BS1 + BS2))                 -- Simplified
     * let:
     *   BS = 1 + BS1 + BS2                                             -- Number of time quanta per bit
     *   PRESCALER_BS = PRESCALER * BS
     * ==>
     *   PRESCALER_BS = PCLK / BITRATE
     */
    const uint32_t prescaler_bs = pclk / target_bitrate;

    /*
     * Searching for such prescaler value so that the number of quanta per bit is highest.
     */
    uint8_t bs1_bs2_sum = (uint8_t)(max_quanta_per_bit - 1);

    while ((prescaler_bs % (1 + bs1_bs2_sum)) != 0) {
        if (bs1_bs2_sum <= 2) {
            return false;          // No solution
        }
        bs1_bs2_sum--;
    }

    const uint32_t prescaler = prescaler_bs / (1 + bs1_bs2_sum);
    if ((prescaler < 1U) || (prescaler > 1024U)) {
        return false;              // No solution
    }

    /*
     * Now we have a constraint: (BS1 + BS2) == bs1_bs2_sum.
     * We need to find the values so that the sample point is as close as possible to the optimal value.
     *
     *   Solve[(1 + bs1)/(1 + bs1 + bs2) == 7/8, bs2]  (* Where 7/8 is 0.875, the recommended sample point location *)
     *   {{bs2 -> (1 + bs1)/7}}
     *
     * Hence:
     *   bs2 = (1 + bs1) / 7
     *   bs1 = (7 * bs1_bs2_sum - 1) / 8
     *
     * Sample point location can be computed as follows:
     *   Sample point location = (1 + bs1) / (1 + bs1 + bs2)
     *
     * Since the optimal solution is so close to the maximum, we prepare two solutions, and then pick the best one:
     *   - With rounding to nearest
     *   - With rounding to zero
     */
    struct BsPair {
        uint8_t bs1;
        uint8_t bs2;
        uint16_t sample_point_permill;
    } solution;

    // First attempt with rounding to nearest
    solution.bs1 = (uint8_t)(((7 * bs1_bs2_sum - 1) + 4) / 8);
    solution.bs2 = (uint8_t)(bs1_bs2_sum - solution.bs1);
    solution.sample_point_permill = (uint16_t)(1000 * (1 + solution.bs1) / (1 + solution.bs1 + solution.bs2));

    if (solution.sample_point_permill > MaxSamplePointLocation) {
        // Second attempt with rounding to zero
        solution.bs1 = (uint8_t)((7 * bs1_bs2_sum - 1) / 8);
        solution.bs2 = (uint8_t)(bs1_bs2_sum - solution.bs1);
        solution.sample_point_permill = (uint16_t)(1000 * (1 + solution.bs1) / (1 + solution.bs1 + solution.bs2));
    }

    /*
     * Final validation
     * Helpful Python:
     * def sample_point_from_btr(x):
     *     assert 0b0011110010000000111111000000000 & x == 0
     *     ts2,ts1,brp = (x>>20)&7, (x>>16)&15, x&511
     *     return (1+ts1+1)/(1+ts1+1+ts2+1)
     *
     */
    if ((target_bitrate != (pclk / (prescaler * (1 + solution.bs1 + solution.bs2)))) ||
        !((solution.bs1 >= 1) && (solution.bs1 <= MaxBS1) &&
          (solution.bs2 >= 1) && (solution.bs2 <= MaxBS2))) {
        return false;
    }

    out_solution->prescaler = (uint16_t)prescaler;
    out_solution->bs1 = solution.bs1;
    out_solution->bs2 = solution.bs2;

    return true;
}
