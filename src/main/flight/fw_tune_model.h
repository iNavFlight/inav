/*
 * This file is part of INAV.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>

// y[k] = -a1*y[k-1] - a2*y[k-2] + b1*u[k-1] + b2*u[k-2] + b3*u[k-3] + offset.
// Samples must be synchronous, uniformly spaced, and expressed in consistent units.
// The model includes the actuator delay and the measurement/filter path at that rate.
#define FW_TUNE_MODEL_TERMS 6

typedef struct {
    float a[2];
    float b[3];
    float offset;
    float samplePeriod;
} fwTuneModel_t;

typedef struct {
    float r[FW_TUNE_MODEL_TERMS][FW_TUNE_MODEL_TERMS];
    float z[FW_TUNE_MODEL_TERMS];
    float u[3];
    float y[2];
    float samplePeriod;
    uint32_t samples;
    uint8_t history;
    bool invalid;
} fwTuneModelFit_t;

typedef struct {
    float y[2];
    float u[3];
} fwTuneModelState_t;

typedef struct {
    fwTuneModel_t model;
    fwTuneModelState_t state;
    float squaredError;
    float mean;
    float squaredDeviation;
    uint32_t samples;
    uint8_t history;
    bool invalid;
} fwTuneModelValidation_t;

void fwTuneModelInit(fwTuneModelFit_t *fit, float samplePeriod);
bool fwTuneModelAdd(fwTuneModelFit_t *fit, float input, float output);
bool fwTuneModelSolve(const fwTuneModelFit_t *fit, fwTuneModel_t *model);
bool fwTuneModelIsStable(const fwTuneModel_t *model);
bool fwTuneModelStaticGain(const fwTuneModel_t *model, float *gain);
// Uses previous inputs: input is the command that will be held after this observation.
float fwTuneModelPredict(const fwTuneModel_t *model, fwTuneModelState_t *state, float input);

// Held-out, free-running prediction: measured outputs initialise the history only.
// A low prediction error is necessary but not sufficient for controller synthesis.
void fwTuneModelValidationInit(fwTuneModelValidation_t *validation, const fwTuneModel_t *model);
bool fwTuneModelValidateSample(fwTuneModelValidation_t *validation, float input, float output);
bool fwTuneModelValidationError(const fwTuneModelValidation_t *validation, float *normalisedError);
