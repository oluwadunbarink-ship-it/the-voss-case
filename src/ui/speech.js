let voiceEnabled = localStorage.getItem('voss-voice-enabled') !== 'false';

export function isVoiceEnabled() { return voiceEnabled; }
export function setVoiceEnabled(v) {
  voiceEnabled = v;
  localStorage.setItem('voss-voice-enabled', String(v));
  if (!v && 'speechSynthesis' in window) window.speechSynthesis.cancel();
}
export function getVoiceEnabled() { return voiceEnabled; }

export function speak(text) {
  if (!voiceEnabled || !('speechSynthesis' in window) || !text.trim()) return;
  window.speechSynthesis.cancel();
  const u = new SpeechSynthesisUtterance(text);
  u.lang = 'en-US';
  u.rate = 0.92;
  u.pitch = 0.88;
  window.speechSynthesis.speak(u);
}
