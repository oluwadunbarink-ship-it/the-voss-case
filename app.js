const API_BASE = 'http://localhost:20128/v1';
const MODEL = 'auto/offline';
const STORAGE_KEY = 'voss-case-investigation-v2';

if (typeof window.structuredClone !== 'function') {
  window.structuredClone = (value) => JSON.parse(JSON.stringify(value));
}

const EVIDENCE = {
  alley: { id: 'alley', number: '01', name: 'Alley Camera', time: '22:19', discovery: '23:02', location: 'Marlowe Bar · service alley', description: 'A grainy exterior camera catches a figure matching Elias behind the Marlowe nine minutes after Mara vanished.', proves: 'Elias was behind the Marlowe at 22:19.', notProves: 'It does not prove he harmed Mara or that he was alone.', suspects: ['Elias Voss', 'Mara Ellison'], events: ['disappearance', 'camera'] },
  phone: { id: 'phone', number: '02', name: "Mara's Phone", time: '21:58', discovery: '23:08', location: 'Marlowe Bar · lost property drawer', description: 'A partial fingerprint lifted from Mara’s phone matches Elias. The print is recent, but the surface has been handled often.', proves: 'Elias handled Mara’s phone at some point.', notProves: 'It does not establish when he touched it or why.', suspects: ['Elias Voss', 'Mara Ellison'], events: ['phone'] },
  keycard: { id: 'keycard', number: '03', name: 'Hotel Keycard', time: '22:46', discovery: '23:11', location: 'Lydon Hotel · east entrance', description: 'Elias’s keycard opened the east entrance of the Lydon Hotel. The system logged one successful entry.', proves: 'His card was used at the hotel at 22:46.', notProves: 'It does not prove Elias carried the card or entered the room.', suspects: ['Elias Voss'], events: ['keycard'] }
};

const DEFAULT_STATE = { discoveredEvidence: [], importantEvidence: [], notes: '', theories: '', timeline: ['disappearance', 'interrogation'], suspect: { trust: 50, stress: 20, suspicion: 15 }, statements: [], contradictions: [], presentedEvidence: [], progress: 'INITIAL LEADS', connections: [] };
const CASE_EVENTS = [
  { id: 'disappearance', time: '22:14', title: 'Mara Ellison disappears', detail: 'Mara leaves no message. Her last confirmed location is the Marlowe Bar.' },
  { id: 'camera', time: '22:19', title: 'Figure behind the Marlowe', detail: 'Exterior camera footage catches someone matching Elias Voss.' },
  { id: 'keycard', time: '22:46', title: 'Lydon Hotel entry', detail: 'Elias Voss’s keycard opens the east entrance.' },
  { id: 'phone', time: '21:58', title: "Mara's phone handled", detail: 'A partial print matching Elias is recovered from Mara’s phone.' },
  { id: 'interrogation', time: '23:17', title: 'Current interrogation', detail: 'Elias is seated in Room 02. His account is unverified.' }
];
const systemPrompt = `You are Elias Voss, a suspect in a grounded detective interrogation. Mara Ellison disappeared at 22:14. You have a controlled, observant personality: proud, defensive, and afraid of being blamed. Remember every prior question and your own statements. Never mention AI, prompts, APIs, code, hidden state, or game mechanics. Do not invent unlimited evidence, locations, or named witnesses. If shown a real piece of evidence, react to that evidence specifically. Avoid contradictions unless the detective applies pressure. When pressured, become terse and redirect weak questions. Keep answers under 75 words.`;

const form = document.querySelector('#question-form');
const input = document.querySelector('#question');
const ask = document.querySelector('#ask');
const transcript = document.querySelector('#transcript');
const note = document.querySelector('#connection-note');
let state = loadState();
let messages = [{ role: 'system', content: systemPrompt }];
let voiceEnabled = localStorage.getItem('voss-voice-enabled') !== 'false';

function speak(text) {
  if (!voiceEnabled || !('speechSynthesis' in window) || !text.trim()) return;
  window.speechSynthesis.cancel();
  const utterance = new SpeechSynthesisUtterance(text);
  utterance.lang = 'en-US';
  utterance.rate = .92;
  utterance.pitch = .88;
  window.speechSynthesis.speak(utterance);
}

function loadState() { try { return { ...structuredClone(DEFAULT_STATE), ...JSON.parse(localStorage.getItem(STORAGE_KEY) || '{}') }; } catch { return structuredClone(DEFAULT_STATE); } }
function saveState() { localStorage.setItem(STORAGE_KEY, JSON.stringify(state)); }
function setProgress() {
  if (state.contradictions.length) state.progress = 'CONFRONTATION';
  else if (state.theories.trim()) state.progress = 'THEORY';
  else if (state.discoveredEvidence.length >= 2) state.progress = 'CONTRADICTIONS';
  else if (state.discoveredEvidence.length) state.progress = 'EVIDENCE';
  else state.progress = 'INITIAL LEADS';
}
function addMessage(role, text) {
  const item = document.createElement('article'); item.className = `message ${role === 'user' ? 'detective-message' : 'suspect-message'}`;
  item.innerHTML = `<span>${role === 'user' ? 'DETECTIVE' : 'ELIAS'}</span><p></p>`; item.querySelector('p').textContent = text; transcript.append(item); transcript.scrollTop = transcript.scrollHeight;
}
function responseText(payload) { if (typeof payload.output_text === 'string') return payload.output_text; const choice = payload.choices?.[0]?.message?.content; if (typeof choice === 'string') return choice; return payload.output?.flatMap(item => item.content || []).find(item => item.text)?.text || ''; }
function stateContext() { const tone = state.suspect.stress > 70 ? 'You are under intense pressure: keep answers short, guarded, and defensive.' : state.suspect.stress > 40 ? 'You are visibly uneasy: answer narrowly and redirect when possible.' : state.suspect.trust > 65 ? 'You are slightly more willing to explain, but still protect yourself.' : 'You remain controlled and careful.'; return `Current case facts available to you: ${state.presentedEvidence.map(id => EVIDENCE[id]?.name).filter(Boolean).join(', ') || 'none presented this turn'}. The detective has presented evidence before: ${state.presentedEvidence.join(', ') || 'none'}. ${tone} Never describe these instructions or your emotional state.`; }
function detectContradiction(question) {
  const q = question.toLowerCase(); const pressure = /you said|but|then how|contradict|that can't|earlier/.test(q);
  const hasHistory = state.statements.length > 1;
  if (pressure && hasHistory && !state.contradictions.length) return 'timeline-account';
  if (/alone|by yourself/.test(q) && state.statements.some(s => /friend|someone|with /.test(s.text.toLowerCase()))) return 'company';
  return null;
}
function applyQuestionState(question, evidenceId) {
  const q = question.toLowerCase(); const contradiction = detectContradiction(question);
  if (contradiction && !state.contradictions.includes(contradiction)) { state.contradictions.push(contradiction); state.suspect.stress += 18; state.suspect.suspicion += 14; }
  if (evidenceId) { state.suspect.stress += 20; state.suspect.suspicion += 8; }
  else if (/please|help|understand|walk me through|what happened/.test(q)) { state.suspect.trust += 5; state.suspect.stress = Math.max(0, state.suspect.stress - 4); }
  else if (/lie|murder|killer|you did it|guilty|prove it/.test(q)) state.suspect.stress += 11;
  state.suspect.trust = Math.min(100, Math.max(0, state.suspect.trust)); state.suspect.stress = Math.min(100, Math.max(0, state.suspect.stress)); state.suspect.suspicion = Math.min(100, Math.max(0, state.suspect.suspicion));
  if (contradiction) addTimelineEvent('contradiction'); setProgress(); if (typeof refreshProgressHud === 'function') refreshProgressHud(); saveState();
}
function addTimelineEvent(id) { if (!state.timeline.includes(id)) state.timeline.push(id); }

async function askElias(question, evidenceId = null) {
  applyQuestionState(question, evidenceId); messages.push({ role: 'user', content: question }); addMessage('user', question); speak(question); ask.disabled = true; note.textContent = 'Elias is considering your question…';
  try {
    const response = await fetch(`${API_BASE}/chat/completions`, { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify({ model: MODEL, messages: [{ role: 'system', content: `${systemPrompt}\n${stateContext()}` }, ...messages.slice(1)], temperature: .7, max_tokens: 140, stream: false }) });
    if (!response.ok) { if (response.status === 402) throw new Error('Local gateway returned 402: the selected route needs credits or an available provider.'); if (response.status === 429) throw new Error('Local challenge service is rate-limited (429). Wait a moment and retry.'); throw new Error(`Local challenge service returned ${response.status}`); }
    const text = responseText(await response.json()); if (!text) throw new Error('Service returned no suspect response');
    messages.push({ role: 'assistant', content: text }); state.statements.push({ text, at: Date.now() }); addMessage('assistant', text); speak(text); note.textContent = 'Connected to localhost:20128'; saveState();
  } catch (error) { messages.pop(); addMessage('assistant', 'The recorder clicks. No response comes through.'); note.textContent = `Challenge API unavailable: ${error.message}`; }
  finally { ask.disabled = false; input.focus(); }
}

function openInvestigation(section = 'overview') {
  document.querySelector('.investigation-overlay')?.remove(); const overlay = document.createElement('section'); overlay.className = 'investigation-overlay'; overlay.innerHTML = `<div class="investigation-frame"><nav class="investigation-nav"><div class="investigation-title"><span>CASE 08–VOSS</span><b>INVESTIGATION MODE</b></div>${['overview','evidence','suspects','timeline','notes','board'].map(tab => `<button class="investigation-tab ${section === tab ? 'active' : ''}" data-section="${tab}">${tab.toUpperCase()}</button>`).join('')}<button class="close-investigation" data-close>RETURN TO INTERROGATION ×</button></nav><div class="investigation-content" id="investigation-content"></div></div>`; document.querySelector('.game-shell').append(overlay); overlay.querySelectorAll('[data-section]').forEach(btn => btn.onclick = () => openInvestigation(btn.dataset.section)); overlay.querySelector('[data-close]').onclick = () => overlay.remove(); renderInvestigation(section); }
function renderInvestigation(section) { const content = document.querySelector('#investigation-content'); if (!content) return; const views = { overview: renderOverview, evidence: renderEvidence, suspects: renderSuspects, timeline: renderTimeline, notes: renderNotes, board: renderBoard }; content.innerHTML = views[section](); bindInvestigation(section); }
function renderOverview() { return `<div class="view-heading"><p class="overline">CASE OVERVIEW · ${state.progress}</p><h2>Follow the<br/><em>thread.</em></h2><p>Every answer is a fragment. Collect what is real, notice what does not fit, and decide what to risk in the room.</p></div><div class="overview-grid"><button data-go="evidence"><b>${state.discoveredEvidence.length}/3</b><span>EVIDENCE RECOVERED</span></button><button data-go="timeline"><b>${state.timeline.length}</b><span>TIMELINE EVENTS</span></button><button data-go="notes"><b>${state.theories ? 'ACTIVE' : 'EMPTY'}</b><span>WORKING THEORY</span></button><button data-go="board"><b>${state.connections.length}</b><span>LINKS UNLOCKED</span></button></div><div class="progress-rail"><span>INITIAL LEADS</span><i></i><span>EVIDENCE</span><i></i><span>CONTRADICTIONS</span><i></i><span>THEORY</span><i></i><span>CONFRONTATION</span></div>`; }
function renderEvidence() { return `<div class="view-heading compact"><p class="overline">EVIDENCE LOCKER</p><h2>What do you<br/><em>actually know?</em></h2></div><div class="evidence-cards">${Object.values(EVIDENCE).map(e => `<article class="evidence-card ${state.discoveredEvidence.includes(e.id) ? 'unlocked' : ''}"><span class="evidence-number">${e.number}</span><div><h3>${e.name}</h3><p>${state.discoveredEvidence.includes(e.id) ? e.description : 'LOCKED · Inspect the lead in the interrogation room to reveal details.'}</p><small>${state.discoveredEvidence.includes(e.id) ? e.location : 'UNDISCLOSED'}</small></div><button data-evidence="${e.id}">${state.discoveredEvidence.includes(e.id) ? 'OPEN FILE' : 'INSPECT LEAD'}</button></article>`).join('')}</div>`; }
function renderSuspects() { return `<div class="view-heading compact"><p class="overline">PERSONS OF INTEREST</p><h2>Read the<br/><em>silences.</em></h2></div><div class="suspect-card"><div class="suspect-avatar">EV</div><div><h3>Elias Voss <small>PRIMARY SUSPECT</small></h3><p>Bar manager. Claims he stayed at the Marlowe all night. His access card and phone contact place him across the city.</p><span class="redacted">MOTIVE: ██████████ · ALIBI: UNVERIFIED</span></div></div><div class="suspect-card muted-card"><div class="suspect-avatar">ME</div><div><h3>Mara Ellison <small>MISSING</small></h3><p>Last confirmed at the Marlowe Bar. Her phone was recovered. The rest of her night is a blank space.</p></div></div>`; }
function renderTimeline() { const events = [...CASE_EVENTS.filter(e => state.timeline.includes(e.id)), ...(state.timeline.includes('contradiction') ? [{ id: 'contradiction', time: '—', title: 'Account under pressure', detail: 'A new statement does not sit cleanly beside an earlier one.' }] : [])].sort((a,b) => a.time.localeCompare(b.time)); return `<div class="view-heading compact"><p class="overline">CHRONOLOGY</p><h2>Time leaves<br/><em>traces.</em></h2></div><div class="timeline">${events.map(e => `<article class="timeline-event ${e.id === 'contradiction' ? 'uneasy' : ''}"><time>${e.time}</time><div><h3>${e.title}</h3><p>${e.detail}</p></div></article>`).join('')}</div>`; }
function renderNotes() { return `<div class="view-heading compact"><p class="overline">FIELD NOTES</p><h2>Build your<br/><em>theory.</em></h2></div><div class="notes-layout"><div><label>PRIVATE NOTES</label><textarea id="notes-editor" placeholder="What does Elias avoid? What feels out of place?">${escapeHtml(state.notes)}</textarea><label>WORKING THEORY</label><textarea id="theory-editor" placeholder="Record a theory without needing to be certain…">${escapeHtml(state.theories)}</textarea><button class="save-notes" id="save-notes">SAVE NOTES</button></div><div class="notes-evidence"><label>COLLECTED EVIDENCE</label>${state.discoveredEvidence.length ? state.discoveredEvidence.map(id => `<button data-evidence="${id}">${EVIDENCE[id].number} · ${EVIDENCE[id].name} ${state.importantEvidence.includes(id) ? '★' : ''}</button>`).join('') : '<p>No evidence collected yet.</p>'}</div></div>`; }
function renderBoard() { const links = state.connections; return `<div class="view-heading compact"><p class="overline">CASE BOARD</p><h2>Connect what<br/><em>you find.</em></h2></div><div class="board"><div class="board-node mara">MARA ELLISON<small>MISSING</small></div><div class="board-node elias">ELIAS VOSS<small>PRIMARY SUSPECT</small></div><div class="board-node bar">MARLOWE BAR<small>LAST SEEN</small></div><div class="board-node hotel">LYDON HOTEL<small>22:46 ENTRY</small></div><div class="board-node phone-node">MARA'S PHONE<small>${links.includes('phone') ? 'PRINT MATCH' : 'LOCKED'}</small></div><div class="board-node camera-node">ALLEY CAMERA<small>${links.includes('alley') ? 'FOOTAGE' : 'LOCKED'}</small></div><div class="board-lines">${links.map((_,i) => `<i style="--n:${i}"></i>`).join('')}</div></div>`; }
function escapeHtml(value) { return value.replace(/[&<>"']/g, c => ({ '&':'&amp;', '<':'&lt;', '>':'&gt;', '"':'&quot;', "'":'&#39;' }[c])); }
function bindInvestigation(section) { document.querySelectorAll('[data-go]').forEach(b => b.onclick = () => openInvestigation(b.dataset.go)); document.querySelectorAll('[data-evidence]').forEach(b => b.onclick = () => showEvidence(b.dataset.evidence)); if (section === 'notes') { document.querySelector('#save-notes').onclick = () => { state.notes = document.querySelector('#notes-editor').value; state.theories = document.querySelector('#theory-editor').value; setProgress(); saveState(); document.querySelector('#save-notes').textContent = 'NOTES SAVED ✓'; }; } }
function showEvidence(id) { const e = EVIDENCE[id]; state.discoveredEvidence = [...new Set([...state.discoveredEvidence, id])]; if (!state.connections.includes(id)) state.connections.push(id); e.events.forEach(eventId => { if (!state.timeline.includes(eventId)) addTimelineEvent(eventId); }); setProgress(); if (typeof refreshProgressHud === 'function') refreshProgressHud(); saveState(); const modal = document.createElement('div'); modal.className = 'evidence-modal'; modal.innerHTML = `<div class="evidence-detail"><button class="modal-close">×</button><p class="overline">EVIDENCE ${e.number} · FILE UNSEALED</p><h2>${e.name}</h2><div class="detail-meta"><span>DISCOVERED ${e.discovery}</span><span>${e.location}</span></div><p class="detail-description">${e.description}</p><div class="proof-grid"><div><label>WHAT IT PROVES</label><p>${e.proves}</p></div><div><label>WHAT IT DOES NOT PROVE</label><p>${e.notProves}</p></div></div><div class="related"><span>RELATED SUSPECTS</span><b>${e.suspects.join(' · ')}</b><span>RELATED EVENTS</span><b>${e.events.map(x => CASE_EVENTS.find(ev => ev.id === x)?.title || x).join(' · ')}</b></div><div class="detail-actions"><button class="important-evidence">${state.importantEvidence.includes(id) ? '★ IMPORTANT' : '☆ MARK IMPORTANT'}</button><button class="present-evidence">PRESENT EVIDENCE</button><button class="note-evidence">ADD TO NOTES</button></div></div>`; document.body.append(modal); modal.querySelector('.modal-close').onclick = () => modal.remove(); modal.querySelector('.important-evidence').onclick = () => { state.importantEvidence = state.importantEvidence.includes(id) ? state.importantEvidence.filter(x => x !== id) : [...state.importantEvidence, id]; saveState(); modal.remove(); showEvidence(id); }; modal.querySelector('.note-evidence').onclick = () => { state.notes += `${state.notes ? '\n\n' : ''}[${e.name}] ${e.proves}`; saveState(); modal.remove(); openInvestigation('notes'); }; modal.querySelector('.present-evidence').onclick = () => { modal.remove(); const line = `We found this: ${e.proves}`; state.presentedEvidence.push(id); saveState(); document.querySelector('.investigation-overlay')?.remove(); input.value = line; askElias(line, id); }; }

const topbar = document.querySelector('.topbar');
let stageNav = document.querySelector('.stage3-nav');
if (!stageNav) {
  stageNav = document.createElement('nav'); stageNav.className = 'stage3-nav';
  topbar.insertBefore(stageNav, document.querySelector('.status'));
}
stageNav.innerHTML = ['overview','evidence','timeline','suspects','notes'].map(tab => `<button data-stage="${tab}">${tab === 'overview' ? 'CASE OVERVIEW' : tab.toUpperCase()}</button>`).join('') + '<button data-stage="interrogation" class="active">INTERROGATION</button>';
stageNav.querySelectorAll('[data-stage]').forEach(button => button.onclick = () => { stageNav.querySelectorAll('button').forEach(item => item.classList.remove('active')); button.classList.add('active'); if (button.dataset.stage === 'interrogation') document.querySelector('.investigation-overlay')?.remove(); else openInvestigation(button.dataset.stage); });
let investigateButton = document.querySelector('.investigate-button');
if (!investigateButton) {
  investigateButton = document.createElement('button'); investigateButton.className = 'investigate-button'; investigateButton.textContent = 'INVESTIGATE';
  topbar.insertBefore(investigateButton, document.querySelector('#reset'));
}
investigateButton.onclick = () => openInvestigation('overview');
let voiceButton = document.querySelector('.voice-toggle');
if (!voiceButton) {
  voiceButton = document.createElement('button'); voiceButton.className = 'voice-toggle'; voiceButton.type = 'button';
  topbar.insertBefore(voiceButton, document.querySelector('#reset'));
}
voiceButton.setAttribute('aria-pressed', String(voiceEnabled)); voiceButton.textContent = voiceEnabled ? 'VOICE ON' : 'VOICE OFF'; voiceButton.title = 'Toggle text-to-speech';
voiceButton.onclick = () => { voiceEnabled = !voiceEnabled; localStorage.setItem('voss-voice-enabled', String(voiceEnabled)); voiceButton.textContent = voiceEnabled ? 'VOICE ON' : 'VOICE OFF'; voiceButton.setAttribute('aria-pressed', String(voiceEnabled)); if (!voiceEnabled && 'speechSynthesis' in window) window.speechSynthesis.cancel(); };
let progressHud = document.querySelector('.progress-hud');
if (!progressHud) {
  progressHud = document.createElement('div'); progressHud.className = 'progress-hud'; progressHud.innerHTML = '<label>INVESTIGATION PROGRESS</label><div><i></i></div><b>INITIAL LEADS</b>'; document.querySelector('.case-panel').insertBefore(progressHud, document.querySelector('.evidence-list'));
}
function refreshProgressHud() { const stages = ['INITIAL LEADS','EVIDENCE','CONTRADICTIONS','THEORY','CONFRONTATION']; const index = Math.max(0, stages.indexOf(state.progress)); progressHud.querySelector('i').style.width = `${Math.max(8, index / (stages.length - 1) * 100)}%`; progressHud.querySelector('b').textContent = `${state.progress} · ${Math.round(index / (stages.length - 1) * 100)}%`; }
refreshProgressHud();
form.addEventListener('submit', e => { e.preventDefault(); const question = input.value.trim(); if (!question || ask.disabled) return; input.value = ''; askElias(question); });
document.querySelectorAll('.evidence-list button').forEach((button, i) => { const id = ['alley','phone','keycard'][i]; button.dataset.evidence = id; button.onclick = () => showEvidence(id); });
document.querySelector('#reset').addEventListener('click', () => { localStorage.removeItem(STORAGE_KEY); state = structuredClone(DEFAULT_STATE); messages = [{ role: 'system', content: systemPrompt }]; transcript.innerHTML = ''; addMessage('assistant', 'You are back already. What else do you want to know?'); note.textContent = 'Case reset · connected to localhost:20128'; document.querySelector('.investigation-overlay')?.remove(); stageNav.querySelectorAll('button').forEach(item => item.classList.toggle('active', item.dataset.stage === 'interrogation')); refreshProgressHud(); if ('speechSynthesis' in window) window.speechSynthesis.cancel(); });
