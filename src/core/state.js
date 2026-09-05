export function clone(value) {
  if (typeof structuredClone === 'function') return structuredClone(value);
  return JSON.parse(JSON.stringify(value));
}

export function loadState(caseObj) {
  try {
    const saved = JSON.parse(localStorage.getItem(caseObj.storageKey) || '{}');
    return { ...clone(caseObj.initialState), ...saved };
  } catch {
    return clone(caseObj.initialState);
  }
}

export function saveState(caseObj, state) {
  localStorage.setItem(caseObj.storageKey, JSON.stringify(state));
}
