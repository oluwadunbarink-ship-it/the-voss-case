const DEFAULT_BASE = 'https://api.openai.com/v1';
const DEFAULT_MODEL = 'gpt-4.1-mini';

async function handleChatRequest(body) {
  const apiKey = process.env.LLM_API_KEY || process.env.OPENAI_API_KEY;
  if (!apiKey) {
    return {
      status: 500,
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ error: { message: 'Server is missing LLM_API_KEY. Set it in your hosting provider environment.' } }),
    };
  }

  const base = (process.env.LLM_BASE_URL || DEFAULT_BASE).replace(/\/+$/, '');
  const model = process.env.LLM_MODEL || body.model || DEFAULT_MODEL;

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
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ error: { message: `LLM provider unreachable: ${err.message}` } }),
    };
  }

  const text = await resp.text();
  return {
    status: resp.status,
    headers: { 'Content-Type': 'application/json' },
    body: text,
  };
}

exports.handler = async (event) => {
  if (event.httpMethod !== 'POST') {
    return { statusCode: 405, headers: { Allow: 'POST' }, body: JSON.stringify({ error: { message: 'Method Not Allowed' } }) };
  }

  let body;
  try {
    body = JSON.parse(event.body || '{}');
  } catch {
    return { statusCode: 400, headers: { 'Content-Type': 'application/json' }, body: JSON.stringify({ error: { message: 'Invalid JSON body' } }) };
  }

  const result = await handleChatRequest(body);
  return { statusCode: result.status, headers: result.headers, body: result.body };
};
