#include "index_html.h"

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <style>
  .col { position: absolute; width: 10px; height: 10px; top: 0;}
  .point { position: absolute; width: 10px; height: 10px; border: 1px solid white; transform: translate(-5px, -5px); pointer-events: none}
  </style>
</head>
<body style="background-color: #111; color: #DDD; margin: 0;"><style>input {width: 50px}</style>
  <img id="cam" src="/image.bmp" style="max-width:100%;height:auto">
  <div id="points" style="position: absolute; top: 0; left: 0; pointer-events: none;"></div>
  <div id="currentColors" style="position: absolute; top: 0; left: 180px; width:210px; height:120px;"></div>
  <div id="info"></div>
  <button id="testMode">Test mode</button>
  <div><table id="allInputs">
    <tr><td><label>Color change speed (0-1):</td><td><input type="text" id="blendFactor"></label></td></tr>
    <tr><td><label>Vignette correction (0-1):</td><td><input type="text" id="vignette"></label> Exp<input type="text" id="vignetteExponent"></td></tr>
    <tr><td><label>Vignette center:</td><td><input type="text" id="hotspotX">X / Y<input type="text" id="hotspotY"></label></td></tr>
    <tr><td><label>Turn off LED below (0-255):</td><td><input type="text" id="black"></label></td></tr>
    <tr><td><label>Frames per second:</td><td><input type="text" id="fps"></label></td></tr>
    <tr><td><label>LED direction:</td><td><input type="text" id="ledDir"></label> First LED idx<input type="text" id="led1st"></td></tr>
    <tr><td><label>Camera brightnes:</td><td><input type="text" id="brigth"></label> Gain<input type="text" id="gain"></td></tr>
    <tr><td><label>Camera contrast:</td><td><input type="text" id="contr"></label></td></tr>
    <tr><td><label>Vertical distortion:</td><td><input type="text" id="vert"></label></td></tr>
    <tr><td><label>ColorCorrection:</td><td><input class="small" type="text" id="correction0"><input class="small" type="text" id="correction1"><input class="small" type="text" id="correction2"></label></td></tr>
    <tr><td><label></td><td><input class="small" type="text" id="correction3"><input class="small" type="text" id="correction4"><input class="small" type="text" id="correction5"></label></td></tr>
    <tr><td><label></td><td><input class="small" type="text" id="correction6"><input class="small" type="text" id="correction7"><input class="small" type="text" id="correction8"></label></td></tr>
    <tr><td><label>Individual gain at point 123:</td><td><input class="small" type="text" id="pointFactor1"><input class="small" type="text" id="pointFactor2"><input class="small" type="text" id="pointFactor3"></label></td></tr>
    <tr><td><label>Individual gain at point 456:</td><td><input class="small" type="text" id="pointFactor4"><input class="small" type="text" id="pointFactor5"><input class="small" type="text" id="pointFactor6"></label></td></tr>
    <tr><td><label>LEDColorCorrection:</td><td>R<input class="small" type="text" id="ledFactor0">G<input class="small" type="text" id="ledFactor1">B<input class="small" type="text" id="ledFactor2">Gamma<input class="small" type="text" id="ledGamma"></label></td></tr>
  </table></div>
  <script>
    document.getElementById('cam').addEventListener('click', function(e) {
      var r = this.getBoundingClientRect();
      var x = Math.round(e.clientX - r.left);
      var y = Math.round(e.clientY - r.top);
      fetch(`/click?x=${x}&y=${y}`)
        .then(res => res.text())
        .then(html => {
          document.getElementById('info').innerHTML += html;
          FetchServerSettings();
        })
        .catch(console.error);
    });
    setInterval(PollForColor, 3000);

    function modPos(x, n){ const r = x % n; return (r < 0) ? r + n : r; }

    function logicalFromIndex(i, firstLed, ledDir, H, V){
      const N = 2*H + 2*V;
      const p = modPos((i - firstLed) * ledDir, N);
      if (p < H)           return { y:0, x: H>1 ? p/(H-1) : 0 };
      if (p < H+V)         return { x:1, y: V>1 ? (p-H)/(V-1) : 0 };
      if (p < H+V+H)       return { y:1, x: H>1 ? (p-H-V)/(H-1) : 0 };
      return                      { x:0, y: V>1 ? 1-((p-H-V-H)/(V-1)) : 0 };
    }

    function PollForColor() {
      fetch(`/leds`)
        .then(res => res.json())
        .then(data => {
          var firstLed = document.getElementById('led1st').value * 1;
          var ledDir = document.getElementById('ledDir').value * 1;
          var container = document.getElementById('currentColors');
          var newHTML = "";
          for (var idx = 0; idx < data.colors.length; idx++) {
            var pos = logicalFromIndex(idx, firstLed, ledDir, data.h, data.v);
            newHTML += "<div class=\"col\" style=\"left: "+(pos.x*100)+"%; top: "+(pos.y*100)+"%; width: "+(100/data.h+1)+"%; height: "+(100/data.v+1)+"%; background-color: "+data.colors[idx]+"\"></div>";
          }
          container.innerHTML = newHTML;
        });
      document.getElementById('cam').src="/image.bmp?"+Date.now();
    }

    function FetchServerSettings() {
      document.getElementById('allInputs').style.opacity = 0.5;
      fetch(`/settings`)
        .then(res => res.text())
        .then(text => {
          const json = JSON.parse(text);
          for (var x in json) {
            if (document.getElementById(x)) document.getElementById(x).value = json[x];
          }
          var container = document.getElementById('points');
          var newHTML = "";
          for (var idx = 1; idx <= 6; idx++) {
            var p = json['points'+idx];
            if (p) {
              newHTML += "<div class=\"point\" style=\"left: "+(p.x)+"px; top: "+(p.y)+"px;\"></div>";
            }
          }
          container.innerHTML = newHTML;
          document.getElementById('allInputs').style.opacity = 1;
        })
    }

    document.getElementById('testMode').onclick = function() { fetch(`/testMode`); };

    function sendInput(id) {
      const el = document.getElementById(id);
      if (!el) return;
      function send(){
        document.getElementById('allInputs').style.opacity = 0.5;
        fetch(`/input?${id}=${el.value}`).then(() => FetchServerSettings());
      }
      el.addEventListener('keydown', function(e){ if (e.key === 'Enter') send(); });
      el.addEventListener('focusout', function(e){ send(); });
    }
    ['blendFactor','vignette','vignetteExponent','hotspotX','hotspotY',
     'black','fps','ledDir','led1st','brigth','gain','contr','ledGamma', 'vert']
     .forEach(sendInput);
    for (var i=0;i<9;i++) sendInput('correction'+i);
    for (var i=1;i<=6;i++) sendInput('pointFactor'+i);
    for (var i=0;i<3;i++) sendInput('ledFactor'+i);
    FetchServerSettings();
  </script>
</body></html>
)rawliteral";
