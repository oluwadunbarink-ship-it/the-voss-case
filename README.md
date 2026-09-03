# The Voss Case

> An AI-driven detective interrogation game. Interrogate Elias Voss, the only suspect in the disappearance of Mara Ellison.

## About

Mara Ellison disappeared from the Marlowe Bar at 22:14. Elias Voss says he never left. You are the detective. Ask questions, present evidence, and decide when to push — Elias's trust, stress, and suspicion shift with every word.

The game runs entirely in the browser. Suspect responses come from a local OpenAI-compatible gateway and are spoken aloud with the Web Speech API.

## Play

Open the site and type a question for Elias. Present evidence from the case panel to raise the stakes. Use INVESTIGATE to open the case file.

## For Developers

### Running locally

```bash
npm install
npm start
```

Then open `http://localhost:3000`.

Requires Node.js 18+. No API key is stored or sent by the client; the local gateway at `localhost:20128` must be running first.

### Key Files

| Path | Purpose |
| --- | --- |
| `index.html` | Single-page game shell with SEO meta tags and structured data |
| `app.js` | All game logic: state, evidence, contradictions, LLM conversation |
| `styles.css` | Complete styled theme for the interrogation room and case UI |
| `404.html` | Custom not-found page |
| `sitemap.xml` | Search-engine index of public pages |
| `robots.txt` | Crawler rules |
| `llms.txt` | Machine-readable summary for LLM tools |

### Evidence

| # | Item | Key fact |
| --- | --- | --- |
| 01 | Alley camera | Elias matched behind the Marlowe at 22:19 |
| 02 | Mara's phone | Partial fingerprint matching Elias |
| 03 | Hotel keycard | Used at the Lydon Hotel at 22:46 |

### Gameplay Systems

- **Hidden state**: Elias tracks trust, stress, and suspicion (0–100).
- **Contradictions**: Pressing Elias on prior statements can crack his story.
- **Progression**: INITIAL LEADS → EVIDENCE → CONTRADICTIONS → THEORY → CONFRONTATION.

## License

Prototype project. Not for commercial distribution.
