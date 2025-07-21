require('dotenv').config();
const express = require('express');
const cors = require('cors');
const axios = require('axios');
const app = express();
const port = 3000;

app.use(cors());
app.use(express.json());

const {
  APP_EUI,
  DEV_EUI,
  APP_KEY,
  TTN_REGION,
  TTN_APP_ID,
  TTN_ACCESS_KEY
} = process.env;

const TTN_API_URL = `https://${TTN_REGION}.cloud.thethings.network/api/v3/as/applications/${TTN_APP_ID}/devices/${DEV_EUI}/down/push`;


async function sendCommandToTTN(command) {
  const byte = command === 'ON' ? 1 : 0;
  const payload = {
    downlinks: [
      {
        f_port: 1,
        frm_payload: Buffer.from([byte]).toString('base64'),
        priority: "NORMAL",
        confirmed: true
      }
    ]
  };
  try {
    const res = await axios.post(TTN_API_URL, payload, {
      headers: {
        Authorization: `Bearer ${TTN_ACCESS_KEY}`,
        'Content-Type': 'application/json'
      }
    });
    return res.data;
  } catch (error) {
    console.log(error)
    throw error.response ? error.response.data : error;
  }
}

app.post('/led/on', async (req, res) => {
  try {
    await sendCommandToTTN('ON');
    res.json({ success: true, message: 'Comando ON enviado' });
  } catch (err) {
    res.status(500).json({ success: false, error: err });
  }
});

app.post('/led/off', async (req, res) => {
  try {
    await sendCommandToTTN('OFF');
    res.json({ success: true, message: 'Comando OFF enviado' });
  } catch (err) {
    res.status(500).json({ success: false, error: err });
  }
});

app.listen(port, () => {
  console.log(`Backend escuchando en http://localhost:${port}`);
}); 