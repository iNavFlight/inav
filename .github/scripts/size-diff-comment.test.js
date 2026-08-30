// Unit tests for size-diff-comment.js (RAM/Flash usage delta PR comment).
//
// Run with: node --test .github/scripts/size-diff-comment.test.js
//
// Pure-logic tests only — no filesystem/network access, matching the module
// under test. Uses Node's built-in test runner (node:test / node:assert),
// no dependencies.

'use strict';

const test = require('node:test');
const assert = require('node:assert/strict');

const {
    REPRESENTATIVE_TARGETS,
    NOISE_THRESHOLD_BYTES,
    diffSizeReports,
    renderComment,
    formatDelta,
} = require('./size-diff-comment.js');

// ---------------------------------------------------------------------------
// formatDelta
// ---------------------------------------------------------------------------

test('formatDelta: positive delta gets a + sign and correct percentage', () => {
    const result = formatDelta(100, 10000);
    assert.equal(result, '+100 B (+1.00%)');
});

test('formatDelta: negative delta gets no extra sign (native minus) and correct percentage', () => {
    const result = formatDelta(-100, 10000);
    assert.equal(result, '-100 B (-1.00%)');
    // Make sure there's no double-minus like "--100" or "(-+1.00%)".
    assert.ok(!result.includes('--'));
});

test('formatDelta: zero delta does not show a + sign', () => {
    const result = formatDelta(0, 10000);
    assert.ok(!result.includes('+'), `expected no "+" in zero-delta output, got: ${result}`);
    assert.equal(result, '±0 B (±0.00%)');
});

test('formatDelta: baseBytes of 0 omits the percentage entirely (no divide-by-zero artifact)', () => {
    const result = formatDelta(50, 0);
    assert.equal(result, '+50 B');
    assert.ok(!result.includes('NaN'));
    assert.ok(!result.includes('Infinity'));
});

// ---------------------------------------------------------------------------
// diffSizeReports
// ---------------------------------------------------------------------------

test('diffSizeReports: target present in both PR and baseline with a real delta -> compared, correct deltas', () => {
    const pr = { MATEKF405: { flash: 500100, ram: 60000 } };
    const base = { MATEKF405: { flash: 500000, ram: 60200 } };
    const rows = diffSizeReports(pr, base);
    const row = rows.find((r) => r.target === 'MATEKF405');

    assert.equal(row.status, 'compared');
    assert.equal(row.flash, 500100);
    assert.equal(row.ram, 60000);
    assert.equal(row.flashDelta, 100);
    assert.equal(row.ramDelta, -200);
});

test('diffSizeReports: notable is gated at exactly NOISE_THRESHOLD_BYTES', () => {
    const baseSizes = { flash: 100000, ram: 50000 };

    const atThreshold = { MATEKF405: { flash: baseSizes.flash + NOISE_THRESHOLD_BYTES, ram: baseSizes.ram } };
    const belowThreshold = { MATEKF405: { flash: baseSizes.flash + NOISE_THRESHOLD_BYTES - 1, ram: baseSizes.ram } };
    const base = { MATEKF405: baseSizes };

    const rowAtThreshold = diffSizeReports(atThreshold, base).find((r) => r.target === 'MATEKF405');
    const rowBelowThreshold = diffSizeReports(belowThreshold, base).find((r) => r.target === 'MATEKF405');

    assert.equal(rowAtThreshold.flashDelta, NOISE_THRESHOLD_BYTES);
    assert.equal(rowAtThreshold.notable, true, 'a delta of exactly NOISE_THRESHOLD_BYTES should be notable');

    assert.equal(rowBelowThreshold.flashDelta, NOISE_THRESHOLD_BYTES - 1);
    assert.equal(rowBelowThreshold.notable, false, 'a delta one byte below NOISE_THRESHOLD_BYTES should not be notable');
});

test('diffSizeReports: notable also triggers from ramDelta alone, and honors negative deltas via Math.abs', () => {
    const base = { MATEKF405: { flash: 100000, ram: 50000 } };
    const pr = { MATEKF405: { flash: 100000, ram: 50000 - 40 } }; // flash unchanged, ram shrank by 40
    const row = diffSizeReports(pr, base).find((r) => r.target === 'MATEKF405');

    assert.equal(row.flashDelta, 0);
    assert.equal(row.ramDelta, -40);
    assert.equal(row.notable, true, 'a -40 ram delta exceeds the 32-byte threshold in magnitude');
});

test('diffSizeReports: target missing from PR report but present in baseline -> missing-from-pr', () => {
    const pr = {}; // MATEKF405 not built this run
    const base = { MATEKF405: { flash: 100000, ram: 50000 } };
    const row = diffSizeReports(pr, base).find((r) => r.target === 'MATEKF405');

    assert.equal(row.status, 'missing-from-pr');
    assert.equal(row.flash, undefined);
    assert.equal(row.ram, undefined);
});

test('diffSizeReports: target missing from both PR and baseline -> not-built', () => {
    const pr = {};
    const base = {};
    const row = diffSizeReports(pr, base).find((r) => r.target === 'MATEKF405');

    assert.equal(row.status, 'not-built');
});

test('diffSizeReports: baselineReport undefined but target present in PR -> no-baseline', () => {
    const pr = { MATEKF405: { flash: 100000, ram: 50000 } };
    const row = diffSizeReports(pr, undefined).find((r) => r.target === 'MATEKF405');

    assert.equal(row.status, 'no-baseline');
    assert.equal(row.flash, 100000);
    assert.equal(row.ram, 50000);
});

test('diffSizeReports: baselineReport null (not just undefined) behaves the same as undefined', () => {
    const pr = { MATEKF405: { flash: 100000, ram: 50000 } };
    const row = diffSizeReports(pr, null).find((r) => r.target === 'MATEKF405');

    assert.equal(row.status, 'no-baseline');
});

test('diffSizeReports: returns exactly one row per representative target, in order', () => {
    const rows = diffSizeReports({}, {});
    assert.equal(rows.length, REPRESENTATIVE_TARGETS.length);
    assert.deepEqual(rows.map((r) => r.target), REPRESENTATIVE_TARGETS);
});

// ---------------------------------------------------------------------------
// renderComment
// ---------------------------------------------------------------------------

function fullReport(sizes) {
    // Helper: build a { target: {flash, ram} } report for all 4 representative
    // targets from a flat { target: [flash, ram] } shorthand.
    const report = {};
    for (const [target, [flash, ram]] of Object.entries(sizes)) {
        report[target] = { flash, ram };
    }
    return report;
}

test('renderComment: baseline present, all 4 targets compared, mix of notable/non-notable', () => {
    const marker = '<!-- size-diff-comment -->';
    const baselineReport = fullReport({
        MATEKF405: [500000, 60000],
        MATEKF722: [510000, 61000],
        MATEKF765: [520000, 62000],
        MATEKH743: [530000, 63000],
    });
    const prReport = fullReport({
        MATEKF405: [500100, 60000], // +100 flash -> notable
        MATEKF722: [510010, 61000], // +10 flash -> not notable
        MATEKF765: [519960, 62000], // -40 flash -> notable
        MATEKH743: [530000, 63000], // no change -> not notable
    });

    const body = renderComment({ prReport, baselineReport, shortSha: 'abc1234', docLink: null, marker });

    // Marker present (and first line, as GitHub comment-identification markers
    // are conventionally expected to be).
    assert.ok(body.includes(marker));
    assert.equal(body.split('\n')[0], marker);

    // Markdown table present.
    assert.ok(body.includes('| Target | Flash Δ | RAM Δ |'));
    assert.ok(body.includes('|---|---|---|'));

    // Notable rows flagged with the warning emoji, non-notable rows are not.
    const lines = body.split('\n');
    const matekf405Line = lines.find((l) => l.startsWith('| MATEKF405'));
    const matekf722Line = lines.find((l) => l.startsWith('| MATEKF722'));
    const matekf765Line = lines.find((l) => l.startsWith('| MATEKF765'));
    const matekh743Line = lines.find((l) => l.startsWith('| MATEKH743'));

    assert.ok(matekf405Line.includes('⚠️'), 'MATEKF405 (+100 flash) should be flagged notable');
    assert.ok(!matekf722Line.includes('⚠️'), 'MATEKF722 (+10 flash) should NOT be flagged notable');
    assert.ok(matekf765Line.includes('⚠️'), 'MATEKF765 (-40 flash) should be flagged notable');
    assert.ok(!matekh743Line.includes('⚠️'), 'MATEKH743 (no change) should NOT be flagged notable');

    // No "no baseline available" note when a baseline was supplied.
    assert.ok(!body.includes('No size baseline is available yet'));
});

test('renderComment: baselineReport falsy -> includes the "no baseline available yet" note', () => {
    const prReport = fullReport({
        MATEKF405: [500000, 60000],
        MATEKF722: [510000, 61000],
        MATEKF765: [520000, 62000],
        MATEKH743: [530000, 63000],
    });

    const body = renderComment({
        prReport,
        baselineReport: undefined,
        shortSha: 'def5678',
        docLink: null,
        marker: '<!-- marker -->',
    });

    assert.ok(body.includes('No size baseline is available yet'));
});

test('renderComment: docLink set -> link line present', () => {
    const prReport = fullReport({ MATEKF405: [500000, 60000] });
    const baselineReport = fullReport({ MATEKF405: [499000, 60000] });

    const body = renderComment({
        prReport,
        baselineReport,
        shortSha: 'abc1234',
        docLink: 'https://example.com/optimization-guide',
        marker: '<!-- marker -->',
    });

    assert.ok(body.includes('[RAM/flash optimization guide](https://example.com/optimization-guide)'));
});

test('renderComment: docLink omitted -> link line absent entirely', () => {
    const prReport = fullReport({ MATEKF405: [500000, 60000] });
    const baselineReport = fullReport({ MATEKF405: [499000, 60000] });

    const body = renderComment({
        prReport,
        baselineReport,
        shortSha: 'abc1234',
        docLink: null,
        marker: '<!-- marker -->',
    });

    assert.ok(!body.includes('optimization guide'));
    assert.ok(!body.includes('See ['));
});

test('renderComment: docLink undefined (key omitted from options) -> link line absent entirely', () => {
    const prReport = fullReport({ MATEKF405: [500000, 60000] });
    const baselineReport = fullReport({ MATEKF405: [499000, 60000] });

    const body = renderComment({
        prReport,
        baselineReport,
        shortSha: 'abc1234',
        marker: '<!-- marker -->',
    });

    assert.ok(!body.includes('optimization guide'));
});

test('renderComment: none of the 4 representative targets built -> fallback message, not an empty/broken table', () => {
    // Neither PR nor baseline built any representative target -> all rows 'not-built'.
    const body = renderComment({
        prReport: {},
        baselineReport: {},
        shortSha: 'abc1234',
        docLink: null,
        marker: '<!-- marker -->',
    });

    assert.ok(
        body.includes('None of the representative targets') && body.includes('were built by this PR'),
        `expected fallback message, got:\n${body}`
    );
    assert.ok(
        REPRESENTATIVE_TARGETS.every((t) => body.includes(t)),
        'fallback message should still name all 4 representative targets'
    );
    // No markdown table header should be emitted since there's nothing to show.
    assert.ok(!body.includes('| Target | Flash Δ | RAM Δ |'));
});

test('renderComment: result always ends with exactly one trailing newline', () => {
    const body = renderComment({
        prReport: fullReport({ MATEKF405: [500000, 60000] }),
        baselineReport: fullReport({ MATEKF405: [499000, 60000] }),
        shortSha: 'abc1234',
        docLink: null,
        marker: '<!-- marker -->',
    });

    assert.ok(body.endsWith('\n'));
    assert.ok(!body.endsWith('\n\n'));
});
