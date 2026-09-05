// HIDDEN canonical solution for CASE: Room 17.
// This module is referenced only by the final-theory scorer. It is never rendered to the player.

export const room17Solution = {
  identity: {
    realName: 'Ingrid Sørensen',
    falseName: 'Elena Hart',
    details: 'A Norwegian-born freelance journalist and photographer, 34. Not a spy, not a client of the scheme. She was investigating a private, off-the-books adoption-brokering network linked to a 1990s clinic and, later, to the Grand Meridian Hotel via a fixer (the duty manager\'s associate).'
  },
  reasonFalseName: 'She used "Elena Hart" to conceal herself while gathering evidence, because the scheme\'s fixer had already threatened a prior journalist ("a company man" silencing people about "the babies"). She feared for her safety but pressed on.',
  missingSixHours: 'Between ~02:00 and ~03:15 on 4 October 2008, having learned her source was too afraid to go on record, Elena decided not to wait. She left Room 17 via the rear service stairs (using a staff service card) around 03:02, walked to a closed station, and took the disused branch line toward the cutting to retrieve a second evidence set she believed had been stashed there before going public. She fell into the cutting in the dark. Lying exposed overnight, she died of hypothermia, compounded by a head injury and a sedative dose (administered to her on Day 3) that impaired her.',
  causeOfDeath: 'Accident. A fall in the dark into a disused railway cutting followed by exposure/hypothermia. Not a premeditated murder by a single "villain". The fixer\'s associate is culpable for drugging her and for the cover-up, but not an active killer at the scene.',
  travelToSite: 'From the hotel to the cutting: rear service exit → short walk to the closed station → the branch line path. No logged taxi; she walked the final leg. Evidence: rear-stairs CCTV (03:02), the staff service key, the untraced walk/cab, the map/route, and the location of the body.',
  whoKnew: {
    fixer: 'The duty manager\'s associate (the "large man"). He knew she was investigating and tried to stop her on Day 3 (the sedative), then withheld the staff-service-card and CCTV-gap details from the original inquiry.',
    manager: 'The duty manager. He knew about the visitor and understated the CCTV gap; cleared of wrongdoing too readily in the original report.',
    maidM: 'A maid. She was the source who was too afraid to talk; knew about the "babies" drop and the threats.'
  },
  belongingsLeft: 'Deliberate. She left her clothes and suitcase so the disappearance would read as a voluntary departure, keeping attention away from her midnight retrieval of records. The unused train ticket and folded map were part of staging / genuine travel planning. Both were misread by the original inquiry.',
  officialTimelineContradiction: 'The room contained a lamp cord run under the door and a freshly slept-in bed (a second cleaner contradicted the first); the CCTV 11-minute gap; and the unsigned service-stairs door log. The official 03:02 "exit" relied on an incomplete record.',
  redHerrings: [
    'The suspicious phone call & the unlisted number (M. Andrade): a genuine source, not the killer.',
    'The "large man": the fixer\'s associate, central to the cover-up but not the killer at the scene.',
    'The cash payment / drained account / shell-company transfer: covered how the scheme paid the fixer; not a murder transaction.',
    'The two café loyalty cards: a second set of fingerprints pointing to the associate\'s visits; cosmetic, not the core.',
    'The unused train ticket: misdirection toward "voluntary departure"; it was real planning, not the route she died on.',
    'The second cleaner\'s "struggle" impression: a misread of a slept-in bed, not violence.',
    'The waxed takeaway cup & folded map: genuine but over-weighted by analysts.',
    'The closed window: she did not exit through it; it was left open for air.'
  ],
  mostImportantRule: 'Every twist is foreshadowed: the sedative bottle (meds-1), the staff key, the CCTV gap, the lamp cord, the sleeping cleaner contradiction, the press badge at the site, the pendant, the hidden photograph, the notebook shorthand, and the deleted "if I am delayed" SMS.'
};

export const room17Scoring = {
  max: 100,
  rubric: [
    { label: 'Correct identity (Ingrid Sørensen / the journalist)', points: 20, evidence: ['emp-1', 'pict-1', 'id-pendant', 'ident-1'] },
    { label: 'Correct timeline (incl. the ~03:02 rear exit and the cutting)', points: 20, evidence: ['cctv-dark', 'key-2', 'risgard', 'forensic-1'] },
    { label: 'Correct explanation of death (accident/exposure in the cutting)', points: 20, evidence: ['forensic-1', 'toxic-1'] },
    { label: 'Correct motive (the adoption-scheme investigation / silence)', points: 15, evidence: ['adoption-1', 'source-1', 'notes-shorthand'] },
    { label: 'Correctly explains major evidence (false name, belongings, ticket, sedative)', points: 15, evidence: ['ident-1', 'ticket-1', 'meds-1', 'phone-sms'] },
    { label: 'Correctly identifies misleading evidence (red herrings)', points: 10, evidence: ['cafe-card', 'r17-window', 'taxi-1'] }
  ]
};
