// Heuristic scorer for CASE: Room 17 final theory.
// Grades the player's free-text theory against the canonical solution by
// checking whether their reasoning touches the required facts. This is a
// reasonable approximation of "does their reasoning match the underlying facts".

const identityHints = [
  /ingrid/i, /sørensen/i, /sorensen/i, /norweg/i, /journalist/i, /photographer/i, /press badge/i
];
const timelineHints = [
  /service (stairs|exit|door)/i, /back (stairs|exit)/i, /03:0[0-5]/i, /cutting/i, /branch line/i, /~?2[0-3]:[0-5]{2}.*cutting/i, /midnight/i, /3:0[0-5]/i
];
const deathHints = [
  /accident/i, /exposure/i, /hypothermia/i, /fell/i, /fall/i, /cutting/i, /cold\b/i, /not (a )?(made|designed|planned) (to be )?murder/i
];
const motiveHints = [
  /adoption/i, /broker/i, /scheme/i, /clinic/i, /silence/i, /threat/i, /investigat/i, /expose/i, /fixer/i
];
const majorEvidenceHints = [
  /false name/i, /elena hart/i, /phony/i, /alias/i, /sedative/i, /drugged/i, /ticket/i, /misdirection/i, /belongings|suitcase/i, /if i am delayed/i
];
const misleadingHints = [
  /red herring/i, /mislead/i, /train ticket/i, /window/i, /two card/i, /loyalty card/i, /taxi/i, /staged|planted/i, /not relevant/i
];

function scoreHints(text, hints) {
  let hits = 0;
  hints.forEach(re => { if (re.test(text)) hits++; });
  return hits;
}

export function scoreRoom17Final(caseObj, state) {
  const text = state.finalTheory || '';
  const rubric = caseObj.scoring.rubric;
  const units = [
    [identityHints, Math.ceil(rubric[0].points / identityHints.length)],
    [timelineHints, Math.ceil(rubric[1].points / timelineHints.length)],
    [deathHints, Math.ceil(rubric[2].points / deathHints.length)],
    [motiveHints, Math.ceil(rubric[3].points / motiveHints.length)],
    [majorEvidenceHints, Math.ceil(rubric[4].points / majorEvidenceHints.length)],
    [misleadingHints, Math.ceil(rubric[5].points / misleadingHints.length)]
  ];
  const points = rubric.map((r, i) => {
    const [hints, unit] = units[i];
    const hits = scoreHints(text, hints);
    const got = Math.max(hits > 0 ? 1 : 0, Math.min(r.points, hits * unit));
    return { label: r.label, max: r.points, got: Math.min(r.points, got) };
  });
  const score = points.reduce((sum, p) => sum + p.got, 0);
  return { score, points };
}
