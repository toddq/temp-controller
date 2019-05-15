import Vue from 'vue'
import App from './App.vue'
import Particle from 'particle-api-js'

require('./assets/main.scss')

Vue.config.productionTip = false

// NOTE: anti-best-practice
// var particleClientId = "temp-controller-6390";
// var particleClientSecret = "504c67db7cbca9659f7c05b38b66d1d5cfdd59de";
var username = 'todd@quessenberry.com'
var password = ''
var deviceId = '240021000447343337373738'

function login () {
  if (!localStorage.getItem('auth-token')) {
    console.debug('logging in')
    let particle = new Particle()
    particle.login({username: username, password: password})
      .then((data) => {
        let authToken = data.body.access_token
        localStorage.setItem('auth-token', authToken)
        console.debug('got auth token:', authToken)
      }).catch((err) => {
        console.error('error logging in:', err)
      })
  } else {
    console.debug('already have auth token')
  }
  if (!localStorage.getItem('deviceId')) {
    localStorage.setItem('deviceId', deviceId)
  }
}
login()

Vue.filter('round', function(value, decimals) {
  if(!value) {
    value = 0
  }

  if(!decimals) {
    decimals = 0
  }

  value = value.toFixed(decimals)
  return value
})

new Vue({
  render: h => h(App),
}).$mount('#app')
