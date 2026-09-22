/*
 * This file is part of INAV.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <cmath>
#include <limits>
#include <gtest/gtest.h>

extern "C" {
#include "flight/fw_tune_model.h"
}

namespace {
// Independent continuous-time two-lag plant integrated at 1 kHz.
// The estimator sees only 50 Hz actuator commands and observations.
struct Plant {
    double servo = 0;
    double rate = 0;
    double update(double command) {
        for (unsigned i = 0; i < 20; i++) {
            servo += (command - servo) * (1.0 - std::exp(-0.001 / 0.055));
            rate += (2.0 * servo - rate) * (1.0 - std::exp(-0.001 / 0.22));
        }
        return rate;
    }
};

float excitation(unsigned sample) {
    return 0.2f * std::sin(sample * 0.13f) + 0.12f * std::sin(sample * 0.61f) +
        0.06f * std::sin(sample * 1.23f);
}
}

TEST(FwTuneModel, IdentifiesIndependentContinuousPlantAndPredictsHeldOutInput)
{
    fwTuneModelFit_t fit;
    fwTuneModelInit(&fit, 0.02f);
    Plant plant;
    float observation = 0;
    for (unsigned i = 0; i < 1500; i++) {
        const float input = excitation(i);
        ASSERT_TRUE(fwTuneModelAdd(&fit, input, observation));
        observation = plant.update(input);
    }
    fwTuneModel_t model = {};
    ASSERT_TRUE(fwTuneModelSolve(&fit, &model));
    float gain = 0;
    ASSERT_TRUE(fwTuneModelStaticGain(&model, &gain));
    EXPECT_NEAR(gain, 2.0f, 0.005f);

    plant = Plant();
    observation = 0;
    fwTuneModelState_t state = {};
    for (unsigned i = 0; i < 1000; i++) {
        const float input = i % 150 < 75 ? 0.3f : -0.2f;
        const float prediction = fwTuneModelPredict(&model, &state, input);
        ASSERT_NEAR(prediction, observation, 0.005f) << "sample " << i;
        observation = plant.update(input);
    }
}

TEST(FwTuneModel, FitsConstantObservationOffsetWithoutChangingStaticGain)
{
    fwTuneModelFit_t fit;
    fwTuneModelInit(&fit, 0.02f);
    Plant plant;
    float observation = 0;
    for (unsigned i = 0; i < 2000; i++) {
        const float input = excitation(i);
        ASSERT_TRUE(fwTuneModelAdd(&fit, input, observation + 0.4f));
        observation = plant.update(input);
    }
    fwTuneModel_t model = {};
    ASSERT_TRUE(fwTuneModelSolve(&fit, &model));
    float gain = 0;
    ASSERT_TRUE(fwTuneModelStaticGain(&model, &gain));
    EXPECT_NEAR(gain, 2.0f, 0.01f);
    EXPECT_NEAR(model.offset / (1 + model.a[0] + model.a[1]), 0.4f, 0.005f);
}

TEST(FwTuneModel, RejectsInsufficientAndConstantExcitationWithoutWritingResult)
{
    fwTuneModelFit_t fit;
    fwTuneModelInit(&fit, 0.02f);
    fwTuneModel_t model = {};
    model.offset = 42;
    EXPECT_FALSE(fwTuneModelSolve(&fit, &model));
    for (unsigned i = 0; i < 500; i++) {
        ASSERT_TRUE(fwTuneModelAdd(&fit, 0.2f, 0.4f));
    }
    EXPECT_FALSE(fwTuneModelSolve(&fit, &model));
    EXPECT_EQ(model.offset, 42);
}

TEST(FwTuneModel, InvalidSamplesLatchFailureUntilReset)
{
    for (float value : {std::numeric_limits<float>::quiet_NaN(),
                       std::numeric_limits<float>::infinity(), -10001.0f}) {
        fwTuneModelFit_t fit;
        fwTuneModelInit(&fit, 0.02f);
        EXPECT_FALSE(fwTuneModelAdd(&fit, value, 0));
        EXPECT_FALSE(fwTuneModelAdd(&fit, 0, 0));
        fwTuneModelInit(&fit, 0.02f);
        EXPECT_TRUE(fwTuneModelAdd(&fit, 0, 0));
        EXPECT_FALSE(fwTuneModelAdd(&fit, 0, value));
    }
}

TEST(FwTuneModel, RejectsInvalidSamplePeriod)
{
    for (float period : {0.0f, -0.02f, std::numeric_limits<float>::quiet_NaN()}) {
        fwTuneModelFit_t fit;
        fwTuneModelInit(&fit, period);
        EXPECT_FALSE(fwTuneModelAdd(&fit, 0, 0));
    }
}

TEST(FwTuneModel, RejectsMarginalUnstableAndReversedPlants)
{
    fwTuneModel_t model = {{-1.5f, 0.56f}, {0.03f, 0.03f, 0}, 0, 0.02f};
    EXPECT_TRUE(fwTuneModelIsStable(&model));
    float gain = 42;
    EXPECT_TRUE(fwTuneModelStaticGain(&model, &gain));
    EXPECT_NEAR(gain, 1, 0.00001f);
    model.b[0] = -0.1f;
    EXPECT_FALSE(fwTuneModelStaticGain(&model, &gain));
    model.a[0] = -1.6f;
    model.a[1] = 0.6f;
    EXPECT_FALSE(fwTuneModelIsStable(&model));
    model.a[0] = -2.0f;
    EXPECT_FALSE(fwTuneModelIsStable(&model));
    model.a[0] = 2.0f;
    EXPECT_FALSE(fwTuneModelIsStable(&model));
    model.a[0] = 0;
    model.a[1] = 1;
    EXPECT_FALSE(fwTuneModelIsStable(&model));
}

TEST(FwTuneModel, IndependentValidationDetectsWrongGainAndLongDelay)
{
    fwTuneModelFit_t fit;
    fwTuneModelInit(&fit, 0.02f);
    Plant plant;
    float observation = 0;
    for (unsigned i = 0; i < 1500; i++) {
        const float input = excitation(i);
        ASSERT_TRUE(fwTuneModelAdd(&fit, input, observation));
        observation = plant.update(input);
    }
    fwTuneModel_t model = {};
    ASSERT_TRUE(fwTuneModelSolve(&fit, &model));

    for (unsigned scenario = 0; scenario < 3; scenario++) {
        fwTuneModelValidation_t validation;
        fwTuneModelValidationInit(&validation, &model);
        plant = Plant();
        observation = 0;
        float history[10] = {};
        for (unsigned i = 0; i < 1000; i++) {
            const float input = excitation(i + 3000);
            const float measured = scenario == 1 ? observation * 1.5f : observation;
            ASSERT_TRUE(fwTuneModelValidateSample(&validation, input, measured));
            const float delayed = history[i % 10];
            history[i % 10] = input;
            observation = plant.update(scenario == 2 ? delayed : input);
        }
        float error = -1;
        ASSERT_TRUE(fwTuneModelValidationError(&validation, &error));
        if (scenario == 0) {
            EXPECT_LT(error, 0.005f);
        } else {
            EXPECT_GT(error, 0.2f);
        }
    }
}

TEST(FwTuneModel, ValidationNeedsIndependentVariationAndFiniteObservations)
{
    const fwTuneModel_t model = {{-1.5f, 0.56f}, {0.03f, 0.03f, 0}, 0, 0.02f};
    fwTuneModelValidation_t validation;
    fwTuneModelValidationInit(&validation, &model);
    float error = 42;
    EXPECT_FALSE(fwTuneModelValidationError(&validation, &error));
    for (unsigned i = 0; i < 500; i++) {
        ASSERT_TRUE(fwTuneModelValidateSample(&validation, 0, 0));
    }
    EXPECT_FALSE(fwTuneModelValidationError(&validation, &error));
    EXPECT_EQ(error, 42);
    EXPECT_FALSE(fwTuneModelValidateSample(&validation, 0, std::numeric_limits<float>::infinity()));
    EXPECT_FALSE(fwTuneModelValidateSample(&validation, 0, 0));
}

TEST(FwTuneModel, ReversedResponseDoesNotProduceUsableModel)
{
    fwTuneModelFit_t fit;
    fwTuneModelInit(&fit, 0.02f);
    Plant plant;
    float observation = 0;
    for (unsigned i = 0; i < 1500; i++) {
        const float input = excitation(i);
        ASSERT_TRUE(fwTuneModelAdd(&fit, input, -observation));
        observation = plant.update(input);
    }
    fwTuneModel_t model = {};
    EXPECT_FALSE(fwTuneModelSolve(&fit, &model));
}
