import { CASES, getCase } from './cases/index.js';
import { loadState, saveState, clone } from './core/state.js';
import { requestChat } from './core/llm.js';
import { el, qs, qsa, clear } from './ui/dom.js';
import { speak, getVoiceEnabled, setVoiceEnabled } from './ui/speech.js';
import { renderCasePanel, renderRoom } from './ui/presenter.js';
import { setProgress, renderProgressHud } from './ui/progress.js';
import { openInvestigation, closeInvestigation, isInvestigationOpen, setCloseCallback } from './ui/investigation.js';

let activeCase = getCase(localStorage.getItem('voss-active-case') || 'voss');
let state = loadState(activeCase);
let messages = []; // per-case LLM memory; keyed by case id

const memoryKeys = {};
function conversationMemory(caseObj) {
  const key = caseObj.id;
  if (!memoryKeys[key]) {
    memoryKeys[key] = activeCase.id === caseObj.id
      ? (messages.length ? messages : [{ role: 'system', content: caseObj.systemPrompt }])
      : [{ role: 'system', content: caseObj.systemPrompt }];
  }
  return memoryKeys[key];
}

/* ------------------------------------------------------------------ */
/*  DOM anchors                                                        */
/* ------------------------------------------------------------------ */
const form = qs('#question-form');
const input = qs('#question');
const askBtn = qs('#ask');
const transcript = qs('#transcript');
const note = qs('#connection-note');

/* ------------------------------------------------------------------ */
/*  Transcript                                                         */
/* ------------------------------------------------------------------ */
function addMessage(role, text) {
  const item = el('article', { class: `message ${role === 'user' ? 'detective-message' : 'suspect-message'}` });
  item.append(
    el('span', { text: role === 'user' ? 'YOU' : activeCase.ui.messageLabel }),
    el('p', { text })
  );
  transcript.append(item);
  transcript.scrollTop = transcript.scrollHeight;
}

/* ------------------------------------------------------------------ */
/*  Interrogation-mode logic (Voss)                                    */
/* ------------------------------------------------------------------ */
function stateContext() {
  const sc = activeCase.mode === 'interrogation' ? state.suspect : null;
  if (!sc) return '';
  const tone = sc.stress > 70 ? 'You are under intense pressure.' : sc.stress > 40 ? 'You are visibly uneasy.' : sc.trust > 65 ? 'You are slightly more willing to explain.' : 'You remain controlled and careful.';
  return `Current case facts available to you: ${state.presentedEvidence.map(id => activeCase.evidence[id]?.name).filter(Boolean).join(', ') || 'none presented this turn'}. The detective has presented evidence before: ${state.presentedEvidence.join(', ') || 'none'}. ${tone}`;
}

function detectContradiction(question) {
  const q = question.toLowerCase();
  const pressure = /you said|but|then how|contradict|that can't|earlier/.test(q);
  if (pressure && state.statements.length > 1 && !state.contradictions.length) return 'timeline-account';
  if (/alone|by yourself/.test(q) && state.statements.some(s => /friend|someone|with /.test(s.text.toLowerCase()))) return 'company';
  return null;
}

function applyQuestionState(question, evidenceId) {
  if (activeCase.mode !== 'interrogation') return;
  const q = question.toLowerCase();
  const contradiction = detectContradiction(question);
  if (contradiction && !state.contradictions.includes(contradiction)) {
    state.contradictions.push(contradiction);
    state.suspect.stress += 18; state.suspect.suspicion += 14;
    if (!state.timeline.includes('contradiction')) state.timeline.push('contradiction');
  }
  if (evidenceId) { state.suspect.stress += 20; state.suspect.suspicion += 8; }
  else if (/please|help|understand|walk me through|what happened/.test(q)) { state.suspect.trust += 5; state.suspect.stress = Math.max(0, state.suspect.stress - 4); }
  else if (/lie|murder|killer|you did it|guilty|prove it/.test(q)) state.suspect.stress += 11;
  ['trust', 'stress', 'suspicion'].forEach(k => {
    state.suspect[k] = Math.min(100, Math.max(0, state.suspect[k]));
  });
  setProgress(activeCase, state);
}

/* ------------------------------------------------------------------ */
/*  Ask the target (suspect or analyst)                                */
/* ------------------------------------------------------------------ */
async function askElias(question, evidenceId = null) {
  applyQuestionState(question, evidenceId);

  const mem = conversationMemory(activeCase);
  mem.push({ role: 'user', content: question });
  addMessage('user', question);
  speak(question);
  askBtn.disabled = true;
  note.textContent = activeCase.ui.consideringNote;

  try {
    const systemContent = activeCase.mode === 'interrogation'
      ? `${activeCase.systemPrompt}\n${stateContext()}`
      : activeCase.analyst.systemPrompt;

    const systemMsg = { role: 'system', content: systemContent };
    const history = mem.slice(1, -1); // skip system + the just-added user
    const text = await requestChat({ system: systemMsg, messages: history.concat(mem.slice(-1)), maxTokens: activeCase.mode === 'interrogation' ? 140 : 180 });

    mem.push({ role: 'assistant', content: text });
    state.statements.push({ text, at: Date.now() });
    addMessage('assistant', text);
    speak(text);
    note.textContent = activeCase.ui.connectedNote;
  } catch (error) {
    mem.pop(); // remove the unanswered user question
    addMessage('assistant', activeCase.mode === 'interrogation' ? 'The recorder clicks. No response comes through.' : 'The analyst pauses. No response comes through.');
    note.textContent = `Service unavailable: ${error.message}`;
  } finally {
    askBtn.disabled = false;
    input.focus();
  }
  saveState(activeCase, state);
}

/* ------------------------------------------------------------------ */
/*  Case switching                                                     */
/* ------------------------------------------------------------------ */
function buildCaseSwitcher() {
  const select = qs('#case-select');
  if (!select) return;
  clear(select);
  CASES.forEach(c => {
    const opt = el('option', { value: c.id, text: `${c.title}: ${c.name}` });
    if (c.id === activeCase.id) opt.selected = true;
    select.append(opt);
  });
  select.onchange = () => switchCase(select.value);
}

function switchCase(id) {
  if (id === activeCase.id) return;
  // Save current memory under its key
  memoryKeys[activeCase.id] = messages;
  activeCase = getCase(id);
  localStorage.setItem('voss-active-case', id);
  messages = conversationMemory(activeCase);
  state = loadState(activeCase);
  bootCase();
}

/* ------------------------------------------------------------------ */
/*  Boot a case                                                        */
/* ------------------------------------------------------------------ */
function bootCase() {
  closeInvestigation();
  const shell = qs('.game-shell');
  shell.classList.remove('theme-voss', 'theme-room17');
  shell.classList.add(`theme-${activeCase.id === 'room17' ? 'room17' : 'voss'}`);
  renderRoom(activeCase);
  renderCasePanel(activeCase);

  // transcript
  clear(transcript);
  input.placeholder = activeCase.ui.inputPlaceholder;

  // voice toggle label
  const vBtn = qs('.voice-toggle');
  if (vBtn) vBtn.textContent = getVoiceEnabled() ? 'VOICE ON' : 'VOICE OFF';

  // seed transcript (interrogation)
  if (activeCase.mode === 'interrogation' && !state.statements.length) {
    addMessage('assistant', activeCase.ui.initialReply || 'You are wasting your time, detective. I told you everything I know.');
  }
  if (activeCase.mode === 'investigation' && !state.statements.length) {
    addMessage('assistant', activeCase.analyst.greeting);
  }

  // progress HUD
  const hud = qs('.progress-hud');
  if (hud) {
    clear(hud);
    hud.append(el('label', { text: 'INVESTIGATION PROGRESS' }), el('div', {}, el('i')),
      el('b', { text: `${activeCase.progressStages[0]}` }));
    renderProgressHud(activeCase, state);
  }

  // stage nav buttons (case-appropriate)
  const stageNav = qs('.stage3-nav');
  const labels = activeCase.mode === 'interrogation'
    ? ['overview', 'evidence', 'timeline', 'suspects', 'notes']
    : ['overview', 'files', 'evidence', 'witnesses', 'notes'];
  clear(stageNav);
  labels.forEach(tab => {
    const b = el('button', { text: tab === 'overview' ? 'CASE OVERVIEW' : tab.toUpperCase(), 'data-stage': tab });
    b.onclick = () => {
      qsa('.stage3-nav button').forEach(x => x.classList.remove('active'));
      b.classList.add('active');
      openInvestigation(activeCase, state, tab);
    };
    stageNav.append(b);
  });
  const inter = el('button', { class: 'active', text: activeCase.mode === 'interrogation' ? 'INTERROGATION' : 'ANALYST', 'data-stage': 'interrogation' });
  inter.onclick = () => {
    qsa('.stage3-nav button').forEach(x => x.classList.remove('active'));
    inter.classList.add('active');
    closeInvestigation();
  };
  stageNav.append(inter);
}

/* ------------------------------------------------------------------ */
/*  Wire static controls                                               */
/* ------------------------------------------------------------------ */
function wireControls() {
  setCloseCallback(() => {
    qsa('.stage3-nav button').forEach(b => b.classList.toggle('active', b.dataset.stage === 'interrogation'));
  });

  // Ask form
  form.addEventListener('submit', e => {
    e.preventDefault();
    const question = input.value.trim();
    if (!question || askBtn.disabled) return;
    input.value = '';
    askElias(question);
  });

  // Evidence quick-prompt buttons (interrogation mode only)
  document.addEventListener('click', e => {
    const btn = e.target.closest('.evidence-list button');
    if (!btn || activeCase.mode !== 'interrogation') return;
    const id = btn.dataset.evidence;
    if (id && activeCase.evidence[id]) {
      state.presentedEvidence.push(id);
      saveState(activeCase, state);
      const line = `We found this: ${activeCase.evidence[id].proves}`;
      askElias(line, id);
    }
  });

  // Investigate button
  const invBtn = qs('.investigate-button');
  if (invBtn) invBtn.onclick = () => openInvestigation(activeCase, state, 'overview');

  // Reset
  qs('#reset').addEventListener('click', () => {
    localStorage.removeItem(activeCase.storageKey);
    state = clone(activeCase.initialState);
    memoryKeys[activeCase.id] = [{ role: 'system', content: activeCase.systemPrompt }];
    messages = memoryKeys[activeCase.id];
    clear(transcript);
    if (activeCase.mode === 'interrogation') addMessage('assistant', activeCase.ui.resetSpeaker);
    else addMessage('assistant', activeCase.analyst.greeting);
    note.textContent = activeCase.ui.connectedNote;
    closeInvestigation();
    qsa('.stage3-nav button').forEach(b => b.classList.toggle('active', b.dataset.stage === 'interrogation'));
    bootCase();
    if ('speechSynthesis' in window) window.speechSynthesis.cancel();
  });
}

/* ------------------------------------------------------------------ */
/*  Init                                                               */
/* ------------------------------------------------------------------ */
function init() {
  buildCaseSwitcher();
  wireControls();
  bootCase();
}

// Voice toggle button (added by boot-independent path; we bind once)
function bindVoiceToggle() {
  const vBtn = qs('.voice-toggle');
  if (!vBtn) return;
  vBtn.onclick = () => {
    const next = !getVoiceEnabled();
    setVoiceEnabled(next);
    vBtn.textContent = next ? 'VOICE ON' : 'VOICE OFF';
    vBtn.setAttribute('aria-pressed', String(next));
  };
}

// Run
init();
bindVoiceToggle();
