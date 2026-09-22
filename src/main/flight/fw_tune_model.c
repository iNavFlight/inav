/*
 * This file is part of INAV.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <math.h>
#include <string.h>

#include "flight/fw_tune_model.h"

#define FW_TUNE_MODEL_MIN_SAMPLES 100
#define FW_TUNE_MODEL_MAX_SAMPLES 10000

void fwTuneModelInit(fwTuneModelFit_t *fit, float samplePeriod)
{
    memset(fit, 0, sizeof(*fit));
    fit->samplePeriod = samplePeriod;
    fit->invalid = !isfinite(samplePeriod) || samplePeriod <= 0;
}

bool fwTuneModelAdd(fwTuneModelFit_t *fit, float input, float output)
{
    if (fit->invalid || !isfinite(input) || !isfinite(output) ||
        fabsf(input) > 10000.0f || fabsf(output) > 10000.0f ||
        fit->samples >= FW_TUNE_MODEL_MAX_SAMPLES) {
        fit->invalid = true;
        return false;
    }

    if (fit->history >= 3) {
        float row[FW_TUNE_MODEL_TERMS] = {
            -fit->y[0], -fit->y[1], fit->u[0], fit->u[1], fit->u[2], 1.0f,
        };
        float residual = output;

        // Incremental QR avoids squaring the condition number through X'X.
        // The intercept fits a constant trim offset without altering the dynamics.
        for (unsigned i = 0; i < FW_TUNE_MODEL_TERMS; i++) {
            const float magnitude = hypotf(fit->r[i][i], row[i]);
            if (magnitude == 0) {
                continue;
            }
            if (!isfinite(magnitude)) {
                fit->invalid = true;
                return false;
            }
            const float c = fit->r[i][i] / magnitude;
            const float s = row[i] / magnitude;
            fit->r[i][i] = magnitude;
            for (unsigned j = i + 1; j < FW_TUNE_MODEL_TERMS; j++) {
                const float value = c * fit->r[i][j] + s * row[j];
                row[j] = -s * fit->r[i][j] + c * row[j];
                fit->r[i][j] = value;
            }
            const float value = c * fit->z[i] + s * residual;
            residual = -s * fit->z[i] + c * residual;
            fit->z[i] = value;
        }
        fit->samples++;
    } else {
        fit->history++;
    }

    fit->u[2] = fit->u[1];
    fit->u[1] = fit->u[0];
    fit->u[0] = input;
    fit->y[1] = fit->y[0];
    fit->y[0] = output;
    return true;
}

static bool modelIsFinite(const fwTuneModel_t *model)
{
    return isfinite(model->a[0]) && isfinite(model->a[1]) &&
        isfinite(model->b[0]) && isfinite(model->b[1]) && isfinite(model->b[2]) &&
        isfinite(model->offset) && isfinite(model->samplePeriod) && model->samplePeriod > 0;
}

bool fwTuneModelIsStable(const fwTuneModel_t *model)
{
    // Strict second-order Jury conditions; marginal poles are not accepted.
    return modelIsFinite(model) && fabsf(model->a[1]) < 1.0f &&
        1.0f + model->a[0] + model->a[1] > 0.00001f &&
        1.0f - model->a[0] + model->a[1] > 0.00001f;
}

bool fwTuneModelStaticGain(const fwTuneModel_t *model, float *gain)
{
    if (!fwTuneModelIsStable(model)) {
        return false;
    }
    const float value = (model->b[0] + model->b[1] + model->b[2]) /
        (1.0f + model->a[0] + model->a[1]);
    if (!isfinite(value) || value <= 0) {
        return false;
    }
    *gain = value;
    return true;
}

bool fwTuneModelSolve(const fwTuneModelFit_t *fit, fwTuneModel_t *model)
{
    if (fit->invalid || fit->samples < FW_TUNE_MODEL_MIN_SAMPLES) {
        return false;
    }
    float coefficients[FW_TUNE_MODEL_TERMS] = {0};
    for (int i = FW_TUNE_MODEL_TERMS - 1; i >= 0; i--) {
        float rowMagnitude = 0;
        for (unsigned j = 0; j <= (unsigned)i; j++) {
            rowMagnitude += fabsf(fit->r[j][i]);
        }
        // Reject insufficiently independent columns, including constant/no input.
        if (!isfinite(rowMagnitude) || fabsf(fit->r[i][i]) <= 0.0001f * rowMagnitude ||
            fabsf(fit->r[i][i]) < 0.00001f) {
            return false;
        }
        float value = fit->z[i];
        for (unsigned j = i + 1; j < FW_TUNE_MODEL_TERMS; j++) {
            value -= fit->r[i][j] * coefficients[j];
        }
        coefficients[i] = value / fit->r[i][i];
        if (!isfinite(coefficients[i])) {
            return false;
        }
    }

    const fwTuneModel_t result = {
        .a = {coefficients[0], coefficients[1]},
        .b = {coefficients[2], coefficients[3], coefficients[4]},
        .offset = coefficients[5],
        .samplePeriod = fit->samplePeriod,
    };
    float gain;
    if (!fwTuneModelStaticGain(&result, &gain)) {
        return false;
    }
    *model = result;
    return true;
}

float fwTuneModelPredict(const fwTuneModel_t *model, fwTuneModelState_t *state, float input)
{
    const float output = -model->a[0] * state->y[0] - model->a[1] * state->y[1] +
        model->b[0] * state->u[0] + model->b[1] * state->u[1] + model->b[2] * state->u[2] + model->offset;
    state->y[1] = state->y[0];
    state->y[0] = output;
    state->u[2] = state->u[1];
    state->u[1] = state->u[0];
    state->u[0] = input;
    return output;
}

void fwTuneModelValidationInit(fwTuneModelValidation_t *validation, const fwTuneModel_t *model)
{
    memset(validation, 0, sizeof(*validation));
    validation->model = *model;
    float gain;
    validation->invalid = !fwTuneModelStaticGain(model, &gain);
}

bool fwTuneModelValidateSample(fwTuneModelValidation_t *validation, float input, float output)
{
    if (validation->invalid || !isfinite(input) || !isfinite(output) ||
        fabsf(input) > 10000.0f || fabsf(output) > 10000.0f ||
        validation->samples >= FW_TUNE_MODEL_MAX_SAMPLES) {
        validation->invalid = true;
        return false;
    }
    const float prediction = fwTuneModelPredict(&validation->model, &validation->state, input);
    if (!isfinite(prediction) || fabsf(prediction) > 10000.0f) {
        validation->invalid = true;
        return false;
    }
    if (validation->history < 3) {
        validation->state.y[0] = output;
        validation->history++;
        return true;
    }

    const float error = prediction - output;
    validation->squaredError += error * error;
    validation->samples++;
    const float delta = output - validation->mean;
    validation->mean += delta / validation->samples;
    validation->squaredDeviation += delta * (output - validation->mean);
    return true;
}

bool fwTuneModelValidationError(const fwTuneModelValidation_t *validation, float *normalisedError)
{
    if (validation->invalid || validation->samples < FW_TUNE_MODEL_MIN_SAMPLES ||
        !isfinite(validation->squaredError) || !isfinite(validation->squaredDeviation) ||
        validation->squaredDeviation < 0.000001f) {
        return false;
    }
    const float value = sqrtf(validation->squaredError / validation->squaredDeviation);
    if (!isfinite(value)) {
        return false;
    }
    *normalisedError = value;
    return true;
}
