'use strict';

const fs = require('node:fs');
const path = require('node:path');
const assert = require('node:assert/strict');
const { test } = require('node:test');

function conditionFor(file, job) {
    const source = fs.readFileSync(path.join(__dirname, '../workflows', file), 'utf8');
    const jobBody = source.split(`\n  ${job}:\n`)[1]?.split(/\n  [\w-]+:\n/)[0];
    const expression = jobBody?.match(/\n    if: >\n((?:      .*\n)+)/)?.[1].trim();
    assert.ok(expression, `Missing condition for ${file}:${job}`);
    // These checked-in conditions use only member access, == and &&, shared
    // by GitHub expressions and JavaScript. Read them directly to catch drift.
    return new Function('github', `return Boolean(${expression});`);
}

function event(overrides = {}) {
    return {
        workflow: { path: '.github/workflows/ci.yml' },
        workflow_run: { name: 'Build firmware', event: 'pull_request', conclusion: 'success' },
        ...overrides,
    };
}

for (const [file, job] of [['pr-test-builds.yml', 'publish'], ['ci-size-report.yml', 'pr-comment']]) {
    const accepts = conditionFor(file, job);
    test(`${file}: accepts firmware without workflow_run.path`, () => {
        assert.equal(accepts({ event: event() }), true);
    });
    test(`${file}: rejects same-name non-code workflow`, () => {
        assert.equal(accepts({ event: event({ workflow: { path: '.github/workflows/non-code-change.yaml' } }) }), false);
    });
    test(`${file}: ignores a misleading run path`, () => {
        const payload = event({ workflow: { path: '.github/workflows/non-code-change.yaml' } });
        payload.workflow_run.path = '.github/workflows/ci.yml';
        assert.equal(accepts({ event: payload }), false);
    });
    test(`${file}: rejects unsuccessful and non-PR runs`, () => {
        for (const conclusion of ['failure', 'cancelled', 'skipped', 'action_required', null]) {
            const payload = event();
            payload.workflow_run.conclusion = conclusion;
            assert.equal(accepts({ event: payload }), false);
        }
        const payload = event();
        payload.workflow_run.event = 'push';
        assert.equal(accepts({ event: payload }), false);
    });
    test(`${file}: rejects missing or unknown workflow paths`, () => {
        for (const workflow of [{}, { path: '.github/workflows/other.yml' }]) {
            assert.equal(accepts({ event: event({ workflow }) }), false);
        }
    });
}
