#pragma once

#include <pgmspace.h>

// The control page served at "/". Plain HTML with inline CSS and JS so it
// works offline on the local network. It talks to the display through the
// same HTTP endpoints documented in README.md (GET/POST /note, POST /clear).
static const char kIndexHtml[] PROGMEM = R"HTML(<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Sticky Note Display</title>
<style>
  body { font-family: -apple-system, system-ui, sans-serif; margin: 0; padding: 16px; background: #111; color: #eee; }
  main { max-width: 480px; margin: 0 auto; }
  h1 { font-size: 1.25rem; margin: 0 0 12px; }
  textarea { width: 100%; box-sizing: border-box; min-height: 9em; font-size: 1.1rem; padding: 8px; border-radius: 6px; border: 1px solid #555; background: #000; color: #fff; }
  .row { display: flex; flex-wrap: wrap; gap: 8px; margin-top: 10px; }
  button { font-size: 1rem; padding: 10px 14px; border-radius: 6px; border: 0; background: #2d6cdf; color: #fff; cursor: pointer; }
  button.key { background: #333; min-width: 44px; }
  button.secondary { background: #555; }
  #status { margin-top: 12px; color: #9a9; min-height: 1.2em; }
  #status.err { color: #e77; }
</style>
</head>
<body>
<main>
  <h1>Sticky Note Display</h1>
  <textarea id="text" placeholder="Song title&#10;Key: G&#10;Capo 2"></textarea>
  <div class="row" id="keys"></div>
  <div class="row">
    <button id="send">Send to display</button>
    <button id="clear" class="secondary">Clear display</button>
  </div>
  <div id="status"></div>
</main>
<script>
  const text = document.getElementById('text');
  const status = document.getElementById('status');
  const keys = document.getElementById('keys');

  function say(msg, err) {
    status.textContent = msg;
    status.className = err ? 'err' : '';
  }

  async function post(path, body) {
    try {
      const r = await fetch(path, { method: 'POST', body });
      if (!r.ok) throw new Error('HTTP ' + r.status);
      say('Sent ' + new Date().toLocaleTimeString());
    } catch (e) {
      say('Failed: ' + e.message, true);
    }
  }

  document.getElementById('send').onclick = () => {
    const body = new URLSearchParams();
    body.set('text', text.value);
    post('/note', body);
  };
  document.getElementById('clear').onclick = () => { text.value = ''; post('/clear'); };

  // Quick buttons that set the "Key:" line without retyping the whole note.
  for (const k of ['C', 'D', 'E', 'F', 'G', 'A', 'B']) {
    const b = document.createElement('button');
    b.className = 'key';
    b.textContent = k;
    b.onclick = () => {
      const lines = text.value.split('\n').filter(l => !/^key:/i.test(l.trim()));
      lines.push('Key: ' + k);
      text.value = lines.join('\n');
    };
    keys.appendChild(b);
  }

  // Load whatever the display currently shows.
  fetch('/note').then(r => r.text()).then(t => { text.value = t; }).catch(() => {});
</script>
</body>
</html>
)HTML";
