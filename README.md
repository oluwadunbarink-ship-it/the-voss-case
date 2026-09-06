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

## Why did I build this project

- The main aim of this project was to generally show one of the unlimited possibilities AI can be used for in the future, which is helping to analyse cases and piece together information as shown in Room 17. The possibilities are endless and i feel there are many ways to achieve a fully functional and advanced world with AI models and agents working around the clock 24/7
- The Voss case was simply inspired by similar game where i found on roblox and the popular 2018 game "Detroit: Became human", this was to make a friendly game where users can interact with a system normally and get used to the feeling, sounds etc of having to fully speak with an AI

- Also, I chose a conversational LLM because the core of The Voss Case is interrogation. A traditional scripted dialogue system would make the investigation predictable because players would only be able to choose from predetermined questions and responses. An LLM allows players to ask questions in their own words and receive responses that feel more like an actual interrogation.

- Additionally, I used an OpenCode model with a carefully designed system prompt rather than allowing the AI to freely generate the entire case. The case facts, evidence, suspect personality, and investigation state are controlled by the application, while the LLM is responsible primarily for generating Elias Voss's responses. This gave me a balance between AI flexibility and maintaining control over the story.

- I also chose a local LLM connection during development because it allowed me to test the game without exposing an API key in the browser. For the deployed version, I added a serverless proxy so the API key remains on the server rather than being sent to the client.


## What parts I mainly struggled with

- Honestly, AI was a chief role in this building project, but there was more to it that.  I had a role to play in sincerely completing this project, I really struggled with having to be able to get an open source AI to be able to answer the questions properly, so i started digging. It began with going through tiktok to see if anyone had created something similar. I found nothing. I went through youtube to link open source AI to a deployable website but i couldnt seem to find what i was looking for, then I began to explore bigger options. I looked for a way to route the AI through my localhost, falling upon OmniRoute, -which is the serverless proxy- it was the way I could route the tokens and get less tokens all for free. But the connecting and setting up was done by me

- Another big problem I had was with the AI's contingency to go outside the parameters which it had been given or being able to lack the answers to some humane questions.

## What skills I learnt in the process of doing all of this
At the beginning, I mainly thought of cybersecurity as protecting a website from obvious attacks. Building The Voss Case made me understand that security also has to be considered in the architecture of an AI application.

One of the biggest lessons was API-key security. I learned that putting an LLM API key directly in frontend JavaScript would expose it to anyone who inspected the website. I therefore separated the browser from the LLM provider by using a serverless backend proxy, keeping the API key in environment variables instead of committing it to the repository.

I also learned about the security risks created by trusting AI-generated content. An LLM can produce information that looks legitimate but is not actually part of the case. This taught me that AI output should not automatically be treated as trusted data. Important game state, evidence, and progression should be validated and controlled by the application.

Additionally I learnt: 
Prompt engineering under constraint
Designing system instructions that reduce hallucination and keep the AI within defined personality and knowledge boundaries.
Security-conscious development practices
Recognizing that AI-generated output cannot be treated as trusted data; validating state changes and evidence presentation on the application side rather than trusting the model.

Overall, the project changed my understanding of cybersecurity from simply protecting a system to designing the entire system so that sensitive information, user input, APIs, and AI-generated content are handled safely.

