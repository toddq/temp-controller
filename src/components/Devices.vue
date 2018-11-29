<template>
<div>
  <ul>
    <li v-for="device in devices" :key="device.id">
      {{ device.name }}
    </li>
  </ul>
</div>
</template>

<script>
import Particle from 'particle-api-js'

export default {
  name: 'devices',
  data () {
    return {
      particle: new Particle(),
      authToken: localStorage.getItem('auth-token'),
      devices: []
    }
  },
  mounted () {
    console.debug('devices mounted')
    // this.login()
    //   .then(this.getDeviceList)
    this.getDeviceList()
      // .then(this.getDeviceAttributes)
  },
  methods: {
    getDeviceList () {
      return this.particle.listDevices({ auth: this.authToken })
        .then((data) => {
          console.log('Devices: ', data.body)
          this.devices = data.body
        }).catch((err) => {
          console.error('List devices call failed: ', err)
        })
    },
    getDeviceAttributes () {
      this.devices.forEach((device) => {
        console.debug('get attributes for', device.name)
        this.particle.getDevice({deviceId: device.id, auth: this.authToken})
          .then((data) => {
            console.log(data)
          }).catch((err) => {
            console.error('error getting attributes', err)
          })
      })

    }
  }
}
</script>

<style scoped>

</style>
