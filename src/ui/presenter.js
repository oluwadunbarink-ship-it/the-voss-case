import { el, qs, clear } from './dom.js';
import { escapeHtml } from '../core/escape.js';

// Renders the case panel (left column) for the active case.
export function renderCasePanel(caseObj) {
  const panel = qs('.case-panel');
  clear(panel);

  const overline = el('p', { class: 'overline', text: caseObj.casePanel.overline });
  const heading = el('h1', { html: caseObj.casePanel.heading });
  const copy = el('p', { class: 'case-copy', text: caseObj.casePanel.copy });

  panel.append(overline, heading, copy);

  if (caseObj.mode === 'interrogation') {
    // Voss-style quick-prompt evidence buttons
    const list = el('div', { class: 'evidence-list' });
    Object.values(caseObj.evidence).forEach(evItem => {
      const btn = el('button', {
        html: `<b>${escapeHtml(evItem.number)}</b> ${escapeHtml(evItem.name)}`
      });
      btn.dataset.prompt = caseObj.evidencePrompt?.(evItem.id) || '';
      btn.dataset.evidence = evItem.id;
      list.append(btn);
    });
    panel.append(list);
  } else {
    // Room 17: show a short archive blurb + file count
    const count = (caseObj.files || []).length;
    const blurb = el('div', { class: 'archive-note' });
    blurb.append(
      el('p', { text: `A live evidence analyst will answer your questions.` }),
      el('p', { class: 'archive-stat', text: `${count} case files · ${Object.keys(caseObj.evidence || {}).length} evidence items` })
    );
    panel.append(blurb);
  }
}

export function renderRoom(caseObj) {
  const room = qs('.room');
  room.setAttribute('data-mode', caseObj.mode);
  room.setAttribute('data-label', caseObj.room.label);
  room.setAttribute('data-meta', caseObj.room.meta);
  room.querySelector('.room-caption').textContent = caseObj.room.caption;
}
