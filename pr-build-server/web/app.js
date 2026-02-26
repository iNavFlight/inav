/* app.js – INAV PR Firmware Builder frontend */
'use strict';

// ── State ─────────────────────────────────────────────────────────────────────
const state = {
  prs: [],
  filteredPrs: [],
  targets: [],
  filteredTargets: [],
  currentJobId: null,
  pollTimer: null,
  queueTimer: null,
};

// ── DOM refs ──────────────────────────────────────────────────────────────────
const $ = id => document.getElementById(id);
const prSelect       = $('pr-select');
const prFilter       = $('pr-filter');
const prInfo         = $('pr-info');
const prListMeta     = $('pr-list-meta');
const targetSelect   = $('target-select');
const targetFilter   = $('target-filter');
const showAllTargets = $('show-all-targets');
const mcuBadge       = $('mcu-badge');
const buildBtn       = $('build-btn');
const selectCard     = $('select-card');
const statusCard     = $('status-card');
const progressBar    = $('progress-bar');
const statusPhase    = $('status-phase');
const statusMessage  = $('status-message');
const statusMeta     = $('status-meta');
const downloadArea   = $('download-area');
const downloadLink   = $('download-link');
const errorArea      = $('error-area');
const errorText      = $('error-text');
const logModal       = $('log-modal');
const logContent     = $('log-content');
const queueActive    = $('queue-active');
const queuePending   = $('queue-pending');
const queueUpdated   = $('queue-updated');

// ── Initialisation ────────────────────────────────────────────────────────────
async function init() {
  await Promise.all([loadPrs(), loadTargets()]);
  updateBuildButton();
  startQueuePolling();
}

// ── PR loading ────────────────────────────────────────────────────────────────
async function loadPrs() {
  try {
    const data = await apiFetch('/api/prs');
    state.prs = data.prs || [];

    if (data.updated_at) {
      const d = new Date(data.updated_at);
      prListMeta.textContent =
        `${state.prs.length} open PRs targeting ${data.base_branch || 'maintenance-9.x'} · updated ${formatDate(d)}`;
    }

    applyPrFilter();
  } catch (e) {
    prSelect.innerHTML = '<option value="">— Failed to load PRs —</option>';
    console.error('Failed to load PRs:', e);
  }
}

function applyPrFilter() {
  const q = prFilter.value.toLowerCase().trim();
  state.filteredPrs = q
    ? state.prs.filter(p =>
        String(p.number).includes(q) ||
        p.title.toLowerCase().includes(q) ||
        p.author.toLowerCase().includes(q))
    : state.prs;

  const prev = prSelect.value;
  prSelect.innerHTML = state.filteredPrs.length
    ? state.filteredPrs.map(p => {
        const draft = p.draft ? ' [DRAFT]' : '';
        return `<option value="${p.number}">#${p.number}${draft} – ${escHtml(p.title)} (${escHtml(p.author)})</option>`;
      }).join('')
    : '<option value="">— No matching PRs —</option>';

  // Try to restore selection
  if (prev && [...prSelect.options].some(o => o.value === prev)) {
    prSelect.value = prev;
  }
  onPrChange();
}

function onPrChange() {
  const num = parseInt(prSelect.value);
  const pr = state.prs.find(p => p.number === num);
  if (!pr) {
    prInfo.classList.add('hidden');
    updateBuildButton();
    return;
  }

  const labels = pr.labels.length
    ? pr.labels.map(l => `<span class="badge">${escHtml(l)}</span>`).join(' ')
    : '';
  const draft = pr.draft ? '<span class="badge" style="background:#fef3e2;color:#92400e">Draft</span> ' : '';

  prInfo.innerHTML = `
    <strong>#${pr.number}</strong> ${draft}<a href="${pr.url}" target="_blank" rel="noopener">${escHtml(pr.title)}</a><br>
    <span style="color:var(--text-muted)">by <strong>${escHtml(pr.author)}</strong>
    · branch: <code>${escHtml(pr.branch)}</code>
    · updated ${formatDate(new Date(pr.updated_at))}</span>
    ${labels ? '<br>' + labels : ''}
  `;
  prInfo.classList.remove('hidden');
  updateBuildButton();
}

// ── Target loading ────────────────────────────────────────────────────────────
async function loadTargets() {
  try {
    const data = await apiFetch('/api/targets');
    state.targets = data;
    applyTargetFilter();
  } catch (e) {
    targetSelect.innerHTML = '<option value="">— Failed to load targets —</option>';
    console.error('Failed to load targets:', e);
  }
}

function applyTargetFilter() {
  const q = targetFilter.value.toLowerCase().trim();
  const showAll = showAllTargets.checked;

  state.filteredTargets = state.targets.filter(t => {
    if (!showAll && t.skip_releases) return false;
    if (!q) return true;
    return t.name.toLowerCase().includes(q) || t.mcu.toLowerCase().includes(q);
  });

  const prev = targetSelect.value;
  targetSelect.innerHTML = state.filteredTargets.length
    ? state.filteredTargets.map(t =>
        `<option value="${t.name}" data-mcu="${t.mcu}">${t.name} (${t.mcu})</option>`
      ).join('')
    : '<option value="">— No matching targets —</option>';

  if (prev && [...targetSelect.options].some(o => o.value === prev)) {
    targetSelect.value = prev;
  }
  onTargetChange();
}

function onTargetChange() {
  const opt = targetSelect.selectedOptions[0];
  if (!opt || !opt.value) {
    mcuBadge.textContent = '—';
    mcuBadge.className = 'badge';
  } else {
    const mcu = opt.dataset.mcu || '—';
    mcuBadge.textContent = mcu;
    mcuBadge.className = 'badge ' + mcuClass(mcu);
  }
  updateBuildButton();
}

function mcuClass(mcu) {
  if (mcu.includes('H7'))  return 'badge-h7';
  if (mcu.includes('F7'))  return 'badge-f7';
  if (mcu.includes('F4'))  return 'badge-f4';
  if (mcu.includes('AT'))  return 'badge-at';
  return '';
}

// ── Build button ──────────────────────────────────────────────────────────────
function updateBuildButton() {
  buildBtn.disabled = !(prSelect.value && targetSelect.value);
}

async function startBuild() {
  const pr = prSelect.value;
  const target = targetSelect.value;
  if (!pr || !target) return;

  buildBtn.disabled = true;

  try {
    const res = await apiFetch('/api/build', {
      method: 'POST',
      headers: {'Content-Type': 'application/json'},
      body: JSON.stringify({pr: parseInt(pr), target}),
    });
    state.currentJobId = res.job_id;
    showStatusCard(pr, target);
    startPolling();
  } catch (e) {
    alert(`Failed to start build: ${e.message}`);
    buildBtn.disabled = false;
  }
}

// ── Status polling ────────────────────────────────────────────────────────────
const PHASE_LABELS = {
  queued:   'Queued',
  fetching: 'Fetching PR',
  merging:  'Merging',
  cmake:    'Configuring (cmake)',
  building: 'Compiling',
  done:     'Complete',
  failed:   'Failed',
};

const PHASE_PROGRESS = {
  queued: 5, fetching: 15, merging: 25, cmake: 40, building: 75, done: 100, failed: 100,
};

function showStatusCard(pr, target) {
  selectCard.classList.add('hidden');
  statusCard.classList.remove('hidden');
  downloadArea.classList.add('hidden');
  errorArea.classList.add('hidden');
  progressBar.style.width = '0%';
  progressBar.classList.add('indeterminate');
  statusCard.className = 'card';
  statusPhase.textContent = 'Queued';
  statusMessage.textContent = `PR #${pr} → ${target}`;
  statusMeta.textContent = '';
}

function startPolling() {
  clearInterval(state.pollTimer);
  state.pollTimer = setInterval(pollStatus, 2000);
  pollStatus();
}

async function pollStatus() {
  if (!state.currentJobId) return;
  try {
    const s = await apiFetch(`/api/status/${state.currentJobId}`);
    renderStatus(s);
    if (s.phase === 'done' || s.phase === 'failed') {
      clearInterval(state.pollTimer);
    }
  } catch (e) {
    console.warn('Status poll failed:', e);
  }
}

function renderStatus(s) {
  const phase = s.phase || 'queued';
  const pct   = PHASE_PROGRESS[phase] || 5;

  statusCard.className = `card phase-${phase}`;
  statusPhase.textContent   = PHASE_LABELS[phase] || phase;
  statusMessage.textContent = s.message || '';

  if (phase === 'done') {
    progressBar.classList.remove('indeterminate');
    progressBar.style.width = '100%';
    const elapsed = s.elapsed_s ? ` in ${s.elapsed_s}s` : '';
    statusMeta.textContent = `Built${elapsed} · SHA: ${(s.merged_sha || '').substring(0, 8)}`;
    downloadLink.href = `/api/download/${s.job_id}`;
    downloadLink.download = s.hex_file || `${s.target}.hex`;
    downloadArea.classList.remove('hidden');

  } else if (phase === 'failed') {
    progressBar.classList.remove('indeterminate');
    progressBar.style.width = '100%';
    progressBar.style.background = 'var(--red)';
    errorText.textContent = `Build failed: ${s.message}`;
    errorArea.classList.remove('hidden');

  } else {
    progressBar.classList.add('indeterminate');
    progressBar.style.width = pct + '%';
    const updated = s.updated_at ? `· ${formatDate(new Date(s.updated_at))}` : '';
    statusMeta.textContent = `Job ${s.job_id} ${updated}`;
  }
}

// ── Queue polling ─────────────────────────────────────────────────────────────
function startQueuePolling() {
  pollQueue();
  state.queueTimer = setInterval(pollQueue, 10000);
}

async function pollQueue() {
  try {
    const q = await apiFetch('/api/queue');
    const a = q.active || [];
    queueActive.textContent  = `Active: ${a.length}`;
    queuePending.textContent = `Pending: ${q.pending_count || 0}`;
    queueUpdated.textContent = `Updated ${formatDate(new Date())}`;
  } catch (e) {
    queueUpdated.textContent = 'Queue unavailable';
  }
}

// ── Log modal ─────────────────────────────────────────────────────────────────
async function openLog() {
  if (!state.currentJobId) return;
  logContent.textContent = 'Loading…';
  logModal.classList.remove('hidden');
  try {
    const resp = await fetch(`/api/log/${state.currentJobId}`);
    if (!resp.ok) throw new Error(`HTTP ${resp.status}`);
    logContent.textContent = await resp.text();
    // Scroll to bottom
    logContent.scrollTop = logContent.scrollHeight;
  } catch (e) {
    logContent.textContent = `Failed to load log: ${e.message}`;
  }
}

// ── Event listeners ───────────────────────────────────────────────────────────
prFilter.addEventListener('input',     () => applyPrFilter());
prSelect.addEventListener('change',    onPrChange);
targetFilter.addEventListener('input', () => applyTargetFilter());
showAllTargets.addEventListener('change', () => applyTargetFilter());
targetSelect.addEventListener('change',   onTargetChange);
buildBtn.addEventListener('click',    startBuild);

$('view-log-btn').addEventListener('click',  openLog);
$('retry-log-btn').addEventListener('click', openLog);

$('log-close').addEventListener('click', () => logModal.classList.add('hidden'));
logModal.addEventListener('click', e => { if (e.target === logModal) logModal.classList.add('hidden'); });

$('new-build-btn').addEventListener('click', () => {
  state.currentJobId = null;
  progressBar.style.background = '';
  statusCard.classList.add('hidden');
  selectCard.classList.remove('hidden');
  buildBtn.disabled = false;
  updateBuildButton();
});

$('retry-build-btn').addEventListener('click', () => {
  state.currentJobId = null;
  progressBar.style.background = '';
  statusCard.classList.add('hidden');
  selectCard.classList.remove('hidden');
  buildBtn.disabled = false;
  updateBuildButton();
});

// ── Utilities ─────────────────────────────────────────────────────────────────
async function apiFetch(url, opts = {}) {
  const resp = await fetch(url, opts);
  if (!resp.ok) {
    let msg = `HTTP ${resp.status}`;
    try { const j = await resp.json(); msg = j.error || msg; } catch (_) {}
    throw new Error(msg);
  }
  return resp.json();
}

function escHtml(s) {
  return String(s)
    .replace(/&/g, '&amp;')
    .replace(/</g, '&lt;')
    .replace(/>/g, '&gt;')
    .replace(/"/g, '&quot;');
}

function formatDate(d) {
  const now = Date.now();
  const diff = Math.round((now - d.getTime()) / 1000);
  if (diff < 60)   return 'just now';
  if (diff < 3600) return `${Math.floor(diff/60)}m ago`;
  if (diff < 86400) return `${Math.floor(diff/3600)}h ago`;
  return d.toLocaleDateString();
}

// ── Boot ──────────────────────────────────────────────────────────────────────
init();
