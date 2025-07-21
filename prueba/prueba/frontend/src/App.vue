<template>
  <div id="app">
    <h1>Control LED LoRaWAN</h1>
    <button @click="encenderLed">Encender LED</button>
    <button @click="apagarLed">Apagar LED</button>
    <p v-if="mensaje">{{ mensaje }}</p>
  </div>
</template>

<script>
import axios from 'axios';

export default {
  name: 'App',
  data() {
    return {
      mensaje: ''
    };
  },
  methods: {
    async encenderLed() {
      try {
        const res = await axios.post('http://localhost:3000/led/on');
        this.mensaje = res.data.message;
      } catch (e) {
        this.mensaje = 'Error al enviar comando ON';
      }
    },
    async apagarLed() {
      try {
        const res = await axios.post('http://localhost:3000/led/off');
        this.mensaje = res.data.message;
      } catch (e) {
        this.mensaje = 'Error al enviar comando OFF';
      }
    }
  }
};
</script>

<style>
#app {
  font-family: Arial, sans-serif;
  text-align: center;
  margin-top: 40px;
}
button {
  margin: 10px;
  padding: 10px 20px;
  font-size: 18px;
}
</style> 