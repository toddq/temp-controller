<template>
<div class="controller container">
  <h2 class="title">
    {{ device.name }} 
    <div :class="['statusIndicator', {online: isConnected, offline: !isConnected}]"></div>
  </h2>
  <div class="box">

    <div class="columns outputContainer">
      <div @click="toggleHeating" :class="['column', 'is-narrow', 'field', 'is-horizontal', 'heatingContainer', {disabled: !heatEnabled}]">
        <div class="field-label">
          <label class="label">Heating</label>
        </div>
        <div class="field-body">
          <div :class="['statusIndicator', {isHeating: isHeating}]"></div>
        </div>
      </div>
      <div @click="toggleCooling" :class="['column', 'is-narrow', 'field', 'is-horizontal', 'coolingContainer', {disabled: !coolEnabled}]" >
        <div class="field-label">
          <label class="label">Cooling</label>
        </div>
        <div class="field-body">
          <div :class="['statusIndicator', {isCooling: isCooling}]"></div>
        </div>
      </div>
    </div>

    <div class="columns">
      <div class="column is-narrow field is-horizontal">
        <!-- TODO: pull this out into a component -->
        <div class="field-label is-normal">
          <label class="label">Control mode:</label>
        </div>
        <div class="field-body">
          <div class="select">
            <select>
              <option value="0">On / Off</option>
              <option value="1">PID</option>
              <option value="2">PWM</option>
            </select>
          </div>
        </div>
      </div>
    </div>

    <div class="columns targetContainer">
      <div class="column is-narrow field is-horizontal">
        <div class="field-label is-normal">
          <label class="label">Target</label>
        </div>
        <div class="field-body">
          <div class="control has-icons-right">
            <input class="input has-text-right" v-model.lazy="setpoint" @focus="$event.target.select()" @keyup.enter="setSetpoint"> 
            <span class="icon is-right has-text-grey-dark">&deg;F</span>
          </div>
        </div>
      </div>
    </div>

    <div class="columns tempContainer">
      <div class="column is-narrow field is-horizontal level">
        <div class="field-label is-normal level-item">
          <label class="label">Actual</label>
        </div>
        <div class="field-body level-item">
          <span class="is-size-2 has-text-weight-bold">{{ this.temperature | round(1) }}&deg;</span>
          <span class="is-size-4 f">F</span>
        </div>
      </div>
    </div>

    
    <button class="button" @click="shoutRainbows">Rainbows</button>
  </div>
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
      isCooling: false,
      heatEnabled: false,
      coolEnabled: false
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
      this.getVariable('heatEnabled')
      this.getVariable('coolEnabled')
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
    setSetpoint (event) {
      // ensure the value is a number, but the api requires it to be wrapped in a string
      if (isNaN(this.setpoint)) {
        console.warn(this.setpoint, 'is not a number')
        return
      }
      console.debug('updating setpoint with ', this.setpoint)
      this.cloudFunction('setpoint', this.setpoint)
        .then(() => {
          this.getVariable('setpoint')
        })
      event.target.blur()
    },
    toggleHeating () {
      console.debug('toggle heating')
      // this.heatEnabled = !this.heatEnabled
      this.cloudFunction('heatEnabled', `${!this.heatEnabled}`)
        .then(() => {
          this.getVariable('heatEnabled')
        })
    },
    toggleCooling () {
      console.debug('toggle cooling')
      // this.coolEnabled = !this.coolEnabled
      this.cloudFunction('coolEnabled', `${!this.coolEnabled}`)
        .then(() => {
          this.getVariable('coolEnabled')
        })
    },
    cloudFunction (name, argument) {
      return this.particle.callFunction({deviceId: this.deviceId, name: name, argument: argument, auth: this.authToken})
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

<style scoped lang="scss">

$offlineColor: #ff0000;
$onlineColor: #08b508;
$heatingColor: #ff0000;
$coolingColor: #0000ff;
$disabledColor: #aaaaaa;

.controller {
  margin-top: 20px;
}

.statusIndicator {
  display: inline-block;
  width: 20px;
  height: 20px;
  border: 1px solid gray;
  border-radius: 50%;

  &.online {
    background-color: $onlineColor;
    border-color: $onlineColor;
  }

  &.offline {
    background-color: $offlineColor;
    border-color: $offlineColor;
  }
}

.heatingContainer, .heatingContainer label,
.coolingContainer, .coolingContainer label {
  cursor: pointer; 
}

.outputContainer {
  .field-label {
    margin-right: 1em;
  }

  .disabled {
    text-decoration: line-through;

    .label {
      color: $disabledColor;
    }

    .statusIndicator {
      background-color: $disabledColor;
    }
  }
}


.statusIndicator.isHeating {
  background-color: red;
  border-color: red;
}

.statusIndicator.isCooling {
  background-color: blue;
  border-color: blue;
}

.targetContainer input {
  width: 100px;
}

.targetContainer input:not(:focus) {
  border: none;
  box-shadow: none;
  cursor: pointer;
}

.tempContainer .f {
  position: relative;
  top: 6px;
  padding-left: 5px;
}

.label {
  white-space: nowrap;
}

</style>
