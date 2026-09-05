# The Voss Case

> An AI-driven detective interrogation game. Interrogate Elias Voss, the only suspect in the disappearance of Mara Ellison.

## About

Mara Ellison disappeared from the Marlowe Bar at 22:14. Elias Voss says he never left. You are the detective. Ask questions, present evidence, and decide when to push. Elias's trust, stress, and suspicion shift with every word.

The game runs in the browser. Suspect responses come from an OpenAI-compatible LLM and are spoken aloud with the Web Speech API. The app works in two modes:

- **Local**: calls a local gateway at `localhost:20128` (development only).
- **Deployed**: calls a serverless proxy on the same origin, which forwards to your LLM provider server-side so your API key never reaches the browser.

## Play

Open the site and type a question for Elias. Present evidence from the case panel to raise the stakes. Use INVESTIGATE to open the case file.

## For Developers

### Running locally

```bash
npm install
npm start
```

Then open `http://localhost:3000`.

Requires Node.js 18+. When run locally, the app expects a local gateway at `localhost:20128`. When hosted on a public domain, it automatically routes through the included serverless proxy instead.

### Deploying online (so people can download and play)

The repo includes serverless proxies for **both Netlify and Vercel**. Deploy to one of them and set these environment variables on the platform:

| Variable | Required | Purpose |
| --- | --- | --- |
| `LLM_API_KEY` (or `OPENAI_API_KEY`) | Yes | Your LLM provider API key. Never committed to the repo. |
| `LLM_BASE_URL` | No | OpenAI-compatible base URL. Defaults to `https://api.openai.com/v1`. |
| `LLM_MODEL` | No | Model to use. Defaults to `gpt-4.1-mini`. |

Netlify: deploy the repo from GitHub (publish dir `.`). Netlify reads `netlify.toml` for the function route and 404 handling; set the env vars under **Site settings → Environment variables**.

Vercel: import the repo (framework = plain static). Vercel reads `vercel.json` for 404 handling; set the env vars under **Project → Settings → Environment Variables**.

Deploying is free on both platforms' hobby/free tiers.

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
| `api/chat/completions.js` | Serverless chat proxy (Vercel) |
| `netlify/functions/chat.js` | Serverless chat proxy (Netlify) |
| `netlify.toml` | Netlify build/redirect/404 config |
| `vercel.json` | Vercel static + 404 config |

### Evidence

| # | Item | Key fact |
| --- | --- | --- |
| 01 | Alley camera | Elias matched behind the Marlowe at 22:19 |
| 02 | Mara's phone | Partial fingerprint matching Elias |
| 03 | Hotel keycard | Used at the Lydon Hotel at 22:46 |

### Gameplay Systems

- **Hidden state**: Elias tracks trust, stress, and suspicion (0-100).
- **Contradictions**: Pressing Elias on prior statements can crack his story.
- **Progression**: INITIAL LEADS → EVIDENCE → CONTRADICTIONS → THEORY → CONFRONTATION.

## License

Prototype project. Not for commercial distribution.
