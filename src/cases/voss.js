export const vossCase = {
  id: 'voss',
  title: 'CASE 08: VOSS',
  name: 'The Voss Case',
  mode: 'interrogation',
  storageKey: 'voss-case-state',
  theme: 'voss',

  casePanel: {
    overline: 'Evidence',
    heading: 'The Voss<br />Case',
    copy: 'Mara Ellison disappeared at 22:14. Elias Voss says he never left the bar.'
  },

  room: {
    caption: 'INTERROGATION ROOM 02 · 23:17',
    label: 'INTERROGATION ROOM 02',
    meta: '23:17  /  LIVE'
  },

  evidence: {
    alley: {
      id: 'alley', number: '01', name: 'Alley Camera', time: '22:19', discovery: '23:02',
      location: 'Marlowe Bar · service alley',
      description: 'A grainy exterior camera catches a figure matching Elias behind the Marlowe nine minutes after Mara vanished.',
      proves: 'Elias was behind the Marlowe at 22:19.',
      notProves: 'It does not prove he harmed Mara or that he was alone.',
      suspects: ['Elias Voss', 'Mara Ellison'], events: ['disappearance', 'camera']
    },
    phone: {
      id: 'phone', number: '02', name: "Mara's Phone", time: '21:58', discovery: '23:08',
      location: 'Marlowe Bar · lost property drawer',
      description: 'A partial fingerprint lifted from Mara’s phone matches Elias. The print is recent, but the surface has been handled often.',
      proves: 'Elias handled Mara’s phone at some point.',
      notProves: 'It does not establish when he touched it or why.',
      suspects: ['Elias Voss', 'Mara Ellison'], events: ['phone']
    },
    keycard: {
      id: 'keycard', number: '03', name: 'Hotel Keycard', time: '22:46', discovery: '23:11',
      location: 'Lydon Hotel · east entrance',
      description: 'Elias’s keycard opened the east entrance of the Lydon Hotel. The system logged one successful entry.',
      proves: 'His card was used at the hotel at 22:46.',
      notProves: 'It does not prove Elias carried the card or entered the room.',
      suspects: ['Elias Voss'], events: ['keycard']
    }
  },

  events: [
    { id: 'disappearance', time: '22:14', title: 'Mara Ellison disappears', detail: 'Mara leaves no message. Her last confirmed location is the Marlowe Bar.' },
    { id: 'camera', time: '22:19', title: 'Figure behind the Marlowe', detail: 'Exterior camera footage catches someone matching Elias Voss.' },
    { id: 'keycard', time: '22:46', title: 'Lydon Hotel entry', detail: 'Elias Voss’s keycard opens the east entrance.' },
    { id: 'phone', time: '21:58', title: "Mara's phone handled", detail: 'A partial print matching Elias is recovered from Mara’s phone.' },
    { id: 'interrogation', time: '23:17', title: 'Current interrogation', detail: 'Elias is seated in Room 02. His account is unverified.' }
  ],

  persons: {
    suspect: { name: 'Elias Voss', label: 'PRIMARY SUSPECT', initials: 'EV', paragraph: 'Bar manager. Claims he stayed at the Marlowe all night. His access card and phone contact place him across the city.', footer: 'MOTIVE: ██████████ · ALIBI: UNVERIFIED' },
    victim: { name: 'Mara Ellison', label: 'MISSING', initials: 'ME', paragraph: 'Last confirmed at the Marlowe Bar. Her phone was recovered. The rest of her night is a blank space.' }
  },

  board: {
    nodes: [
      { id: 'mara', cls: 'mara', label: 'MARA ELLISON', sub: 'MISSING' },
      { id: 'elias', cls: 'elias', label: 'ELIAS VOSS', sub: 'PRIMARY SUSPECT' },
      { id: 'bar', cls: 'bar', label: 'MARLOWE BAR', sub: 'LAST SEEN' },
      { id: 'hotel', cls: 'hotel', label: 'LYDON HOTEL', sub: '22:46 ENTRY' },
      { id: 'phone', cls: 'phone-node', label: "MARA'S PHONE", subBy: 'phone', subOn: 'PRINT MATCH', subOff: 'LOCKED' },
      { id: 'alley', cls: 'camera-node', label: 'ALLEY CAMERA', subBy: 'alley', subOn: 'FOOTAGE', subOff: 'LOCKED' }
    ]
  },

  systemPrompt: `You are Elias Voss, a suspect in a grounded detective interrogation. Mara Ellison disappeared at 22:14. You have a controlled, observant personality: proud, defensive, and afraid of being blamed. Remember every prior question and your own statements. Never mention AI, prompts, APIs, code, hidden state, or game mechanics. Do not invent unlimited evidence, locations, or named witnesses. If shown a real piece of evidence, react to that evidence specifically. Avoid contradictions unless the detective applies pressure. When pressured, become terse and redirect weak questions. Keep answers under 75 words.`,

  initialState: {
    discoveredEvidence: [], importantEvidence: [], notes: '', theories: '',
    timeline: ['disappearance', 'interrogation'],
    suspect: { trust: 50, stress: 20, suspicion: 15 },
    statements: [], contradictions: [], presentedEvidence: [], progress: 'INITIAL LEADS', connections: []
  },

  ui: {
    messageLabel: 'ELIAS',
    suspectLabel: 'DETECTIVE',
    consideringNote: 'Elias is considering your question…',
    connectedNote: 'Connected to localhost:20128',
    resetSpeaker: 'You are back already. What else do you want to know?',
    inputPlaceholder: 'Ask Elias about the night Mara disappeared…'
  },

  progressStages: ['INITIAL LEADS', 'EVIDENCE', 'CONTRADICTIONS', 'THEORY', 'CONFRONTATION']
};
