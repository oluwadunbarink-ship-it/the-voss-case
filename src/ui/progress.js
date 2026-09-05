import { el, qs, qsa, clear } from './dom.js';
import { escapeHtml } from '../core/escape.js';
import { clone } from '../core/state.js';

// Progress stages per case.
export function stageIndex(caseObj, state) {
  const stages = caseObj.progressStages || [];
  const idx = stages.indexOf(state.progress);
  return idx < 0 ? 0 : idx;
}

export function setProgress(caseObj, state) {
  if (state.contradictions.length) state.progress = caseObj.progressStages[caseObj.progressStages.length - 1];
  else if (state.theories.trim()) state.progress = caseObj.progressStages[3] || 'THEORY';
  else if ((state.discoveredFiles?.length || 0) + (state.discoveredEvidence?.length || 0) >= 2) state.progress = caseObj.progressStages[2];
  else if ((state.discoveredFiles?.length || 0) + (state.discoveredEvidence?.length || 0) >= 1) state.progress = caseObj.progressStages[1];
  else state.progress = caseObj.progressStages[0];
}

export function renderProgressHud(caseObj, state) {
  const hud = qs('.progress-hud');
  if (!hud) return;
  const stages = caseObj.progressStages || [];
  const idx = stageIndex(caseObj, state);
  const pct = Math.max(8, Math.round((idx / (stages.length - 1)) * 100));
  qs('i', hud).style.width = `${pct}%`;
  qs('b', hud).textContent = `${state.progress} · ${pct}%`;
}
