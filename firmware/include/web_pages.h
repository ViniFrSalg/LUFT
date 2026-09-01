#pragma once

#include <Arduino.h>

const char DASHBOARD_HTML[] PROGMEM = R"rawliteral(
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <title>LUFT Live Data</title>
  <style>
    body { margin: 0; font-family: Arial, sans-serif; background: linear-gradient(180deg, #e9f7ff, #f6fcff); color: #1f3a60; }
    .page { max-width: 900px; margin: 0 auto; padding: 1.5rem; }
    .hero { background: #d9efff; border-radius: 20px; padding: 2rem; box-shadow: 0 16px 35px rgba(94, 160, 230, 0.16); }
    .hero h1 { margin: 0 0 0.5rem; font-size: 2.4rem; }
    .hero p { margin: 0; color: #3d5a7a; }
    .cards { display: grid; grid-template-columns: repeat(auto-fit, minmax(220px, 1fr)); gap: 1rem; margin-top: 1.5rem; }
    .card { background: white; border-radius: 18px; padding: 1.4rem; border: 1px solid rgba(94, 200, 255, 0.4); box-shadow: 0 10px 22px rgba(94, 200, 255, 0.12); }
    .card h2 { margin: 0 0 0.75rem; color: #1a4f7a; }
    .card p { margin: 0; font-size: 1.3rem; line-height: 1.5; }
    .status { margin-top: 1.5rem; color: #3d5a7a; font-size: 0.96rem; }
    .footer { margin-top: 2rem; text-align: center; color: #6d7c96; font-size: 0.95rem; }
    .page-switch { display: inline-block; margin-top: 1.25rem; padding: 0.75rem 1rem; border-radius: 999px; background: #1a4f7a; color: white; font-weight: 700; text-decoration: none; }
    .page-switch:hover, .page-switch:focus-visible { background: #123b5d; }
  </style>
</head>
<body>
  <div class="page">
    <section class="hero">
      <h1>LUFT Live Sensor Data</h1>
      <p>Current air quality readings from the ESP32-S3: CO₂, bVOC, and particles from the optical sensor.</p>
      <a class="page-switch" href="/sophia">Learn about LUFT</a>
    </section>
    <div class="cards">
      <div class="card">
        <h2>CO₂</h2>
        <p id="co2">Loading...</p>
      </div>
      <div class="card">
        <h2>bVOC</h2>
        <p id="bvoc">Loading...</p>
      </div>
      <div class="card">
        <h2>Particles</h2>
        <p id="pm">Loading...</p>
      </div>
    </div>
    <div class="status">Data refreshes every 2 seconds. If you see dashes, the sensor is still warming up.</div>
    <div class="footer">ESP32-S3 web server is serving this page from your local network.</div>
  </div>
  <script>
    async function updateData() {
      try {
        const res = await fetch('/data');
        if (!res.ok) return;
        const json = await res.json();
        document.getElementById('co2').textContent = json.co2.toFixed(1) + ' ppm';
        document.getElementById('bvoc').textContent = json.bvoc.toFixed(2) + ' ppm';
        document.getElementById('pm').textContent = json.pm.toFixed(1) + ' µg/m³';
      } catch (err) {
        console.error(err);
      }
    }
    setInterval(updateData, 2000);
    updateData();
  </script>
</body>
</html>
)rawliteral";

const char SOPHIA_INDEX_HTML[] PROGMEM = R"rawliteral(
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width,initial-scale=1" />
  <title>LUFT — Air Quality</title>
  <link rel="stylesheet" href="/sophia/styles.css">
</head>
<body>
  <header class="site-header">
    <div class="container header-content">
      <h1 class="brand">LUFT</h1>
      <nav class="nav" aria-label="Main navigation">
        <a href="#about">About</a>
        <a href="#health">Health Issues</a>
        <a href="#solutions">Solutions</a>
      </nav>
      <a class="page-switch" href="/">View live data</a>
    </div>
  </header>

  <main>
    <section class="hero">
      <div class="container">
        <h2>LUFT — Community Air Quality</h2>
        <p class="subtitle">Tracking air quality and sharing simple, practical steps to protect health at home and in the city.</p>
      </div>
    </section>

    <section id="about" class="section container">
      <div class="panel">
        <h3>About LUFT</h3>
        <p>LUFT is a collaborative project to measure local air quality and help communities reduce exposure to harmful pollutants. We combine easy-to-use sensors, clear data, and practical guidance for everyday life.</p>
      </div>
    </section>

    <section id="health" class="section alt">
      <div class="container">
        <div class="panel">
          <h3>Health Issues from Air Pollution</h3>
          <p>Air pollution contains gases and tiny particles that can penetrate the lungs and bloodstream. Short-term exposure can cause coughing, throat irritation, and asthma attacks. Long-term exposure increases the risk of chronic respiratory disease, heart disease, stroke, and reduced lung development in children.</p>
          <ul class="health-list">
            <li><strong>Respiratory effects:</strong> worsened asthma, bronchitis, and reduced lung function.</li>
            <li><strong>Cardiovascular effects:</strong> increased risk of heart attacks and strokes.</li>
            <li><strong>Vulnerable groups:</strong> children, older adults, and people with preexisting conditions are at higher risk.</li>
            <li><strong>Long-term impacts:</strong> chronic illness, increased mortality, and developmental harms.</li>
          </ul>
        </div>
      </div>
    </section>

    <section id="solutions" class="section container">
      <div class="panel">
        <h3>Simple Solutions — Home &amp; City</h3>

        <div class="cards">
          <div class="card">
            <h4>At Home</h4>
            <ul>
              <li>Use an air purifier with a HEPA filter in frequently used rooms.</li>
              <li>Ventilate when outdoor air is cleaner; avoid opening windows during heavy traffic or high pollution.</li>
              <li>Keep indoor pollution low: avoid smoking indoors, minimize incense, and use vented range hoods when cooking.</li>
              <li>Maintain HVAC filters and consider upgrading to higher-MERV filters where compatible.</li>
            </ul>
          </div>

          <div class="card">
            <h4>In Your City</h4>
            <ul>
              <li>Support active transport and public transit to reduce vehicle emissions.</li>
              <li>Advocate for more green spaces and tree planting to improve local air quality.</li>
              <li>Encourage low-emission zones, anti-idling policies, and cleaner fuels for buses and trucks.</li>
              <li>Educate neighbors about simple behavior changes that reduce pollution and exposure.</li>
            </ul>
          </div>
        </div>
      </div>
    </section>

    <section id="how" class="section container">
      <div class="panel">
        <h3>How LUFT Works</h3>
        <p>Our sensors continuously monitor your environment in three simple steps.</p>

        <div class="info-grid">
          <div class="info-card">
            <h4>Collect</h4>
            <p>Our sensors measure temperature, humidity, pressure, and particle matter floating in the air around you.</p>
          </div>
          <div class="info-card">
            <h4>Process</h4>
            <p>Data is analyzed instantly to calculate real air quality metrics you can understand and act on.</p>
          </div>
          <div class="info-card">
            <h4>Share</h4>
            <p>Results are displayed clearly so you know what you're breathing and make informed decisions.</p>
          </div>
        </div>
      </div>
    </section>

    <section id="pollutants" class="section container">
      <div class="panel">
        <h3>Understanding Key Pollutants</h3>
        <p>Different pollutants affect our health in different ways. LUFT monitors the most harmful ones affecting communities worldwide.</p>

        <div class="info-grid">
          <div class="info-card">
            <h4>PM2.5</h4>
            <p>Fine particles that penetrate deep into the lungs and bloodstream, causing long-term respiratory and cardiovascular harm.</p>
          </div>
          <div class="info-card">
            <h4>NO₂</h4>
            <p>Nitrogen dioxide from vehicle exhaust and industrial sources worsens asthma and reduces lung function over time.</p>
          </div>
          <div class="info-card">
            <h4>O₃</h4>
            <p>Ground-level ozone forms on hot days, causing coughing, chest pain, and reduced athletic performance.</p>
          </div>
        </div>
      </div>
    </section>

    <section id="impact" class="section alt">
      <div class="container">
        <div class="panel">
          <h3>Community Impact &amp; Change</h3>
          <p>Real data empowers real change. Here's what communities have achieved through air quality monitoring and collective action.</p>

          <div class="cards">
            <div class="card">
              <h4>🌍 Awareness</h4>
              <p>Communities using local data reduce personal exposure by making informed daily choices about outdoor activity and ventilation.</p>
            </div>
            <div class="card">
              <h4>📢 Policy Advocacy</h4>
              <p>Clear local data supports petitions for cleaner transit, emissions reduction zones, and green space expansion.</p>
            </div>
            <div class="card">
              <h4>💡 Innovation</h4>
              <p>Crowdsourced air quality data helps city planners identify pollution hotspots and design better infrastructure.</p>
            </div>
          </div>
        </div>
      </div>
    </section>

    <section id="getting-started" class="section container">
      <div class="panel">
        <h3>Getting Started with LUFT</h3>
        <p>Whether you're a resident, educator, or activist, there's a role for you in improving your community's air quality.</p>

        <div class="info-grid">
          <div class="info-card">
            <h4>Step 1: Learn</h4>
            <p>Explore our guides on air quality health impacts and practical protection strategies for your home and family.</p>
          </div>
          <div class="info-card">
            <h4>Step 2: Connect</h4>
            <p>Find local measurement stations and join a community. Follow live data and participate in discussions about local air quality.</p>
          </div>
          <div class="info-card">
            <h4>Step 3: Act</h4>
            <p>Share findings with neighbors, support local campaigns, or host a sensor to contribute to the growing community network.</p>
          </div>
        </div>
      </div>
    </section>
  </main>

  <footer class="site-footer">
    <div class="container">
      <p>LUFT — helping communities breathe easier.</p>
    </div>
  </footer>
</body>
</html>
)rawliteral";

const char SOPHIA_STYLES_CSS[] PROGMEM = R"rawliteral(
:root {
  --light-blue: #e6f8ff;
  --lighter-blue: #f0fcff;
  --sky-blue: #d4f1ff;
  --accent-blue: #1689bd;
  --dark-blue: #233445;
  --muted: #5c6d79;
  --max-width: 1000px;
}

* { box-sizing: border-box; }

html { scroll-behavior: smooth; }

body {
  margin: 0;
  color: var(--dark-blue);
  background: linear-gradient(180deg, #fbfdff 0%, var(--light-blue) 100%);
  font-family: Inter, system-ui, -apple-system, "Segoe UI", Roboto, Arial, sans-serif;
  font-size: 18px;
  line-height: 1.65;
}

h1, h2, h3, h4 { color: var(--dark-blue); font-weight: 700; }

.container { max-width: var(--max-width); margin: 0 auto; padding: 2rem; }

.site-header {
  border-bottom: 1px solid rgba(94, 200, 255, 0.2);
  background: rgba(255, 255, 255, 0.88);
  position: sticky;
  top: 0;
  z-index: 1;
}

.header-content {
  display: flex;
  align-items: center;
  gap: 1.5rem;
  padding-top: 1rem;
  padding-bottom: 1rem;
}

.brand { margin: 0; font-size: 1.5rem; letter-spacing: 2px; }

.nav { display: flex; gap: 1rem; margin-left: auto; }
.nav a { color: var(--dark-blue); font-weight: 600; text-decoration: none; }
.nav a:hover, .nav a:focus-visible { color: var(--accent-blue); }

.page-switch {
  display: inline-block;
  padding: 0.7rem 1rem;
  border-radius: 999px;
  background: var(--dark-blue);
  color: #fff;
  font-weight: 700;
  text-decoration: none;
  white-space: nowrap;
}

.page-switch:hover, .page-switch:focus-visible { background: var(--accent-blue); }

.hero {
  padding: 4rem 0;
  border-bottom: 2px solid rgba(94, 200, 255, 0.2);
  background: linear-gradient(135deg, rgba(94, 200, 255, 0.18), rgba(212, 241, 255, 0.1));
}

.hero h2 { margin: 0 0 0.5rem; font-size: clamp(2.1rem, 7vw, 4.5rem); line-height: 1.1; }
.subtitle { margin: 0; color: var(--muted); }

.section { padding-top: 3rem; padding-bottom: 3rem; }
.section.alt { background: linear-gradient(180deg, var(--light-blue), var(--lighter-blue)); }

.panel {
  margin-bottom: 1.25rem;
  padding: clamp(1.5rem, 5vw, 3.5rem);
  border: 1.5px solid rgba(94, 200, 255, 0.18);
  border-radius: 20px;
  background: linear-gradient(135deg, #fff, var(--lighter-blue));
  box-shadow: 0 12px 30px rgba(94, 200, 255, 0.12);
}

.panel h3 { margin: 0 0 1rem; font-size: clamp(1.7rem, 5vw, 2.7rem); line-height: 1.2; }
.health-list { margin: 1rem 0 0; padding-left: 1.1rem; color: var(--muted); }
.health-list li strong { color: var(--accent-blue); }

.cards, .info-grid { display: grid; gap: 1rem; margin-top: 1rem; }
.cards { grid-template-columns: repeat(2, minmax(0, 1fr)); }
.info-grid { grid-template-columns: repeat(3, minmax(0, 1fr)); }

.card, .info-card {
  padding: 1.25rem;
  border: 1.5px solid rgba(94, 200, 255, 0.22);
  border-radius: 14px;
  background: linear-gradient(135deg, #fff, var(--sky-blue));
  box-shadow: 0 10px 30px rgba(94, 200, 255, 0.14);
}

.card h4, .info-card h4 { margin: 0 0 0.5rem; font-size: 1.35rem; }
.card ul { margin: 0; padding-left: 1.1rem; color: var(--muted); }
.info-card { text-align: center; }
.info-card h4 { color: var(--accent-blue); }
.info-card p { margin: 0; color: var(--muted); font-size: 0.95rem; }

.site-footer {
  padding: 2rem 0;
  border-top: 2px solid rgba(94, 200, 255, 0.3);
  background: linear-gradient(180deg, var(--light-blue), #5ec8ff);
  color: var(--muted);
  text-align: center;
}

@media (max-width: 800px) {
  .header-content { flex-wrap: wrap; gap: 0.75rem; }
  .nav { display: none; }
  .page-switch { margin-left: auto; }
  .cards, .info-grid { grid-template-columns: 1fr; }
  .container { padding-left: 1rem; padding-right: 1rem; }
}
)rawliteral";
