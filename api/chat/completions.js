const DEFAULT_BASE = 'https://api.openai.com/v1';
const DEFAULT_MODEL = 'gpt-4.1-mini';

async function handleChatRequest(body) {
  const apiKey = process.env.LLM_API_KEY || process.env.OPENAI_API_KEY;
  if (!apiKey) {
    return {
      status: 500,
      body: { error: { message: 'Server is missing LLM_API_KEY. Set it in your hosting provider environment.' } },
    };
  }

  const base = (process.env.LLM_BASE_URL || DEFAULT_BASE).replace(/\/+$/, '');
  const model = process.env.LLM_MODEL || (body && body.model) || DEFAULT_MODEL;

  const payload = { ...body, model };

  let resp;
  try {
    resp = await fetch(`${base}/chat/completions`, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
        'Authorization': `Bearer ${apiKey}`,
      },
      body: JSON.stringify(payload),
    });
  } catch (err) {
    return {
      status: 502,
      body: { error: { message: `LLM provider unreachable: ${err.message}` } },
    };
  }

  const text = await resp.text();
  return { status: resp.status, body: text };
}

export default async function handler(req, res) {
  if (req.method !== 'POST') {
    return res.status(405).json({ error: { message: 'Method Not Allowed' } });
  }

  const result = await handleChatRequest(req.body);
  res.status(result.status).setHeader('Content-Type', 'application/json').send(result.body);
}
