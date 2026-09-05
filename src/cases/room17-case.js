export const room17Case = {
  id: 'room17',
  title: 'CASE 04: ROOM 17',
  name: 'The Woman in Room 17',
  mode: 'investigation',
  storageKey: 'room17-case-state',
  theme: 'room17',

  casePanel: {
    overline: 'Cold case · Reopened',
    heading: 'Room<br />17',
    copy: 'A woman vanished from Room 17 at the Grand Meridian in October 2008. Six days later, a body was found 40 kilometres away. Reopen the file.'
  },

  room: {
    caption: 'GRAND MERIDIAN · ROOM 17 · OCT 2008',
    label: 'EVIDENCE ARCHIVE',
    meta: 'REOPENED  /  FILE 04'
  },

  // Investigation mode uses analyst Q&A, not an interrogation room.
  analyst: {
    name: 'Chief Analyst',
    greeting: 'Sit down. You have the file. I help you read it. I do not solve it for you. Where do you want to start?',
    systemPrompt: `You are a senior cold-case investigative analyst reviewing the 2008 Grand Meridian Hotel case (the woman in Room 17). You are a professional analyst, NOT a narrator or storyteller. You answer questions about the evidence on file and reason about the case. Rules:
- NEVER reveal the canonical solution or "who did it" outright. 
- When asked a direct conclusion question (e.g. "who killed her?"), respond with the strongest current theories, the evidence that supports each, and what is still unknown.
- When asked to show/compare/cross-reference evidence, do so precisely and flag contradictions.
- Use evidence classifications: CONFIRMED (directly supported by reliable evidence), PROBABLE (strongly supported but not fully proven), SPECULATIVE (possible reading with insufficient evidence). Never present speculation as fact.
- For a proposed theory: (1) supporting evidence, (2) contradicting evidence, (3) missing evidence, (4) a confidence score out of 100, (5) the single most useful next investigative action.
- Keep answers focused and evidence-grounded, under 170 words. You may reference witness reliability and red herrings. Never invent evidence that is not on file.`
  },

  initialState: {
    discoveredFiles: ['file-001'],   // only initial police report is open at first
    openedEvidence: [],              // evidence items opened
    importantEvidence: [],
    notes: '',
    theories: '',
    finalTheory: '',
    statements: [],
    contradictions: [],
    finalScore: null,
    points: null,
    progress: 'INITIAL LEADS',
    connections: []
  },

  ui: {
    consideringNote: 'The analyst is reviewing the file…',
    connectedNote: 'Connected to localhost:20128',
    resetSpeaker: 'The archive is reset. Where do you want to begin?',
    inputPlaceholder: 'Ask the analyst about the evidence…'
  },

  progressStages: ['INITIAL LEADS', 'IDENTITY', 'TIMELINE', 'TRUTH', 'CONCLUSION']
};
