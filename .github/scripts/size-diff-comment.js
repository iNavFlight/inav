// Pure logic for the "PR RAM/Flash usage delta" comment: diffs a PR's
// size-report.json against a base-branch baseline and renders the markdown
// comment body. Deliberately has no network/filesystem/GitHub Actions
// dependency so it can be unit tested directly and reused (via require)
// from the actions/github-script step in ci-size-report.yml.
//
// Report shape: { "<target>": { "flash": <bytes>, "ram": <bytes> }, ... }

'use strict';

// 4 representative targets spanning flash/RAM size tiers (manager-approved
// set, 2026-08-17). Note MATEKF722 and MATEKF765 are both STM32F7 parts —
// "one per family" in the loose sense of distinct flash/RAM budgets, not
// one per silicon line. No AT32 target is covered; flagged back to the
// manager as a possible coverage gap, not decided unilaterally here.
const REPRESENTATIVE_TARGETS = ['MATEKF405', 'MATEKF722', 'MATEKF765', 'MATEKH743'];

// Below this magnitude a delta is noise (rounding/toolchain jitter), not a
// real change worth calling out.
const NOISE_THRESHOLD_BYTES = 32;

function formatDelta(deltaBytes, baseBytes) {
    const sign = deltaBytes > 0 ? '+' : deltaBytes < 0 ? '' : '±';
    const pct = baseBytes > 0 ? (deltaBytes / baseBytes) * 100 : 0;
    const pctStr = baseBytes > 0 ? ` (${sign}${pct.toFixed(2)}%)` : '';
    return `${sign}${deltaBytes} B${pctStr}`;
}

// Returns an array of row objects, one per representative target, each
// either a comparison row or a status row (missing from PR/baseline).
function diffSizeReports(prReport, baselineReport) {
    return REPRESENTATIVE_TARGETS.map((target) => {
        const pr = prReport[target];
        const base = baselineReport ? baselineReport[target] : undefined;

        if (!pr && !base) {
            return { target, status: 'not-built' };
        }
        if (!pr) {
            return { target, status: 'missing-from-pr' };
        }
        if (!base) {
            return { target, status: 'no-baseline', flash: pr.flash, ram: pr.ram };
        }

        const flashDelta = pr.flash - base.flash;
        const ramDelta = pr.ram - base.ram;
        return {
            target,
            status: 'compared',
            flash: pr.flash,
            ram: pr.ram,
            baseFlash: base.flash,
            baseRam: base.ram,
            flashDelta,
            ramDelta,
            notable: Math.abs(flashDelta) >= NOISE_THRESHOLD_BYTES || Math.abs(ramDelta) >= NOISE_THRESHOLD_BYTES,
        };
    });
}

// docLink: string URL to link, or null/undefined to omit the doc-link line.
function renderComment({ prReport, baselineReport, shortSha, docLink, marker }) {
    const rows = diffSizeReports(prReport, baselineReport);
    const lines = [marker, '**RAM / Flash usage vs. base branch** — commit `' + shortSha + '`', ''];

    if (!baselineReport) {
        lines.push(
            '> No size baseline is available yet for this PR\'s base branch ' +
            '(first run after this feature shipped, or a new branch). ' +
            'This comment will show deltas once a baseline exists.',
            ''
        );
    }

    const anyComparable = rows.some((r) => r.status === 'compared' || r.status === 'no-baseline');
    if (anyComparable) {
        lines.push('| Target | Flash Δ | RAM Δ |', '|---|---|---|');
        for (const row of rows) {
            if (row.status === 'compared') {
                const flashCell = formatDelta(row.flashDelta, row.baseFlash);
                const ramCell = formatDelta(row.ramDelta, row.baseRam);
                const notableMark = row.notable ? ' ⚠️' : '';
                lines.push(`| ${row.target}${notableMark} | ${flashCell} | ${ramCell} |`);
            } else if (row.status === 'no-baseline') {
                lines.push(`| ${row.target} | ${row.flash} B (no baseline) | ${row.ram} B (no baseline) |`);
            } else if (row.status === 'missing-from-pr') {
                lines.push(`| ${row.target} | not built by this PR | not built by this PR |`);
            }
            // 'not-built': omit entirely, nothing meaningful to say
        }
        lines.push('');
    } else {
        lines.push(
            '_None of the representative targets (' + REPRESENTATIVE_TARGETS.join(', ') + ') ' +
            'were built by this PR — no size comparison to show._',
            ''
        );
    }

    if (docLink) {
        lines.push(`See [RAM/flash optimization guide](${docLink}) for techniques to reduce usage.`, '');
    }

    return lines.join('\n').trimEnd() + '\n';
}

module.exports = { REPRESENTATIVE_TARGETS, NOISE_THRESHOLD_BYTES, diffSizeReports, renderComment, formatDelta };
