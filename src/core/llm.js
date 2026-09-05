const IS_DEPLOYED = !['localhost', '127.0.0.1', '0.0.0.0'].includes(window.location.hostname);
const ENV_BASE = window.__LLM_BASE_URL__ || '';
export const API_BASE = ENV_BASE
  ? ENV_BASE.replace(/\/+$/, '')
  : (IS_DEPLOYED ? '/api' : 'http://localhost:20128/v1');
export const MODEL = IS_DEPLOYED ? 'auto-hosted' : 'auto/offline';

export function responseText(payload) {
  if (typeof payload.output_text === 'string') return payload.output_text;
  const choice = payload.choices?.[0]?.message?.content;
  if (typeof choice === 'string') return choice;
  return payload.output?.flatMap(item => item.content || []).find(item => item.text)?.text || '';
}

export async function requestChat({ system, messages, temperature = 0.7, maxTokens = 140 }) {
  const response = await fetch(`${API_BASE}/chat/completions`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({
      model: MODEL,
      messages: [system, ...messages],
      temperature,
      max_tokens: maxTokens,
      stream: false
    })
  });
  if (!response.ok) {
    if (response.status === 402) throw new Error('Local gateway returned 402: the selected route needs credits or an available provider.');
    if (response.status === 429) throw new Error('Local challenge service is rate-limited (429). Wait a moment and retry.');
    throw new Error(`Service returned ${response.status}`);
  }
  const text = responseText(await response.json());
  if (!text) throw new Error('Service returned no response');
  return text;
}
