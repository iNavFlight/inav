// Pure logic for the "PR RAM/Flash usage delta" comment: diffs a PR's
// size-report.json against a base-branch baseline and renders the markdown
// comment body. Deliberately has no network/filesystem/GitHub Actions
// dependency so it can be unit tested directly and reused (via require)
// from the actions/github-script step in ci-size-report.yml.
//
// Report shape: { "<target>": { "flash": <bytes>, "ram": <bytes>,
//   "regions": { "<name>": <bytes>, ... } }, ... }
//
// "regions" is optional and, when present, breaks "ram" down by linker
// memory region (e.g. RAM/CCM on F4/F7 parts, RAM/DTCM on H7) — "ram" alone
// is the sum of those regions and stays purely additive/informational once
// a regional breakdown exists. A region delta is only rendered when BOTH
// the PR and baseline entries carry "regions" for that target AND both
// sides name the exact same set of regions; if either side predates the
// field (e.g. an old stored baseline) or the region set differs (schema
// drift, or a target's regions being renamed/restructured), the row falls
// back to the combined "ram" figure instead of a partial or misleading
// per-region breakdown.

'use strict';

// 4 representative targets spanning flash/RAM size tiers (manager-approved
// set, 2026-08-17). Note MATEKF722 and MATEKF765 are both STM32F7 parts —
// "one per family" in the loose sense of distinct flash/RAM budgets, not
// one per silicon line. No AT32 target is covered; flagged back to the
// manager as a possible coverage gap, not decided unilaterally here.
const REPRESENTATIVE_TARGETS = ['MATEKF405', 'MATEKF722', 'MATEKF765', 'MATEKH743'];

// Below these magnitudes a delta is noise, not worth flagging.
const FLASH_NOISE_THRESHOLD_BYTES = 4096;
const RAM_NOISE_THRESHOLD_BYTES = 1024;

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

        // Only trust a per-region breakdown when both sides have the exact
        // same set of region names - a target whose regions were renamed or
        // restructured between the two commits (e.g. a linker script split
        // one region into two) would otherwise show the old name's entire
        // usage as a "loss" and the new name's as a "gain", a purely
        // cosmetic swap with zero actual growth reported as a large,
        // notable delta in both directions.
        let regionDeltas;
        let regionSetChanged = false;
        if (pr.regions && base.regions) {
            const prNames = Object.keys(pr.regions).sort();
            const baseNames = Object.keys(base.regions).sort();
            const sameRegions = prNames.length === baseNames.length
                && prNames.every((name, i) => name === baseNames[i]);
            if (sameRegions) {
                regionDeltas = prNames.map((name) => {
                    const prBytes = pr.regions[name];
                    const baseBytes = base.regions[name];
                    return { name, delta: prBytes - baseBytes, baseBytes };
                });
            } else {
                // Falling back to the combined RAM delta is correct (no
                // spurious per-region numbers), but silently doing so would
                // hide that the target's linker layout itself changed shape
                // - worth a reviewer's attention even at zero net growth.
                regionSetChanged = true;
            }
        }

        const notable = regionDeltas
            ? Math.abs(flashDelta) >= FLASH_NOISE_THRESHOLD_BYTES || regionDeltas.some((r) => Math.abs(r.delta) >= RAM_NOISE_THRESHOLD_BYTES)
            : Math.abs(flashDelta) >= FLASH_NOISE_THRESHOLD_BYTES || Math.abs(ramDelta) >= RAM_NOISE_THRESHOLD_BYTES;

        return {
            target,
            status: 'compared',
            flash: pr.flash,
            ram: pr.ram,
            baseFlash: base.flash,
            baseRam: base.ram,
            flashDelta,
            ramDelta,
            regionDeltas,
            regionSetChanged,
            notable,
        };
    });
}

// docLink: string URL to link, or null/undefined to omit the doc-link line.
// baselineCommit: short SHA of the baseline commit the delta was computed
//   against, or null/undefined to fall back to the generic "base branch"
//   wording. baselineIsNearest: true when the exact base commit had no
//   stored baseline and a nearest-ancestor baseline was used instead.
function renderComment({ prReport, baselineReport, shortSha, baselineCommit, baselineIsNearest, docLink, marker }) {
    const rows = diffSizeReports(prReport, baselineReport);
    // Only name the baseline commit when there is actually a baseline to
    // compare against (the workflow only sets baselineCommit in that case,
    // but the renderer must not emit a contradictory header otherwise).
    const vs = (baselineReport && baselineCommit)
        ? `vs. base commit \`${baselineCommit}\`` : 'vs. base branch';
    const lines = [marker, `**RAM / Flash usage ${vs}** — commit \`${shortSha}\``, ''];

    if (!baselineReport) {
        lines.push(
            '> No size baseline is available yet for this PR\'s base commit ' +
            '(no per-commit baseline has been published for it). This comment ' +
            'will show deltas once one exists — rebasing the PR refreshes its ' +
            'base commit.',
            ''
        );
    } else if (baselineIsNearest) {
        lines.push(
            '> Using the nearest available size baseline — the PR\'s exact base ' +
            'commit has no stored baseline yet.',
            ''
        );
    }

    const anyComparable = rows.some((r) => r.status === 'compared' || r.status === 'no-baseline');
    if (anyComparable) {
        lines.push('| Target | Flash Δ | RAM Δ |', '|---|---|---|');
        for (const row of rows) {
            if (row.status === 'compared') {
                const flashCell = formatDelta(row.flashDelta, row.baseFlash);
                const ramCell = row.regionDeltas
                    ? row.regionDeltas.map((r) => `${r.name}: ${formatDelta(r.delta, r.baseBytes)}`).join('<br>')
                    : formatDelta(row.ramDelta, row.baseRam)
                        + (row.regionSetChanged ? '<br><sub>region layout changed since baseline</sub>' : '');
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

module.exports = {
    REPRESENTATIVE_TARGETS,
    FLASH_NOISE_THRESHOLD_BYTES,
    RAM_NOISE_THRESHOLD_BYTES,
    diffSizeReports,
    renderComment,
    formatDelta,
};
