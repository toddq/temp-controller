<template>
<div>
  <h2>{{ device.name }} <span :class="{networkStatus: true, online: isConnected}">&nbsp;&nbsp;&nbsp;</span></h2>
  <p>
    <span style="color: red" v-if="isHeating">Heating</span>
    <span style="color: blue" v-if="isCooling">Cooling</span>
  </p>
  <p>
    <span>Temperature:</span>
    <span>{{ this.temperature }}</span>
  </p>
  <p>
    <span>Setpoint:</span>
    <span>{{ this.setpoint }}</span>
  </p>
  <p>
    <span>Mode:</span>
    <span>{{ this.mode }}</span>
  </p>
  <button @click="editSetpoint">Change setpoint</button>
  <button @click="shoutRainbows">Rainbows</button>
</div>
</template>

<script>
import Particle from 'particle-api-js'

export default {
  name: 'controller',
  data () {
    return {
      deviceId: localStorage.getItem('deviceId'),
      authToken: localStorage.getItem('auth-token'),
      particle: new Particle(),
      device: {},
      temperature: null,
      setpoint: null,
      mode: null,
      isHeating: false,
        isCooling: false
    }
  },
  mounted () {
    this.getValues()
    this.particle.getEventStream({deviceId: this.deviceId, auth: this.authToken})
      .then((stream) => {
        stream.on('event', (event) => {
          console.log("Event ", event.name, event.data, event.published_at)
          if (event.name === 'spark/status') {
            this.device.connected = event.data === 'online' ? true : false
          }
          if (event.name === 'values') {
            let data = JSON.parse(event.data)
            for (var key in data) {
              this[key] = data[key]
            }
          }
          this.device.last_heard = new Date(event.published_at).getTime()
        })
      })
    setInterval(() => {
      this.getDeviceInfo()
    }, 60000)
  },
  computed: {
    isConnected () {
      console.log('connected: ', this.device.connected)
      console.log('last_heard:', this.device.last_heard)
      console.log(Date.now() - this.device.last_heard)

      return this.device.connected && ((Date.now() - this.device.last_heard) < 60000)
    },
  },
  methods: {
    getValues () {
      this.getDeviceInfo()
      this.getVariable('temperature')
      this.getVariable('setpoint')
      this.getVariable('mode')
    },
    getDeviceInfo () {
      this.particle.getDevice({deviceId: this.deviceId, auth: this.authToken})
        .then((data) => {
          this.device = data.body
          this.device.last_heard = new Date(this.device.last_heard).getTime()
        })
    },
    getVariable (name) {
      this.particle.getVariable({deviceId: this.deviceId, name: name, auth: this.authToken})
        .then((data) => {
          console.log(name, data.body.result)
          this[name] = data.body.result
        }).catch((err) => {
          console.error(`error getting variable '${name}': ${err}`)
        })
    },
    editSetpoint () {
      let newSetpoint = window.prompt('Enter new setpoint value:', this.setpoint)
      if (newSetpoint  != null) {
        this.setSetpoint(newSetpoint)
      }
    },
    setSetpoint (value) {
      this.particle.callFunction({deviceId: this.deviceId, name: 'setpoint', argument: value, auth: this.authToken})
        .then(() => {
          this.getVariable('setpoint')
        })
    },
    shoutRainbows () {
      this.particle.signalDevice({deviceId: this.deviceId, signal: true, auth: this.authToken})
      setTimeout(() => {
        this.particle.signalDevice({deviceId: this.deviceId, signal: false, auth: this.authToken})
      }, 3000)
    }
  }
}
</script>

<style scoped>
.networkStatus {
  font-size: .5em;
  vertical-align: middle;
  background-color: red;
}
.networkStatus.online {
  background-color: green;
}
.networkStatus.offline {
  background-color: red;
}
</style>
