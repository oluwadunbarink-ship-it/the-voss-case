import { vossCase } from './voss.js';
import { room17Case } from './room17-case.js';
import { room17Files, room17Evidence } from './room17-evidence.js';
import { room17Solution, room17Scoring } from './room17-solution.js';
import { scoreRoom17Final } from './room17-scoring.js';

// Attach mode-specific data to each case object.
room17Case.files = room17Files;
room17Case.evidence = room17Evidence;
room17Case.solution = room17Solution;
room17Case.scoring = room17Scoring;
room17Case.scoreFinal = scoreRoom17Final;

export const CASES = [vossCase, room17Case];

export function getCase(id) {
  return CASES.find(c => c.id === id) || CASES[0];
}
