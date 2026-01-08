#pragma once
#include <pgmspace.h>

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Robot Arm 2.0</title>

<style>
  html, body {
    height: 100%;
    margin: 0;
    overscroll-behavior: none;
    background: #111;
    color: #eee;
    font-family: Arial, sans-serif;
  }

  body {
    padding: 15px;
  }

  h1 {
    text-align: center;
    margin-bottom: 20px;
  }

  .main {
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 20px;
    min-height: 100vh;
  }

  .panel {
    background: #1e1e1e;
    padding: 15px 20px;
    border-radius: 10px;
  }

  .panel h2 {
    text-align: center;
    margin-bottom: 10px;
  }

  button {
    background: #333;
    color: #fff;
    border: none;
    border-radius: 8px;
    padding: 12px 18px;
    font-size: 16px;
    user-select: none;
    -webkit-user-select: none;
    touch-action: none; /* CRITICAL: prevents mobile double-fire */
  }

  button:active {
    background: #00aaff;
  }

  .dpad {
    display: grid;
    grid-template-columns: 60px 60px 60px;
    grid-template-rows: 60px 60px 60px;
    gap: 6px;
    justify-items: center;
    align-items: center;
  }

  .up    { grid-column: 2; grid-row: 1; }
  .left  { grid-column: 1; grid-row: 2; }
  .right { grid-column: 3; grid-row: 2; }
  .down  { grid-column: 2; grid-row: 3; }

  .stack {
    display: flex;
    flex-direction: column;
    gap: 10px;
  }

  .functions {
    display: grid;
    grid-template-columns: repeat(3, 1fr);
    gap: 10px;
  }

  #save_button.active {
    background-color: aquamarine;
    color: #000;
  }

  /* Desktop / tablet */
  @media (min-width: 900px) {
    .main {
      flex-direction: row;
      flex-wrap: wrap;
      justify-content: center;
      align-items: flex-start;
    }
  }

  /* Phone landscape */
  @media (orientation: landscape) and (max-height: 500px) {
    body {
      padding: 8px;
    }

    h1 {
      display: none;
    }

    .main {
      flex-direction: row;
      flex-wrap: nowrap;
      height: 100%;
      gap: 10px;
    }

    .panel {
      padding: 10px;
    }

    button {
      font-size: 18px;
      padding: 16px 22px;
    }

    .dpad {
      grid-template-columns: 70px 70px 70px;
      grid-template-rows: 70px 70px 70px;
      gap: 8px;
    }

    .stack {
      gap: 12px;
    }

    .functions {
      grid-template-columns: repeat(2, 1fr);
      gap: 12px;
    }
  }
</style>
</head>

<body>

<h1>Robot Arm 2.0</h1>

<div class="main">

  <section class="panel">
    <h2>Movement</h2>
    <div class="dpad">
      <button class="up"
        onpointerdown="press('FORWARD')"
        onpointerup="release()"
        onpointercancel="release()">↑</button>

      <button class="left"
        onpointerdown="press('LEFT')"
        onpointerup="release()"
        onpointercancel="release()">←</button>

      <button class="right"
        onpointerdown="press('RIGHT')"
        onpointerup="release()"
        onpointercancel="release()">→</button>

      <button class="down"
        onpointerdown="press('BACKWARD')"
        onpointerup="release()"
        onpointercancel="release()">↓</button>
    </div>
  </section>

  <section class="panel">
    <h2>Vertical</h2>
    <div class="stack">
      <button onpointerdown="press('UP')" onpointerup="release()" onpointercancel="release()">Up</button>
      <button onpointerdown="press('DOWN')" onpointerup="release()" onpointercancel="release()">Down</button>
    </div>
  </section>

  <section class="panel">
    <h2>Pitch</h2>
    <div class="stack">
      <button onpointerdown="press('PITCHUP')" onpointerup="release()" onpointercancel="release()">Pitch +</button>
      <button onpointerdown="press('PITCHDOWN')" onpointerup="release()" onpointercancel="release()">Pitch -</button>
    </div>
  </section>

  <section class="panel">
    <h2>Gripper</h2>
    <div class="stack">
      <button onpointerdown="press('OPEN')" onpointerup="release()" onpointercancel="release()">Open</button>
      <button onpointerdown="press('CLOSE')" onpointerup="release()" onpointercancel="release()">Close</button>
    </div>
  </section>

  <section class="panel">
    <h2>Functions</h2>
    <div class="functions">
      <button onclick="cmd('SAVE')" id="save_button">Save</button>
      <button onclick="cmd('HOME')">Home</button>
      <button onclick="cmd('POSITION1')">1</button>
      <button onclick="cmd('POSITION2')">2</button>
      <button onclick="cmd('POSITION3')">3</button>
      <button onclick="cmd('POSITION4')">4</button>
    </div>
  </section>

</div>

<script>
  let active = false;

  function press(cmd) {
    if (active) return;
    active = true;
    fetch('/' + cmd);
  }

  function release() {
    if (!active) return;
    active = false;
    fetch('/STOP');
  }

  function cmd(c) {
    const saveBtn = document.getElementById('save_button');

    if (c === 'SAVE') {
      saveBtn.classList.toggle('active');
    } else if (c !== 'HOME') {
      saveBtn.classList.remove('active');
    }

    fetch('/' + c);
  }
</script>

</body>
</html>

)rawliteral";
