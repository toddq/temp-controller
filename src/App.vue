<template>
  <div id="app">
    <Devices></Devices>
    <Controller></Controller>
  </div>
</template>

<script>
import Devices from './components/Devices.vue'
import Controller from './components/Controller'

export default {
  name: 'app',
  components: {
    Devices,
    Controller
  }
}
</script>

<style>
#app {
  font-family: 'Avenir', Helvetica, Arial, sans-serif;
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;
  /* text-align: center; */
  color: #2c3e50;
  margin-top: 20px;
}
</style>
