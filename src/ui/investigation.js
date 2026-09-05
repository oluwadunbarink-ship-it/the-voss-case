import { el, qs, qsa, clear } from './dom.js';
import { escapeHtml } from '../core/escape.js';
import { clone } from '../core/state.js';
import { saveState } from '../core/state.js';
import { setProgress, stageIndex } from './progress.js';

let activeOverlay = null;
let closeCallback = null;

export function isInvestigationOpen() { return !!activeOverlay; }
export function setCloseCallback(fn) { closeCallback = fn; }

export function closeInvestigation() {
  if (activeOverlay) activeOverlay.remove();
  activeOverlay = null;
}

export function openInvestigation(caseObj, state, section = 'overview') {
  closeInvestigation();
  const overlay = el('section', { class: 'investigation-overlay' });
  overlay.setAttribute('data-mode', caseObj.mode);

  const tabs = caseObj.mode === 'interrogation'
    ? ['overview', 'evidence', 'suspects', 'timeline', 'notes', 'board']
    : ['overview', 'files', 'evidence', 'witnesses', 'notes', 'conclusion'];

  const nav = el('nav', { class: 'investigation-nav' });
  const title = el('div', { class: 'investigation-title' });
  title.append(
    el('span', { text: caseObj.title }),
    el('b', { text: 'INVESTIGATION MODE' })
  );
  nav.append(title);

  tabs.forEach(tab => {
    const b = el('button', { class: `investigation-tab${section === tab ? ' active' : ''}`, text: tab.toUpperCase() });
    b.dataset.section = tab;
    nav.append(b);
  });

  const closeBtn = el('button', { class: 'close-investigation', text: 'RETURN ×', 'data-close': '' });
  nav.append(closeBtn);

  const content = el('div', { class: 'investigation-content', id: 'investigation-content' });
  const frame = el('div', { class: 'investigation-frame' });
  frame.append(nav, content);
  overlay.append(frame);

  document.querySelector('.game-shell').append(overlay);
  activeOverlay = overlay;

  nav.querySelectorAll('[data-section]').forEach(btn => {
    btn.onclick = () => openInvestigation(caseObj, state, btn.dataset.section);
  });
  closeBtn.onclick = () => { closeInvestigation(); closeCallback && closeCallback(); };

  renderSection(caseObj, state, section);
}

function renderSection(caseObj, state, section) {
  const content = qs('#investigation-content');
  if (!content) return;
  clear(content);

  const renderers = caseObj.mode === 'interrogation' ? interrogationRenderers : investigationRenderers;
  const fn = renderers[section];
  if (fn) content.append(...fn(caseObj, state));
  bind(caseObj, state, section);
}

/* ------------------------------------------------------------------ */
/*  INTERROGATION MODE (Voss)                                          */
/* ------------------------------------------------------------------ */

const interrogationRenderers = {
  overview(c, s) {
    const heading = el('div', { class: 'view-heading' });
    heading.append(
      el('p', { class: 'overline', text: `CASE OVERVIEW · ${s.progress}` }),
      el('h2', { html: 'Follow the<br/><em>thread.</em>' }),
      el('p', { text: 'Every answer is a fragment. Collect what is real, notice what does not fit, and decide what to risk in the room.' })
    );
    const grid = el('div', { class: 'overview-grid' });
    grid.append(
      tile('evidence', `${s.discoveredEvidence.length}/${Object.keys(c.evidence).length}`, 'EVIDENCE RECOVERED'),
      tile('timeline', s.timeline.length, 'TIMELINE EVENTS'),
      tile('notes', s.theories ? 'ACTIVE' : 'EMPTY', 'WORKING THEORY'),
      tile('board', s.connections.length, 'LINKS UNLOCKED')
    );
    const rail = progressRail(c.progressStages, stageIndex(c, s));
    return [heading, grid, rail];
  },

  evidence(c, s) {
    return [viewHeading('EVIDENCE LOCKER', 'What do you', 'actually know?'),
      el('div', { class: 'evidence-cards' },
        Object.values(c.evidence).map(e =>
          el('article', { class: `evidence-card ${s.discoveredEvidence.includes(e.id) ? 'unlocked' : ''}` },
            el('span', { class: 'evidence-number', text: e.number }),
            el('div', {},
              el('h3', { text: e.name }),
              el('p', { text: s.discoveredEvidence.includes(e.id) ? e.description : 'LOCKED · Inspect the lead in the interrogation room to reveal details.' }),
              el('small', { text: s.discoveredEvidence.includes(e.id) ? e.location : 'UNDISCLOSED' })
            ),
            el('button', { text: s.discoveredEvidence.includes(e.id) ? 'OPEN FILE' : 'INSPECT LEAD', 'data-evidence': e.id })
          )
        )
      )
    ];
  },

  suspects(c, s) {
    const card = (person, cls) =>
      el('div', { class: `suspect-card ${cls}` },
        el('div', { class: 'suspect-avatar', text: person.initials }),
        el('div', {},
          el('h3', {}, c.title && null, person.name, el('small', { text: person.label })),
          el('p', { text: person.paragraph }),
          person.footer ? el('span', { class: 'redacted', text: person.footer }) : null
        )
      );
    return [viewHeading('PERSONS OF INTEREST', 'Read the', 'silences.'),
      card(c.persons.suspect, ''),
      card(c.persons.victim, 'muted-card')];
  },

  timeline(c, s) {
    const events = c.events
      .filter(e => s.timeline.includes(e.id))
      .concat(s.timeline.includes('contradiction') ? [{ id: 'contradiction', time: '--', title: 'Account under pressure', detail: 'A new statement does not sit cleanly beside an earlier one.' }] : [])
      .sort((a, b) => a.time.localeCompare(b.time));
    return [viewHeading('CHRONOLOGY', 'Time leaves', 'traces.'),
      el('div', { class: 'timeline' },
        events.map(e =>
          el('article', { class: `timeline-event${e.id === 'contradiction' ? ' uneasy' : ''}` },
            el('time', { text: e.time }),
            el('div', {}, el('h3', { text: e.title }), el('p', { text: e.detail }))
          )
        )
      )];
  },

  notes(c, s) {
    return [viewHeading('FIELD NOTES', 'Build your', 'theory.'),
      el('div', { class: 'notes-layout' },
        el('div', {},
          el('label', { text: 'PRIVATE NOTES' }),
          el('textarea', { id: 'notes-editor', placeholder: 'What does Elias avoid? What feels out of place?' }, s.notes),
          el('label', { text: 'WORKING THEORY' }),
          el('textarea', { id: 'theory-editor', placeholder: 'Record a theory without needing to be certain…' }, s.theories),
          el('button', { class: 'save-notes', id: 'save-notes', text: 'SAVE NOTES' })
        ),
        el('div', { class: 'notes-evidence' },
          el('label', { text: 'COLLECTED EVIDENCE' }),
          s.discoveredEvidence.length
            ? s.discoveredEvidence.map(id => el('button', { text: `${c.evidence[id].number} · ${c.evidence[id].name}${s.importantEvidence.includes(id) ? ' ★' : ''}`, 'data-evidence': id }))
            : el('p', { text: 'No evidence collected yet.' })
        )
      )];
  },

  board(c, s) {
    return [viewHeading('CASE BOARD', 'Connect what', 'you find.'),
      el('div', { class: 'board' },
        c.board.nodes.map(n => {
          const sub = n.subBy ? (s.connections.includes(n.subBy) ? n.subOn : n.subOff) : n.sub;
          return el('div', { class: `board-node ${n.cls}`, 'html': `${escapeHtml(n.label)}<small>${escapeHtml(sub)}</small>` });
        }),
        el('div', { class: 'board-lines' }, s.connections.map((_, i) => el('i', { style: `--n:${i}` })))
      )
    ];
  }
};

/* ------------------------------------------------------------------ */
/*  INVESTIGATION MODE (Room 17)                                       */
/* ------------------------------------------------------------------ */

const investigationRenderers = {
  overview(c, s) {
    const filesOpen = (s.discoveredFiles || []);
    const evOpen = (s.openedEvidence || []);
    const heading = el('div', { class: 'view-heading' });
    heading.append(
      el('p', { class: 'overline', text: `CASE OVERVIEW · ${s.progress}` }),
      el('h2', { html: 'Reconstruct<br/><em>the night.</em>' }),
      el('p', { text: 'You have the file and an analyst. Open evidence files, cross-reference the items, and identify what does not add up.' })
    );
    const grid = el('div', { class: 'overview-grid' });
    grid.append(
      tile('files', `${filesOpen.length}/${c.files.length}`, 'FILES OPEN'),
      tile('evidence', `${evOpen.length}/${Object.keys(c.evidence).length}`, 'EVIDENCE OPEN'),
      tile('notes', s.theories ? 'ACTIVE' : 'EMPTY', 'WORKING THEORY'),
      tile('conclusion', s.finalScore != null ? s.finalScore : '--', 'FINAL SCORE')
    );
    const rail = progressRail(c.progressStages, stageIndex(c, s));
    return [heading, grid, rail, theoryCard(c, s)];
  },

  files(c, s) {
    const open = s.discoveredFiles || [];
    return [viewHeading('EVIDENCE FILES', 'Open the', 'archive.'),
      el('div', { class: 'file-grid' },
        c.files.map(f =>
          el('article', { class: `file-card ${open.includes(f.id) ? 'unlocked' : ''}` },
            el('span', { class: 'file-number', text: `FILE ${f.num}` }),
            el('div', {},
              el('h3', { text: f.name }),
              el('p', { text: f.blurb })
            ),
            el('button', { text: open.includes(f.id) ? 'OPEN' : 'SEALED', 'data-file': f.id })
          )
        )
      )];
  },

  evidence(c, s) {
    const open = s.openedEvidence || [];
    const relevant = Object.values(c.evidence).filter(e => (s.discoveredFiles || []).includes(e.file));
    return [viewHeading('EVIDENCE ITEMS', 'Read the', 'records.'),
      el('div', { class: 'evidence-cards' },
        relevant.map(e =>
          el('article', { class: `evidence-card ${open.includes(e.id) ? 'unlocked' : ''}` },
            el('span', { class: 'evidence-number', text: e.num }),
            el('div', {},
              el('h3', { text: `${e.type.toUpperCase()} · ${e.title}` }),
              el('p', { text: open.includes(e.id) ? e.description : 'SEALED · Open this case file to reveal the item.' }),
              el('small', { text: `CLASSIFICATION: ${e.cls}` })
            ),
            el('button', { text: open.includes(e.id) ? 'OPEN FILE' : 'UNSEAL', 'data-evidence': e.id })
          )
        )
      )];
  },

  witnesses(c, s) {
    if (!c.evidence) return [el('p', { text: 'No witness files.' })];
    const witnesses = Object.values(c.evidence).filter(e => e.type === 'testimony');
    return [viewHeading('WITNESSES', 'Whose word', 'do you trust?'),
      el('div', { class: 'witness-list' },
        witnesses.map(w =>
          el('article', { class: 'witness-card' },
            el('h3', { text: w.title }),
            el('p', { class: 'w-reliability', text: `RELIABILITY: ${w.reliability || 'Unknown'}` }),
            el('p', { text: w.description }),
            el('span', { class: 'w-class', text: `CLASSIFICATION: ${w.cls}` })
          )
        )
      )];
  },

  notes(c, s) {
    return [viewHeading('FIELD NOTES', 'Build your', 'theory.'),
      el('div', { class: 'notes-layout' },
        el('div', {},
          el('label', { text: 'PRIVATE NOTES' }),
          el('textarea', { id: 'notes-editor', placeholder: 'Record what does not add up…' }, s.notes),
          el('label', { text: 'WORKING THEORY' }),
          el('textarea', { id: 'theory-editor', placeholder: 'State your theory and the evidence behind it…' }, s.theories),
          el('button', { class: 'save-notes', id: 'save-notes', text: 'SAVE NOTES' })
        ),
        el('div', { class: 'notes-evidence' },
          el('label', { text: 'OPENED EVIDENCE' }),
          (s.openedEvidence || []).length
            ? s.openedEvidence.map(id => el('button', { text: `${c.evidence[id]?.num} · ${c.evidence[id]?.title}${s.importantEvidence.includes(id) ? ' ★' : ''}`, 'data-evidence': id }))
            : el('p', { text: 'No evidence opened yet.' })
        )
      )];
  },

  conclusion(c, s) {
    const scored = s.finalScore != null;
    const points = s.points || null;
    return [viewHeading('FINAL THEORY', 'Submit your', 'conclusion.'),
      el('p', { class: 'conclusion-intro', text: 'When you can state the identity, the timeline, the manner of death, and the motive. Submit your final theory. The analyst will score it out of 100.' }),
      el('div', { class: 'conclusion-box' },
        el('label', { text: 'YOUR FINAL THEORY' }),
        el('textarea', { id: 'final-theory', placeholder: 'Identity, timeline, method, motive, the false name, the missing six hours, the misleading evidence…' }, s.finalTheory || ''),
        el('button', { class: 'save-notes', id: 'submit-theory', text: scored ? `RESUBMIT (SCORED ${scored ? s.finalScore : ''})` : 'SUBMIT FINAL THEORY', disabled: scored && s.finalScoreNeedsReset })
      ),
      scored ? [el('div', { class: 'score-box' },
        el('h3', { text: `SCORE: ${s.finalScore}/100` }),
        points && points.length ? el('ul', {}, points.map(p => el('li', { text: `${p.label}: ${p.got}/${p.max}` }))) : null
      )] : null
    ];
  }
};

/* ------------------------------------------------------------------ */
/*  SHARED HELPERS                                                     */
/* ------------------------------------------------------------------ */

function tile(go, big, label) {
  return el('button', { 'data-go': go, 'html': `<b>${escapeHtml(String(big))}</b><span>${escapeHtml(label)}</span>` });
}

function viewHeading(overline, before, em) {
  return el('div', { class: 'view-heading compact' },
    el('p', { class: 'overline', text: overline }),
    el('h2', { html: `${escapeHtml(before)}<br/><em>${escapeHtml(em)}</em>` })
  );
}

function progressRail(stages, idx) {
  const rail = el('div', { class: 'progress-rail' });
  stages.forEach((s, i) => {
    if (i > 0) rail.append(el('i'));
    rail.append(el('span', { class: i <= idx ? 'reached' : '', text: s }));
  });
  return rail;
}

function theoryCard(c, s) {
  if (s.theories) {
    return el('div', { class: 'theory-card' },
      el('h3', { text: 'YOUR WORKING THEORY' }),
      el('p', { text: s.theories })
    );
  }
  return el('div', { class: 'theory-card empty', text: 'No working theory yet. Use Notes to record one.' });
}

/* ------------------------------------------------------------------ */
/*  BINDING                                                            */
/* ------------------------------------------------------------------ */

function bind(caseObj, state, section) {
  qsa('[data-go]').forEach(b => b.onclick = () => openInvestigation(caseObj, state, b.dataset.go));
  qsa('[data-file]').forEach(b => b.onclick = () => {
    const id = b.dataset.file;
    if (!state.discoveredFiles.includes(id)) state.discoveredFiles = [...state.discoveredFiles, id];
    setProgress(caseObj, state); saveState(caseObj, state); reload(caseObj, state);
    revealCaseFiles(caseObj, state);
  });
  qsa('[data-evidence]').forEach(b => b.onclick = () => {
    const id = b.dataset.evidence;
    const e = caseObj.evidence[id];
    if (!e) return;
    if (caseObj.mode === 'interrogation') {
      if (!state.discoveredEvidence.includes(id)) state.discoveredEvidence = [...state.discoveredEvidence, id];
      if (!state.connections.includes(id)) state.connections.push(id);
    } else {
      if (!state.openedEvidence.includes(id)) state.openedEvidence = [...state.openedEvidence, id];
      const file = e.file;
      if (file && !state.discoveredFiles.includes(file)) state.discoveredFiles = [...state.discoveredFiles, file];
    }
    setProgress(caseObj, state); saveState(caseObj, state); reload(caseObj, state);
    openEvidenceModal(caseObj, state, id);
  });
  if (section === 'notes') {
    const saveBtn = qs('#save-notes');
    if (saveBtn) saveBtn.onclick = () => {
      state.notes = qs('#notes-editor').value;
      state.theories = qs('#theory-editor').value;
      setProgress(caseObj, state); saveState(caseObj, state);
      saveBtn.textContent = 'NOTES SAVED ✓';
      reload(caseObj, state);
    };
  }
  if (section === 'conclusion') {
    const submit = qs('#submit-theory');
    if (submit) submit.onclick = () => {
      state.finalTheory = qs('#final-theory').value;
      const scorer = typeof caseObj.scoreFinal === 'function' ? caseObj.scoreFinal : null;
      if (scorer) {
        const result = scorer(caseObj, state);
        state.finalScore = result.score;
        state.points = result.points;
        state.finalScoreNeedsReset = true;
      }
      saveState(caseObj, state); reload(caseObj, state);
    };
  }
}

function openEvidenceModal(caseObj, state, id) {
  const e = caseObj.evidence[id];
  if (!e) return;
  const modal = el('div', { class: 'evidence-modal' });
  const detail = el('div', { class: 'evidence-detail' });
  detail.append(
    el('button', { class: 'modal-close', text: '×' }),
    el('p', { class: 'overline', text: `EVIDENCE ${e.number || e.num} · FILE UNSEALED` }),
    el('h2', { text: e.title || e.name }),
    el('div', { class: 'detail-meta', text: `${e.type.toUpperCase()}${e.location ? ' · ' + e.location : ''}${e.time ? ' · ' + e.time : ''}` }),
    el('p', { class: 'detail-description', text: e.description })
  );
  if (e.interpretations) {
    detail.append(el('div', { class: 'interpretations' },
      el('label', { text: 'POSSIBLE READINGS' }),
      e.interpretations.map((it, i) => el('p', { text: `${String.fromCharCode(65 + i)}. ${it}` }))
    ));
  }
  if (e.proves || e.notProves) {
    detail.append(el('div', { class: 'proof-grid' },
      el('div', {}, el('label', { text: 'WHAT IT PROVES' }), el('p', { text: e.proves })),
      el('div', {}, el('label', { text: 'WHAT IT DOES NOT PROVE' }), el('p', { text: e.notProves }))
    ));
  }
  detail.append(el('div', { class: 'detail-actions' },
    el('button', { class: 'important-evidence', text: state.importantEvidence.includes(id) ? '★ IMPORTANT' : '☆ MARK IMPORTANT' })
  ));
  modal.append(detail);
  document.body.append(modal);
  detail.querySelector('.modal-close').onclick = () => modal.remove();
  detail.querySelector('.important-evidence').onclick = () => {
    state.importantEvidence = state.importantEvidence.includes(id) ? state.importantEvidence.filter(x => x !== id) : [...state.importantEvidence, id];
    saveState(caseObj, state); modal.remove(); reload(caseObj, state);
  };
}

// Re-render the open investigation (used after state changes).
function reload(caseObj, state) {
  if (!activeOverlay) return;
  const content = qs('#investigation-content');
  if (!content) return;
  clear(content);
  const renderers = caseObj.mode === 'interrogation' ? interrogationRenderers : investigationRenderers;
  const open = qsa('.investigation-tab.active');
  const section = open.length ? open[0].dataset.section : 'overview';
  const fn = renderers[section];
  if (fn) content.append(...fn(caseObj, state));
  bind(caseObj, state, section);
}

// When a file is opened, also unlock its evidence items' container (already handled in bind via state.discoveredFiles).
function revealCaseFiles(caseObj, state) {
  // evidence list filters by discoveredFiles; no extra action needed
}
